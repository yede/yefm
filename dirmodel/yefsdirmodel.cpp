#include <stdio.h>

#include <QUrl>
#include <QDebug>

#include "yefsdirmodel.h"
#include "yefswatcher.h"
#include "yefsthread.h"
#include "yefsnode.h"
#include "yefsmisc.h"

#include "yefileutils.h"
//==============================================================================================================================

#define use_dbg 0

#if use_dbg
# define dbg(...) qDebug(__VA_ARGS__)
#else
# define dbg(...)
#endif //USE_DEBUG
//==============================================================================================================================

FsDirModel::FsDirModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	static int i = 0;
	snprintf(m_name, sizeof(m_name), "{m%d}", i++);

	m_thread  = new FsThread(this);
	m_watcher = new FsWatcher(this);
}

FsDirModel::~FsDirModel()
{
	m_thread->ensureStop();
	delete m_watcher;
	delete m_thread;
}
//==============================================================================================================================

QModelIndex FsDirModel::index(const QString &path) const
{
	FsNode *node = getNodeByPath(path);
	int row = (node == NULL) ? -1 : m_visibleChildren.indexOf(node);
	if (row < 0) {
		dbg("FsModelBase::index(path).1: path=%s", qPrintable(path));
		return QModelIndex();
	}

	dbg("FsModelBase::index(path).2: row=%d, col=%d, path=%s", row, 0, qPrintable(path));
	return createIndex(row, 0, node);
}

QModelIndex FsDirModel::index(FsNode *node) const
{
	Q_ASSERT(node != NULL);

	int row = m_visibleChildren.indexOf(node);
	if (row < 0) {
		dbg("FsModelBase::index(node).1: path=%s", cFilePath(node));
		return QModelIndex();
	}

	dbg("FsModelBase::index(node).2: row=%d, col=%d, path=%s", row, 0, cFilePath(node));
	return createIndex(row, 0, node);
}

QModelIndex FsDirModel::index(int row, int col, const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	if (row < 0 || row >= m_visibleChildren.size() || col < 0) {
		dbg("FsModelBase::index(): visibleChildren.size=%d, row=%d, col=%d", m_visibleChildren.size(), row, col);
		return QModelIndex();
	}

	FsNode *node = m_visibleChildren.at(row);
	dbg("FsModelBase::index(): row=%d, col=%d, path=%s", row, col, cFilePath(node));
	return createIndex(row, col, node);
}

QModelIndex FsDirModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);

	return QModelIndex();
}
//==============================================================================================================================

int FsDirModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

//	dbg("FsModelBase::rowCount(): m_baseItem=NULL");
	return count();
}

int FsDirModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return 5;//(parent.column() > 0) ? 0 : 5;
}

bool FsDirModel::canFetchMore(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return false;
}

void FsDirModel::fetchMore(const QModelIndex &parent)
{
	Q_UNUSED(parent);
	m_thread->fetchMore();
}
//==============================================================================================================================

Qt::DropActions FsDirModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

QStringList FsDirModel::mimeTypes() const
{
	return QStringList("text/uri-list");
}

QMimeData *FsDirModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *data = new QMimeData();
	QList<QUrl> files;

	foreach (QModelIndex index, indexes) {
		FsNode *item = static_cast<FsNode *>(index.internalPointer());
		Q_ASSERT(item != NULL);
		files.append(QUrl::fromLocalFile(item->filePath()));
	}
	data->setUrls(files);

	return data;
}
//==============================================================================================================================

bool FsDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	bool ok = false;

	if (role == Qt::EditRole) {
		FsNode *node = getNode(index);
		Q_ASSERT(node != NULL);

		QString fileName = value.toString();
		QString filePath = node->getFilePath(fileName);
		ok = QFile::rename(node->filePath(), filePath);

		if (ok) {
			node->clearMimeType();
			node->setFileName(fileName);
		//	emit dataChanged(index, index);
		}
	}

	return ok;
}
//==============================================================================================================================

bool FsDirModel::isDir(const QModelIndex &index) const
{
	FsNode *item = static_cast<FsNode *>(index.internalPointer());
	return item == NULL ? false : item->fileInfo().isDir();
}

qint64 FsDirModel::size(const QModelIndex &index) const
{
	FsNode *item = static_cast<FsNode *>(index.internalPointer());
	return item == NULL ? 0 : item->fileInfo().size();
}

QFileInfo FsDirModel::fileInfo(const QModelIndex &index) const
{
	FsNode *item = static_cast<FsNode *>(index.internalPointer());
	return item == NULL ? QFileInfo() : item->fileInfo();
}

QString FsDirModel::fileName(const QModelIndex &index) const
{
	FsNode *item = static_cast<FsNode *>(index.internalPointer());
	return item == NULL ? QString() : item->fileName();
}

QString FsDirModel::filePath(const QModelIndex &index) const
{
	FsNode *item = static_cast<FsNode *>(index.internalPointer());
	return item == NULL ? QString() : item->filePath();
}
//==============================================================================================================================

void FsDirModel::setRootPath(const QString &directoryPath)
{
	m_thread->fetchChildren(directoryPath);
}

void FsDirModel::populate(FsFetchResult *data, int mode)
{
	switch (mode) {
		case FsPopulateMode::FirstRet: populateFirstRet(data); break;
		case FsPopulateMode::Continue: populateContinue(data); break;
	}
	m_thread->setPopulated(data);
}

void FsDirModel::populateFirstRet(FsFetchResult *data)
{
	m_watcher->registerWatcher(data->directoryPath);

	FsMisc::clearNodes(m_children);
	m_visibleChildren.clear();

	m_rootPath = data->directoryPath;
	foreach (FsNode *node, data->nodes) m_children.insert(node->fileName(), node);

	m_visibleChildren.append(data->nodes);
	resetModel();

	emit rootPathChanged(QModelIndex());
}

void FsDirModel::populateContinue(FsFetchResult *data)
{
	foreach (FsNode *node, data->nodes) m_children.insert(node->fileName(), node);

	int pos = m_visibleChildren.size();
	int end = pos + data->nodes.size() - 1;

	beginInsertRows(QModelIndex(), pos, end);
	m_visibleChildren.append(data->nodes);
	endInsertRows();
}
//==============================================================================================================================

void FsDirModel::resetModel()
{
	beginResetModel();
	endResetModel();
}

void FsDirModel::invalidate()
{
	FsMisc::clearNodes(m_children);
	m_visibleChildren.clear();
	m_rootPath.clear();
	resetModel();
	emit invalidated();
}

void FsDirModel::tryGoUp()
{
	QString up_path = QFileInfo(m_rootPath).path();
	if (QDir(up_path).exists()) {
		dbg("FsModelBase::goUp(): %s up_path=%s", m_name, qPrintable(up_path));
		emit goParent();
		return;
	}

	invalidate();
}
//==============================================================================================================================

void FsDirModel::createFile()
{
	if (!isWritable()) return;

	QString name = tr("new_file");
	m_editFileName = FileUtils::getUniqueFileName(m_rootPath, name);

	QFile newFile(m_rootPath + "/" + m_editFileName);
	if (newFile.open(QIODevice::WriteOnly)) {
		newFile.close();
	} else {
		m_editFileName.clear();
	}
}

void FsDirModel::createFolder()
{
	if (!isWritable()) return;

	QString name = tr("new_folder");
	m_editFileName = FileUtils::getUniqueFileName(m_rootPath, name);

	if (!QDir(m_rootPath).mkdir(m_editFileName)) {
		m_editFileName.clear();
	}
}
//==============================================================================================================================

void FsDirModel::insertNode(const QString &fileName)
{
	dbg("FsModelBase::insertNode(): %s fileName=\"%s\"", m_name, qPrintable(fileName));
	FsNode *node = getNode(fileName);
	if (node != NULL) {
		updateNode(fileName);
		return;
	}

	QString filePath = getNodePath(fileName);
	node = new FsNode(QFileInfo(filePath));

	m_children.insert(fileName, node);
	int row = m_visibleChildren.size();

	beginInsertRows(QModelIndex(), row, row);
	m_visibleChildren.append(node);
	endInsertRows();

	QModelIndex index = this->index(node);
	if (fileName == m_editFileName) {
		m_editFileName.clear();
		emit edit(index);
	} else {
		emit select(index);
	}
}

void FsDirModel::deleteNode(const QString &fileName)
{
	dbg("FsModelBase::deleteNode(): %s fileName=\"%s\"", m_name, qPrintable(fileName));
	FsNode *node = getNode(fileName);
	if (node == NULL) return;

	int row = m_visibleChildren.indexOf(node);
	if (row >= 0) {
		beginRemoveRows(QModelIndex(), row, row);
		m_visibleChildren.removeOne(node);
		endRemoveRows();
	}

	m_children.remove(fileName);
}

void FsDirModel::updateNode(const QString &fileName)
{
	dbg("FsModelBase::updateNode(): %s fileName=\"%s\"", m_name, qPrintable(fileName));
	FsNode *node = getNode(fileName);
	if (node == NULL) return;

	node->refresh();

	int row = m_visibleChildren.indexOf(node);
	if (row >= 0) {
		QModelIndex index = this->index(node);
		emit dataChanged(index, index);
	}
}

void FsDirModel::renameNode(const QString &fromName, const QString &toName)
{
	dbg("FsModelBase::renameNode(): %s fromName=\"%s\", toName=\"%s\"", m_name, qPrintable(fromName), qPrintable(toName));
	FsNode *node = getNode(fromName);
	if (node == NULL) return;

	FsNode *nod2 = getNode(toName);
	if (nod2 != NULL) {
		updateNode(toName);
		deleteNode(fromName);
		return;
	}

	m_children.remove(fromName);
	m_children.insert(toName, node);
	node->setFileName(toName);

	int row = m_visibleChildren.indexOf(node);
	if (row >= 0) {
		QModelIndex index = this->index(node);
		emit dataChanged(index, index);
	}
}
//==============================================================================================================================

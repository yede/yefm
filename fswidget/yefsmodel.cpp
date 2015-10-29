#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QDebug>

#include "yefsmodel.h"
#include "yetreemodel.h"
#include "yemime.h"

#include "yefswidget.h"
#include "yefslistview.h"
#include "yefstreeview.h"

#include "yefileutils.h"
#include "yepastemgr.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconcache.h"
#include "yestyle.h"
#include "yesessiondata.h"

#define DUPLICAT_SIZE_LIMIT 5000000
//==============================================================================================================================
#if USE_qsys

FsModel::FsModel(FsWidget *widget, QObject *parent)
	: QFileSystemModel(parent)
	, m_widget(widget)
	, m_cache(IconCache::instance())
	, m_showThumbs(false)
{
}
#endif
//==============================================================================================================================
#if USE_node
#include "yefsnode.h"

FsModel::FsModel(FsWidget *widget, QObject *parent)
	: FsDirModel(parent)
	, m_widget(widget)
	, m_style(Style::instance())
	, m_cache(IconCache::instance())
	, m_showThumbs(false)
{
}
#endif
//==============================================================================================================================

FsModel::~FsModel()
{
}
//==============================================================================================================================

void FsModel::setMode(bool icons)
{
	m_showThumbs = icons;
}

void FsModel::setHiddenVisible(bool visible)
{
#if USE_node
	Q_UNUSED(visible);
#else
	QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::System;
	if (visible) filters |= QDir::Hidden;
	setFilter(filters);
#endif
}
//==============================================================================================================================

namespace FsModelCol {
	enum { Name = 0, Size, Type, Date, User, Count };
}

void FsModel::initColWidths(QList<int> &cols)
{
	for (int i = FsModelCol::Name; i < FsModelCol::Count; i++) {
		cols.append(-1);
	}
}

void FsModel::updateColSizes(FsTreeView *p)
{
	SessionData *d = SessionData::instance();
	for (int i = FsModelCol::Size; i < FsModelCol::Count; i++) {
		int w = d->cols.at(i);
		if (w > 0) p->setColumnWidth(i, w);
	}
	p->resizeColumnToContents(FsModelCol::Name);
}

void FsModel::saveColSizes(FsTreeView *p)
{
	SessionData *d = SessionData::instance();
	for (int i = FsModelCol::Name; i < FsModelCol::Count; i++) {
		d->cols[i] = p->columnWidth(i);
	}
}
//==============================================================================================================================
/*
QStringList	FsModel::mimeTypes() const
{
	QStringList types;
	types << TreeModel::mimeType() << "text/uri-list";
	return types;
} */

Qt::ItemFlags FsModel::flags(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return Qt::NoItemFlags | Qt::ItemIsDropEnabled;
	}

	Qt::ItemFlags ret = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
	QFileInfo info = fileInfo(index);
	if (info.isWritable()) {
		ret |= Qt::ItemIsEditable;
		if (info.isDir()) ret |= Qt::ItemIsDropEnabled;
	}

	return ret;
}

QVariant FsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
		switch (section) {
			case FsModelCol::Name: return tr("Name");
			case FsModelCol::Size: return tr("Size");
			case FsModelCol::Type: return tr("Type");
			case FsModelCol::Date: return tr("Date Modified");
			case FsModelCol::User: return tr("Owner");
		}
#if USE_qsys
	return QFileSystemModel::headerData(section, orientation, role);
#endif
#if USE_node
	Q_UNUSED(orientation);
	return QVariant();
#endif
}

int FsModel::columnCount(const QModelIndex &parent) const
{
	return (parent.column() > 0) ? 0 : FsModelCol::Count;
}
//==============================================================================================================================
#if USE_node
QVariant FsModel::data(const QModelIndex &index, int role) const
{
	FsNode *node = getNode(index);
	if (node == NULL) return QVariant();

	if (role == Qt::ForegroundRole) {
		const QFileInfo &info = node->fileInfo();
		Style::FileViewStyle &m = m_style->fvStyle;
		if (m_cutItems.contains(info.filePath())) return QBrush(m.cutsColor);
		if (info.isSymLink())    return QBrush(m.symbolColor);
		if (info.isDir())        return QBrush(m.folderColor);
		if (info.isExecutable()) return QBrush(m.execColor);
		if (info.isHidden())     return QBrush(m.hiddenColor);
		return QBrush(m.fileColor);
	}

	if (role == Qt::TextAlignmentRole) {
		return (index.column() == FsModelCol::Size) ? Qt::AlignRight + Qt::AlignVCenter : Qt::AlignLeft + Qt::AlignVCenter;
	}

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
			case FsModelCol::Name: return node->fileName();
			case FsModelCol::Size: return node->isDir() ? "" : FileUtils::formatSize(node->size());
			case FsModelCol::Type: return node->getMimeType();
			case FsModelCol::Date: return node->getDataTime();
			case FsModelCol::User: return node->getPermissions();
		}
		return QVariant();
	}

	if (role == Qt::DecorationRole) {
		return (index.column() != FsModelCol::Name) ? QVariant() : m_cache->findIcon(node->filePath());
	}

	if (role == Qt::FontRole) {
		QFont ft = App::app()->font();
		Style::FileViewStyle &m = m_style->fvStyle;
		if (m.strikeOutCuts) {
			if (m_cutItems.contains(node->filePath())) ft.setStrikeOut(true);
		}
		return ft;
	}

	if (role == Qt::EditRole) {
		return node->fileName();
	}

	if (role == Qt::StatusTipRole) {
		return node->fileName();
	}

	return QVariant();
}
#endif

#if USE_qsys
QVariant FsModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::ForegroundRole) {
		QFileInfo info(filePath(index));
		FmStyle::FileViewStyle &m = m_style->fvStyle;
		if (m_cutItems.contains(info.filePath())) return QBrush(m.cutsColor);
		if (info.isSymLink())    return QBrush(m.symbolColor);
		if (info.isDir())        return QBrush(m.folderColor);
		if (info.isExecutable()) return QBrush(m.execColor);
		if (info.isHidden())     return QBrush(m.hiddenColor);
		return QBrush(m.fileColor);
	}

	if (role == Qt::TextAlignmentRole) {
		return (index.column() == FsModelCol::Size) ? Qt::AlignRight + Qt::AlignVCenter : Qt::AlignLeft + Qt::AlignVCenter;
	}

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
			case FsModelCol::Name: break;
			case FsModelCol::Size: break;
			case FsModelCol::Type: break;
			case FsModelCol::Date: {
				QFileInfo info(filePath(index));
				QDateTime date = info.lastModified();
				return QString("%1 %2")
						.arg(date.toString(Config::instance()->fileDateFormat))
						.arg(date.toString(Config::instance()->fileTimeFormat));
			}
			case FsModelCol::User: {
					QString str;
#if QT_VERSION >= 0x050000
					QFile::Permissions perms = permissions(index);
					str.append(perms.testFlag(QFileDevice::ReadOwner ) ? "r" : "-");
					str.append(perms.testFlag(QFileDevice::WriteOwner) ? "w" : "-");
					str.append(perms.testFlag(QFileDevice::ExeOwner  ) ? "x" : "-");
					str.append(perms.testFlag(QFileDevice::ReadGroup ) ? "r" : "-");
					str.append(perms.testFlag(QFileDevice::WriteGroup) ? "w" : "-");
					str.append(perms.testFlag(QFileDevice::ExeGroup  ) ? "x" : "-");
					str.append(perms.testFlag(QFileDevice::ReadOther ) ? "r" : "-");
					str.append(perms.testFlag(QFileDevice::WriteOther) ? "w" : "-");
					str.append(perms.testFlag(QFileDevice::ExeOther  ) ? "x" : "-");
#else
					QFlags<QFile::Permissions> perms = permissions(index);
					str.append(perms.testFlag(QFile::ReadOwner ) ? "r" : "-");
					str.append(perms.testFlag(QFile::WriteOwner) ? "w" : "-");
					str.append(perms.testFlag(QFile::ExeOwner  ) ? "x" : "-");
					str.append(perms.testFlag(QFile::ReadGroup ) ? "r" : "-");
					str.append(perms.testFlag(QFile::WriteGroup) ? "w" : "-");
					str.append(perms.testFlag(QFile::ExeGroup  ) ? "x" : "-");
					str.append(perms.testFlag(QFile::ReadOther ) ? "r" : "-");
					str.append(perms.testFlag(QFile::WriteOther) ? "w" : "-");
					str.append(perms.testFlag(QFile::ExeOther  ) ? "x" : "-");
#endif
					str.append(" " + fileInfo(index).owner() + " " + fileInfo(index).group());
					return str;
			}
		}

		return QFileSystemModel::data(index, role);
	}

	if (role == Qt::DecorationRole) {
		if (index.column() != FsModelCol::Name) return QVariant();

		return m_cache->findIcon(filePath(index));
	}

	if (role == Qt::FontRole) {
		FmStyle::FileViewStyle &m = m_style->fvStyle;
		if (m.strikeOutCuts) {
			QFileInfo info(filePath(index));
			if (m_cutItems.contains(info.filePath())) {
				QFont ft = App::app()->font();
				ft.setStrikeOut(true);
				return ft;
			}
		}
	}

	return QFileSystemModel::data(index, role);
//	return QVariant();
}
#endif
//==============================================================================================================================

bool FsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(action);
	Q_UNUSED(row);
	Q_UNUSED(column);

	if (data->urls().isEmpty()) return true;

	QString destFolder = filePath(parent);
	QFileInfo dest(destFolder);

	if (dest.isDir()) {
		if (dest.isSymLink()) destFolder = dest.symLinkTarget();
	} else {
		destFolder = rootPath();
	}

	if (data->urls().size() == 1) {
		QFileInfo src(data->urls().at(0).toLocalFile());
		if (src.isFile() && src.size() < DUPLICAT_SIZE_LIMIT && src.path() == destFolder) {
			FileUtils::duplicate(src, destFolder);
			return true;
		}
	}

	startPaste(data, destFolder, PasteType::Move);

	return true;
}
//==============================================================================================================================

bool FsModel::remove(const QModelIndex &index) const
{
	QString path = filePath(index);
	QDirIterator it(path, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden, QDirIterator::Subdirectories);
	QStringList files;

	while (it.hasNext()) {
		files.prepend(it.next());
	}

	files.append(path);
	files.removeDuplicates();
	int cnt = files.count();
	bool ok = true;

	for (int i = 0; i < cnt; i++) {
		QFileInfo info(files.at(i));
		if (info.isDir()) {
			if (!QDir().rmdir(info.filePath())) {
				ok = false;
				qDebug() << "FsModel::remove(): FAILED on QDir().rmdir(), path=" << info.filePath();
			}
		} else {
			if (!QFile::remove(info.filePath())) {
				ok = false;
				qDebug() << "FsModel::remove(): FAILED on QFile::remove(), path=" << info.filePath();
			}
		}
	}

	return ok;
}
//==============================================================================================================================

QString FsModel::tempFile()
{
	return QDir::tempPath() + "/yefm.temp";
}

void FsModel::setCutItems(const QStringList &files)
{
	m_cutItems = files;
}

void FsModel::clearCutItems()
{
	m_cutItems.clear();

	QFile temp(tempFile());
	if (temp.exists()) temp.remove();
}

bool FsModel::isCutAction()
{
	return !m_cutItems.isEmpty();
}
//==============================================================================================================================

void FsModel::handleCut()
{
	if (m_widget == NULL) return;

	QModelIndexList sels = m_widget->getSourceSelection();
	QStringList files;
	foreach (QModelIndex item, sels) files.append(filePath(item));

	clearCutItems();
	setCutItems(files);
	QApplication::clipboard()->setMimeData(mimeData(sels));

	QFile temp(tempFile());
	temp.open(QIODevice::WriteOnly);
	QDataStream out(&temp);
	out << files;
	temp.close();
}

void FsModel::handleCopy()
{
	if (m_widget == NULL) return;

	clearCutItems();

	QModelIndexList sels = m_widget->getSourceSelection();
	QStringList files;
	foreach (QModelIndex item, sels) files.append(filePath(item));

	QApplication::clipboard()->setText(files.join("\n"), QClipboard::Selection);
	QApplication::clipboard()->setMimeData(mimeData(sels));
}

void FsModel::handlePaste()
{
	if (m_widget == NULL) return;

	QStringList cutList;		// check list of files that are to be cut

	QFile temp(tempFile());
	if (temp.exists()) {
		temp.open(QIODevice::ReadOnly);
		QDataStream out(&temp);
		out >> cutList;
		temp.close();
	}

	QString destFolder = m_widget->getPastePath();
	const QMimeData *data = QApplication::clipboard()->mimeData();

	int pasteType = PasteType::Copy;
	if (cutList.size() == data->urls().size()) pasteType = PasteType::Move;

	startPaste(data, destFolder, pasteType);
}

void FsModel::startPaste(const QMimeData *srcData, const QString &destFolder, int pasteType)
{
	if (m_widget == NULL) return;

	PasteMgr::launcher(srcData, destFolder, pasteType, m_widget);
}
//==============================================================================================================================

static void msgDeleteFailed()
{
	QString t = QObject::tr("Failed");
	QString s = QObject::tr("Could not delete some items...");
	s += QObject::tr("do you have the right permissions?");
	App::msg(t, s);
}

static Mbtn askDelete(const QFileInfo &file, bool &yesToAll)
{
	QString t = QObject::tr("Careful");
	QString s = QObject::tr("Be sure to delete:");
	s += "<p>\"<b>" + file.filePath() + "</b>\"?</p>";
	return App::ask(t, s, yesToAll);
}

/*static int askConfirm()
{
	QString t = QObject::tr("Delete confirmation");
	QString s = QObject::tr("Do you want to confirm all delete operations?");
	return App::ask(t, s);
}*/

void FsModel::handleDelete()
{
	if (m_widget == NULL) return;

	QModelIndexList sels = m_widget->getSourceSelection();
	bool yesToAll = false;
	bool ok = false;
	bool confirm = AppCfg::instance()->confirmDelete;

	for (int i = 0; i < sels.count(); ++i) {
		QFileInfo file(filePath(sels.at(i)));
		if (file.isWritable()) {
			if (file.isSymLink()) ok = QFile::remove(file.filePath());
			else {
				if (confirm && !yesToAll)
					if (askDelete(file, yesToAll) == QMessageBox::No) return;
				ok = remove(sels.at(i));
			}
		}
		else if (file.isSymLink()) ok = QFile::remove(file.filePath());
		else ok = false;

		if (!ok) break;
	}

	if (!ok) msgDeleteFailed();
}
//==============================================================================================================================

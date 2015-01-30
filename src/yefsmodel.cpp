#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <QApplication>
#include <QClipboard>
#include <QDebug>

#include "yefscache.h"
#include "yefsmodel.h"
#include "yefsmodelitem.h"
#include "yetreemodel.h"
#include "yemime.h"

#include "yefswidget.h"
#include "yefslistview.h"
#include "yefstreeview.h"

#include "yefileutils.h"
#include "yepastemgr.h"

#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
#include "yeapplication.h"
#include "yemainwindow.h"
#include "yestylesettings.h"
//==============================================================================================================================

FsModel::FsModel(YeApplication *app, QObject *parent)
	: QFileSystemModel(parent)
	, m_app(app)
	, m_cache(app->cache())
	, m_widget(NULL)
	, m_showThumbs(false)
{
}

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
	QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::System;
	if (visible) filters |= QDir::Hidden;
	setFilter(filters);
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

void FsModel::updateColSizes(YeApplication *app, FsTreeView *p)
{
	SessionData &d = app->win()->sessionData();
	for (int i = FsModelCol::Size; i < FsModelCol::Count; i++) {
		int w = d.cols.at(i);
		if (w > 0) p->setColumnWidth(i, w);
	}
	p->resizeColumnToContents(FsModelCol::Name);
}

void FsModel::saveColSizes(YeApplication *app, FsTreeView *p)
{
	SessionData &d = app->win()->sessionData();
	for (int i = FsModelCol::Name; i < FsModelCol::Count; i++) {
		d.cols[i] = p->columnWidth(i);
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
	Qt::ItemFlags ret =	Qt::NoItemFlags;	//Qt::NoItemFlags Qt::ItemIsUserCheckable

	if (!index.isValid()) {
		ret |= Qt::ItemIsDropEnabled;
	} else {
		ret |= Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
		QFileInfo info = fileInfo(index);
		if (info.isWritable()) {
			ret |= Qt::ItemIsEditable;
			if (info.isDir()) ret |= Qt::ItemIsDropEnabled;
		}
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
			default: return QVariant();
		}

	return QFileSystemModel::headerData(section, orientation, role);
}

int FsModel::columnCount(const QModelIndex &parent) const
{
	return (parent.column() > 0) ? 0 : FsModelCol::Count;
}

QVariant FsModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::ForegroundRole) {
		QFileInfo info(filePath(index));
		FmStyle::FileViewStyle &m = R::app()->fmStyle()->fvStyle;
		if (m_cutItems.contains(info.filePath())) return QBrush(m.cutsColor);
		if (info.isSymLink())    return QBrush(m.symbolColor);
		if (info.isDir())        return QBrush(m.folderColor);
		if (info.isExecutable()) return QBrush(m.execColor);
		if (info.isHidden())     return QBrush(m.hiddenColor);
		return QBrush(m.fileColor);
	}
	else if (role == Qt::TextAlignmentRole) {
		if (index.column() == 1) return Qt::AlignRight + Qt::AlignVCenter;
	}
	else if (role == Qt::DisplayRole) {
		switch (index.column()) {
			case FsModelCol::Name: break;
			case FsModelCol::Size: break;
			case FsModelCol::Type: break;
			case FsModelCol::Date: {
				QFileInfo info(filePath(index));
				QDateTime date = info.lastModified();
				return QString("%1 %2")
						.arg(date.toString(R::data().fileDateFormat))
						.arg(date.toString(R::data().fileTimeFormat));
			}
			case FsModelCol::User: {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
				QFile::Permissions perms = permissions(index);
#else
				QFlags<QFile::Permissions> perms = permissions(index);
#endif
				QString str;
				if (perms.testFlag(QFile::ReadOwner )) str.append("r"); else str.append(("-"));
				if (perms.testFlag(QFile::WriteOwner)) str.append("w"); else str.append(("-"));
				if (perms.testFlag(QFile::ExeOwner  )) str.append("x"); else str.append(("-"));
				if (perms.testFlag(QFile::ReadGroup )) str.append("r"); else str.append(("-"));
				if (perms.testFlag(QFile::WriteGroup)) str.append("w"); else str.append(("-"));
				if (perms.testFlag(QFile::ExeGroup  )) str.append("x"); else str.append(("-"));
				if (perms.testFlag(QFile::ReadOther )) str.append("r"); else str.append(("-"));
				if (perms.testFlag(QFile::WriteOther)) str.append("w"); else str.append(("-"));
				if (perms.testFlag(QFile::ExeOther  )) str.append("x"); else str.append(("-"));
				str.append(" " + fileInfo(index).owner() + " " + fileInfo(index).group());
				return str;
			}
		}
	}
	else if (role == Qt::DecorationRole) {
		if (index.column() != 0) return QVariant();

		return m_cache->findIcon(filePath(index));
	}
	else if (role == Qt::FontRole) {
		FmStyle::FileViewStyle &m = R::app()->fmStyle()->fvStyle;
		if (m.strikeOutCuts) {
			QFileInfo info(filePath(index));
			if (m_cutItems.contains(info.filePath())) {
				QFont ft = m_app->font();
				ft.setStrikeOut(true);
				return ft;
			}
		}
	}

	return QFileSystemModel::data(index, role);
}

bool FsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(action);
	Q_UNUSED(row);
	Q_UNUSED(column);

	QString destFolder = filePath(parent);
	QFileInfo destInfo(destFolder);

	if (destInfo.isDir()) {
		if (destInfo.isSymLink())
			destFolder = destInfo.symLinkTarget();
	} else {
		destFolder = rootPath();
	}
//	qDebug() << "destFolder" << destFolder << rootPath();

	QList<QUrl> files = data->urls();
	QStringList cutList;
	QString path;

	foreach(QUrl url, files) {
		path = url.path();
		cutList.append(path);
	}

	if (!files.isEmpty()) {
		execPaste(destFolder, data, cutList);
	}

	return true;
}
//==============================================================================================================================

bool FsModel::remove(const QModelIndex &theIndex) const
{
	QString path = filePath(theIndex);

	QDirIterator it(path,
					QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot |
					QDir::Hidden, QDirIterator::Subdirectories);
	QStringList children;
	while (it.hasNext())
		children.prepend(it.next());
	children.append(path);

	children.removeDuplicates();

	bool error = false;
	for (int i = 0; i < children.count(); i++) {
		QFileInfo info(children.at(i));
		if (info.isDir()) error |= QDir().rmdir(info.filePath());
		else error |= QFile::remove(info.filePath());
	}
	return error;
}
//==============================================================================================================================

QString FsModel::tempFile() const
{
	return QDir::tempPath() + "/yefm.temp";
}

void FsModel::addCutItems(QStringList files)
{
	m_cutItems = files;
}

void FsModel::clearCutItems()
{
	m_cutItems.clear();
	QFile(tempFile()).remove();
}
//==============================================================================================================================

void FsModel::handleCut()
{
	if (m_widget == NULL) return;

	QModelIndexList sels = m_widget->getSourceSelection();
	QStringList files;
	foreach (QModelIndex item, sels)
		files.append(filePath(item));

	clearCutItems();
	addCutItems(files);
	QApplication::clipboard()->setMimeData(mimeData(sels));

	QFile temp(tempFile());
	temp.open(QIODevice::WriteOnly);
	QDataStream out(&temp);
	out << files;
	temp.close();

//	m_sortModel->invalidate();
//	m_listSeleModel->clear();
}

void FsModel::handleCopy()
{
	if (m_widget == NULL) return;

	clearCutItems();

	QModelIndexList sels = m_widget->getSourceSelection();
	QStringList files;
	foreach (QModelIndex item, sels)
		files.append(filePath(item));

	QApplication::clipboard()->setText(files.join("\n"), QClipboard::Selection);
	QApplication::clipboard()->setMimeData(mimeData(sels));
}

void FsModel::handlePaste()
{
	if (m_widget == NULL) return;

	QStringList cutList;

	//check list of files that are to be cut
	QFile temp(tempFile());
	if (temp.exists()) {
	//	QModelIndexList list;
		temp.open(QIODevice::ReadOnly);
		QDataStream out(&temp);
		out >> cutList;
		temp.close();
	}

	QString destPath = m_widget->getPastePath();
	const QMimeData *data = QApplication::clipboard()->mimeData();

	execPaste(destPath, data, cutList);
}

void FsModel::execPaste(const QString &destPath, const QMimeData *srcData, const QStringList &cutList)
{
	PasteMgr::exec(destPath, srcData, cutList, this, m_widget);
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
	bool confirm = R::data().confirmDelete;

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

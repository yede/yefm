#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QSettings>
#include <QMimeData>
#include <QAction>
#include <QMenu>
#include <QDebug>

#include "yeuserbmkmodel.h"
#include "yeuserbmkdlg.h"
#include "yeuserview.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
#include "yestyle.h"
#include "yeshortcuts.h"

#include "yefileutils.h"
//==============================================================================================================================

UserBmkModel::UserBmkModel(QObject *parent)
	: TreeModel(parent)
	, m_style(Style::instance())
	, m_contextNode(NULL)
	, m_actAddSep(NULL)
	, m_dragKind(0)
	, m_dragView(NULL)
	, m_dropView(NULL)
	, m_allowOnItem(false)
{
	m_rootNode = new TreeNode(NodeType::Bookmark);
}

UserBmkModel::~UserBmkModel()
{
	delete m_rootNode;
}

void UserBmkModel::lateStart()
{
	updateIconTheme();
	connect(App::app(), SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
}

void UserBmkModel::updateIconTheme()
{
	if (m_actAddSep == NULL) return;

	m_actAddSep->setIcon(IconLoader::menuIcon("list-add"));
	m_actRename->setIcon(IconLoader::menuIcon("gtk-edit"));
	m_actRemove->setIcon(IconLoader::menuIcon("list-remove"));
}
//==============================================================================================================================

void UserBmkModel::saveBookmarkList()
{
	QStringList names;
	QStringList paths;
	QList<TreeNode *> &list = m_rootNode->children();

	foreach (TreeNode *node, list) {
		names.append(node->title());
		paths.append(node->path());
	}

	QSettings s(App::getBookmarkFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	s.setValue("names", names);
	s.setValue("paths", paths);
	s.endGroup();
}

void UserBmkModel::loadBookmarkList()
{
	QSettings s(App::getBookmarkFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	QStringList names = s.value("names").toStringList();
	QStringList paths = s.value("paths").toStringList();
	s.endGroup();

	int count = paths.size();
	if (names.size() != count) {
		qDebug() << "UserBmkModel::loadBookmarkList(): error bookmark-list size (names, paths):"
				 << names.size() << paths.size();
		return;
	}

	clear();

	beginInsertRows(QModelIndex(), 0, count - 1);
	for (int i = 0; i < count; i++) {
		QString name = names.at(i);
		QString path = paths.at(i);
		insertNode(name, path);
	}
	endInsertRows();
}
//==============================================================================================================================

TreeNode *UserBmkModel::insertNode(const QString &title, const QString &path, int pos)
{
	TreeNode *node = new TreeNode(NodeType::Bookmark, m_rootNode);
	node->setTitle(title);
	node->setPath(path);
	if (path == "::") node->setSeparator(true);
	m_rootNode->insertChild(node, pos);

	return node;
}

void UserBmkModel::onAddSep()
{
	if (m_contextNode == NULL) return;

	int pos = m_contextNode->row();
	beginInsertRows(QModelIndex(), pos, pos);
	insertNode(tr("Separator"), "::", pos);
	endInsertRows();

	saveBookmarkList();
}

void UserBmkModel::onEdit()
{
	if (m_contextNode == NULL) return;

	QString title = m_contextNode->title();
	QString path  = m_contextNode->path();

	UserBmkDlg dlg(App::win());
	if (dlg.showDialog(title, path)) {
		m_contextNode->setTitle(title);
		m_contextNode->setPath(path);
		saveBookmarkList();
	}
}

void UserBmkModel::removeNode(TreeNode *node)
{
	if (node == NULL) return;

	int pos = node->row();
	beginRemoveRows(QModelIndex(), pos, pos);
	m_rootNode->removeChild(node);
	endRemoveRows();

	saveBookmarkList();
	delete node;
}

void UserBmkModel::onRemove()
{
	removeNode(m_contextNode);
}

void UserBmkModel::removeSelection(UserView *view)
{
	TreeNode *node = view->getSelectedNode();
	removeNode(node);
}

void UserBmkModel::askRemove(UserView *view)
{
	if (App::confirm(tr("Be sure to remove bookmark?"))) removeSelection(view);
}
//==============================================================================================================================

QString UserBmkModel::getStatusMessage(const QModelIndex &index) const
{
	return index.isValid() ? getNode(index)->path() : QString();
}

void UserBmkModel::showContextMenu(UserView *view, QContextMenuEvent *event)
{
	if (m_actAddSep == NULL) {
		m_actAddSep = new QAction(tr("Insert Separator"), this);
		m_actRename = new QAction("", this);
		m_actRemove = new QAction("", this);

		connect(m_actAddSep, SIGNAL(triggered()), this, SLOT(onAddSep()));
		connect(m_actRename, SIGNAL(triggered()), this, SLOT(onEdit()));
		connect(m_actRemove, SIGNAL(triggered()), this, SLOT(onRemove()));

		updateIconTheme();
	}

	QModelIndex curr = view->currentIndex();
	m_contextNode = curr.isValid() ? getNode(curr) : NULL;
	bool flag = m_contextNode != NULL;

	QString txt1 = flag ? QString("%1 \"%2\" ...").arg(tr("Edit")).arg(m_contextNode->title())
						: tr("Edit item");
	QString txt2 = flag ? QString("%1 \"%2\"").arg(tr("Remove")).arg(m_contextNode->title())
						: tr("Remove item");

	m_actRename->setText(txt1);
	m_actRemove->setText(txt2);

	m_actAddSep->setEnabled(flag);
	m_actRename->setEnabled(flag);
	m_actRemove->setEnabled(flag);

	QMenu menu;
	menu.addAction(m_actAddSep);
	menu.addAction(m_actRename);
	menu.addSeparator();
	menu.addAction(m_actRemove);
	menu.exec(event->globalPos());

	m_contextNode = NULL;
}
//==============================================================================================================================

bool UserBmkModel::handleKeyPress(UserView *view, QKeyEvent *event)
{
	int actionId = ShortcutMgr::instance()->findActionId("UserBmkModel", event);
	if (actionId < 0) return false;

	switch (actionId) {
		case BmkModelAction::Activate: view->activate(); break;
		case BmkModelAction::AddBmk  : break;
		case BmkModelAction::AddSep  : onAddSep();  break;
		case BmkModelAction::Edit    : onEdit();    break;
		case BmkModelAction::Remove  : onRemove();  break;
		case BmkModelAction::RemoveA : askRemove(view); break;
	}

	return true;
}

void UserBmkModel::setupShortcuts()
{
	ShortcutMgr *m = ShortcutMgr::instance();
	ShortcutGroup *grp = m->addGroup("UserBmkModel");

	grp->add("Return",   "Activate", BmkModelAction::Activate);
	grp->add("Ctrl+Ins", "AddBmk",   BmkModelAction::AddBmk);
	grp->add("Ctrl+T",   "AddSep",   BmkModelAction::AddSep);
	grp->add("F2",       "Edit",     BmkModelAction::Edit);
	grp->add("Ctrl+Del", "Remove",   BmkModelAction::Remove);
	grp->add("Del",      "AskDel",   BmkModelAction::RemoveA);
}
//==============================================================================================================================

QVariant UserBmkModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeNode *node = getNode(index);
	bool sep = node->isSeparator();
	Style::BmkViewStyle &m = m_style->bmStyle;

	switch (role) {
		case Qt::DisplayRole   : return node->title();
		case Qt::DecorationRole: return sep ? QVariant() : IconLoader::icon("bookmark-item");
	//	case Qt::ToolTipRole:    return node->path();
		case Qt::ForegroundRole: return sep ? QBrush(m.sepFgColor) : QBrush(m.itemColor);
		case Qt::BackgroundRole: return sep ? QBrush(m.sepBgColor) : QVariant();
	//	case Qt::TextAlignmentRole: return sep ? QVariant(Qt::AlignRight | Qt::AlignVCenter)
	//										   : QVariant(Qt::AlignLeft | Qt::AlignVCenter);
	}

	return QVariant();
}

bool UserBmkModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(action);
	Q_UNUSED(column);
	Q_UNUSED(parent);

	Q_ASSERT(m_dropView != NULL);

	if (m_dropIndex.isValid()) {
		row = m_dropIndex.row();
		if (m_dropArea == DropArea::BelowItem) row++;
	} else {
		row = m_rootNode->ownCount();
	}

	if (m_dragKind == DragKind::Bookmark) {
		bool flag = false;
		QList<TreeNode *> bmks;
		decodeDragMimeData(bmks, data, TreeModel::mimeType());
		foreach (TreeNode *node, bmks) {
			int pos = node->row();
			if (pos == row) break;
			if (pos < row) row--;
			if (moveNodeTo(node, m_rootNode, row)) {
				m_dropView->setCurrentNode(node);
				flag = true;
				break;
			}
		}
		if (flag) saveBookmarkList();
		return true;
	}

	if (m_dragKind == DragKind::FileView) {
		if (data->hasUrls()) {
			TreeNode *node = NULL;
			foreach (QUrl url, data->urls()) {
				QString path = url.toLocalFile();
				QString name = FileUtils::getTitleFromPath(path);
				if (QFileInfo(path).isDir()) {
					beginInsertRows(parent, row, row);
					node = insertNode(name, path, row);
					endInsertRows();
				}
			}
			if (node != NULL) {
				m_dropView->setCurrentNode(node);
				saveBookmarkList();
			}
		}
		return true;
	}

	return false;
}
//==============================================================================================================================

void UserBmkModel::cleanupDrag()
{
	m_dragKind = DragKind::Unknown;
	m_dragView = NULL;
	m_dropView = NULL;
	m_dropArea = DropArea::None;
}

int UserBmkModel::indicatorArea(UserView *view, const QModelIndex &index)
{
	Q_UNUSED(view);

	if (index != m_dropIndex) return DropArea::None;

	int from = m_allowOnItem ? DropArea::OnItem : DropArea::AboveItem;

	return m_dropArea >= from ? m_dropArea : DropArea::None;
}

bool UserBmkModel::handleDragEnter(UserView *view, QDragEnterEvent *event)
{
	QObject *source = event->source();

	if (source == NULL) {
		return false;
	}

	if (source == view) {
		m_dragKind = DragKind::Bookmark;
		return true;
	}

	if (source->inherits("FsListView") || source->inherits("FsTreeView")) {
		m_dragKind = DragKind::FileView;
	}

	return m_dragKind != DragKind::Unknown;
}

bool UserBmkModel::handleDragMove(UserView *view, QDragMoveEvent *event)
{
	m_dropIndex = view->indexAt(event->pos());

	if (!m_dropIndex.isValid()) {
		m_dropArea = DropArea::None;
	//	return false;
	} else {
		QRect r = view->visualRect(m_dropIndex);
	//	m_allowOnItem = true;
		int a = view->calcDropArea(event->pos().y(), r, m_allowOnItem);
		if (m_dropArea != a) {
			m_dropArea = a;
			view->update(m_dropIndex);
		}
	}

	return true;
}

bool UserBmkModel::handleDrop(UserView *view, QDropEvent *event)
{
	Q_UNUSED(view);
	Q_UNUSED(event);
	if (m_dragKind == DragKind::Bookmark) {
		m_dropView = view;
		return true;
	//	qDebug() << "FsBmkView::dropEvent: self..";
	//	view->oldDropEvent(event);
	//	event->accept();	// do not accept()! eat bmk when on-item ?
	}
	else if (m_dragKind == DragKind::FileView) {
		m_dropView = view;
		return true;
	//	qDebug() << "FsBmkView::dropEvent" << m_dragView;
	//	view->oldDropEvent(event);
	//	event->accept();	// after: QListView::dropEvent(event)
	}
	return false;
}

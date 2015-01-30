#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QSettings>
#include <QMimeData>
#include <QDebug>

#include "yeusertabmodel.h"
#include "yeuserview.h"

#include "yefilepane.h"
#include "yefswidget.h"
#include "yefslistview.h"

#include "yeapp.h"
#include "yeappresources.h"
#include "yeapplication.h"
#include "yestylesettings.h"
//==============================================================================================================================

UserTabModel::UserTabModel(YeFilePane *pane, QObject *parent)
	: TreeModel(parent)
	, m_pane(pane)
	, m_view(NULL)
	, m_dragKind(0)
	, m_allowOnItem(false)
{
	m_rootNode = new TreeNode(NodeType::Tab);

	m_timer.setSingleShot(true);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

UserTabModel::~UserTabModel()
{
}
//==============================================================================================================================

void UserTabModel::saveTabs()
{
	QStringList names;
	QStringList paths;
	QList<TreeNode *> &list = m_rootNode->children();

	foreach (TreeNode *node, list) {
		names.append(node->title());
		paths.append(node->path());
	}

	QSettings s(App::getSessionTabsFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	s.setValue("names", names);
	s.setValue("paths", paths);
	s.endGroup();
}

void UserTabModel::loadTabs()
{
	QSettings s(App::getSessionTabsFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	QStringList names = s.value("names").toStringList();
	QStringList paths = s.value("paths").toStringList();
	s.endGroup();

	int count = paths.size();
	if (names.size() != count) {
		qDebug() << "UserTabModel::loadTabs(): error list size (names, paths):"
				 << names.size() << paths.size();
		return;
	}

	clear();

	beginInsertRows(QModelIndex(), 0, count - 1);
	for (int i = 0; i < count; i++) {
		QString name = names.at(i);
		QString path = paths.at(i);
		doAddTab(name, path, i);
	}
	endInsertRows();
}

TreeNode *UserTabModel::doAddTab(const QString &name, const QString &path, int pos)
{
	TreeNode *node = new TreeNode(NodeType::Tab, m_rootNode);
	node->setTitle(name);
	node->setPath(path);
	m_rootNode->insertChild(node, pos);

	return node;
}

TreeNode *UserTabModel::insertTab(int pos, const QString &path)
{
	int i = path.lastIndexOf(QChar('/')) + 1;
	QString name = i > 1 ? path.mid(i) : path;
	if (name.isEmpty()) name = path;

	int cnt = m_rootNode->ownCount();
	if (pos >= cnt || pos < 0) pos = cnt;

	beginInsertRows(QModelIndex(), pos, pos);
	TreeNode *node = doAddTab(name, path, pos);
	endInsertRows();

	return node;
}

void UserTabModel::deleteTab(TreeNode *node)
{
	if (node == NULL) return;

	int pos = node->row();
	beginRemoveRows(QModelIndex(), pos, pos);
	m_rootNode->removeChild(node);
	endRemoveRows();

	delete node;
}
//==============================================================================================================================

QString UserTabModel::getStatusMessage(const QModelIndex &index) const
{
	return index.isValid() ? getNode(index)->path() : QString();
}
/*
void UserTabModel::handleMousePress(UserView *view, QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		QModelIndex curr = view->currentIndex();
		if (curr.isValid()) {
			TreeNode *node = getNode(curr);
			int pos = event->x() >= view->iconAreaWidth() ? UserPressArea::Text : UserPressArea::Icon;
			emit view->itemClicked(pos, node);
		}
	}
}*/
//==============================================================================================================================

QVariant UserTabModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeNode *node = getNode(index);
//	bool sep = node->isSeparator();
	FmStyle::BmkViewStyle &m = m_pane->app()->fmStyle()->bmStyle;

	switch (role) {
		case Qt::DisplayRole   : return node->title();
		case Qt::DecorationRole: return R::icon("folder");
	//	case Qt::ToolTipRole:    return node->path();
		case Qt::ForegroundRole: return QBrush(m.itemColor);
	//	case Qt::BackgroundRole: return QVariant();
	//	case Qt::TextAlignmentRole: return sep ? QVariant(Qt::AlignRight | Qt::AlignVCenter)
	//										   : QVariant(Qt::AlignLeft | Qt::AlignVCenter);
	}

	return QVariant();
}

bool UserTabModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(action);
	Q_UNUSED(column);
	Q_UNUSED(parent);
//	qDebug() << "UserTabModel::dropMimeData" << row << column;

	if (m_dropIndex.isValid()) {
		row = m_dropIndex.row();
		if (m_dropArea == DropArea::BelowItem) row++;
	} else {
		row = m_rootNode->ownCount();
	}

	if (m_dragKind == DragKind::TabSelf) {
		QList<TreeNode *> items;
		decodeDragMimeData(items, data, TreeModel::mimeType());
		foreach (TreeNode *node, items) {
			int pos = node->row();
			if (pos == row) break;
			if (pos < row) row--;
			if (moveNodeTo(node, m_rootNode, row)) m_view->setCurrentIndex(getNodeIndex(node));
			return true;
		}
		return false;
	}

	if (m_dragKind == DragKind::TabItem) {
		QList<TreeNode *> items;
		decodeDragMimeData(items, data, TreeModel::mimeType());
		foreach (TreeNode *node, items) {
			FsWidget *view = static_cast<FsWidget *>(node->arg());
			if (view == NULL) break;
		//	qDebug() << view->pane() << m_pane;
			if (view->pane()->takeTab(view)) {
				if (m_pane->insertTab(view, row, true)) return true;
				qDebug() << "FAILED: move tab...";
			}
			break;
		}
		return false;
	}

	if (m_dragKind == DragKind::Bookmark) {
		QList<TreeNode *> items;
		decodeDragMimeData(items, data, TreeModel::mimeType());
		foreach (TreeNode *node, items) {
			if (m_pane->insertTab(node->path(), row, true)) return true;
			break;
		}
		return false;
	}

	if (m_dragKind == DragKind::FileView) {
		m_dragPaths.clear();
		if (data->hasUrls()) {
			foreach (QUrl url, data->urls()) {
				QString path = url.toLocalFile();
				if (QFileInfo(path).isDir()) m_dragPaths.append(path);
			}
		}
		if (m_dragPaths.size() > 0) {
			m_dropRow = row;
			m_timer.start(120);
		}
		return true;
	}

	return false;
}

void UserTabModel::onTimeout()
{
	FsWidget *view = NULL;
	foreach (QString path, m_dragPaths) {
		view = m_pane->insertTab(path, m_dropRow, false);
	}
	if (view != NULL) {
		m_pane->setWorkView(view);
	}
}
//==============================================================================================================================

void UserTabModel::cleanupDrag()
{
	m_dragKind = DragKind::Unknown;
	m_dropArea = DropArea::None;
}

int UserTabModel::indicatorArea(UserView *view, const QModelIndex &index)
{
	Q_UNUSED(view);

	if (index != m_dropIndex) return DropArea::None;

	int from = m_allowOnItem ? DropArea::OnItem : DropArea::AboveItem;

	return m_dropArea >= from ? m_dropArea : DropArea::None;
}

bool UserTabModel::handleDragEnter(UserView *view, QDragEnterEvent *event)
{
	QObject *source = event->source();

	if (source == NULL) {
		return false;
	}

	if (source->inherits("UserView")) {
		UserView *p = qobject_cast<UserView *>(source);
		if (p != NULL) {
			switch (p->viewType()) {
				case UserViewType::Bmks: m_dragKind = DragKind::Bookmark; break;
				case UserViewType::Tabs: m_dragKind = (p == view) ? DragKind::TabSelf : DragKind::TabItem; break;
			}
		}
		return m_dragKind != DragKind::Unknown;
	}

	if (source->inherits("FsListView") || source->inherits("FsTreeView")) {
		m_dragKind = DragKind::FileView;
	}

	return m_dragKind != DragKind::Unknown;
}

bool UserTabModel::handleDragMove(UserView *view, QDragMoveEvent *event)
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

bool UserTabModel::handleDrop(UserView *view, QDropEvent *event)
{
	Q_UNUSED(view);
	Q_UNUSED(event);
	return true;
}

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDrag>
#include <QDebug>

#include "yeuserview.h"
#include "yetreemodel.h"
#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
//==============================================================================================================================
#define SP_ROOT_EX 4

UserViewDelegate::UserViewDelegate(UserView *view)
	: QItemDelegate(view)
	, m_view(view)
{
}

void UserViewDelegate::updateRowHeight()
{
	m_rowHeight = AppCfg::instance()->iconSize + 2;
}

QSize UserViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QSize size = QItemDelegate::sizeHint(option, index);
	return QSize(size.width(), m_rowHeight);
}

void UserViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QItemDelegate::paint(painter, option, index);

	int a = m_view->indicatorArea(index);

	if (a != DropArea::None) {
		QRect r = option.rect;
		painter->setPen(QColor(255, 0, 0));
		if (a == DropArea::OnItem) {
			painter->drawRect(r);
		} else if (a == DropArea::AboveItem) {
			int x1 = r.x();
			int y1 = r.y();
			int x2 = x1 + r.width() - 1;
			painter->drawLine(x1, y1, x2, y1);
		} else if (a == DropArea::BelowItem) {
			int x1 = r.x();
			int y1 = r.y() + r.height() - 1;
			int x2 = x1 + r.width() - 1;
			painter->drawLine(x1, y1, x2, y1);
		}
		painter->setPen(Qt::NoPen);		// prevent other painting
	}
}

//==============================================================================================================================
// class UserView
//==============================================================================================================================

UserView::UserView(int paneIndex, int type, QWidget *parent)
	: QListView(parent)
	, m_paneIndex(paneIndex)
	, m_type(type)
	, m_model(NULL)
{
	setDefaultDropAction(Qt::MoveAction);
	setDragDropMode(QAbstractItemView::DragDrop);
	setDropIndicatorShown(true);
	setMouseTracking(true);

	m_delegate = new UserViewDelegate(this);
	setItemDelegate(m_delegate);

	updateIconTheme();	// after: setItemDelegate()

	connect(App::app(), SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));
	connect(this, SIGNAL(entered(QModelIndex)), this, SLOT(onHover(QModelIndex)));
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
}

UserView::~UserView()
{
}
//==============================================================================================================================

int UserView::currentRow() const
{
	TreeNode *node = m_model->getNode(currentIndex());
	return (node == NULL) ? -1 : node->row();
}

TreeNode *UserView::getSelectedNode() const
{
//	QModelIndex curr = currentIndex();
//	TreeNode *node = curr.isValid() ? m_model->getNode(curr) : NULL;

	QModelIndexList l = selectionModel()->selectedIndexes();
	if (l.size() != 1) return NULL;

	QModelIndex index = l.at(0);
	return index.isValid() ? static_cast<TreeNode *>(index.internalPointer()) : NULL;
}

void UserView::setCurrentNode(TreeNode *node)
{
	if (node == NULL) return;

	QModelIndex index = m_model->getNodeIndex(node);
	if (index.isValid()) setCurrentIndex(index);
}

void UserView::setViewModel(TreeModel *model)
{
	m_model = model;
	QListView::setModel(model);
}

void UserView::updateNode(TreeNode *node, const QString &name, const QString &path)
{
	if (node == NULL) return;

	node->setTitle(name);
	node->setPath(path);
	update(m_model->getNodeIndex(node));
}

void UserView::activate()
{
	TreeNode *node = getSelectedNode();
	if (node) {
		emit itemClicked(node);
	}
}
//==============================================================================================================================

void UserView::updateIconTheme()
{
	m_delegate->updateRowHeight();
	int sz = AppCfg::instance()->iconSize;
	setIconSize(QSize(sz, sz));
}

void UserView::onIconThemeChanged()
{
	updateIconTheme();
	if (m_model) m_model->refresh();
}

void UserView::onHover(const QModelIndex &index)
{
	QString msg = m_model->getStatusMessage(index);
	if (!msg.isEmpty()) {
		App::app()->showStatusMessage(msg, m_paneIndex, 12000);
	}
}

void UserView::onClicked(const QModelIndex &index)
{
	if (!index.isValid()) return;

	TreeNode *node = m_model->getNode(index);
	if (node != NULL) emit itemClicked(node);
}
//==============================================================================================================================

void UserView::keyPressEvent(QKeyEvent *event)
{
	bool ok = m_model->handleKeyPress(this, event);
	if (!ok) QListView::keyPressEvent(event);
}

void UserView::contextMenuEvent(QContextMenuEvent *event)
{
	m_model->showContextMenu(this, event);
}
//==============================================================================================================================

int UserView::calcDropArea(int y, const QRect &itemRect, bool allowOnItem)
{
	int y1 = itemRect.y();
	int y2 = y1 + 2;
	int y4 = itemRect.y() + itemRect.height();
	int y3 = y4 - 2;
	int ym = y2 + (y4 - y2) / 2;
	int a;

	if      (y < y1) a = DropArea::None;
	else if (y < y2) a = DropArea::AboveItem;
	else if (y < y3) a = allowOnItem ? DropArea::OnItem : y < ym ? DropArea::AboveItem : DropArea::BelowItem;
	else if (y < y4) a = DropArea::BelowItem;
	else             a = DropArea::None;

	return a;
}

int UserView::indicatorArea(const QModelIndex &index)
{
	return m_model->indicatorArea(this, index);
}
//==============================================================================================================================

void UserView::dragEnterEvent(QDragEnterEvent *event)
{
//	qDebug() << "UserView::dragEnterEvent";
	m_model->cleanupDrag();
	bool ok = m_model->handleDragEnter(this, event);
	if (ok) {
		event->accept();
		QListView::dragEnterEvent(event);
	}
}

void UserView::dragLeaveEvent(QDragLeaveEvent *event)
{
//	qDebug() << "UserView::dragLeaveEvent";
	QListView::dragLeaveEvent(event);
	event->accept();
	m_model->cleanupDrag();
}

void UserView::dragMoveEvent(QDragMoveEvent *event)
{
	QListView::dragMoveEvent(event);
	bool ok = m_model->handleDragMove(this, event);
	event->setAccepted(ok);
}

void UserView::dropEvent(QDropEvent *event)
{
//	qDebug() << "UserView::dropEvent";
	bool ok = m_model->handleDrop(this, event);
	if (ok) {
		QListView::dropEvent(event);
		event->accept();
	}
	m_model->cleanupDrag();
}
//==============================================================================================================================

void UserView::startDrag(Qt::DropActions supportedActions)
{
	QModelIndexList indexes = this->selectedIndexes();
	if (indexes.count() > 0) {
		QMimeData *data = m_model->mimeData(indexes);
		if (!data)
			return;

		QRect rect;
		rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);

		QDrag *drag = new QDrag(this);
		drag->setMimeData(data);
		drag->setHotSpot(viewport()->mapFromGlobal(QCursor::pos()) - rect.topLeft());
		//drag->setPixmap(pixmap);

		Qt::DropAction defaultDropAction = Qt::IgnoreAction;
		if (this->defaultDropAction() != Qt::IgnoreAction && (supportedActions & this->defaultDropAction()))
			defaultDropAction = this->defaultDropAction();
		else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
			defaultDropAction = Qt::CopyAction;

		drag->exec(supportedActions, defaultDropAction);
	}
}
//==============================================================================================================================

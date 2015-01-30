#include <QMouseEvent>
#include <QDebug>

#include "yefstreeview.h"
#include "yefswidget.h"
#include "yefshandler.h"
#include "yefsmodel.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
//==============================================================================================================================

FsTreeDelegate::FsTreeDelegate(FsTreeView *view)
	: QItemDelegate(view)
	, m_view(view)
{
}

void FsTreeDelegate::updateRowHeight()
{
	m_rowHeight = R::data().iconSize + 2;
}

QSize FsTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QSize size = QItemDelegate::sizeHint(option, index);
	return QSize(size.width(), m_rowHeight);
}

void FsTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QItemDelegate::paint(painter, option, index);

/*	if (m_view->m_hoverIndex == index) {
		QRect o = option.rect;
		QRect r = QRect(o.x(), o.y(), o.height() + SP_ROOT_EX, o.height() - 1);
	//	qDebug() << "FsListDelegate::paint" << r;

		if (m_view->m_hoverState == FsListViewHoverState::Icon) {
			QColor bg(255, 0, 168, 40);
			QBrush brush(bg);
		//	painter->setBrush(brush);
			QColor c = QColor(255, 0, 168, 192);
			painter->setPen(c);
			painter->drawRect(r);
			painter->fillRect(r, brush);
		}
		else {
			QColor bg(0, 160, 255, 40);
			QBrush brush(bg);
		//	painter->setBrush(brush);
			QColor c = QColor(0, 160, 255, 192);
			painter->setPen(c);
			painter->drawRect(r);
			painter->fillRect(r, brush);
		}
	} */
}

//==============================================================================================================================
// class FsTreeView
//==============================================================================================================================

FsTreeView::FsTreeView(FsWidget *widget, QWidget *parent)
	: QTreeView(parent)
	, m_widget(widget)
	, m_handler(widget->handler())
	, m_inited(false)
	, m_clickEnter(R::data().clickEnter)
	, m_hoverArea(FsHoverArea::None)
{
	setRootIsDecorated(false);
	setItemsExpandable(false);
	setUniformRowHeights(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setDragDropMode(QAbstractItemView::DragDrop);
	setDefaultDropAction(Qt::CopyAction);
//	setDefaultDropAction(Qt::MoveAction);
	setDropIndicatorShown(true);
	setEditTriggers(QAbstractItemView::EditKeyPressed /*| QAbstractItemView::SelectedClicked*/ );
	setDefaultDropAction(Qt::IgnoreAction);
	setMouseTracking(true);

	m_delegate = new FsTreeDelegate(this);
	setItemDelegate(m_delegate);
}

FsTreeView::~FsTreeView()
{
}

void FsTreeView::lateStart()
{
	updateIconTheme();	// after: setItemDelegate()
	updateSettings();

	connect(this, SIGNAL(entered(QModelIndex)), m_handler, SLOT(onItemHovered(QModelIndex)));
}

void FsTreeView::updateSettings()
{
	if (m_inited) {
		if (R::data().clickEnter == m_clickEnter) return;
		if (m_clickEnter == ClickEnter::DoubleClick) {
			disconnect(this, SIGNAL(doubleClicked(QModelIndex)), m_handler, SLOT(onItemActivated(QModelIndex)));
		} else {
			disconnect(this, SIGNAL(clicked(QModelIndex)), m_handler, SLOT(onItemActivated(QModelIndex)));
		}
	}

	m_inited = true;
	m_clickEnter = R::data().clickEnter;

	if (m_clickEnter == ClickEnter::DoubleClick) {
		connect(this, SIGNAL(doubleClicked(QModelIndex)), m_handler, SLOT(onItemActivated(QModelIndex)));
	} else {
		connect(this, SIGNAL(clicked(QModelIndex)), m_handler, SLOT(onItemActivated(QModelIndex)));
	}

	if (m_clickEnter != ClickEnter::SingleClick) clearHoverState();
}

void FsTreeView::updateIconTheme()
{
	m_delegate->updateRowHeight();

	int sz = R::data().iconSize;
	setIconSize(QSize(sz, sz));
}

void FsTreeView::updateCursor(Qt::CursorShape cursor)
{
	if (m_cursor != cursor) {
		m_cursor = cursor;
		setCursor(cursor);
	}
}

bool FsTreeView::canHoverSelect(bool isDir)
{
	bool ok = (m_clickEnter == ClickEnter::SingleClick) && isDir;
	updateCursor(ok ? Qt::PointingHandCursor : Qt::ArrowCursor);
	return ok;
}

void FsTreeView::clearHoverState()
{
	m_handler->stopHoverSelect();
	updateCursor(Qt::ArrowCursor);
}

void FsTreeView::execRename()
{
	QModelIndex index = currentIndex();
	if (index.isValid()) edit(index);
}
//==============================================================================================================================

bool FsTreeView::viewportEvent(QEvent *event)
{
	if (m_clickEnter == ClickEnter::SingleClick && event->type() == QEvent::Leave) {
		clearHoverState();
	}
	return QTreeView::viewportEvent(event);
}

void FsTreeView::mouseMoveEvent(QMouseEvent *event)
{
	QTreeView::mouseMoveEvent(event);

	switch (m_clickEnter) {
		case ClickEnter::DoubleClick: break;
		case ClickEnter::SingleClick:
			if (m_cursor != Qt::ArrowCursor && event->button() == Qt::NoButton) {
				bool keyStop = R::data().keyStopHover && QApplication::keyboardModifiers() != Qt::NoModifier;
				if (keyStop || !indexAt(event->pos()).isValid()) clearHoverState();
			}
			break;
		case ClickEnter::ClickIcon:
			m_hoverArea = FsHoverArea::None;
			if (event->button() == Qt::NoButton) {
				QModelIndex index = indexAt(event->pos());
				if (index.isValid()) {
					QRect r = visualRect(index);
					int x0 = r.x() + 4;
					int y0 = r.y() + 1;
					int x1 = x0 + 16;
					int y1 = y0 + 16;
					int x = event->pos().x();
					int y = event->pos().y();
					m_hoverArea = (x >= x0 && x <= x1 && y >= y0 && y <= y1) ? FsHoverArea::Icon : FsHoverArea::Text;
				//	qDebug() << event->pos() << x0 << x1 << y0 << y1 << r;
				}
			}
			updateCursor(m_hoverArea == FsHoverArea::Icon ? Qt::PointingHandCursor : Qt::ArrowCursor);
			break;
	}
}

void FsTreeView::keyPressEvent(QKeyEvent *event)
{
	m_handler->handleKeyPress(event);
}

void FsTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	m_handler->showContextMenu(event);
}
//==============================================================================================================================

void FsTreeView::updateColSizes() { FsModel::updateColSizes(m_widget->app(), this); }
void FsTreeView::saveColSizes()   { FsModel::saveColSizes(m_widget->app(), this); }

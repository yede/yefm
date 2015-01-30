#include <QDropEvent>
#include <QDebug>

#include "yefsubmkview.h"
#include "yefslistview.h"
#include "yefilepane.h"
#include "yeapp.h"
//==============================================================================================================================

FsUbmkView::FsUbmkView(YeFilePane *pane, QWidget *parent)
	: UserView(pane, parent)
{
}

FsUbmkView::~FsUbmkView()
{
}
//==============================================================================================================================

int FsUbmkView::indicatorArea(const QModelIndex &index)
{
	if (index != m_dropIndex) return DropArea::Unknown;

	int from = m_allowOnItem ? DropArea::OnItem : DropArea::AboveItem;

	return m_dropArea >= from ? m_dropArea : DropArea::Unknown;
}

void FsUbmkView::doDragEnter(QDragEnterEvent *event)
{
	QWidget *source = event->source();

	if (source == this) {
		m_dragKind = UbmkDragKind::Bookmark;
		return;
	}

	if (source->inherits("FsListView")) {
		m_dragView = qobject_cast<FsListView *>(source);
		if (m_dragView != NULL) m_dragKind = UbmkDragKind::FilePath;
	}
//	qDebug() << "FsUbmkView::dropEvent" << m_dragView;
}

void FsUbmkView::doDragMove(QDragMoveEvent *event)
{
	QListView::dragMoveEvent(event);
	m_dropIndex = indexAt(event->pos());

	if (!m_dropIndex.isValid()) {
		m_dropArea = DropArea::Unknown;
		event->setAccepted(false);
		return;
	}

	QRect r = visualRect(m_dropIndex);
//	QStandardItem *item = m_model->itemFromIndex(m_dropIndex);

	m_allowOnItem = true;
	int a = calcDropArea(event->pos().y(), r, m_allowOnItem);

//	Qt::DropAction action = m_model->allowDrop(item, event->pos(), area);
//	event->setDropAction(action);
	event->setAccepted(a != DropArea::Unknown);
//	qDebug() << flag << area;

	if (m_dropArea != a) {
		m_dropArea = a;
		update(m_dropIndex);
	}
}

void FsUbmkView::doDrop(QDropEvent *event)
{
	Q_UNUSED(event);
	if (m_dragKind == UbmkDragKind::Bookmark) {
	//	qDebug() << "FsBmkView::dropEvent: self..";
		QListView::dropEvent(event);
	//	event->accept();	// do not accept()! eat bmk when on-item ?
	}
	else if (m_dragKind == UbmkDragKind::FilePath) {
	//	qDebug() << "FsBmkView::dropEvent" << m_dragView;
		QListView::dropEvent(event);
	//	event->accept();	// after: QListView::dropEvent(event)
	}
}

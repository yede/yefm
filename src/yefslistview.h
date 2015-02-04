#ifndef YE_LISTVIEW_H
#define YE_LISTVIEW_H

#include <QItemDelegate>
#include <QListView>
#include <QTreeView>
#include <QFileSystemModel>
//==============================================================================================================================

class FsWidget;
class FsHandler;
class FsListView;

class FsListDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	explicit FsListDelegate(FsListView *view);

	void updateRowHeight();
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	FsListView *m_view;
	int         m_rowHeight;
};
//==============================================================================================================================

class FsListView : public QListView
{
	Q_OBJECT
public:
	explicit FsListView(FsWidget *widget, QWidget *parent = 0);
	~FsListView();

	void lateStart();
	void updateIconTheme();
	void updateSettings();

	int  getHoverArea() const { return m_hoverArea; }
	bool canHoverSelect(bool isDir);
	void clearHoverState();

	void execRename();

protected:
	void keyPressEvent(QKeyEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
	bool viewportEvent(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);

//	void dragEnterEvent(QDragEnterEvent *event);
//	void dragLeaveEvent(QDragLeaveEvent *event);
//	void dragMoveEvent(QDragMoveEvent *event);
//	void dropEvent(QDropEvent *event);
	void startDrag(Qt::DropActions supportedActions);

private:
	void updateCursor(Qt::CursorShape cursor);
	void clearOrRemove();

signals:

public slots:

private:
	friend class FsListDelegate;

	FsWidget        *m_widget;
	FsHandler       *m_handler;
	bool m_inited;
	int m_clickEnter;
	int m_hoverArea;
	Qt::CursorShape m_cursor;

	FsListDelegate  *m_delegate;
};

#endif // YE_LISTVIEW_H

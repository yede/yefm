#ifndef YE_FS_TREEVIEW_H
#define YE_FS_TREEVIEW_H

#include <QItemDelegate>
#include <QTreeView>
#include <QFileSystemModel>
//==============================================================================================================================

class FsWidget;
class FsHandler;
class FsTreeView;

class FsTreeDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	explicit FsTreeDelegate(FsTreeView *view);

	void updateRowHeight();
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	FsTreeView *m_view;
	int         m_rowHeight;
};
//==============================================================================================================================

class FsTreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit FsTreeView(FsWidget *widget, QWidget *parent = 0);
	~FsTreeView();

	void lateStart();
	void updateIconTheme();
	void updateSettings();

	void updateColSizes();
	void saveColSizes();

	int  getHoverArea() const { return m_hoverArea; }
	bool canHoverSelect(bool isDir);
	void clearHoverState();

	void execRename();
	void edit(const QModelIndex &index) { QTreeView::edit(index); }

private:
	void updateCursor(Qt::CursorShape cursor);

protected:
	void keyPressEvent(QKeyEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
	bool viewportEvent(QEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	bool event(QEvent *event);

	void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
	bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);

signals:

public slots:

private:
	FsWidget  *m_widget;
	FsHandler *m_handler;
	bool m_inited, dummy_1, dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;
	int m_clickEnter;
	int m_hoverArea;
	Qt::CursorShape m_cursor;

	FsTreeDelegate  *m_delegate;
};

#endif // YE_FS_TREEVIEW_H

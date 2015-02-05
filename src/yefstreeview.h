#ifndef YE_FSTREEVIEW_H
#define YE_FSTREEVIEW_H

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

private:
	void updateCursor(Qt::CursorShape cursor);

protected:
	void keyPressEvent(QKeyEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
	bool viewportEvent(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

signals:

public slots:

private:
	FsWidget  *m_widget;
	FsHandler *m_handler;
	bool m_inited;
	int m_clickEnter;
	int m_hoverArea;
	Qt::CursorShape m_cursor;

	FsTreeDelegate  *m_delegate;
};

#endif // YE_FSTREEVIEW_H

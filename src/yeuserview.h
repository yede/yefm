#ifndef YE_USERVIEW_H
#define YE_USERVIEW_H

#include <QListView>
#include <QItemDelegate>
//==============================================================================================================================

class YeFilePane;
class FsListView;
class UserView;
class TreeModel;
class TreeNode;

namespace UserViewType {
	enum { Tabs, Bmks, Mnts, Cmds };
}

namespace UserPressArea {
	enum { None, Icon, Text };
}
//==============================================================================================================================

class UserViewDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	UserViewDelegate(UserView *view);

	void updateRowHeight();
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	UserView *m_view;
	int       m_rowHeight;
};
//==============================================================================================================================

class UserView : public QListView
{
	Q_OBJECT
public:
	explicit UserView(YeFilePane *pane, int type, QWidget *parent = 0);
	~UserView();

	void setViewModel(TreeModel *model);
	void setCurrentNode(TreeNode *node);
	void updateNode(TreeNode *node, const QString &name, const QString &path);

	int currentRow() const;
	int viewType() const { return m_type; }

protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void startDrag(Qt::DropActions supportedActions);

public:
	int calcDropArea(int y, const QRect &itemRect, bool allowOnItem);
	int indicatorArea(const QModelIndex &index);

private:
	void updateIconTheme();

signals:
	void itemClicked(TreeNode *node);

public slots:
	void onIconThemeChanged();
	void onHover(const QModelIndex &index);
	void onClicked(const QModelIndex &index);

protected:
	friend class UserViewDelegate;
	friend class UserTabModel;
	friend class UserBmkModel;

	YeFilePane *m_pane;
	int         m_type;
	TreeModel  *m_model;

	UserViewDelegate *m_delegate;
};

#endif // YE_USERVIEW_H

#ifndef YE_USERVIEW_H
#define YE_USERVIEW_H

#include <QListView>
#include <QItemDelegate>
//==============================================================================================================================

class UserView;
class TreeModel;
class TreeNode;

namespace UserViewType {
	enum { Tabs,	// tab list
		   Bmks,	// bookmark list
		   Devs,	// device  list
		   Cmds,	// command list
		   Hiss		// history list
		 };
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
	explicit UserView(int paneIndex, int type, QWidget *parent = 0);
	~UserView();

	void setViewModel(TreeModel *model);
	void setCurrentNode(TreeNode *node);
	void updateNode(TreeNode *node, const QString &name, const QString &path);
	void activate();

	int currentRow() const;
	int viewType() const { return m_type; }
	TreeNode *getSelectedNode() const;

protected:
	void keyPressEvent(QKeyEvent *event);
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

	int         m_paneIndex;
	int         m_type;
	TreeModel  *m_model;

	UserViewDelegate *m_delegate;
};

#endif // YE_USERVIEW_H

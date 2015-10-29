#ifndef YE_TREEMODEL_H
#define YE_TREEMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QDropEvent>
#include "yetreenode.h"
//==============================================================================================================================

namespace DragKind {
	enum { Unknown, Bookmark, Command, TabItem, TabSelf, FileView };
}

namespace DropArea {
	enum { None, OnItem, AboveItem, BelowItem };
}
//==============================================================================================================================

class TreeNode;
class UserView;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(QObject *parent = 0);
	~TreeModel();

	void clear();
	void refresh();

	static QString  mimeType();
	QStringList	    mimeTypes() const;
	QMimeData      *mimeData(const QModelIndexList &indexes) const;
	Qt::DropActions supportedDropActions() const;

	Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual int rowFetch(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
	bool canFetchMore(const QModelIndex &parent) const;
	void fetchMore(const QModelIndex &parent);

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	void   setRootNode(TreeNode *node) { m_rootNode = node; }
	TreeNode *rootNode()         const { return m_rootNode; }

	TreeNode   *getNode(const QModelIndex &index) const;
	QModelIndex getNodeIndex(TreeNode *node)      const;
	QModelIndex getParentIndex(TreeNode *node)    const;

	bool insertNode(TreeNode *node, TreeNode *parent, int pos = -1);
	bool removeNode(TreeNode *node);
	bool moveNodeTo(TreeNode *node, TreeNode *newParent, int newPos);

	virtual QString getStatusMessage(const QModelIndex &) const { return QString(); }

protected:
	virtual bool loadChildren(TreeNode *) { return false; }

protected:

public:
	static QMimeData *createDragMimeData(const QModelIndexList &indexes, const QString &mimeType);
	static void decodeDragMimeData(QList<TreeNode*> &result, const QMimeData *data, const QString &mimeType);

	virtual void cleanupDrag();
	virtual int indicatorArea(UserView *view, const QModelIndex &index);
	virtual bool handleDragEnter(UserView *view, QDragEnterEvent *event);
	virtual bool handleDragMove(UserView *view, QDragMoveEvent *event);
	virtual bool handleDrop(UserView *view, QDropEvent *event);

	virtual void handleMousePress(UserView *view, QMouseEvent *event);
	virtual void showContextMenu(UserView *view, QContextMenuEvent *event);
	virtual bool handleKeyPress(UserView *view, QKeyEvent *event);

signals:

public slots:

protected:
	TreeNode *m_rootNode;
};

#endif // YE_TREEMODEL_H

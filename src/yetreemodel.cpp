#include <QDataStream>
#include <QMimeData>
#include <QDebug>

#include "yetreemodel.h"
#include "yetreenode.h"
//==============================================================================================================================

TreeModel::TreeModel(QObject *parent)
	: QAbstractItemModel(parent)
	, m_rootNode(NULL)
{
}

TreeModel::~TreeModel()
{
}

void TreeModel::clear()
{
	int count = m_rootNode->numFetch();
	if (count < 1) return;

	beginRemoveRows(QModelIndex(), 0, count - 1);
	m_rootNode->clear();
	endRemoveRows();
}

void TreeModel::refresh()
{
	beginResetModel();
	endResetModel();
}
//==============================================================================================================================
#define UDEF_MIME_TYPE "yefm/udef"

QString TreeModel::mimeType()
{
	return UDEF_MIME_TYPE;
}

QStringList	TreeModel::mimeTypes() const
{
	QStringList types;
	types << mimeType() << "text/uri-list";
	return types;
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
	return createDragMimeData(indexes, mimeType());
}

Qt::DropActions TreeModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}
//==============================================================================================================================

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
		   Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

int TreeModel::rowFetch(const QModelIndex &parent) const
{
	return getNode(parent)->numFetch();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	return getNode(parent)->ownCount();
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_rootNode->columnCount();
}

bool TreeModel::hasChildren(const QModelIndex &parent) const
{
	return getNode(parent)->ownCount() > 0;
}

bool TreeModel::canFetchMore(const QModelIndex &parent) const
{
	return getNode(parent)->canFetchMore();
}

void TreeModel::fetchMore(const QModelIndex &parent)
{
	TreeNode *node = getNode(parent);
	int startRow = node->numFetch();
	int lastRow = node->ownCount() - 1;

	beginInsertRows(parent, startRow, lastRow);
	bool success = loadChildren(node);
	endInsertRows();

	if (success) {}
}
//==============================================================================================================================

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	TreeNode *p = getNode(parent);
	TreeNode *child = p->child(row);

	return (child == NULL) ? QModelIndex() : createIndex(row, column, child);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	TreeNode *p = getNode(index)->parent();

	return (p == m_rootNode || p == NULL) ? QModelIndex() : createIndex(p->row(), 0, p);
}
//==============================================================================================================================

TreeNode *TreeModel::getNode(const QModelIndex &index) const
{
	if (index.isValid()) {
		TreeNode *node = static_cast<TreeNode *>(index.internalPointer());
		if (node != NULL) return node;
	}
	return m_rootNode;
}

QModelIndex TreeModel::getNodeIndex(TreeNode *node) const
{
	if (node == m_rootNode) return QModelIndex();

	return createIndex(node->row(), 0, node);
}

QModelIndex TreeModel::getParentIndex(TreeNode *node) const
{
	if (node == m_rootNode || node == NULL) return QModelIndex();

	return getNodeIndex(node->parent());
}
//==============================================================================================================================

bool TreeModel::insertNode(TreeNode *node, TreeNode *parent, int pos)
{
	if (node == NULL || parent == NULL) return false;

	if (pos > parent->ownCount() || pos < 0) pos = parent->ownCount();

	QModelIndex pindex = getNodeIndex(parent);

	beginInsertRows(pindex, pos, pos);
	parent->insertChild(node, pos);
	node->setParent(parent);
	endInsertRows();

	return true;
}

bool TreeModel::removeNode(TreeNode *node)
{
	if (node == NULL) return false;

	TreeNode *p = node->parent();
	if (p == NULL) return false;

	int pos = node->row();
	if (pos < 0) return false;

	QModelIndex pindex = getNodeIndex(p);

	beginRemoveRows(pindex, pos, pos);
	p->removeChild(node);
	endRemoveRows();

	return true;
}

bool TreeModel::moveNodeTo(TreeNode *node, TreeNode *newParent, int newPos)
{
	if (removeNode(node)) {
		return insertNode(node, newParent, newPos);
	}

	return false;
}
//==============================================================================================================================

void TreeModel::cleanupDrag()
{
}

int TreeModel::indicatorArea(UserView *view, const QModelIndex &index)
{
	Q_UNUSED(view);
	Q_UNUSED(index);
	return DropArea::None;
}

bool TreeModel::handleDragEnter(UserView *view, QDragEnterEvent *event)
{
	Q_UNUSED(view);
	Q_UNUSED(event);
	return false;
}

bool TreeModel::handleDragMove(UserView *view, QDragMoveEvent *event)
{
	Q_UNUSED(view);
	Q_UNUSED(event);
	return false;
}

bool TreeModel::handleDrop(UserView *view, QDropEvent *event)
{
	Q_UNUSED(view);
	Q_UNUSED(event);
	return false;
}

void TreeModel::handleMousePress(UserView *view, QMouseEvent *event)
{
	Q_UNUSED(view);
	Q_UNUSED(event);
}

void TreeModel::showContextMenu(UserView *view, QContextMenuEvent *event)
{
	Q_UNUSED(view);
	Q_UNUSED(event);
}
//==============================================================================================================================

QMimeData *TreeModel::createDragMimeData(const QModelIndexList &indexes, const QString &mimeType)
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	quint64 pointer;	// 32,64 compatible
	foreach (QModelIndex index, indexes) {
		if (index.isValid()) {
			pointer = reinterpret_cast<quint64>(index.internalPointer());	// node*
			stream << pointer;
		}
	}
	mimeData->setData(mimeType, encodedData);
	return mimeData;
}

void TreeModel::decodeDragMimeData(QList<TreeNode*> &result, const QMimeData *data, const QString &mimeType)
{
	QByteArray encodedData = data->data(mimeType);
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	TreeNode *item;
	quint64 pointer;	// 32,64 compatible
	while (!stream.atEnd()) {
		stream >> pointer;
		item = reinterpret_cast<TreeNode *>(pointer);
		result.append(item);
	}
}
//==============================================================================================================================

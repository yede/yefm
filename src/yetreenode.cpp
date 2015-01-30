#include <QDebug>
#include "yetreenode.h"
//==============================================================================================================================

TreeNode::TreeNode(int type, TreeNode *parent)
	: m_parent(parent)
	, m_ownCount(0)
	, m_id(0)
	, m_type(type & 0x00ff)
	, m_arg(NULL)
{
}

TreeNode::~TreeNode()
{
	qDeleteAll(m_children);
}
//==============================================================================================================================

int TreeNode::row() const
{
	if (!m_parent) return -1;
	return m_parent->m_children.indexOf(const_cast<TreeNode*>(this));
}

const QString &TreeNode::tips() const
{
	return m_tips.isEmpty() ? m_title : m_tips;
}

TreeNode *TreeNode::child(int row) const
{
	return (row < 0 || row >= m_children.size()) ? NULL : m_children.at(row);
}
//==============================================================================================================================

void TreeNode::appendChild(TreeNode *child)
{
	if (!child) return;

	m_children.append(child);
	incOwnCount();
}

void TreeNode::insertChild(TreeNode *child, int pos)
{
	if (!child) return;

	int cnt = m_children.size();
	if (pos < 0 || pos > cnt) pos = cnt;
	m_children.insert(pos, child);
	incOwnCount();
}

void TreeNode::removeChild(TreeNode *child)
{
	if (!child) return;

	if (m_children.removeOne(child))
		decOwnCount();
}

void TreeNode::clear()
{
	foreach (TreeNode *node, m_children) {
		delete node;
	}
	m_children.clear();
}
//==============================================================================================================================

void TreeNode::dump(TreeNode *node)
{
	qDebug() << "rowCount:" << node->m_ownCount
			 << "id:" << node->m_id
			 << "type:" << node->m_type
			 << "title:" << node->m_title
			 << "tips:" << node->m_tips
			 << "path:" << node->m_path
			 << "parm:" << node->m_parm;
}

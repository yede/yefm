#ifndef YE_TREENODE_H
#define YE_TREENODE_H

#include <QList>
#include <QVariant>
//==============================================================================================================================

namespace NodeType {
	enum { Unknown = 0,
		   Bookmark, Command, Device, Tab, Folder, File };
}

namespace PosModus {
	enum { PosUnknown = 0,
		   AppendFirst, AppendLast, InsertBefore, InsertAfter };
}
//==============================================================================================================================

class TreeNode
{
public:
	explicit TreeNode(int type, TreeNode *parent = NULL);
	virtual ~TreeNode();

	void          *arg()   const { return m_arg; }
	quint64        id()    const { return m_id; }
	int            type()  const { return m_type & 0x00ff; }
	const QString &title() const { return m_title; }
	const QString &tips()  const;
	const QString &path()  const { return m_path; }
	const QString &parm()  const { return m_parm; }

	void setArg(void *arg)             { m_arg = arg; }
	void setId(quint64 id)             { m_id = id; }
	void setType(int type)             { m_type = (type & 0x00ff) | (m_type & 0xff00); }
	void setTitle(const QString &text) { m_title = text; }
	void setTips(const QString &text)  { m_tips = text; }
	void setPath(const QString &text)  { m_path = text; }
	void setParm(const QString &text)  { m_parm = text; }

	bool isSeparator() const     { return (m_type & 0x0100) != 0; }
	void setSeparator(bool flag) { m_type = (flag ? 0x0100 : 0x0000) | (m_type & 0x00ff); }

	TreeNode          *parent()       const { return m_parent; }
	TreeNode          *child(int row) const;
	QList<TreeNode *> &children()           { return m_children; }

	void setParent(TreeNode *parent) { m_parent = parent; }
	void appendChild(TreeNode *child);
	void insertChild(TreeNode *child, int pos = -1);
	void removeChild(TreeNode *child);
	void clear();

	bool canFetchMore() const { return m_children.count() < m_ownCount; }
	int columnCount() const { return 1; }
	int row() const;									// row number of this node
	int numFetch() const { return m_children.count(); }	// fetched children
	int ownCount() const { return m_ownCount; }			// owned count

	void setOwnCount(int value) { m_ownCount = value; }
	void incOwnCount() { m_ownCount ++; }
	void decOwnCount() { m_ownCount --; }

	static void dump(TreeNode *node);
	void        dump() { dump(this); }

protected:
	QList<TreeNode *> m_children;	// fetched children
	TreeNode *m_parent;
	int       m_ownCount;			// owned count (include not fetched)
	quint64   m_id;
	quint32   m_type;
	void     *m_arg;

	QString   m_title;
	QString   m_tips;
	QString   m_path;
	QString   m_parm;
};

#endif // YE_TREENODE_H

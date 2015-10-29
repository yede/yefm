#ifndef YE_USERBMKMODEL_H
#define YE_USERBMKMODEL_H

#include <QColor>
#include "yetreemodel.h"
//==============================================================================================================================

class Style;
class FsListView;

namespace BmkModelAction {
	enum {
		None,
		Activate,
		AddBmk,
		AddSep,
		Edit,
		Remove,		// Directly, without confirmation
		RemoveA		// with asking
	};
}

class UserBmkModel : public TreeModel
{
	Q_OBJECT
public:
	explicit UserBmkModel(QObject *parent = 0);
	~UserBmkModel();

	void lateStart();

	void saveBookmarkList();
	void loadBookmarkList();

	TreeNode *insertNode(const QString &title, const QString &path, int pos = -1);

	QString getStatusMessage(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

	void cleanupDrag();
	int indicatorArea(UserView *view, const QModelIndex &index);
	bool handleDragEnter(UserView *view, QDragEnterEvent *event);
	bool handleDragMove(UserView *view, QDragMoveEvent *event);
	bool handleDrop(UserView *view, QDropEvent *event);

	void showContextMenu(UserView *view, QContextMenuEvent *event);
	bool handleKeyPress(UserView *view, QKeyEvent *event);

	static UserBmkModel *instance() { return m_instance; }
	static void createInstance()    { if (!m_instance) m_instance = new UserBmkModel; }
	static void deleteInstance()    { delete m_instance; m_instance = NULL; }
	static void setupShortcuts();

private:
	void askRemove(UserView *view);
	void removeSelection(UserView *view);
	void removeNode(TreeNode *node);

signals:

public slots:
	void updateIconTheme();
	void onAddSep();
	void onEdit();
	void onRemove();

private:
	Style     *m_style;
	TreeNode    *m_contextNode;
	QAction     *m_actAddSep;
	QAction     *m_actRename;
	QAction     *m_actRemove;

	int          m_dragKind;
	FsListView  *m_dragView;
	UserView    *m_dropView;
	bool         m_allowOnItem, dummy_1, dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;

	QModelIndex  m_dropIndex;
	int          m_dropArea;

	static UserBmkModel *m_instance;
};

#endif // YE_USERBMKMODEL_H

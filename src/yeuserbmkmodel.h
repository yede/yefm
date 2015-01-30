#ifndef YE_USERBMKMODEL_H
#define YE_USERBMKMODEL_H

#include <QColor>
#include "yetreemodel.h"
//==============================================================================================================================

class YeApplication;
class FsListView;

class UserBmkModel : public TreeModel
{
	Q_OBJECT
public:
	explicit UserBmkModel(YeApplication *app, QObject *parent = 0);
	~UserBmkModel();

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

protected:

signals:

public slots:
	void onIconThemeChanged();
	void onAddSep();
	void onEdit();
	void onRemove();

private:
	YeApplication *m_app;

	TreeNode    *m_contextNode;
	QAction     *m_actAddSep;
	QAction     *m_actRename;
	QAction     *m_actRemove;

	int          m_dragKind;
	FsListView  *m_dragView;
	UserView    *m_dropView;
	bool         m_allowOnItem;

	QModelIndex  m_dropIndex;
	int          m_dropArea;
};

#endif // YE_USERBMKMODEL_H

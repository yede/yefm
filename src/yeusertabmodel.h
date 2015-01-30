#ifndef YE_USERTABMODEL_H
#define YE_USERTABMODEL_H

#include <QTimer>
#include "yetreemodel.h"
//==============================================================================================================================

class YeFilePane;
class FsListView;
class UserView;

class UserTabModel : public TreeModel
{
	Q_OBJECT
public:
	explicit UserTabModel(YeFilePane *pane, QObject *parent = 0);
	~UserTabModel();

	void saveTabs();
	void loadTabs();

	TreeNode *insertTab(int pos, const QString &path);
	void deleteTab(TreeNode *node);
	void setUserView(UserView *view) { m_view = view; }

	QVariant data(const QModelIndex &index, int role) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

	void cleanupDrag();
	int indicatorArea(UserView *view, const QModelIndex &index);
	bool handleDragEnter(UserView *view, QDragEnterEvent *event);
	bool handleDragMove(UserView *view, QDragMoveEvent *event);
	bool handleDrop(UserView *view, QDropEvent *event);

	QString getStatusMessage(const QModelIndex &index) const;

private:
	TreeNode *doAddTab(const QString &name, const QString &path, int pos);

public:

signals:

public slots:
	void onTimeout();

private:
	YeFilePane *m_pane;
	UserView   *m_view;

	int         m_dragKind;
	bool        m_allowOnItem;

	QModelIndex m_dropIndex;
	int         m_dropArea;

	QTimer      m_timer;
	QStringList m_dragPaths;
	int         m_dropRow;
};

#endif // YE_USERTABMODEL_H

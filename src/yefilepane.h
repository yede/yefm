#ifndef YEFILEPANE_H
#define YEFILEPANE_H

#include <QWidget>
//==============================================================================================================================

class QMenu;
class QStackedLayout;
class QStackedWidget;

class YeApplication;
class YeMainWindow;
class Splitter;
class PathWidget;
class SideSysView;
class SideMntView;
class SideCmdView;
class FsWidget;
class LineEdit;
class LineEditButton;
class UserTabModel;
class UserView;
class TreeNode;
class ToolBar;
class ToolIcon;

class YeFilePane : public QWidget
{
	Q_OBJECT
public:
	explicit YeFilePane(YeMainWindow *mainWindow, QWidget *parent = 0);
	~YeFilePane();

	void loadSessionTabs();
	void saveSessionTabs();

	void showPath(FsWidget *widget, const QString &path);
	void setWorkPath(const QString &path);
	void setWorkView(FsWidget *view, bool showPath = true);

	bool addTab(bool focus = true);
	bool addTab(const QString &path, bool focus);
	FsWidget *insertTab(const QString &path, int pos, bool focus);
	bool insertTab(FsWidget *view, int pos, bool focus = true);
	bool takeTab(FsWidget *view);
	void deleteTab(FsWidget *view);
	void updateTab(TreeNode *node, const QString &name, const QString &path);

	YeApplication *app()         const { return m_app; }
	YeMainWindow  *win()         const { return m_win; }
	FsWidget      *currentView() const { return m_current; }
	bool isHiddenVisible() const;
	int  currentViewMode() const;

private:
	void updateIcons();
	void showCurrentPath(const QString &path);
	void setSideStackMode(int mode);
	QWidget *setupTabsToolBar();
	QWidget *setupSideToolBar();
	QWidget *setupPathToolBar();

signals:
	void toggleHidden(bool visible);

public slots:
	void onIconThemeChanged();

	void onBookmarkList();
	void onCommandList();
	void onMountedDevices();
	void onFileTree();

	void onViewDetails();
	void onHidden();
	void onRefresh();
	void onGoUp();

	void openTerminal();
	void on2pane();

	void onTogglePathEdit();
	void onPathChanged();
	void onPathCleared();

	void onSysClicked(const QString &path);
	void onBmkClicked(TreeNode *node);
	void onMntClicked(TreeNode *node);
	void onCmdClicked(TreeNode *node);
	void onTabClicked(TreeNode *node);

	void onAddTab();
	void onDeleteTab();

	void onSettings();
	void onStyleSettings();
	void onMimeSettings();
	void onCustomOpenSettings();

private:
	YeApplication  *m_app;
	YeMainWindow   *m_win;
	bool            m_has2paneButton;
	bool            m_rightSide;
	FsWidget       *m_current;

	Splitter       *m_spmain;
	QStackedWidget *m_fileStack;
	QStackedLayout *m_sideStack;

	UserView       *m_bmkView;
	SideSysView    *m_sysView;
	UserView       *m_mntView;
	UserView       *m_cmdView;

	UserView       *m_tabList;
	UserTabModel   *m_tabModel;

	PathWidget     *m_pathWidget;
	LineEdit       *m_pathEdit;
	QStackedWidget *m_pathStack;
	//--------------------------------------------------------------------------------------------------------------------------

	ToolBar *m_tabsToolBar;
	ToolBar *m_sideToolBar;
	ToolBar *m_pathToolBar;

	ToolIcon *m_btBookmarkList;
	ToolIcon *m_btMountedDevices;
	ToolIcon *m_btFileTree;
	ToolIcon *m_btViewDetails;
	ToolIcon *m_btHidden;
	ToolIcon *m_btRefresh;
	ToolIcon *m_btUp;
	ToolIcon *m_btTerm;
	ToolIcon *m_bt2pane;
	ToolIcon *m_btTogglePathEdit;

	ToolIcon *m_btAddTab;
	ToolIcon *m_btDeleteTab;

	LineEditButton *m_btPathClear;
	//--------------------------------------------------------------------------------------------------------------------------

	ToolIcon *m_btMenu;
	QMenu    *m_menu;

	QAction  *m_actSettings;
	QAction  *m_actStyleSettings;
	QAction  *m_actMimeSettings;
	QAction  *m_actFsActions;
};

#endif // YEFILEPANE_H

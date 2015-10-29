#ifndef YEFILEPANE_H
#define YEFILEPANE_H

#include <QWidget>
//==============================================================================================================================

class QMenu;
class QStackedLayout;
class QStackedWidget;

class Application;
class AppCfg;
class MainWindow;

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
class UDevView;
class TreeNode;
class ToolBar;
class ToolIcon;
class FilePaneDelegate;

class FilePane : public QWidget
{
	Q_OBJECT
public:
	explicit FilePane(QWidget *statusbar, int index, QWidget *parent = 0);
	~FilePane();

	void loadSessionTabs(const QStringList &startPaths);
	void saveSessionTabs();
	void updateSessionData();

	void showPath(FsWidget *widget, const QString &path);
	void setWorkView(FsWidget *view, bool showPath = true);

	bool addTab(bool focus = true);
	bool addTab(const QString &path, bool focus);
	FsWidget *insertTab(const QString &path, int pos, bool focus);
	bool insertTab(FsWidget *view, int pos, bool focus = true);
	bool takeTab(FsWidget *view);
	void deleteTab(FsWidget *view);
	void updateTab(TreeNode *node, const QString &name, const QString &path);

	void setupWidget(FsWidget *widget);

	FsWidget *currentView()     const { return m_current; }
	int       index()           const { return m_index; }
	int       currentViewMode() const;
	bool      isHiddenVisible() const;

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
	void setWorkPath(const QString &path);

	void onBookmarkList();
	void onCommandList();
	void onDevList();
	void onFileTree();
	void onHistory();

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
	void onDevClicked(const QString &path);
	void onBmkClicked(TreeNode *node);
	void onCmdClicked(TreeNode *node);
	void onTabClicked(TreeNode *node);
	void onHisClicked(TreeNode *node);

	void onAddTab();
	void onDeleteTab();

	void onSettings();
	void onStyleSettings();
	void onMimeSettings();
	void onCustomOpenSettings();
	void onShowPastePad();

private:
	Application    *m_app;
	AppCfg         *m_cfg;
	MainWindow     *m_win;
	int             m_index;
	bool            m_has2paneButton;
	bool            m_rightSide, dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;
	FsWidget       *m_current;

	FilePaneDelegate *m_delegate;

	Splitter       *m_spside;
	Splitter       *m_spmain;
	QStackedWidget *m_fileStack;
	QStackedLayout *m_sideStack;

	UserView       *m_bmkView;
	UserView       *m_hisView;
	SideSysView    *m_sysView;
	UDevView       *m_devView;
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
	ToolIcon *m_btHistory;
	ToolIcon *m_btDevList;
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
	QAction  *m_actShowPastePad;
};

#endif // YEFILEPANE_H

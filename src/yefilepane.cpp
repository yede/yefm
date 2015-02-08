#include <QVBoxLayout>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QFile>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QSettings>
#include <QDebug>

#include "yefilepane.h"
#include "yefswidget.h"

#include "yesidesysview.h"
#include "yeuserview.h"
#include "yeuserbmkmodel.h"
#include "yesidemntmodel.h"
#include "yesidecmdmodel.h"
#include "yeusertabmodel.h"

#include "yesplitter.h"
#include "yepathwidget.h"
#include "yetoolbar.h"
#include "yetoolitem.h"
#include "yelineedit.h"

#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
#include "yeapplication.h"
#include "yemainwindow.h"
//==============================================================================================================================

YeFilePane::YeFilePane(YeMainWindow *mainWindow, int index, QWidget *parent)
	: QWidget(parent)
	, m_app(mainWindow->app())
	, m_win(mainWindow)
	, m_index(index)
	, m_has2paneButton(mainWindow->pane0() == NULL)
	, m_rightSide(R::data().rightSide)
	, m_current(NULL)
{
	QWidget *tabList = setupTabsToolBar();
	QWidget *bmkList = setupSideToolBar();

	Splitter *spside = new Splitter;
	spside->setDirection(SplitterDirection::Bottom, 120);
	spside->setClient(tabList, bmkList);
	//--------------------------------------------------------------------------------------------------------------------------

	m_fileStack = new QStackedWidget;
	m_fileStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_spmain = new Splitter;
	m_spmain->setDirection(m_rightSide ? SplitterDirection::Right : SplitterDirection::Left, 180);
	m_spmain->setClient(spside, m_fileStack);
	//--------------------------------------------------------------------------------------------------------------------------

	QWidget *toolBar = setupPathToolBar();

	QVBoxLayout *box = new QVBoxLayout(this);
	box->setSpacing(0);
	box->setContentsMargins(0, 0, 0, 0);
	box->addWidget(toolBar);
	box->addWidget(m_spmain);
	//--------------------------------------------------------------------------------------------------------------------------

	updateIcons();
	connect(m_app, SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));
	//--------------------------------------------------------------------------------------------------------------------------

	onBookmarkList();
}

YeFilePane::~YeFilePane()
{
}
//==============================================================================================================================

static ToolBar *createSideToolBar(bool tail, bool rightSide)
{
	ToolBar *toolBar = new ToolBar(Qt::Vertical);

	int w = R::iconSize();
	int l = rightSide ? 0 : 2;
	int r = rightSide ? 2 : 0;

	toolBar->setIconSize(w, w);
	toolBar->setBasePads(l, 0, r, 1);
	toolBar->setItemPads(3, 3, 3, 3);
	toolBar->setItemSpacing(1);

	toolBar->setupLayout();
	if (tail) toolBar->setupTailLayout();

	return toolBar;
}

QWidget *YeFilePane::setupSideToolBar()
{
	m_sideToolBar = createSideToolBar(true, m_rightSide);

	m_btBookmarkList   = m_sideToolBar->addToolIcon(tr("Bookmark List"), this, SLOT(onBookmarkList()));
	m_btMountedDevices = m_sideToolBar->addToolIcon(tr("Mounted Devices"), this, SLOT(onMountedDevices()));
	m_btFileTree       = m_sideToolBar->addToolIcon(tr("File System"), this, SLOT(onFileTree()));
//	m_sideToolBar->addSeparator(4);

	m_btViewDetails    = m_sideToolBar->addToolIcon(tr("Toggle list/details mode"), this, SLOT(onViewDetails()));

	m_btBookmarkList->setAutoCheck(true);
	m_btMountedDevices->setAutoCheck(true);
	m_btFileTree->setAutoCheck(true);
	m_btViewDetails->setAutoCheck(true);

	m_btHidden  = m_sideToolBar->addToolIcon(tr("Toggle Hidden Files"), this, SLOT(onHidden()));
	m_btRefresh = m_sideToolBar->addToolIcon(tr("Refresh"), this, SLOT(onRefresh()));
	m_btUp      = m_sideToolBar->addToolIcon(tr("Go Up"), this, SLOT(onGoUp()));

	m_btTerm    = m_sideToolBar->addTailIcon(tr("Open Terminal..."), this, SLOT(openTerminal()));

	m_btHidden->setAutoCheck(true);
	//--------------------------------------------------------------------------------------------------------------------------

	m_sysView = new SideSysView(this);
	m_mntView = new UserView(this, UserViewType::Mnts);
	m_bmkView = new UserView(this, UserViewType::Bmks);
//	m_cmdView = new UserView(this, UserViewType::Cmds);

	m_bmkView->setViewModel(m_app->bmkModel());
	m_mntView->setViewModel(m_app->mntModel());

	connect(m_bmkView, SIGNAL(itemClicked(TreeNode*)), this, SLOT(onBmkClicked(TreeNode*)));
	connect(m_mntView, SIGNAL(itemClicked(TreeNode*)), this, SLOT(onMntClicked(TreeNode*)));
	connect(m_sysView, SIGNAL(itemClicked(QString)), this, SLOT(onSysClicked(QString)));
	//--------------------------------------------------------------------------------------------------------------------------

	m_sideStack = new QStackedLayout;
	m_sideStack->setContentsMargins(0, 0, 0, 0);
	m_sideStack->setSpacing(0);

	m_sideStack->addWidget(m_bmkView);
	m_sideStack->addWidget(m_sysView);
	m_sideStack->addWidget(m_mntView);
//	m_sideStack->addWidget(m_cmdView);
	//--------------------------------------------------------------------------------------------------------------------------

	QWidget *widget = new QWidget;
	QHBoxLayout *box = new QHBoxLayout(widget);
	box->setSpacing(0);
	box->setContentsMargins(0, 0, 0, 0);

	if (m_rightSide) {
		box->addWidget(m_sideToolBar);
		box->addLayout(m_sideStack);
	} else {
		box->addLayout(m_sideStack);
		box->addWidget(m_sideToolBar);
	}

	return widget;
}

QWidget *YeFilePane::setupTabsToolBar()
{
	m_tabsToolBar = createSideToolBar(m_has2paneButton, m_rightSide);

	m_btAddTab = m_tabsToolBar->addToolIcon(tr("Add tab"), this, SLOT(onAddTab()));
	m_btDeleteTab = m_tabsToolBar->addToolIcon(tr("Delete tab"), this, SLOT(onDeleteTab()));

	if (m_has2paneButton) {
		m_bt2pane   = m_tabsToolBar->addTailIcon(tr("Toggle 2 panes"), this, SLOT(on2pane()));
		m_bt2pane->setAutoCheck(true);
	}
	//--------------------------------------------------------------------------------------------------------------------------

	m_tabModel = new UserTabModel(this, this);
	m_tabList = new UserView(this, UserViewType::Tabs);
	m_tabList->setViewModel(m_tabModel);
	m_tabModel->setUserView(m_tabList);

	connect(m_tabList, SIGNAL(itemClicked(TreeNode*)), this, SLOT(onTabClicked(TreeNode*)));
	//--------------------------------------------------------------------------------------------------------------------------

	QWidget *widget = new QWidget;
	QHBoxLayout *box = new QHBoxLayout(widget);
	box->setSpacing(0);
	box->setContentsMargins(0, 0, 0, 0);

	if (m_rightSide) {
		box->addWidget(m_tabsToolBar);
		box->addWidget(m_tabList);
	} else {
		box->addWidget(m_tabList);
		box->addWidget(m_tabsToolBar);
	}

	return widget;
}
//==============================================================================================================================

QWidget *YeFilePane::setupPathToolBar()
{
	m_pathToolBar = new ToolBar;

	int w = R::iconSize();
	int top = m_has2paneButton ? 1 : 0;

	m_pathToolBar->setIconSize(w, w);
	m_pathToolBar->setBasePads(0, top, 0, 1);
	m_pathToolBar->setItemPads(4, 4, 4, 4);
	m_pathToolBar->setItemSpacing(4);
	m_pathToolBar->setHasSpacer(false);

	m_pathToolBar->setupLayout();
	m_pathToolBar->setupTailLayout();
	//--------------------------------------------------------------------------------------------------------------------------

	m_pathWidget = new PathWidget(this);
	m_pathEdit   = new LineEdit;
	m_pathStack  = new QStackedWidget;

	m_pathStack->addWidget(m_pathWidget);
	m_pathStack->addWidget(m_pathEdit);

	m_btPathClear = m_pathEdit->addButton(LineEdit::RightSide, tr("Clear"), this, SLOT(onPathCleared()));

	connect(m_pathEdit, SIGNAL(editingFinished()), this, SLOT(onPathChanged()));
	//--------------------------------------------------------------------------------------------------------------------------

	m_btTogglePathEdit = m_pathToolBar->addToolIcon(tr("Toggle Path Edit"), this, SLOT(onTogglePathEdit()));
	m_btTogglePathEdit->setAutoCheck(true);

	m_pathToolBar->addItem(m_pathStack, 10, 0);
	//--------------------------------------------------------------------------------------------------------------------------

	m_menu = new QMenu;
	m_actSettings      = m_menu->addAction(tr("Settings..."), this, SLOT(onSettings()));
	m_actStyleSettings = m_menu->addAction(tr("Style settings..."), this, SLOT(onStyleSettings()));
	m_actMimeSettings  = m_menu->addAction(tr("Mime open settings..."), this, SLOT(onMimeSettings()));
	m_actFsActions     = m_menu->addAction(tr("Custom open settings..."), this, SLOT(onCustomOpenSettings()));
//	m_menu->addSeparator();
	//--------------------------------------------------------------------------------------------------------------------------

	m_btMenu = m_pathToolBar->addTailIcon(tr("Menu"));
	m_btMenu->setMenu(m_menu);
	//--------------------------------------------------------------------------------------------------------------------------

	return m_pathToolBar;
}

void YeFilePane::updateIcons()
{
	m_btBookmarkList->setIcon(R::icon("star"));
	m_btMountedDevices->setIcon(R::icon("device"));
	m_btFileTree->setIcon(R::icon("view-list-tree"));
	m_btViewDetails->setIcon(R::icon("view-list-text"));	// view-list-text  view-list-details  view-list-icons

	m_btHidden->setIcon(R::icon("view-hidden"));
	m_btRefresh->setIcon(R::icon("view-refresh"));
	m_btUp->setIcon(R::icon("go-up"));
	m_btTerm->setIcon(R::icon("utilities-terminal"));

	m_btAddTab->setIcon(R::icon("list-add"));
	m_btDeleteTab->setIcon(R::icon("list-remove"));
	if (m_has2paneButton) m_bt2pane->setIcon(R::icon("view-split-top-bottom"));

	m_btTogglePathEdit->setIcon(R::icon("gtk-edit"));
	m_btPathClear->setPixmap(R::pixmap("clear"));
	m_btMenu->setIcon(R::icon("menu"));

	const QIcon &iconSettings = R::icon("preferences-desktop");
	m_actSettings->setIcon(iconSettings);
	m_actStyleSettings->setIcon(iconSettings);
	m_actMimeSettings->setIcon(iconSettings);
	m_actFsActions->setIcon(iconSettings);
	//--------------------------------------------------------------------------------------------------------------------------

	int w = R::iconSize();
	m_tabList->setIconSize(QSize(w, w));
}

void YeFilePane::onIconThemeChanged()
{
	int w = R::iconSize();
	m_tabsToolBar->updateIconSize(w, w);
	m_sideToolBar->updateIconSize(w, w);
	m_pathToolBar->updateIconSize(w, w);
	//--------------------------------------------------------------------------------------------------------------------------

	updateIcons();
	//--------------------------------------------------------------------------------------------------------------------------

	m_btBookmarkList->update();
	m_btMountedDevices->update();
	m_btFileTree->update();

	m_btRefresh->update();
	m_btHidden->update();
	m_btUp->update();
	m_btTerm->update();

	m_btAddTab->update();
	m_btDeleteTab->update();
	if (m_has2paneButton) m_bt2pane->update();

	m_btTogglePathEdit->update();
	m_btPathClear->update();
	m_btMenu->update();
	m_menu->update();
}
//==============================================================================================================================

void YeFilePane::loadSessionTabs(const QStringList &startPaths)
{
	FsWidget *view = NULL;
	QStringList tabs;

	QSettings s(App::getSessionTabsFile(), QSettings::IniFormat);
	s.beginGroup(QString("pane-%1").arg(m_index));
	tabs = s.value("tabs").toStringList();
	s.endGroup();

	foreach (const QString &path, tabs) {
		if (QDir(path).exists()) view = insertTab(path, -1, false);
	}
	foreach (const QString &path, startPaths) {
		if (QDir(path).exists()) view = insertTab(path, -1, false);
	}

	if (view == NULL) {
		addTab();
	} else {
		setWorkView(view, true);
	}
}

void YeFilePane::saveSessionTabs()
{
	QStringList paths;
	TreeNode *root = m_tabModel->rootNode();
	QList<TreeNode *> &tabs = root->children();

	foreach (TreeNode *node, tabs) {
		FsWidget *view = static_cast<FsWidget *>(node->arg());
		paths.append(view->workPath());
	}

	QSettings s(App::getSessionTabsFile(), QSettings::IniFormat);
	s.beginGroup(QString("pane-%1").arg(m_index));
	s.setValue("tabs", paths);
	s.endGroup();
}
//==============================================================================================================================

int  YeFilePane::currentViewMode() const { return m_btViewDetails->isChecked() ? FsWidgetMode::Tree : FsWidgetMode::List; }
bool YeFilePane::isHiddenVisible() const { return m_btHidden->isChecked(); }

void YeFilePane::showPath(FsWidget *widget, const QString &path)
{
	if (m_current == widget) {
		if (m_current != NULL) showCurrentPath(path);
	}
}

void YeFilePane::showCurrentPath(const QString &path)
{
	m_pathWidget->showPath(path);
	if (m_pathEdit->text() != path) m_pathEdit->setText(path);
}
//==============================================================================================================================

void YeFilePane::setWorkView(FsWidget *view, bool showPath)
{
	if (m_current != NULL) {
		m_current->saveColSizes();
	}

	if (view != NULL) {
		TreeNode *node = view->tab();
		view->setViewMode(currentViewMode());
		m_tabList->setCurrentNode(node);
		m_fileStack->setCurrentWidget(view);

		if (showPath) showCurrentPath(view->workPath());
		view->updateColSizes();
	}

	m_current = view;
}

void YeFilePane::setWorkPath(const QString &path)
{
	if (m_current != NULL) m_current->setWorkPath(path);
}

void YeFilePane::updateTab(TreeNode *node, const QString &name, const QString &path)
{
	m_tabList->updateNode(node, name, path);
}

bool YeFilePane::addTab(bool focus)
{
	return addTab(QDir::homePath(), focus);
}

bool YeFilePane::addTab(const QString &path, bool focus)
{
	if (!QDir(path).exists()) {
		QString text = QString("%1   \n\n%2   \n").arg(tr("Path not exests:")).arg(path);
		m_win->showPadMessage(text, "", 3000);
		return false;
	}

	int pos = m_tabList->currentRow() + 1;
	FsWidget *view = insertTab(path, pos, focus);

	return view != NULL;
}

FsWidget *YeFilePane::insertTab(const QString &path, int pos, bool focus)
{
	if (!QDir(path).exists()) return NULL;

	TreeNode *node = m_tabModel->insertTab(pos, path);
	FsWidget *view = new FsWidget(this, node);
	node->setArg(view);
	m_fileStack->insertWidget(pos, view);

	if (m_fileStack->count() == 1 || focus) {
		setWorkView(view, false);
	}
	view->setWorkPath(path);	// after: setCurrentWidget()

	return view;
}

bool YeFilePane::insertTab(FsWidget *view, int pos, bool focus)
{
	if (view == NULL) return false;

	TreeNode *node = view->tab();
	if (!m_tabModel->insertNode(node, m_tabModel->rootNode(), pos)) return false;

	view->setDockPane(this);
	m_fileStack->insertWidget(pos, view);
	if (focus) {
		setWorkView(view);
	}

	return true;
}

bool YeFilePane::takeTab(FsWidget *view)
{
	if (view == NULL) return false;

	int cnt = m_fileStack->count();
	bool isCurrent = (m_current == view);
	FsWidget *next = NULL;

	if (isCurrent) {
		int pos = m_fileStack->currentIndex();
		int last = cnt - 1;
		if (pos < last) pos ++; else pos --;
		if (pos >= 0) next = qobject_cast<FsWidget *>(m_fileStack->widget(pos));
	}

	if (!m_tabModel->removeNode(view->tab())) {
		return false;
	}

	if (next != NULL) {
		setWorkView(next);
	} else {
		if (cnt < 2 && !addTab(true) && isCurrent) m_current = NULL;
	}

	m_fileStack->removeWidget(view);

	return true;
}

void YeFilePane::deleteTab(FsWidget *view)
{
	if (view == NULL) return;

	int cnt = m_fileStack->count();
	bool isCurrent = (m_current == view);
	FsWidget *next = NULL;

	if (isCurrent) {
		int pos = m_fileStack->currentIndex();
		int last = cnt - 1;
		if (pos < last) pos ++; else pos --;
		if (pos >= 0) next = qobject_cast<FsWidget *>(m_fileStack->widget(pos));
	}

	if (next != NULL) {
		setWorkView(next);
	} else {
		if (cnt < 2 && !addTab(true) && isCurrent) m_current = NULL;
	}

	m_tabModel->deleteTab(view->tab());
	m_fileStack->removeWidget(view);
	delete view;
}
//==============================================================================================================================

void YeFilePane::onSysClicked(const QString &path)
{
	if (m_current == NULL || path.isEmpty()) return;

	m_current->setWorkPath(path);
}

void YeFilePane::onBmkClicked(TreeNode *node)
{
	if (m_current == NULL || node == NULL || node->isSeparator()) return;

//	addTab(node->path(), true);
	m_current->setWorkPath(node->path());
}

void YeFilePane::onMntClicked(TreeNode *node)
{
	if (m_current == NULL || node == NULL || node->isSeparator()) return;

	m_current->setWorkPath(node->path());
}

void YeFilePane::onCmdClicked(TreeNode *node)
{
	Q_UNUSED(node);
}

void YeFilePane::onTabClicked(TreeNode *node)
{
	if (node == NULL) return;

	FsWidget *view = static_cast<FsWidget *>(node->arg());
	setWorkView(view);
}
//==============================================================================================================================

void YeFilePane::onViewDetails()
{
	if (m_current != NULL) m_current->setViewMode(currentViewMode());
}

void YeFilePane::onHidden()
{
	emit toggleHidden(isHiddenVisible());
}

void YeFilePane::onRefresh()
{
	if (m_current != NULL) m_current->refresh();
}

void YeFilePane::onGoUp()
{
	if (m_current != NULL) m_current->goUp();
}

void YeFilePane::on2pane()
{
	m_win->on2pane(m_bt2pane->isChecked());
}

void YeFilePane::openTerminal()
{
	if (m_current == NULL) return;

	QString cmd = R::data().terminal;
	QStringList args = cmd.split(' ');
	if (args.size() < 1) return;

	cmd = args.takeAt(0);
	QProcess::startDetached(cmd, args, m_current->workPath());
}

void YeFilePane::onAddTab()
{
	if (m_current == NULL) {
		addTab();
	} else {
		addTab(m_current->workPath(), true);
	}
}

void YeFilePane::onDeleteTab()
{
	deleteTab(m_current);
}
//==============================================================================================================================

namespace SideStackMode {
	enum { Bmk, Mnt, Sys, Cmd };
}

void YeFilePane::setSideStackMode(int mode)
{
	if (mode != SideStackMode::Bmk && m_btBookmarkList->isChecked())   m_btBookmarkList->setChecked(false);
	if (mode != SideStackMode::Mnt && m_btMountedDevices->isChecked()) m_btMountedDevices->setChecked(false);
	if (mode != SideStackMode::Sys && m_btFileTree->isChecked())       m_btFileTree->setChecked(false);

	if (mode == SideStackMode::Bmk && !m_btBookmarkList->isChecked())   m_btBookmarkList->setChecked(true);
	if (mode == SideStackMode::Mnt && !m_btMountedDevices->isChecked()) m_btMountedDevices->setChecked(true);
	if (mode == SideStackMode::Sys && !m_btFileTree->isChecked())       m_btFileTree->setChecked(true);
}

void YeFilePane::onBookmarkList()
{
	m_sideStack->setCurrentWidget(m_bmkView);
	setSideStackMode(SideStackMode::Bmk);
}

void YeFilePane::onMountedDevices()
{
	m_sideStack->setCurrentWidget(m_mntView);
	setSideStackMode(SideStackMode::Mnt);
}

void YeFilePane::onCommandList()
{
//	m_sideStack->setCurrentWidget(m_cmdView);
//	setSideStackMode(SideStackMode::Cmd);
}

void YeFilePane::onFileTree()
{
	m_sysView->updateWorkPath();
	m_sideStack->setCurrentWidget(m_sysView);
	setSideStackMode(SideStackMode::Sys);
}
//==============================================================================================================================

void YeFilePane::onSettings()           { m_app->showSettingsDlg(); }
void YeFilePane::onStyleSettings()      { m_app->showStyleSettings(); }
void YeFilePane::onMimeSettings()       { m_app->showMimeDlg(); }
void YeFilePane::onCustomOpenSettings() { m_app->showFsActionsDlg(); }

void YeFilePane::onTogglePathEdit()
{
	m_pathStack->setCurrentIndex(m_btTogglePathEdit->isChecked() ? 1 : 0);
}

void YeFilePane::onPathChanged()
{
	QString path = m_pathEdit->text().trimmed();
	setWorkPath(path);
}

void YeFilePane::onPathCleared()
{
	m_pathEdit->clear();
	m_pathEdit->setFocus();
}
//==============================================================================================================================

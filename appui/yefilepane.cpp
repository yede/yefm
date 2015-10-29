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

#include "yeapplication.h"
#include "yemainwindow.h"
#include "yefilepane.h"
#include "yefilepanedelegate.h"
#include "yefswidget.h"

#include "yesidesysview.h"
#include "yeuserview.h"
#include "yeuserbmkmodel.h"
#include "yesidecmdmodel.h"
#include "yeusertabmodel.h"
#include "yehistorymodel.h"

#include "yeudev.h"
#include "yeudevview.h"

#include "yesplitter.h"
#include "yepathwidget.h"
#include "yetoolbar.h"
#include "yetoolitem.h"
#include "yelineedit.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
#include "yesessiondata.h"
//==============================================================================================================================

FilePane::FilePane(QWidget *statusbar, int index, QWidget *parent)
	: QWidget(parent)
	, m_app(Application::uiapp())
	, m_cfg(AppCfg::instance())
	, m_win(MainWindow::win())
	, m_index(index)
	, m_has2paneButton(MainWindow::win()->pane0() == NULL)
	, m_rightSide(AppCfg::instance()->rightSide)
	, m_current(NULL)
{
	m_delegate = new FilePaneDelegate(this);
	m_delegate->m_index = &FilePane::index;
	m_delegate->m_currentViewMode = &FilePane::currentViewMode;
	m_delegate->m_isHiddenVisible = &FilePane::isHiddenVisible;
	m_delegate->m_showPath = &FilePane::showPath;
	m_delegate->m_updateTab = &FilePane::updateTab;

	SessionData *d = SessionData::instance();
	int sideWidth  = d->side0_width;
	int tabsHeight = index > 0 ? d->tabs1_height : d->tabs0_height;
	if (sideWidth < 30) sideWidth = 30;
	if (tabsHeight < 30) tabsHeight = 30;

	QWidget *tabList = setupTabsToolBar();
	QWidget *bmkList = setupSideToolBar();

	m_spside = new Splitter;
	m_spside->setDirection(SplitterDirection::Bottom, tabsHeight);
	m_spside->setClient(tabList, bmkList);
	//--------------------------------------------------------------------------------------------------------------------------

	m_fileStack = new QStackedWidget;
	m_fileStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_spmain = new Splitter;
	m_spmain->setDirection(m_rightSide ? SplitterDirection::Right : SplitterDirection::Left, sideWidth);
	m_spmain->setClient(m_spside, m_fileStack);
	//--------------------------------------------------------------------------------------------------------------------------

	QWidget *toolBar = setupPathToolBar();

	QVBoxLayout *box = new QVBoxLayout(this);
	box->setSpacing(0);
	box->setContentsMargins(0, 0, 0, 0);
	box->addWidget(toolBar);
	box->addWidget(m_spmain);

	if (statusbar) box->addWidget(statusbar);
	//--------------------------------------------------------------------------------------------------------------------------

	updateIcons();
	connect(m_app, SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));
	//--------------------------------------------------------------------------------------------------------------------------

	onBookmarkList();
}

FilePane::~FilePane()
{
	delete m_delegate;
}
//==============================================================================================================================

static ToolBar *createSideToolBar(bool tail, bool rightSide, int iconSize)
{
	ToolBar *toolBar = new ToolBar(Qt::Vertical);

	int w = iconSize;
	int l = rightSide ? 0 : 2;
	int r = rightSide ? 2 : 0;

	toolBar->setIconSize(w, w);
	toolBar->setBasePads(l, 0, r, 1);
	toolBar->setItemPads(3, 3, 3, 3);
	toolBar->setItemSpacing(1);

//	toolBar->setCheckMode(ToolBar::cmIndicator);	// after: setIconSize()
//	toolBar->setIndicatorEdge(ToolBar::ieRight);
//	toolBar->setIndicatorSide(3);

	toolBar->setupLayout();
	if (tail) toolBar->setupTailLayout();

	return toolBar;
}

QWidget *FilePane::setupSideToolBar()
{
	m_sideToolBar = createSideToolBar(true, m_rightSide, m_cfg->iconSize);

	m_btUp      = m_sideToolBar->addToolIcon(tr("Go Up"), this, SLOT(onGoUp()));
	m_btHistory = m_sideToolBar->addToolIcon(tr("History"), this, SLOT(onHistory()));

	m_btBookmarkList = m_sideToolBar->addToolIcon(tr("Bookmark List"), this, SLOT(onBookmarkList()));
	m_btDevList      = m_sideToolBar->addToolIcon(tr("Mountable Devices"), this, SLOT(onDevList()));
	m_btFileTree     = m_sideToolBar->addToolIcon(tr("File System"), this, SLOT(onFileTree()));
//	m_sideToolBar->addSeparator(4);

	m_btViewDetails  = m_sideToolBar->addToolIcon(tr("Toggle list/details mode"), this, SLOT(onViewDetails()));

	m_btHidden  = m_sideToolBar->addToolIcon(tr("Toggle Hidden Files"), this, SLOT(onHidden()));
	m_btRefresh = m_sideToolBar->addToolIcon(tr("Refresh"), this, SLOT(onRefresh()));

	m_btTerm    = m_sideToolBar->addTailIcon(tr("Open Terminal..."), this, SLOT(openTerminal()));

	m_btBookmarkList->setAutoCheck(true);
	m_btDevList->setAutoCheck(true);
	m_btFileTree->setAutoCheck(true);
	m_btViewDetails->setAutoCheck(true);
	m_btHidden->setAutoCheck(true);
	m_btHistory->setAutoCheck(true);
	//--------------------------------------------------------------------------------------------------------------------------

	m_sysView = new SideSysView(NULL);
	m_devView = new UDevView(UDev::instance());
	m_bmkView = new UserView(m_index, UserViewType::Bmks);
//	m_cmdView = new UserView(m_index, UserViewType::Cmds);
	m_hisView = new UserView(m_index, UserViewType::Hiss);

	m_bmkView->setViewModel(UserBmkModel::instance());
	m_hisView->setViewModel(HistoryModel::instance());

	connect(m_sysView, SIGNAL(itemClicked(QString)),   this, SLOT(onSysClicked(QString)));
	connect(m_devView, SIGNAL(deviceClicked(QString)), this, SLOT(onDevClicked(QString)));
	connect(m_bmkView, SIGNAL(itemClicked(TreeNode*)), this, SLOT(onBmkClicked(TreeNode*)));
	connect(m_hisView, SIGNAL(itemClicked(TreeNode*)), this, SLOT(onHisClicked(TreeNode*)));
	//--------------------------------------------------------------------------------------------------------------------------

	m_sideStack = new QStackedLayout;
	m_sideStack->setContentsMargins(0, 0, 0, 0);
	m_sideStack->setSpacing(0);

	m_sideStack->addWidget(m_bmkView);
	m_sideStack->addWidget(m_sysView);
	m_sideStack->addWidget(m_devView);
//	m_sideStack->addWidget(m_cmdView);
	m_sideStack->addWidget(m_hisView);
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

QWidget *FilePane::setupTabsToolBar()
{
	m_tabsToolBar = createSideToolBar(m_has2paneButton, m_rightSide, m_cfg->iconSize);

	m_btAddTab = m_tabsToolBar->addToolIcon(tr("Add tab"), this, SLOT(onAddTab()));
	m_btDeleteTab = m_tabsToolBar->addToolIcon(tr("Remove tab"), this, SLOT(onDeleteTab()));

	if (m_has2paneButton) {
		m_bt2pane   = m_tabsToolBar->addTailIcon(tr("Toggle 2 panes"), this, SLOT(on2pane()));
		m_bt2pane->setAutoCheck(true);
	}
	//--------------------------------------------------------------------------------------------------------------------------

	m_tabModel = new UserTabModel(this, this);
	m_tabList = new UserView(m_index, UserViewType::Tabs);
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

QWidget *FilePane::setupPathToolBar()
{
	m_pathToolBar = new ToolBar;

	int w = m_cfg->iconSize;
	int top = m_has2paneButton ? 1 : 0;

	m_pathToolBar->setIconSize(w, w);
	m_pathToolBar->setBasePads(0, top, 0, 1);
	m_pathToolBar->setItemPads(4, 4, 4, 4);
	m_pathToolBar->setItemSpacing(4);
	m_pathToolBar->setHasSpacer(false);

	m_pathToolBar->setupLayout();
	m_pathToolBar->setupTailLayout();
	//--------------------------------------------------------------------------------------------------------------------------

	m_pathWidget = new PathWidget(m_index);
	m_pathEdit   = new LineEdit;
	m_pathStack  = new QStackedWidget;

	m_pathStack->addWidget(m_pathWidget);
	m_pathStack->addWidget(m_pathEdit);

	m_btPathClear = m_pathEdit->addButton(LineEdit::RightSide, tr("Clear"), this, SLOT(onPathCleared()));

	connect(m_pathEdit, SIGNAL(editingFinished()), this, SLOT(onPathChanged()));
	connect(m_pathWidget, SIGNAL(setWorkPath(QString)), this, SLOT(setWorkPath(QString)));
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
	m_menu->addSeparator();
	m_actShowPastePad  = m_menu->addAction(tr("Show paste manager..."), this, SLOT(onShowPastePad()));
	//--------------------------------------------------------------------------------------------------------------------------

	m_btMenu = m_pathToolBar->addTailIcon(tr("Menu"));
	m_btMenu->setMenu(m_menu);
	//--------------------------------------------------------------------------------------------------------------------------

	return m_pathToolBar;
}

void FilePane::updateIcons()
{
	m_btBookmarkList->setIcon(IconLoader::icon("bookmark-book"));
	m_btDevList->setIcon(IconLoader::icon("usb"));
	m_btFileTree->setIcon(IconLoader::icon("view-list-tree"));
	m_btViewDetails->setIcon(IconLoader::icon("view-list-text"));	// view-list-text  view-list-details  view-list-icons
	m_btHistory->setIcon(IconLoader::icon("history-time"));

	m_btHidden->setIcon(IconLoader::icon("eye"));
	m_btRefresh->setIcon(IconLoader::icon("sync"));
	m_btUp->setIcon(IconLoader::icon("point-up"));
	m_btTerm->setIcon(IconLoader::icon("utilities-terminal"));

	m_btAddTab->setIcon(IconLoader::icon("list-add"));
	m_btDeleteTab->setIcon(IconLoader::icon("list-remove"));
	if (m_has2paneButton) m_bt2pane->setIcon(IconLoader::icon("view-split-top-bottom"));

	m_btTogglePathEdit->setIcon(IconLoader::icon("gtk-edit"));
	m_btPathClear->setPixmap(IconLoader::pixmap("clear"));
	m_btMenu->setIcon(IconLoader::icon("menu"));

	const QIcon &iconSettings = IconLoader::icon("preferences-desktop");
	m_actSettings->setIcon(iconSettings);
	m_actStyleSettings->setIcon(iconSettings);
	m_actMimeSettings->setIcon(iconSettings);
	m_actFsActions->setIcon(iconSettings);
	m_actShowPastePad->setIcon(IconLoader::icon("edit-paste"));
	//--------------------------------------------------------------------------------------------------------------------------

	int w = m_cfg->iconSize;
	m_tabList->setIconSize(QSize(w, w));
}

void FilePane::onIconThemeChanged()
{
	int w = m_cfg->iconSize;
	m_tabsToolBar->updateIconSize(w, w);
	m_sideToolBar->updateIconSize(w, w);
	m_pathToolBar->updateIconSize(w, w);
	//--------------------------------------------------------------------------------------------------------------------------

	updateIcons();
	//--------------------------------------------------------------------------------------------------------------------------

	m_btBookmarkList->update();
	m_btDevList->update();
	m_btFileTree->update();
	m_btHistory->update();

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

void FilePane::loadSessionTabs(const QStringList &startPaths)
{
	FsWidget *view = NULL;
	QStringList tabs;

	QSettings s(App::getSessionTabsFile(), QSettings::IniFormat);
	s.beginGroup(QString("pane-%1").arg(m_index));
	tabs = s.value("tabs").toStringList();
	s.endGroup();

	HistoryModel::instance()->startTemporarilySkip();

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

void FilePane::saveSessionTabs()
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

void FilePane::updateSessionData()
{
	SessionData *d = SessionData::instance();
	if (m_index > 0) {
		d->side1_width  = m_spmain->sideSize();
		d->tabs1_height = m_spside->sideSize();
	} else {
		d->side0_width  = m_spmain->sideSize();
		d->tabs0_height = m_spside->sideSize();
	}
}
//==============================================================================================================================

int  FilePane::currentViewMode() const { return m_btViewDetails->isChecked() ? FsWidgetMode::Tree : FsWidgetMode::List; }
bool FilePane::isHiddenVisible() const { return m_btHidden->isChecked(); }

void FilePane::showPath(FsWidget *widget, const QString &path)
{
	if (m_current == widget) {
		if (m_current != NULL) showCurrentPath(path);
	}
}

void FilePane::showCurrentPath(const QString &path)
{
	m_pathWidget->showPath(path);
	if (m_pathEdit->text() != path) m_pathEdit->setText(path);
	HistoryModel::instance()->addPath(path);
}
//==============================================================================================================================

void FilePane::setupWidget(FsWidget *widget)
{
	if (widget->pane() != NULL) {
		disconnect(widget->pane()->m_pane, SIGNAL(toggleHidden(bool)), widget, SLOT(updateHiddenState(bool)));
	}
	widget->setPane(m_delegate);
	connect(this, SIGNAL(toggleHidden(bool)), widget, SLOT(updateHiddenState(bool)));
}

void FilePane::setWorkView(FsWidget *view, bool showPath)
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
		view->validatePath();
		view->updateColSizes();
	}

	m_current = view;
}

void FilePane::setWorkPath(const QString &path)
{
	if (m_current != NULL) m_current->setWorkPath(path);
}

void FilePane::updateTab(TreeNode *node, const QString &name, const QString &path)
{
	m_tabList->updateNode(node, name, path);
}

bool FilePane::addTab(bool focus)
{
	return addTab(QDir::homePath(), focus);
}

bool FilePane::addTab(const QString &path, bool focus)
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

FsWidget *FilePane::insertTab(const QString &path, int pos, bool focus)
{
	if (!QDir(path).exists()) return NULL;

	TreeNode *node = m_tabModel->insertTab(pos, path);
	FsWidget *view = new FsWidget(node);
	node->setArg(view);

	setupWidget(view);
	m_fileStack->insertWidget(pos, view);

	if (m_fileStack->count() == 1 || focus) {
		setWorkView(view, false);
	}
	view->setWorkPath(path);	// after: setCurrentWidget()

	return view;
}

bool FilePane::insertTab(FsWidget *view, int pos, bool focus)
{
	if (view == NULL) return false;

	TreeNode *node = view->tab();
	if (!m_tabModel->insertNode(node, m_tabModel->rootNode(), pos)) return false;

	setupWidget(view);
	m_fileStack->insertWidget(pos, view);

	if (focus) {
		setWorkView(view);
	}

	return true;
}

bool FilePane::takeTab(FsWidget *view)
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

void FilePane::deleteTab(FsWidget *view)
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

void FilePane::onSysClicked(const QString &path)
{
	if (m_current == NULL || path.isEmpty()) return;
	m_current->setWorkPath(path);
}

void FilePane::onBmkClicked(TreeNode *node)
{
	if (m_current == NULL || node == NULL || node->isSeparator()) return;
	m_current->setWorkPath(node->path());
}

void FilePane::onDevClicked(const QString &path)
{
	if (m_current != NULL) m_current->setWorkPath(path);
}

void FilePane::onCmdClicked(TreeNode *node)
{
	Q_UNUSED(node);
}

void FilePane::onTabClicked(TreeNode *node)
{
	if (node == NULL) return;
	FsWidget *view = static_cast<FsWidget *>(node->arg());
	setWorkView(view);
}

void FilePane::onHisClicked(TreeNode *node)
{
	if (m_current == NULL || node == NULL || node->isSeparator()) return;
	HistoryModel::instance()->startTemporarilySkip();
	m_current->setWorkPath(node->path());
}
//==============================================================================================================================

void FilePane::onViewDetails()
{
	if (m_current != NULL) m_current->setViewMode(currentViewMode());
}

void FilePane::onHidden()
{
	emit toggleHidden(isHiddenVisible());
}

void FilePane::onRefresh()
{
	if (m_current != NULL) m_current->refresh();
}

void FilePane::onGoUp()
{
	if (m_current != NULL) m_current->handleGoParent();
}

void FilePane::on2pane()
{
	m_win->on2pane(m_bt2pane->isChecked());
}

void FilePane::openTerminal()
{
	if (m_current == NULL) return;

	QString cmd = m_cfg->terminal;
	QStringList args = cmd.split(' ');
	if (args.size() < 1) return;

	cmd = args.takeAt(0);
	QProcess::startDetached(cmd, args, m_current->workPath());
}

void FilePane::onAddTab()
{
	if (m_current == NULL) {
		addTab();
	} else {
		addTab(m_current->workPath(), true);
	}
}

void FilePane::onDeleteTab()
{
	deleteTab(m_current);
}
//==============================================================================================================================

namespace SideStackMode {
	enum { Bmk, Mnt, Sys, Cmd, His };
}

void FilePane::setSideStackMode(int mode)
{
	if (mode != SideStackMode::Bmk && m_btBookmarkList->isChecked())  m_btBookmarkList->setChecked(false);
	if (mode != SideStackMode::Mnt && m_btDevList->isChecked())       m_btDevList->setChecked(false);
	if (mode != SideStackMode::Sys && m_btFileTree->isChecked())      m_btFileTree->setChecked(false);
	if (mode != SideStackMode::His && m_btHistory->isChecked())       m_btHistory->setChecked(false);

	if (mode == SideStackMode::Bmk && !m_btBookmarkList->isChecked()) m_btBookmarkList->setChecked(true);
	if (mode == SideStackMode::Mnt && !m_btDevList->isChecked())      m_btDevList->setChecked(true);
	if (mode == SideStackMode::Sys && !m_btFileTree->isChecked())     m_btFileTree->setChecked(true);
	if (mode == SideStackMode::His && !m_btHistory->isChecked())      m_btHistory->setChecked(true);
}

void FilePane::onBookmarkList()
{
	m_sideStack->setCurrentWidget(m_bmkView);
	setSideStackMode(SideStackMode::Bmk);
}

void FilePane::onDevList()
{
	m_sideStack->setCurrentWidget(m_devView);
	setSideStackMode(SideStackMode::Mnt);
}

void FilePane::onCommandList()
{
//	m_sideStack->setCurrentWidget(m_cmdView);
//	setSideStackMode(SideStackMode::Cmd);
}

void FilePane::onFileTree()
{
	m_sysView->updateWorkPath();
	m_sideStack->setCurrentWidget(m_sysView);
	setSideStackMode(SideStackMode::Sys);
}

void FilePane::onHistory()
{
	m_sideStack->setCurrentWidget(m_hisView);
	setSideStackMode(SideStackMode::His);
}
//==============================================================================================================================

void FilePane::onSettings()           { m_app->showSettingsDlg(); }
void FilePane::onStyleSettings()      { m_app->showStyleSettings(); }
void FilePane::onMimeSettings()       { m_app->showMimeSettingsDlg(); }
void FilePane::onCustomOpenSettings() { m_app->showCustomActionsDlg(); }
void FilePane::onShowPastePad()       { m_win->showPastePad(); }

void FilePane::onTogglePathEdit()
{
	m_pathStack->setCurrentIndex(m_btTogglePathEdit->isChecked() ? 1 : 0);
}

void FilePane::onPathChanged()
{
	QString path = m_pathEdit->text().trimmed();
	setWorkPath(path);
}

void FilePane::onPathCleared()
{
	m_pathEdit->clear();
	m_pathEdit->setFocus();
}
//==============================================================================================================================

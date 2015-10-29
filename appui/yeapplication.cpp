#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QKeySequence>
#include <QDebug>

#include "yemime.h"
#include "yeudev.h"
#include "yeucs2table.h"

#include "yedbus.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
#include "yeiconcache.h"
#include "yestyle.h"
#include "yeshortcuts.h"
#include "yesessiondata.h"
#include "yeappdelegate.h"

#include "yesettingsdlg.h"
#include "yepropertiesdlg.h"
#include "yestylesettings.h"

#include "yeactionmgr.h"
#include "yecustomactionsdlg.h"
#include "yedefaultactions.h"

#include "yepastemgr.h"

#include "yeapplication.h"
#include "yemainwindow.h"

#include "yeuserbmkmodel.h"
#include "yehistorymodel.h"
#include "yesidecmdmodel.h"
#include "yemimedlg.h"

#include "yefsnode.h"
#include "yefsmodel.h"

#include "yefilepane.h"
#include "yefshandler.h"
//==============================================================================================================================

Mime         *Mime::m_instance        = NULL;	// yemime.h			(global)
UDev         *UDev::m_instance        = NULL;	// yeudev.h			(udev)
Ucs2Table    *Ucs2Table::m_instance   = NULL;	// yeucs2table.h	(dirmodel)
ActionMgr    *ActionMgr::m_instance   = NULL;	// yeactionmgr.h	(actionmgr)
PasteMgr     *PasteMgr::m_instance    = NULL;	// yepastemgr.h		(pastemgr)

App          *App::m_app              = NULL;	// yeapp.h			(app)
QMainWindow  *App::m_win              = NULL;	// yeapp.h
AppCfg       *AppCfg::m_instance      = NULL;	// yeappcfg.h
IconLoader   *IconLoader::m_instance  = NULL;	// yeiconloader.h
IconCache    *IconCache::m_instance   = NULL;	// yeiconcache.h
Style        *Style::m_instance       = NULL;	// yestyle.h
ShortcutMgr  *ShortcutMgr::m_instance = NULL;	// yeshortcuts.h
SessionData  *SessionData::m_instance = NULL;	// yesessiondata.h
AppDelegate  *AppDelegate::m_instance = NULL;	// yeappdelegate.h

UserBmkModel *UserBmkModel::m_instance= NULL;	// yeuserbmkmodel.h	(appui)
HistoryModel *HistoryModel::m_instance= NULL;	// yehistorymodel.h	(appui)

MainWindow   *MainWindow::m_win       = NULL;	// yemainwindow.h	(appui)
QStringList   FsModel::m_cutItems;				// yefsmodel.h
const QString FsNode::sep = QString("/");		// yefsnode.h

//==============================================================================================================================

Application::Application(int &argc, char **argv)
	: App(argc, argv)
	, m_mimeDlg(NULL)
	, m_settingsDlg(NULL)
	, m_propertiesDlg(NULL)
	, m_customActionsDlg(NULL)
	, m_styleSettingsDlg(NULL)
{
	m_app = this;
	m_bus = new DBus(this);
}

Application::~Application()
{
	delete m_bus;
	m_app = NULL;
}
//==============================================================================================================================

bool Application::hasPriorInstance() const
{
	return m_bus->hasPriorInstance();
}

void Application::callPriorInstance(int &argc, char **argv)
{
	m_bus->callPriorInstance(argc, argv);
}
//==============================================================================================================================

int Application::availableDesktopHeight()
{
	QDesktopWidget *d = this->desktop();
	return d->availableGeometry().height();
}

void Application::loadTranslator()
{
	QString lang = App::getUserLanguage();
	QString path = QString("%1/translations/%2_%3.qm")
				   .arg(App::getDataDir())
				   .arg(App::getAppName())
				   .arg(lang);

	if (!QFile(path).exists()) {
		qDebug() << "translator not exists:" << path;
		return;
	}

	if (m_translator.load(path)) {
		installTranslator(&m_translator);
	} else {
		qDebug() << "Failed to load translator:" << path;
	}
}

void Application::loadQtTranslator()
{
	QString qt = QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + QLocale::system().name() + ".qm";
	if (m_qtTranslator.load(qt)) {
		installTranslator(&m_qtTranslator);
	} else {
		qDebug() << "Failed to load translator:" << qt;
	}
}

void Application::deleteResources()
{
	AppCfg::deleteInstance();
	Style::deleteInstance();
	Mime::deleteInstance();
	UDev::deleteInstance();
	Ucs2Table::deleteInstance();
	IconLoader::deleteInstance();
	IconCache::deleteInstance();
	UserBmkModel::deleteInstance();
	HistoryModel::deleteInstance();
	ActionMgr::deleteInstance();
	ShortcutMgr::deleteInstance();
	SessionData::deleteInstance();
	AppDelegate::deleteInstance();
}

void Application::createResources()
{
	AppCfg::createInstance();
	Style::createInstance();
	Mime::createInstance();
	UDev::createInstance();
	Ucs2Table::createInstance();
	IconLoader::createInstance();		// after: AppCfg::createInstance()
	IconCache::createInstance();		// after: IconLoader::createInstance()
	UserBmkModel::createInstance();		// after: Style::createInstance()
	HistoryModel::createInstance();		// after: Style::createInstance()
	ActionMgr::createInstance();
	ShortcutMgr::createInstance();
	SessionData::createInstance();
	AppDelegate::createInstance();

	AppDelegate::instance()->init(this, &Application::showMimeDlg);
	DefaultActions::setupShortcuts();	// after: ShortcutMgr::createInstance()
	UserBmkModel::setupShortcuts();		// after: ShortcutMgr::createInstance()
	HistoryModel::setupShortcuts();		// after: ShortcutMgr::createInstance()

	App::initAppDirs();

	AppCfg::instance()->loadSettings();
	Style::instance()->loadSettings();
	Mime::instance()->loadDefaults();

	changeIconTheme();

	UserBmkModel::instance()->lateStart();
	UserBmkModel::instance()->loadBookmarkList();

	HistoryModel::instance()->lateStart();
	HistoryModel::instance()->load();

	loadTranslator();
	loadQtTranslator();
}

void Application::createMainWindow()
{
	MainWindow *win = new MainWindow(NULL);
	m_win = win;

	win->setupWindow();
	setWindowIcon(IconLoader::instance()->iconLogo);

	connect(m_bus, SIGNAL(openFolder(QString)), win, SLOT(openFolder(QString)));
	connect(this, SIGNAL(properties(QStringList)), this, SLOT(showPropertiesDlg(QStringList)));
}

bool Application::startSession(int &argc, char **argv)
{
	createResources();
	createMainWindow();

//	m_cache->loadIconCache();
	MainWindow *win = (MainWindow *) m_win;
	win->startSession(argc, argv);
	UDev::instance()->start();

	return true;
}

void Application::stopSession()
{
	UDev::instance()->stop();
//	m_cache->saveIconCache();

	deleteDialogs();
	deleteResources();
}
//==============================================================================================================================

void Application::changeIconTheme()
{
	IconLoader::instance()->reset();
	if (IconCache::instance() != NULL) {
		IconCache::instance()->updateIconTheme();
		emit iconThemeChanged();
	}
}

void Application::changeSettings()
{
	emit settingsChanged();
}
//==============================================================================================================================

void Application::deleteDialogs()
{
	delete m_styleSettingsDlg;
	delete m_customActionsDlg;
	delete m_propertiesDlg;
	delete m_settingsDlg;
	delete m_mimeDlg;

	delete m_win; m_win = NULL;
}

void Application::showMenu(QMenu *menu, QWidget *button)
{
	if (!menu) return;

	menu->adjustSize();	// for first showing ONLY

	QPoint p(0, button->height() - 1);
	p = button->mapToGlobal(p);
	if (p.x() + menu->width() > QApplication::desktop()->width())
		p.setX(QApplication::desktop()->width() - menu->width());
	if (p.y() + menu->height() > QApplication::desktop()->height())
		p.setY(p.y() - menu->height() - button->height());

	menu->popup(p);
}

void Application::createMimeDlg()
{
	if (m_mimeDlg != NULL) return;

	m_mimeDlg = new MimeDlg();
	connect(this, SIGNAL(iconThemeChanged()), m_mimeDlg, SLOT(updateIconTheme()));
}

bool Application::showMimeDlg(QString &resultApp, const QString &mimeType)
{
	createMimeDlg();
	return m_mimeDlg->showDialog(resultApp, mimeType);
}

void Application::showMimeSettingsDlg()
{
	createMimeDlg();
	m_mimeDlg->showDialog();
}

void Application::showSettingsDlg()
{
	if (m_settingsDlg == NULL) {
		m_settingsDlg = new SettingsDlg(m_win);
		connect(this, SIGNAL(iconThemeChanged()), m_settingsDlg, SLOT(updateIconTheme()));
		connect(m_settingsDlg, SIGNAL(changeIconTheme()), this, SLOT(changeIconTheme()));
		connect(m_settingsDlg, SIGNAL(changeSettings()), this, SLOT(changeSettings()));
	}
	m_settingsDlg->showDialog();
}

void Application::showPropertiesDlg(const QStringList &files)
{
	PropertiesDlg::showProperties(files);
}

void Application::showCustomActionsDlg()
{
	if (m_customActionsDlg == NULL) m_customActionsDlg = new CustomActionsDlg(m_win);
	m_customActionsDlg->showDialog();
}

void Application::showStyleSettings()
{
	if (m_styleSettingsDlg == NULL) m_styleSettingsDlg = new StyleSettings(m_win);
	m_styleSettingsDlg->showDialog();
}
//==============================================================================================================================

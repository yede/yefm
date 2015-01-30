#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QKeySequence>
#include <QDebug>

#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
#include "yeapplication.h"
#include "yemainwindow.h"

#include "yedbus.h"
#include "yefscache.h"
#include "yeuserbmkmodel.h"
#include "yesidemntmodel.h"
#include "yesidecmdmodel.h"
#include "yemimedlg.h"
#include "yemime.h"

#include "yefilepane.h"
#include "yefsactions.h"
#include "yefshandler.h"

#include "yepastemgr.h"
#include "yesettingsdlg.h"
#include "yepropertiesdlg.h"
#include "yefsactionsdlg.h"
#include "yestylesettings.h"
//==============================================================================================================================

YeApplication::YeApplication(int &argc, char **argv)
	: QApplication(argc, argv)
	, m_bus(NULL)
	, m_fmStyle(NULL)
	, m_data(NULL)
	, m_resources(NULL)
	, m_win(NULL)
	, m_cache(NULL)
	, m_bmkModel(NULL)
	, m_mntModel(NULL)
	, m_cmdModel(NULL)
	, m_mimeDlg(NULL)
	, m_mime(NULL)
	, m_settingsDlg(NULL)
	, m_propertiesDlg(NULL)
	, m_fsActionsDlg(NULL)
	, m_fsActions(NULL)
	, m_styleSettings(NULL)
{
	m_bus = new DBus(this);
}

YeApplication::~YeApplication()
{
	delete m_fsActionsDlg;
	delete m_propertiesDlg;
	delete m_settingsDlg;
	delete m_mimeDlg;
	delete m_win;

	delete m_cache;
	delete m_bmkModel;
	delete m_mntModel;
	delete m_cmdModel;
	delete m_fsActions;
	delete m_mime;
	delete m_resources;
	delete m_data;
	delete m_fmStyle;

	delete m_bus;
}

bool YeApplication::hasPriorInstance() const                  { return m_bus->hasPriorInstance();     }
void YeApplication::callPriorInstance(int &argc, char **argv) { m_bus->callPriorInstance(argc, argv); }
//==============================================================================================================================

int YeApplication::availableDesktopHeight()
{
	QDesktopWidget *d = this->desktop();
	return d->availableGeometry().height();
}

void YeApplication::loadTranslator()
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

void YeApplication::loadQtTranslator()
{
	QString qt = QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + QLocale::system().name() + ".qm";
	if (m_qtTranslator.load(qt)) {
		installTranslator(&m_qtTranslator);
	} else {
		qDebug() << "Failed to load translator:" << qt;
	}
}

void YeApplication::createResources()
{
	m_fmStyle = new FmStyle;
	m_data = new AppData;
	m_resources = new R(this, *m_data);

	App::initAppDirs();
	m_data->loadSettings();
	StyleSettings::loadSettings();
	updateIconTheme();			// before new FsCache()

	m_mime = new Mime(this);	// before: new FsCache()
	m_cache = new FsCache(this);

	m_bmkModel = new UserBmkModel(this);
	m_bmkModel->loadBookmarkList();

	m_mntModel = new SideMntModel(this);
	m_mntModel->startReadMounts();

//	m_cmdModel = new SideCmdModel(this);
//	m_cmdModel->loadCommandList();

	m_fsActions = new FsActions(this);

	loadTranslator();
	loadQtTranslator();
	initKeyShortcuts();
}

void YeApplication::createMainWindow()
{
	m_win = new YeMainWindow(this, NULL);
	m_win->setupWindow();

	setWindowIcon(m_resources->iconLogo);
}

bool YeApplication::startSession()
{
	createResources();
	createMainWindow();

//	m_cache->loadIconCache();
	m_win->startSession();

	return true;
}

void YeApplication::stopSession()
{
//	m_cache->saveIconCache();
}

void YeApplication::updateIconTheme()
{
	R::updateIconTheme();
	if (m_cache != NULL) {
		m_cache->updateIconTheme();
		emit iconThemeChanged();
	}
}

void YeApplication::updateSettings()
{
	emit settingsChanged();
}
//==============================================================================================================================

void YeApplication::showMenu(QMenu *menu, QWidget *button)
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

bool YeApplication::showMimeDlg(QString &resultApp, const QString &mimeType)
{
	if (m_mimeDlg == NULL) {
		m_mimeDlg = new MimeDlg(this);
	}
	return m_mimeDlg->showDialog(resultApp, mimeType);
}

void YeApplication::showMimeDlg()
{
	if (m_mimeDlg == NULL) {
		m_mimeDlg = new MimeDlg(this);
	}
	m_mimeDlg->showDialog();
}

void YeApplication::showSettingsDlg()
{
	if (m_settingsDlg == NULL) m_settingsDlg = new SettingsDlg(this, m_win);
	m_settingsDlg->showDialog();
}

void YeApplication::showPropertiesDlg(const QStringList &files)
{
	PropertiesDlg *dlg = new PropertiesDlg(this);
	dlg->showDialog(files);
}

void YeApplication::showFsActionsDlg()
{
	if (m_fsActionsDlg == NULL) m_fsActionsDlg = new FsActionsDlg(this, m_win);
	m_fsActionsDlg->showDialog();
}

void YeApplication::showStyleSettings()
{
	if (m_styleSettings == NULL) m_styleSettings = new StyleSettings(this, m_win);
	m_styleSettings->showDialog();
}
//==============================================================================================================================

void YeApplication::initKeyShortcuts()
{
	m_fsShortcuts.insert("Ctrl+X", "Cut");
	m_fsShortcuts.insert("Ctrl+C", "Copy");
	m_fsShortcuts.insert("Ctrl+V", "Paste");
	m_fsShortcuts.insert("Del"   , "Delete");
	m_fsShortcuts.insert("F2"    , "Rename");
}

int YeApplication::getFileViewKeyAction(int key)
{
	QKeySequence keySeq(key);
	QString keyStr = keySeq.toString(QKeySequence::NativeText);

	if (m_fsShortcuts.contains(keyStr)) {
		QString keyVal = m_fsShortcuts.value(keyStr);
		if (keyVal == "NewFile")    return FsHandler::KeyAction::NewFile;
		if (keyVal == "NewFolder")  return FsHandler::KeyAction::NewFolder;
		if (keyVal == "Run")        return FsHandler::KeyAction::Run;
		if (keyVal == "Cut")        return FsHandler::KeyAction::Cut;
		if (keyVal == "Copy")       return FsHandler::KeyAction::Copy;
		if (keyVal == "Paste")      return FsHandler::KeyAction::Paste;
		if (keyVal == "Delete")     return FsHandler::KeyAction::Delete;
		if (keyVal == "Rename")     return FsHandler::KeyAction::Rename;
		if (keyVal == "Properties") return FsHandler::KeyAction::Properties;
	}

	return FsHandler::KeyAction::None;
}

QKeySequence YeApplication::getFileViewShortcut(const QString &name)
{
	QString key = m_fsShortcuts.key(name, QString());
	return key.isEmpty() ? QKeySequence() : QKeySequence(key);
}
//==============================================================================================================================

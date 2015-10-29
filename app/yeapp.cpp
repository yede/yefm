#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "yeapp.h"
#include "yedbus.h"
//==============================================================================================================================

#define DATA_DIR  D_RES_DIR

QString App::getAppName()           { return DBus::appName(); }

QString App::getConfigDir()         { return QDir::homePath() + "/.config/" + getAppName(); }
QString App::getCacheDir()          { return QDir::homePath() + "/.cache/"  + getAppName(); }
QString App::getDataDir()           { return QString(DATA_DIR); }

QString App::getConfigFile()        { return getConfigDir() + "/settings.ini"; }
QString App::getBookmarkFile()      { return getConfigDir() + "/bookmarks.ini"; }
QString App::getCustomActionsFile() { return getConfigDir() + "/custom-actions.ini"; }
QString App::getIconAliasesFile()   { return getConfigDir() + "/icon-aliases.ini"; }
QString App::getStyleFile()         { return getConfigDir() + "/fmstyle.ini"; }
QString App::getSessionFile()       { return getCacheDir()  + "/session.ini"; }
QString App::getSessionTabsFile()   { return getCacheDir()  + "/tabs.ini"; }
QString App::getHistoryFile()       { return getCacheDir()  + "/history.ini"; }
//==============================================================================================================================

static void initDir(const QString &path)
{
	QDir dir;
	if (!dir.exists(path)) dir.mkpath(path);
}

void App::initAppDirs()
{
	QString configDir = getConfigDir();
	QString cacheDir  = getCacheDir();

	initDir(configDir);
	initDir(cacheDir);
}

QString App::getUserLanguage()
{
	QLocale locale = QLocale::system();
	QString lang = locale.name();

	if (lang.toLower() == "c") {
		QString envLang = getenv("LANG");
		int pos = envLang.indexOf('.');
		if (pos > 0) {
			lang = (pos > 0) ? envLang.left(pos) : envLang;
		}
	}

	return lang;
}
//==============================================================================================================================

void App::message(const QString &text, const QString &title, QWidget *parent)
{
	if (parent == NULL) parent = m_win;
	QMessageBox::information(parent,
							 title.isEmpty() ? QObject::tr("Information") : title,
							 text);
}

void App::warning(const QString &text, const QString &title, QWidget *parent)
{
	if (parent == NULL) parent = m_win;
	QMessageBox::warning(parent,
						 title.isEmpty() ? QObject::tr("Warning") : title,
						 text);
}

bool App::confirm(const QString &text, const QString &title, QWidget *parent)
{
	if (parent == NULL) parent = m_win;
	int ret = QMessageBox::question(parent,
									title.isEmpty() ? QObject::tr("Question") : title,
									text,
									QMessageBox::Yes | QMessageBox::No,
									QMessageBox::No);
	return (ret == QMessageBox::Yes);
}

void App::message(const QString &text, QWidget *parent) { message(text, QString(), parent); }
void App::warning(const QString &text, QWidget *parent) { warning(text, QString(), parent); }
bool App::confirm(const QString &text, QWidget *parent) { return confirm(text, QString(), parent); }
//==============================================================================================================================

Mbtn App::msg(const QString &title, const QString &text, Mbts bts, Mbtn def)
{
	return QMessageBox::information(App::m_win, title, text, bts, def);
}

Mbtn App::msg(const QString &title, const QString &text, Mbts bts)
{
	Mbtn def = QMessageBox::NoButton;
	return QMessageBox::information(m_win, title, text, bts, def);
}

void App::msg(const QString &title, const QString &text)
{
	Mbtn btn = QMessageBox::Ok;
	QMessageBox::information(m_win, title, text, btn, btn);
}

bool App::ask(const QString &title, const QString &text)
{
	Mbts bts = QMessageBox::Yes | QMessageBox::No;
	Mbtn def = QMessageBox::Yes;
	Mbtn ret = QMessageBox::question(m_win, title, text, bts, def);
	return ret == QMessageBox::Yes;
}

Mbtn App::ask(const QString &title, const QString &text, bool &yesToAll)
{
	Mbts bts = QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll;
	Mbtn def = QMessageBox::No;
	Mbtn ret = QMessageBox::information(m_win, title, text, bts, def);
	if (ret == QMessageBox::YesToAll) yesToAll = true;
	return ret;
}

//==============================================================================================================================
// class App
//==============================================================================================================================

App::App(int &argc, char **argv)
	: QApplication(argc, argv)
{
}

App::~App()
{
}
//==============================================================================================================================

void App::showStatusMessage(const QString &msg, int paneIndex, int time)
{
	emit statusMessage(msg, paneIndex, time);
}

void App::showStatusMessage(const QStringList &msgList, int paneIndex, int time)
{
	emit statusMessage(msgList, paneIndex, time);
}

void App::showPadMessage(const QString &text, const QString &title, int time)
{
	emit padMessage(text, title, time);
}

void App::showProperties(const QStringList &files)
{
	emit properties(files);
}

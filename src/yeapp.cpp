#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "yedbus.h"
#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
#include "yeapplication.h"
#include "yemainwindow.h"
#include "yestylesettings.h"
//==============================================================================================================================

#define DATA_DIR  D_RES_DIR

QString App::getAppName()           { return DBus::appName(); }
QString App::getConfigDir()         { return QDir::homePath() + "/.config/" + getAppName(); }
QString App::getCacheDir()          { return QDir::homePath() + "/.cache/" + getAppName(); }
QString App::getDataDir()           { return QString(DATA_DIR); }
QString App::getConfigFile()        { return getConfigDir() + "/settings.ini"; }
QString App::getBookmarkFile()      { return getConfigDir() + "/bookmarks.ini"; }
QString App::getCustomActionsFile() { return getConfigDir() + "/custom-actions.ini"; }
QString App::getIconAliasesFile()   { return getConfigDir() + "/icon-aliases.ini"; }
QString App::getStyleFile()         { return getConfigDir() + "/fmstyle.ini"; }
QString App::getSessionFile()       { return getCacheDir() + "/session.ini"; }
QString App::getSessionTabsFile()   { return getCacheDir() + "/tabs.ini"; }
//==============================================================================================================================

void App::initAppDirs()
{
	QString configDir = getConfigDir();
	QString cacheDir = getCacheDir();

	QDir dir;
	if (!dir.exists(configDir)) dir.mkpath(configDir);
	if (!dir.exists(cacheDir))  dir.mkpath(cacheDir);
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
	if (parent == NULL) parent = R::win();
	QMessageBox::information(parent,
							 title.isEmpty() ? QObject::tr("Information") : title,
							 text);
}

void App::warning(const QString &text, const QString &title, QWidget *parent)
{
	if (parent == NULL) parent = R::win();
	QMessageBox::warning(parent,
						 title.isEmpty() ? QObject::tr("Warning") : title,
						 text);
}

bool App::confirm(const QString &text, const QString &title, QWidget *parent)
{
	if (parent == NULL) parent = R::win();
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
	return QMessageBox::information(R::win(), title, text, bts, def);
}

Mbtn App::msg(const QString &title, const QString &text, Mbts bts)
{
	Mbtn def = QMessageBox::NoButton;
	return QMessageBox::information(R::win(), title, text, bts, def);
}

void App::msg(const QString &title, const QString &text)
{
	Mbtn btn = QMessageBox::Ok;
	QMessageBox::information(R::win(), title, text, btn, btn);
}

bool App::ask(const QString &title, const QString &text)
{
	Mbts bts = QMessageBox::Yes | QMessageBox::No;
	Mbtn def = QMessageBox::Yes;
	Mbtn ret = QMessageBox::question(R::win(), title, text, bts, def);
	return ret == QMessageBox::Yes;
}

Mbtn App::ask(const QString &title, const QString &text, bool &yesToAll)
{
	Mbts bts = QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll;
	Mbtn def = QMessageBox::No;
	Mbtn ret = QMessageBox::information(R::win(), title, text, bts, def);
	if (ret == QMessageBox::YesToAll) yesToAll = true;
	return ret;
}
//==============================================================================================================================

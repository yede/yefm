#ifndef YEAPP_H
#define YEAPP_H

#include <QWidget>
#include <QMessageBox>
//==============================================================================================================================

typedef QMessageBox::StandardButtons Mbts;
typedef QMessageBox::StandardButton  Mbtn;

class YeApplication;

class App
{
public:
	static void initAppDirs();
	static QString getUserLanguage();

	static QString getAppName();
	static QString getConfigDir();
	static QString getCacheDir();
	static QString getDataDir();
	static QString getConfigFile();
	static QString getBookmarkFile();
	static QString getCustomActionsFile();
	static QString getIconAliasesFile();
	static QString getStyleFile();
	static QString getSessionFile();
	static QString getSessionTabsFile();

	static void message(const QString &text, const QString &title = QString(), QWidget *parent = NULL);
	static void warning(const QString &text, const QString &title = QString(), QWidget *parent = NULL);
	static bool confirm(const QString &text, const QString &title = QString(), QWidget *parent = NULL);
	static void message(const QString &text, QWidget *parent);
	static void warning(const QString &text, QWidget *parent);
	static bool confirm(const QString &text, QWidget *parent);

	static Mbtn msg(const QString &title, const QString &text, Mbts bts, Mbtn def);
	static Mbtn msg(const QString &title, const QString &text, Mbts bts);
	static void msg(const QString &title, const QString &text);
	static bool ask(const QString &title, const QString &text);
	static Mbtn ask(const QString &title, const QString &text, bool &yesToAll);

private:
};

#endif // YEAPP_H

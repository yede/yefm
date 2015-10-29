#ifndef YE_APP_H
#define YE_APP_H

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
//==============================================================================================================================

typedef QMessageBox::StandardButtons Mbts;
typedef QMessageBox::StandardButton  Mbtn;

class App : public QApplication
{
	Q_OBJECT
public:
	explicit App(int &argc, char **argv);
	virtual ~App();

	void showStatusMessage(const QString &msg, int paneIndex, int time);
	void showStatusMessage(const QStringList &msgList, int paneIndex, int time);
	void showPadMessage(const QString &text, const QString &title, int time);
	void showProperties(const QStringList &files);

protected:
	void    initAppDirs();
	QString getUserLanguage();

signals:
	void iconThemeChanged();
	void settingsChanged();
	void statusMessage(const QString &msg, int paneIndex, int time);
	void statusMessage(const QStringList &msgList, int paneIndex, int time);
	void padMessage(const QString &text, const QString &title, int time);
	void properties(const QStringList &files);

public slots:

public:
	static QString getAppName();
	static QString getConfigDir();
	static QString getCacheDir();
	static QString getDataDir();
	static QString getConfigFile();
	static QString getBookmarkFile();
	static QString getHistoryFile();
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

public:
	static App         *app() { return m_app; }
	static QMainWindow *win() { return m_win; }

protected:
	static App         *m_app;
	static QMainWindow *m_win;
};

#endif // YE_APP_H

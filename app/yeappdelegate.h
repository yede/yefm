#ifndef YE_APP_DELEGATES_H
#define YE_APP_DELEGATES_H

#include <QFileInfo>
#include <QModelIndex>
#include <QContextMenuEvent>
//==============================================================================================================================

class Application;

typedef bool (Application::*_showMimeDlg) (QString &resultApp, const QString &mimeType);

//==============================================================================================================================

class AppDelegate
{
public:
	bool showMimeDlg(QString &resultApp, const QString &mimeType);

	void init(Application *app, _showMimeDlg func)
	{
		m_app = app;
		m_showMimeDlg = func;
	}

	static AppDelegate *instance() { return m_instance; }
	static void createInstance()   { if (!m_instance) m_instance = new AppDelegate; }
	static void deleteInstance()   { delete m_instance; m_instance = NULL; }

private:
	friend class FsHandler;

	Application *m_app;
	_showMimeDlg m_showMimeDlg;

	static AppDelegate *m_instance;
};
//==============================================================================================================================

#endif // YE_APP_DELEGATES_H

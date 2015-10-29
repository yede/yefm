#ifndef YE_CONFIG_H
#define YE_CONFIG_H

#include <QString>
#include <QColor>
//==============================================================================================================================

namespace ClickEnter {
	enum { DoubleClick, SingleClick, ClickIcon };
}

class AppCfg
{
	static AppCfg *m_instance;

public:
	AppCfg();

	void loadSettings();
	void saveSettings();

	static AppCfg *instance()    { return m_instance; }
	static void createInstance() { if (!m_instance) m_instance = new AppCfg; }
	static void deleteInstance() { delete m_instance; m_instance = NULL; }

public:
	// window
	bool expand2paneWinHeight;
	bool rightSide;				// sidebar position
	bool statusbarFloating;
	int  statusbarHeight;

	// click & enter
	int  clickEnter;
	int  hoverTime;
	bool keyStopHover;

	// icon theme
	QString iconTheme;
	int     iconSize;
	int     menuIconSize;

	// program files
	QString terminal;

	// time-format
	QString statDateFormat;
	QString statTimeFormat;
	QString fileDateFormat;
	QString fileTimeFormat;
	QString propDateFormat;
	QString propTimeFormat;

	// misc
	int maxHistory;

public:
	bool confirmDelete;

	// debug: don't save these group
	bool dumpOpenFilesCmd;
};

#endif // YE_CONFIG_H

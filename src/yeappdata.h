#ifndef YE_APPDATA_H
#define YE_APPDATA_H

#include <QString>
#include <QColor>
//==============================================================================================================================

namespace ClickEnter {
	enum { DoubleClick, SingleClick, ClickIcon };
}

class AppData
{
public:
	void loadSettings();
	void saveSettings();

public:
	// window
	bool expand2paneWinHeight;
	bool rightSide;				// sidebar position

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

public:
	bool confirmDelete;
};

#endif // YE_APPDATA_H

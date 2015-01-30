#include <QSettings>
#include <QDebug>

#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
//==============================================================================================================================

void AppData::loadSettings()
{
	QSettings s(App::getConfigFile(), QSettings::IniFormat);

	s.beginGroup("window");
	expand2paneWinHeight = s.value("expand2paneWinHeight", true).toBool();
	rightSide            = s.value("rightSide", false).toBool();
	s.endGroup();

	s.beginGroup("click-enter");
	clickEnter   = s.value("clickEnter", ClickEnter::DoubleClick).toInt();
	hoverTime    = s.value("hoverTime", 800).toInt();
	keyStopHover = s.value("keyStopHover", false).toBool();
	s.endGroup();

	s.beginGroup("icon-theme");
	iconTheme    = s.value("iconTheme", "oxygen").toString();
	iconSize     = s.value("iconSize", 16).toInt();
	menuIconSize = 16;	//s.value("menuIconSize", 16).toInt();
	s.endGroup();

	s.beginGroup("program-files");
	terminal     = s.value("terminal", "urxvt -pt Root").toString();
	s.endGroup();

	s.beginGroup("time-format");
	statDateFormat = s.value("statDateFormat", "yyyy-MM-dd").toString();
	statTimeFormat = s.value("statTimeFormat", "hh:mm:ss").toString();
	fileDateFormat = s.value("fileDateFormat", "yyyy-MM-dd").toString();
	fileTimeFormat = s.value("fileTimeFormat", "hh:mm:ss").toString();
	propDateFormat = s.value("propDateFormat", "yyyy-MM-dd").toString();
	propTimeFormat = s.value("propTimeFormat", "hh:mm:ss").toString();
	s.endGroup();

	confirmDelete = true;
}

void AppData::saveSettings()
{
	QSettings s(App::getConfigFile(), QSettings::IniFormat);

	s.beginGroup("window");
	s.setValue("expand2paneWinHeight", expand2paneWinHeight);
	s.setValue("rightSide", rightSide);
	s.endGroup();

	s.beginGroup("click-enter");
	s.setValue("clickEnter",   clickEnter);
	s.setValue("hoverTime",    hoverTime);
	s.setValue("keyStopHover", keyStopHover);
	s.endGroup();

	s.beginGroup("icon-theme");
	s.setValue("iconTheme"   , iconTheme);
	s.setValue("iconSize"    , iconSize);
	s.setValue("menuIconSize", menuIconSize);
	s.endGroup();

	s.beginGroup("program-files");
	s.setValue("terminal"    , terminal);
	s.endGroup();

	s.beginGroup("time-format");
	s.setValue("statDateFormat", statDateFormat);
	s.setValue("statTimeFormat", statTimeFormat);
	s.setValue("fileDateFormat", fileDateFormat);
	s.setValue("fileTimeFormat", fileTimeFormat);
	s.setValue("propDateFormat", propDateFormat);
	s.setValue("propTimeFormat", propDateFormat);
	s.endGroup();
}

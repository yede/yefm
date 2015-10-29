#include <QSettings>
#include <QDebug>

#include "yestyle.h"

#include "yeapp.h"
//==============================================================================================================================

void Style::loadSettings()
{
	QSettings s(App::getStyleFile(), QSettings::IniFormat);

	s.beginGroup("file-view");
	fvStyle.fileColor     = QColor(s.value("fileColor"  , "#404040").toString());
	fvStyle.folderColor   = QColor(s.value("folderColor", "#404080").toString());
	fvStyle.symbolColor   = QColor(s.value("symbolColor", "#40a0a0").toString());
	fvStyle.execColor     = QColor(s.value("execColor"  , "#00a000").toString());
	fvStyle.hiddenColor   = QColor(s.value("hiddenColor", "#80a080").toString());
	fvStyle.cutsColor     = QColor(s.value("cutsColor"  , "#a08080").toString());
	fvStyle.strikeOutCuts = s.value("strikeOutCuts", true).toBool();
	fvStyle.spacing       = s.value("spacing", 2).toInt();
	s.endGroup();

	s.beginGroup("bookmark-list");
	bmStyle.sepFgColor = QColor(s.value("sepFgColor", "#80a0a0").toString());
	bmStyle.sepBgColor = QColor(s.value("sepBgColor", "#f0f0f8").toString());
	bmStyle.itemColor  = QColor(s.value("itemColor" , "#404040").toString());
	bmStyle.spacing    = s.value("spacing", 2).toInt();
	s.endGroup();

	s.beginGroup("status-bar");
	stStyle.fontName  = s.value("fontName", "Monospace").toString();
	stStyle.fontSize  = s.value("fontSize", 12).toInt();
	stStyle.fgColor   = QColor(s.value("fgColor",   "#404040").toString());
	stStyle.bgColor   = QColor(s.value("bgColor",   "#e8e8e8").toString());
	stStyle.useWinBg  = s.value("useWinBg", true).toBool();
	stStyle.pmsColor  = QColor(s.value("pmsColor" , "#800080").toString());
	stStyle.usrColor  = QColor(s.value("usrColor" , "#00a000").toString());
	stStyle.grpColor  = QColor(s.value("grpColor" , "#808000").toString());
	stStyle.sizeColor = QColor(s.value("sizeColor", "#0080a0").toString());
	stStyle.dateColor = QColor(s.value("dateColor", "#c08000").toString());
	stStyle.timeColor = QColor(s.value("timeColor", "#00a080").toString());
	stStyle.fileColor = QColor(s.value("fileColor", "#404040").toString());
	s.endGroup();
}

void Style::saveSettings()
{
	QSettings s(App::getStyleFile(), QSettings::IniFormat);

	s.beginGroup("file-view");
	s.setValue("fileColor",     fvStyle.fileColor.name());
	s.setValue("folderColor",   fvStyle.folderColor.name());
	s.setValue("symbolColor",   fvStyle.symbolColor.name());
	s.setValue("execColor",     fvStyle.execColor.name());
	s.setValue("hiddenColor",   fvStyle.hiddenColor.name());
	s.setValue("cutsColor",     fvStyle.cutsColor.name());
	s.setValue("strikeOutCuts", fvStyle.strikeOutCuts);
	s.setValue("spacing",       fvStyle.spacing);
	s.endGroup();

	s.beginGroup("bookmark-list");
	s.setValue("sepFgColor", bmStyle.sepFgColor.name());
	s.setValue("sepBgColor", bmStyle.sepBgColor.name());
	s.setValue("itemColor",  bmStyle.itemColor.name());
	s.setValue("spacing",    bmStyle.spacing);
	s.endGroup();

	s.beginGroup("status-bar");
	s.setValue("fontName",  stStyle.fontName);
	s.setValue("fontSize",  stStyle.fontSize);
	s.setValue("fgColor",   stStyle.fgColor.name());
	s.setValue("bgColor",   stStyle.bgColor.name());
	s.setValue("useWinBg",  stStyle.useWinBg);
	s.setValue("pmsColor",  stStyle.pmsColor.name());
	s.setValue("usrColor",  stStyle.usrColor.name());
	s.setValue("grpColor",  stStyle.grpColor.name());
	s.setValue("sizeColor", stStyle.sizeColor.name());
	s.setValue("dateColor", stStyle.dateColor.name());
	s.setValue("timeColor", stStyle.timeColor.name());
	s.setValue("fileColor", stStyle.fileColor.name());
	s.endGroup();
}

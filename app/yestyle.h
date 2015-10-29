#ifndef YE_STYLE_H
#define YE_STYLE_H

#include <QWidget>
//==============================================================================================================================

struct Style
{
	void loadSettings();
	void saveSettings();

	static Style *instance()     { return m_instance; }
	static void createInstance() { if (!m_instance) m_instance = new Style; }
	static void deleteInstance() { delete m_instance; m_instance = NULL; }

	struct FileViewStyle {
		QColor fileColor;
		QColor folderColor;
		QColor symbolColor;
		QColor execColor;
		QColor hiddenColor;
		QColor cutsColor;
		bool   strikeOutCuts;
		int    spacing;
	};

	struct BmkViewStyle {
		QColor sepFgColor;
		QColor sepBgColor;
		QColor itemColor;
		int    spacing;
	};

	struct StatBarStyle {
		QString fontName;
		int     fontSize;
		QColor  fgColor;
		QColor  bgColor;
		bool    useWinBg;	// skip bgColor above, use window's bg instead

		QColor  pmsColor;
		QColor  usrColor;
		QColor  grpColor;
		QColor  sizeColor;
		QColor  dateColor;
		QColor  timeColor;
		QColor  fileColor;
	};

	FileViewStyle fvStyle;
	BmkViewStyle  bmStyle;
	StatBarStyle  stStyle;

private:
	static Style *m_instance;
};
//==============================================================================================================================

#endif	// YE_STYLE_H

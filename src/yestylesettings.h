#ifndef YESTYLESETTINGS_H
#define YESTYLESETTINGS_H

#include <QWidget>
//==============================================================================================================================

struct FmStyle {

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
};

//==============================================================================================================================

namespace Ui {
class StyleSettings;
}

class YeApplication;

class StyleSettings : public QWidget
{
	Q_OBJECT

public:
	explicit StyleSettings(YeApplication *app, QWidget *parent = 0);
	~StyleSettings();

	void showDialog();

	static void loadSettings();
	static void saveSettings();

public slots:
	void onAccept();
	void onReject();
	void onTest();
	void onPageChanged(int index);

private:
	Ui::StyleSettings *ui;
	YeApplication *m_app;
};

#endif // YESTYLESETTINGS_H

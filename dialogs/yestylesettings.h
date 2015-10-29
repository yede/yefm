#ifndef YE_STYLE_SETTINGS_H
#define YE_STYLE_SETTINGS_H

#include <QWidget>
//==============================================================================================================================

namespace Ui {
class StyleSettings;
}

class StyleSettings : public QWidget
{
	Q_OBJECT

public:
	explicit StyleSettings(QWidget *parent = 0);
	~StyleSettings();

	void showDialog();

public slots:
	void onAccept();
	void onReject();
	void onTest();
	void onPageChanged(int index);

private:
	Ui::StyleSettings *ui;
};

#endif // YE_STYLE_SETTINGS_H

#ifndef YESETTINGSDLG_H
#define YESETTINGSDLG_H

#include <QWidget>
//==============================================================================================================================

namespace Ui {
class SettingsDlg;
}

class QListWidgetItem;
class YeApplication;

class SettingsDlg : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsDlg(YeApplication *app, QWidget *parent = 0);
	~SettingsDlg();

	void showDialog();

protected:
	void closeEvent(QCloseEvent *event);

private:
	void undoIconTheme();
	void initIconThemes();
	QString getSelectedIconTheme();
	void updateIconTheme();

public slots:
	void accept();
	void reject();
	void onPageChanged(int index);
	void onCurrentItemChanged(QListWidgetItem *curr, QListWidgetItem *);
	void onIconSizeChanged(bool);
	void onIconThemeChanged();

private:
	Ui::SettingsDlg *ui;
	YeApplication   *m_app;
	bool    m_ready;
	QString m_savedIconTheme;
	int     m_savedIconSize;
};

#endif // YESETTINGSDLG_H

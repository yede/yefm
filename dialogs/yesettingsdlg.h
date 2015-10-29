#ifndef YESETTINGSDLG_H
#define YESETTINGSDLG_H

#include <QWidget>
//==============================================================================================================================

namespace Ui {
class SettingsDlg;
}

class QListWidgetItem;
class AppCfg;

class SettingsDlg : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsDlg(QWidget *parent = 0);
	~SettingsDlg();

	void showDialog();

protected:
	void closeEvent(QCloseEvent *event);

private:
	void undoIconTheme();
	void initIconThemes();
	QString getSelectedIconTheme();

signals:
	void changeIconTheme();
	void changeSettings();

public slots:
	void accept();
	void reject();
	void onPageChanged(int index);
	void onCurrentItemChanged(QListWidgetItem *curr, QListWidgetItem *);
	void onIconSizeChanged(bool);
	void updateIconTheme();

private:
	Ui::SettingsDlg *ui;
	AppCfg          *m_cfg;
	bool    m_ready;
	QString m_savedIconTheme;
	int     m_savedIconSize;
};

#endif // YESETTINGSDLG_H

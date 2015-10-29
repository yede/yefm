#ifndef YE_APPLICATION_H
#define YE_APPLICATION_H

#include <QMenu>
#include <QTranslator>

#include "yeapp.h"
//==============================================================================================================================

class DBus;

class MimeDlg;
class SettingsDlg;
class PropertiesDlg;
class CustomActionsDlg;
class StyleSettings;

class Application : public App
{
	Q_OBJECT
public:
	explicit Application(int &argc, char **argv);
	virtual ~Application();

	void callPriorInstance(int &argc, char **argv);
	bool hasPriorInstance() const;

	bool startSession(int &argc, char **argv);
	void stopSession();

	void showMenu(QMenu *menu, QWidget *button);
	bool showMimeDlg(QString &resultApp, const QString &mimeType);
	void showMimeSettingsDlg();
	void showSettingsDlg();
	void showCustomActionsDlg();
	void showStyleSettings();
	void showPasteMgr();

	static Application *uiapp() { return (Application *) m_app; }

	int availableDesktopHeight();

private:
	void loadTranslator();
	void loadQtTranslator();
	void deleteDialogs();
	void deleteResources();
	void createResources();
	void createMainWindow();
	void createMimeDlg();

signals:

protected slots:
	void changeIconTheme();
	void changeSettings();
	void showPropertiesDlg(const QStringList &files);

private:
	DBus *m_bus;

	MimeDlg          *m_mimeDlg;
	SettingsDlg      *m_settingsDlg;
	PropertiesDlg    *m_propertiesDlg;
	CustomActionsDlg *m_customActionsDlg;
	StyleSettings    *m_styleSettingsDlg;

	QTranslator m_translator;
	QTranslator m_qtTranslator;
	QHash<QString, QString> m_fsShortcuts;
};

#endif // YE_APPLICATION_H

#ifndef YE_APPLICATION_H
#define YE_APPLICATION_H

#include <QApplication>
#include <QMenu>
#include <QFileInfo>
#include <QTranslator>
//==============================================================================================================================

struct FmStyle;

class DBus;
class AppData;
class R;
class YeMainWindow;
class FsCache;
class UserBmkModel;
class SideMntModel;
class SideCmdModel;
class MimeDlg;
class Mime;
class SettingsDlg;
class PropertiesDlg;
class FsActionsDlg;
class FsActions;
class StyleSettings;

class YeApplication : public QApplication
{
	Q_OBJECT
public:
	explicit YeApplication(int &argc, char **argv);
	virtual ~YeApplication();

	void callPriorInstance(int &argc, char **argv);
	bool hasPriorInstance() const;

	bool startSession(int &argc, char **argv);
	void stopSession();

	void updateIconTheme();
	void updateSettings();

	void showMenu(QMenu *menu, QWidget *button);
	bool showMimeDlg(QString &resultApp, const QString &mimeType);
	void showMimeDlg();
	void showSettingsDlg();
	void showPropertiesDlg(const QStringList &files);
	void showFsActionsDlg();
	void showStyleSettings();

	FmStyle      *fmStyle()   const { return m_fmStyle; }
	YeMainWindow *win()       const { return m_win; }
	FsCache      *cache()     const { return m_cache; }
	UserBmkModel *bmkModel()  const { return m_bmkModel; }
	SideMntModel *mntModel()  const { return m_mntModel; }
	SideCmdModel *cmdModel()  const { return m_cmdModel; }
	Mime         *mime()      const { return m_mime; }
	FsActions    *fsActions() const { return m_fsActions; }

	int availableDesktopHeight();

	void         initKeyShortcuts();
	int          getFileViewKeyAction(int key);
	QKeySequence getFileViewShortcut(const QString &name);

private:
	void loadTranslator();
	void loadQtTranslator();
	void createResources();
	void createMainWindow();

signals:
	void iconThemeChanged();
	void settingsChanged();

protected slots:

private:
	DBus          *m_bus;
	FmStyle       *m_fmStyle;
	AppData       *m_data;
	R             *m_resources;
	YeMainWindow  *m_win;
	FsCache       *m_cache;
	UserBmkModel  *m_bmkModel;
	SideMntModel  *m_mntModel;
	SideCmdModel  *m_cmdModel;
	MimeDlg       *m_mimeDlg;
	Mime          *m_mime;
	SettingsDlg   *m_settingsDlg;
	PropertiesDlg *m_propertiesDlg;
	FsActionsDlg  *m_fsActionsDlg;
	FsActions     *m_fsActions;
	StyleSettings *m_styleSettings;

	QTranslator m_translator;
	QTranslator m_qtTranslator;
	QHash<QString, QString> m_fsShortcuts;
};

#endif // YE_APPLICATION_H

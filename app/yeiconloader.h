#ifndef YE_ICON_LOADER_H
#define YE_ICON_LOADER_H

#include <QPixmap>
#include <QIcon>
#include <QHash>
//======================================================================================================================

class AppCfg;
class DesktopFile;
class IconTheme;

class IconLoader
{
public:
	explicit IconLoader();
	virtual ~IconLoader();

	void reset();
	void loadIconAliases();
	void saveIconAliases();

	static const QIcon appIcon(const QString &app, int size = 0);
	static const QIcon appIcon(const DesktopFile &app, int size = 0);
	static const QIcon &defaultIcon(const QString &name, int size = 0);
	static const QIcon &icon(const QString &name, int size = 0);
	static const QIcon &menuIcon(const QString &name);
	static const QPixmap &pixmap(const QString &name);

	static bool findIcon(QIcon &icon, const QString &name, int size = 0);
	static bool findMimeIcon(QIcon &icon, const QString &name, int size = 0);

	static IconLoader *instance() { return m_instance; }
	static void createInstance()  { if (!m_instance) m_instance = new IconLoader; }
	static void deleteInstance()  { delete m_instance; m_instance = NULL; }

private:
	static AppCfg *cfg() { return m_instance->m_cfg; }

	const QIcon &getIcon(int size, const QString &name);
	const QIcon &getDefaultIcon(int size, const QString &name);
	const QIcon &getDefaultAppIcon(int size);

	bool searchThemeIcon(QIcon &icon, int size, const QString &name, int mode);

	bool buildIconFromFile(QIcon &icon, int size, const QString &file);
	bool searchIconByName(QIcon &icon, int size, const QString &name);
	bool searchAppIcon(QIcon &icon, int size, const QString &app);
	bool searchAppIcon(QIcon &icon, int size, const DesktopFile &app);

private:
	AppCfg    *m_cfg;
	IconTheme *m_theme;
	bool       m_aliasReady;
	bool       m_themePathReady;

	QHash<QString, QString> m_iconAliases;

	QHash<QString, QIcon> m_defaultIcons16;
	QHash<QString, QIcon> m_defaultIcons22;

	QHash<QString, QIcon> m_icons16;
	QHash<QString, QIcon> m_icons22;

	QHash<QString, QPixmap> m_pixmaps;

	static IconLoader *m_instance;

public:
	QIcon   iconLogo;
	QIcon   iconNone;
	QPixmap pixSep;
	QPixmap pixNone;
};
//==============================================================================================================================

#endif // YE_ICON_LOADER_H

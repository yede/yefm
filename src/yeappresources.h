#ifndef YE_APPRESOURCES_H
#define YE_APPRESOURCES_H

#include <QPixmap>
#include <QIcon>
#include <QHash>
//==============================================================================================================================

class AppData;
class YeApplication;
class YeMainWindow;

class R
{
public:
	R(YeApplication *app, AppData &d);

	void loadIconAliases();
	void saveIconAliases();

	static void updateIconTheme();

	static AppData       &data();
	static YeApplication *app();
	static YeMainWindow  *win();

	static int iconSize();
	static int menuIconSize();
	static const QIcon &defaultIcon(const QString &name, int size = 0);
	static const QIcon &icon(const QString &name, int size = 0);
	static const QIcon &menuIcon(const QString &name);
	static const QPixmap &pixmap(const QString &name);

	static bool getMimeIcon(QIcon &icon, const QString &name, int size = 0);
	static bool getValidIconTheme(QList<int> &types, QStringList &paths, const QString &iconTheme);

private:
	void initThemePaths();
	void reset();

	const QIcon &getIcon(int size, const QString &name);
	const QIcon &getDefaultIcon(int size, const QString &name);

	bool searchThemeIcon(QIcon &icon, int size, const QString &name, int mode);
	bool searchIconDirs(QIcon &icon, int size, const QString &name, const QString &path, int type, int mode);
	bool searchIconExts(QIcon &icon, const QString &name, const QString &path);

private:
	static R      *m_res;
	AppData       &m_data;
	YeApplication *m_app;
	bool           m_aliasReady;
	bool           m_themePathReady;

	QList<int>  m_themeTypes;
	QStringList m_themePaths;

	QStringList m_exts;
	QStringList m_dirs;
	QStringList m_mimeDirs;

	QHash<QString, QString> m_iconAliases;

	QHash<QString, QIcon> m_defaultIcons16;
	QHash<QString, QIcon> m_defaultIcons22;

	QHash<QString, QIcon> m_icons16;
	QHash<QString, QIcon> m_icons22;

	QHash<QString, QPixmap> m_pixmaps;

public:
	QIcon   iconLogo;
	QIcon   iconNone;
	QPixmap pixSep;
	QPixmap pixNone;
};
//==============================================================================================================================

#endif // YE_APPRESOURCES_H

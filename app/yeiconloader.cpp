#include <QDirIterator>
#include <QSettings>
#include <QDebug>

#include "yeiconcache.h"
#include "yeiconloader.h"
#include "yeappcfg.h"
#include "yeapp.h"

#include "yedesktopfile.h"
//==============================================================================================================================

namespace IconThmemType {
	enum { Unknown, SizeStart, SizeEnd };
}

namespace IconSearchMode {
	enum { AppIcon, MimeIcon };
}

//==============================================================================================================================

const QIcon IconLoader::appIcon(const QString &app, int size)
{
	if (size < 1) size = cfg()->iconSize;
	QIcon icon;
	if (m_instance->searchAppIcon(icon, size, app)) return icon;
	return m_instance->getDefaultAppIcon(size);
}

const QIcon IconLoader::appIcon(const DesktopFile &app, int size)
{
	if (size < 1) size = cfg()->iconSize;
	QIcon icon;
	if (m_instance->searchAppIcon(icon, size, app)) return icon;
	return m_instance->getDefaultAppIcon(size);
}

const QIcon &IconLoader::defaultIcon(const QString &name, int size)
{
	return m_instance->getDefaultIcon(size > 0 ? size : cfg()->iconSize, name);
}

const QIcon &IconLoader::menuIcon(const QString &name)
{
	return m_instance->getIcon(cfg()->menuIconSize, name);
}

const QIcon &IconLoader::icon(const QString &name, int size)
{
	return m_instance->getIcon(size > 0 ? size : cfg()->iconSize, name);
}

const QPixmap &IconLoader::pixmap(const QString &name)
{
	if (!m_instance->m_pixmaps.contains(name)) {
		qDebug() << "ERROR pixmap():" << name;
		return m_instance->pixNone;
	}
	return m_instance->m_pixmaps[name];
}

bool IconLoader::findIcon(QIcon &icon, const QString &name, int size)
{
	return m_instance->searchThemeIcon(icon, size > 0 ? size : cfg()->iconSize, name, IconSearchMode::AppIcon);
}

bool IconLoader::findMimeIcon(QIcon &icon, const QString &name, int size)
{
	return m_instance->searchThemeIcon(icon, size > 0 ? size : cfg()->iconSize, name, IconSearchMode::MimeIcon);
}
//==============================================================================================================================

IconLoader::IconLoader()
	: m_cfg(AppCfg::instance())
	, m_aliasReady(false)
	, m_themePathReady(false)
{
	iconLogo = QIcon(":/file-manager");
	iconNone = QIcon(":/exclamation");
	pixSep   = QPixmap(":/sep17");
	pixNone  = QPixmap(":/exclamation");

	m_pixmaps.insert("clear", QPixmap(":/close"));

	m_exts  << "png" << "svg" << "xpm";

	m_dirs  << "actions" << "categories" << "apps" << "places" << "devices" << "mimetypes" << "status"
			<< "animations" << "emblems" << "emotes"
			<< "extras" << "stock";

	m_mimeDirs << "places" << "mimetypes" << "devices" << "actions" << "categories" << "apps" << "status";
}

void IconLoader::reset()
{
	QIcon::setThemeName(m_cfg->iconTheme);

	m_aliasReady = false;
	m_themePathReady = false;

	m_icons16.clear();
	m_icons22.clear();

	initThemePaths();
	loadIconAliases();
}
//==============================================================================================================================

const QIcon &IconLoader::getIcon(int size, const QString &name)
{
	if (name.isEmpty()) return iconNone;

	QHash<QString, QIcon> &icons = (size == 16) ? m_icons16 : m_icons22;

	if (!icons.contains(name)) {
		bool ok = false;
		QIcon icon;				//	qDebug() << "";
		const QList<QString> aliases = m_iconAliases.values(name);

		if (aliases.isEmpty()) {
			ok = searchThemeIcon(icon, size, name, IconSearchMode::AppIcon);
		//	qDebug() << "getIcon(): aliases.isEmpty():" << size << name << ok;
		} else {
			foreach (const QString &alias, aliases) {
				ok = searchThemeIcon(icon, size, alias, IconSearchMode::AppIcon);
		//		qDebug() << "getIcon(): alias:" << ok << size << alias;
				if (ok) break;
			}
		}

		if (!ok) {
			icon = getDefaultIcon(size, name);
		//	qDebug() << "getIcon(): getDefaultIcon():" << size << name;
		}

		icons.insert(name, icon);
	}

	return icons[name];
}

const QIcon &IconLoader::getDefaultIcon(int size, const QString &name)
{
	if (name.isEmpty()) return iconNone;

	QHash<QString, QIcon> &icons = (size == 16) ? m_defaultIcons16 : m_defaultIcons22;

	if (!icons.contains(name)) {
		QIcon icon = QIcon(QString(":/%1/%2").arg(size).arg(name));
		if (icon.isNull()) {
			icon = iconNone;
			qDebug() << "ERROR getDefaultIcon():" << size << name;
		}
		icons.insert(name, icon);
	//	qDebug() << "getDefaultIcon():" << size << name << icon;
	}

	return icons[name];
}

const QIcon &IconLoader::getDefaultAppIcon(int size)
{
	return getDefaultIcon(size, "application-x-executable");
}
//==============================================================================================================================

void IconLoader::loadIconAliases()
{
	QString path = App::getIconAliasesFile();
	if (!QFile(path).exists()) {
		saveIconAliases();
	}

	QSettings s(path, QSettings::IniFormat);
	s.beginGroup("settings");

	QStringList groups = s.value("groups").toStringList();
	QString key;
	QString val;
	QChar sep(':');

	foreach (QString name, groups) {
		if (m_cfg->iconTheme.startsWith(name)) {
			key = name;
			break;
		}
	}
//	qDebug() << "\nloadIconAliases().1:" << key << groups;

	m_iconAliases.clear();

	if (!key.isEmpty()) {
		QStringList items = s.value(key).toStringList();
		foreach (QString item, items) {
			int n = item.indexOf(sep);
			if (n > 0) {
				key = item.left(n);
				val = item.mid(n + 1);
				if (!val.isEmpty()) m_iconAliases.insert(key, val);
			}
		}
	}
//	qDebug() << "loadIconAliases().2:" << m_iconAliases;

	s.endGroup();
	m_aliasReady = true;
}

void IconLoader::saveIconAliases()
{
	QSettings s(App::getIconAliasesFile(), QSettings::IniFormat);
	s.beginGroup("settings");
	s.setValue("groups",  "oxygen, gnome, Faience, Faenza, Human, Mint");
	s.setValue("oxygen",  "bookmarks:favorites, device:drive-harddisk");
	s.setValue("gnome",   "bookmarks:user-bookmarks, device:drive-harddisk");
	s.setValue("Faience", "bookmarks:user-bookmarks, device:drive-harddisk");
	s.setValue("Faenza",  "bookmarks:user-bookmarks, device:drive-harddisk");
	s.setValue("Mint",    "bookmarks:star, device:drive-harddisk");
	s.setValue("Human",   "bookmarks:bookmark-new, device:gnome-dev-harddisk");
	s.endGroup();
}
//==============================================================================================================================

void IconLoader::initThemePaths()
{
	m_themePathReady = true;

	getValidIconTheme(m_themeTypes, m_themePaths, m_cfg->iconTheme);
}

bool IconLoader::getValidIconTheme(QList<int> &types, QStringList &paths, const QString &iconTheme)
{
	QString basePath = "/usr/share/icons";
	QString path = QString("%1/%2").arg(basePath).arg(iconTheme);

	types.clear();
	paths.clear();

	if (!QDir(path).exists()) {
		qDebug() << "getValidIconTheme(): theme-path not found:" << path;
		return false;
	}

	QDirIterator it(basePath, QDir::Dirs | QDir::NoDotAndDotDot);
	QString mainPath;

	while (it.hasNext()) {
		it.next();
		if (it.fileInfo().isDir()) {
			QString dirName = it.fileName();
			if (dirName != iconTheme) {
				if (iconTheme.startsWith(dirName)) {
					int i = 0;
					int n = it.filePath().length();
					foreach (QString path, paths) {
						if (path.length() < n) break;
						i++;
					}
					paths.insert(i, it.filePath());
				}
			} else {
				mainPath = it.filePath();
			}
		}
	}

	if (!mainPath.isEmpty()) {
		paths.insert(0, mainPath);
	}

	int n = paths.size();
	QStringList dirs;
	dirs << "actions" << "places" << "app" << "categories" << "devices" << "extras" << "status" << "stock";
//	qDebug() << "\ngetValidIconTheme():" << paths;

	for (int i = 0; i < n; i++)
	{
		int type = IconThmemType::Unknown;
		bool found = false;

		for (int t = IconThmemType::SizeStart; t <= IconThmemType::SizeEnd; t++)
		{
			for (int d = 0; d < dirs.size(); d++) {
				path = (t == IconThmemType::SizeStart) ? QString("%1/16x16/%2").arg(paths.at(i)).arg(dirs.at(d))
													   : QString("%1/%2/16").arg(paths.at(i)).arg(dirs.at(d));
				if (QDir(path).exists()) {
				//	qDebug() << "getValidIconTheme(): OK!" << t << path;
					found = true; type = t; break;
			//	} else {
			//		qDebug() << "getValidIconTheme(): ---" << t << path;
				}
			}
			if (found) break;
		}

		types.append(type);
	}

	int i = 0;
	while (i < n) {
		if (types.at(i) == IconThmemType::Unknown) {
			types.removeAt(i);
			paths.removeAt(i);
			n--;
		} else {
			i++;
		}
	}

//	qDebug() << "getValidIconTheme():" << types << paths;

	return paths.size() > 0;
}
//==============================================================================================================================

bool IconLoader::searchThemeIcon(QIcon &icon, int size, const QString &name, int mode)
{
	if (name.isEmpty()) return false;

	QHash<QString, QIcon> &icons = (size == 16) ? m_icons16 : m_icons22;

	if (icons.contains(name)) {
		icon = icons[name];
		return true;
	}

	int n = m_themePaths.size();
	if (m_themeTypes.size() != n) return false;

	for (int i = 0; i < n; i++) {
		QString path = m_themePaths.at(i);
		int type = m_themeTypes.at(i);
		if (searchIconDirs(icon, size, name, path, type, mode)) {
			icons.insert(name, icon);
			return true;
		}
	}

	return false;
}

bool IconLoader::searchIconDirs(QIcon &icon, int size, const QString &name, const QString &path, int type, int mode)
{
	bool flag = (type == IconThmemType::SizeStart);
	QString dest;
	QStringList &dirs = (mode == IconSearchMode::MimeIcon) ? m_mimeDirs : m_dirs;

	foreach (QString dir, dirs) {
		dest = flag ? QString("%1/%2x%2/%3").arg(path).arg(size).arg(dir)
					: QString("%1/%2/%3").arg(path).arg(dir).arg(size);
		if (QDir(dest).exists() && searchIconExts(icon, name, dest)) return true;
	}

	foreach (QString dir, dirs) {
		dest = flag ? QString("%1/scalable/%2").arg(path).arg(dir)
					: QString("%1/%2/scalable").arg(path).arg(dir);
		if (QDir(dest).exists() && searchIconExts(icon, name, dest)) return true;
	}

	return false;
}

bool IconLoader::searchIconExts(QIcon &icon, const QString &name, const QString &path)
{
	foreach (QString ext, m_exts) {
		QString file = QString("%1/%2.%3").arg(path).arg(name).arg(ext);
		if (QFile(file).exists()) {
			icon = QIcon(file);
			if (!icon.isNull()) {
			//	qDebug() << "searchIconExts(): found:" << file;
				return true;
			}
		}
	}

//	qDebug() << "searchIconExts(): not found:" << name << path;
	return false;
}
//==============================================================================================================================

void IconLoader::resizeIcon(QIcon &icon, int size)
{
	QPixmap pixmap = icon.pixmap(size, size);
	icon = QIcon(pixmap);
}

bool IconLoader::buildIconFromFile(QIcon &icon, int size, const QString &file)
{
	if (!QFile(file).exists()) return false;

	QPixmap pixmap(file);
	if (pixmap.isNull()) return false;

	if (pixmap.size() != QSize(size, size)) pixmap = pixmap.scaled(size, size);
	icon = QIcon(pixmap);

	return !icon.isNull();
}

bool IconLoader::searchIconByName(QIcon &icon, int size, const QString &name)
{
	if (name.isEmpty()) return false;
	if (buildIconFromFile(icon, size, name)) return true;

	QString iconDir = "/usr/share/pixmaps";
	QDir appIcons(iconDir, "", 0, QDir::Files | QDir::NoDotAndDotDot);
	QStringList iconFiles = appIcons.entryList();
	QStringList searchIcons = iconFiles.filter(name);
	if (searchIcons.count() > 0) {
		QString file = QString("%1/%2").arg(iconDir).arg(searchIcons.at(0));
		if (buildIconFromFile(icon, size, file)) return true;
	}

	if (searchThemeIcon(icon, size, name, IconSearchMode::AppIcon)) {
		resizeIcon(icon, size);
		return true;
	}

	icon = QIcon::fromTheme(name);
	if (!icon.isNull()) {
		resizeIcon(icon, size);
		return true;
	}

	return false;
}

bool IconLoader::searchAppIcon(QIcon &icon, int size, const QString &app)
{
	if (app.isEmpty()) return false;

	QString ext = ".desktop";
	QString name;

	if (app.endsWith(ext)) {
		int n = app.length() - ext.length();
		name = app.left(n);
	} else {
		name = app;
	}

	if (searchIconByName(icon, size, name)) return true;

	QString path = "/usr/share/applications/";
	QString file = QString("%1/%2%3").arg(path).arg(name).arg(ext);
	DesktopFile df = DesktopFile(file);

	return searchAppIcon(icon, size, df);
}

bool IconLoader::searchAppIcon(QIcon &icon, int size, const DesktopFile &app)
{
	QString name = app.getIcon();

	if (searchIconByName(icon, size, name)) return true;

/*	name = app.getFileName().remove(".desktop").split("/").last();	// try luck with application name
	icon = QIcon::fromTheme(name);
	if (!icon.isNull()) {
		int size = IconLoader::cfg()->menuIconSize;
		resizeIcon(icon, size);
		return icon;
	}*/

	return false;
}
//==============================================================================================================================

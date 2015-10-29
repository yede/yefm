#ifndef YE_ICON_CACHE_H
#define YE_ICON_CACHE_H

#include <QString>
#include <QByteArray>
#include <QHash>
#include <QCache>
#include <QIcon>
#include <QModelIndexList>
//==============================================================================================================================

class AppCfg;
class IconLoader;

class IconCache : public QObject
{
	Q_OBJECT
public:
	IconCache(QObject *parent = 0);
	~IconCache();

	void updateIconTheme();

	void loadIconCache();
	void saveIconCache();
	void clearIconCache();

	void loadMimeTypes();
//	void loadThumbs(QModelIndexList indexes, FsModel *model);
	QByteArray buildThumb(QString item);

	QVariant findIcon(const QString &path);

	QHash<QString,QIcon> &folderIcons() { return m_folderIcons; }
	QHash<QString,QIcon> &mimeIcons()   { return m_mimeIcons; }

	void drawSymbolicLink(QIcon &icon);

	static IconCache *instance() { return m_instance; }
	static void createInstance() { if (!m_instance) m_instance = new IconCache; }
	static void deleteInstance() { delete m_instance; m_instance = NULL; }

private:

public slots:

private:
	AppCfg     *m_cfg;
	IconLoader *m_loader;

	int  m_thumbCount;
	bool m_showThumbs;
	QPixmap m_symbolLink;

	QHash<QString, QString>    m_mimeGlob;
	QHash<QString, QString>    m_mimeGeneric;
	QHash<QString, QByteArray> m_thumbs;

	QHash<QString, QIcon>      m_mimeIcons;
	QHash<QString, QIcon>      m_folderIcons;
	QCache<QString, QIcon>     m_icons;

	static IconCache *m_instance;
};

#endif // YE_ICON_CACHE_H

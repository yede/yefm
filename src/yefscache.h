#ifndef YE_FSCACHE_H
#define YE_FSCACHE_H

#include <QPalette>
#include <QString>
#include <QByteArray>
#include <QHash>
#include <QCache>
#include <QIcon>
#include <QModelIndexList>
#include <QFileIconProvider>
//==============================================================================================================================

class YeApplication;
class FsModel;
class Mime;

class FsCache : public QObject
{
	Q_OBJECT

public:
	FsCache(YeApplication *app, QObject *parent = 0);
	~FsCache();

	void updateIconTheme();

	void loadIconCache();
	void saveIconCache();
	void clearIconCache();

	void loadMimeTypes();
	void loadThumbs(QModelIndexList indexes, FsModel *model);
	QByteArray buildThumb(QString item);

	QVariant findIcon(const QString &path);
	QVariant findMimeIcon(const QString &path);

	QHash<QString,QIcon> &folderIcons() { return m_folderIcons; }
	QHash<QString,QIcon> &mimeIcons()   { return m_mimeIcons; }

	void drawSymbolicLink(QIcon &icon);

private:

public slots:

private:
	YeApplication *m_app;
	Mime *m_mime;
	int  m_thumbCount;
	bool m_showThumbs;
	QPixmap m_symbolLink;
//	QFileIconProvider *m_iconFactory;

	QHash<QString, QString>    m_mimeGlob;
	QHash<QString, QString>    m_mimeGeneric;
	QHash<QString, QByteArray> m_thumbs;

	QHash<QString, QIcon>      m_mimeIcons;
	QHash<QString, QIcon>      m_folderIcons;
	QCache<QString, QIcon>     m_icons;

	QString m_iconTheme;
	int     m_iconSize;
};

#endif // YE_FSCACHE_H

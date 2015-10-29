
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QTextStream>
#include <QBuffer>
#include <QFile>
#include <QApplication>
#include <QPalette>
#include <QDir>
#include <QDebug>

#include "yeiconcache.h"
#include "yeiconloader.h"
#include "yeappcfg.h"
#include "yeapp.h"
//==============================================================================================================================

IconCache::IconCache(QObject *parent)
	: QObject(parent)
	, m_cfg(AppCfg::instance())
	, m_loader(IconLoader::instance())
	, m_thumbCount(0)
	, m_showThumbs(false)
	, m_symbolLink(":/emblem-symbolic-link")
{
	m_icons.setMaxCost(500);
}

IconCache::~IconCache()
{
}
//==============================================================================================================================

#define MIME_CACHE_FILE   "/mime.cache"
#define FOLDER_CACHE_FILE "/folder.cache"
#define THUMBS_CACHE_FILE "/thumbs.cache"
//==============================================================================================================================

void IconCache::updateIconTheme()
{
	m_mimeIcons.clear();
	m_folderIcons.clear();

//	saveIconCache();
//	loadIconCache();
}

void IconCache::clearIconCache()
{
	m_folderIcons.clear();
	m_mimeIcons.clear();
	if (m_cfg->iconTheme.isEmpty()) return;

	QString destDir = QString("%1/%2.%3").arg(App::getCacheDir()).arg(m_cfg->iconTheme).arg(m_cfg->iconSize);
	QFile(destDir + FOLDER_CACHE_FILE).remove();
	QFile(destDir + MIME_CACHE_FILE).remove();
}

void IconCache::loadIconCache()
{
	if (m_cfg->iconTheme.isEmpty()) return;

	QString destDir = QString("%1/%2.%3").arg(App::getCacheDir()).arg(m_cfg->iconTheme).arg(m_cfg->iconSize);
	QDir dir(destDir);
	if (!dir.exists()) {
		dir.mkpath(destDir);
		return;
	}

	m_mimeIcons.clear();
	m_folderIcons.clear();

	QFile fileIcons(destDir + MIME_CACHE_FILE);			// Loads cached mime icons
	if (fileIcons.open(QIODevice::ReadOnly)) {
		QDataStream out(&fileIcons);
		out >> m_mimeIcons;
		fileIcons.close();
	}

	fileIcons.setFileName(destDir + FOLDER_CACHE_FILE);	// Loads folder cache
	if (fileIcons.open(QIODevice::ReadOnly)) {
		QDataStream out(&fileIcons);
		out >> m_folderIcons;
		fileIcons.close();
	}
}

void IconCache::saveIconCache()
{
	if (m_cfg->iconTheme.isEmpty()) return;

	QString destDir = QString("%1/%2.%3").arg(App::getCacheDir()).arg(m_cfg->iconTheme).arg(m_cfg->iconSize);
	QDir dir(destDir);
	if (!dir.exists()) dir.mkpath(destDir);

	QFile fileIcons(destDir + MIME_CACHE_FILE);
	if (fileIcons.open(QIODevice::WriteOnly)) {
		QDataStream out(&fileIcons);
		out << m_mimeIcons;
		fileIcons.close();
	}

	fileIcons.setFileName(destDir + FOLDER_CACHE_FILE);
	if (fileIcons.open(QIODevice::WriteOnly)) {
		QDataStream out(&fileIcons);
		out << m_folderIcons;
		fileIcons.close();
	}

	if (m_thumbs.count() > m_thumbCount) {
		fileIcons.setFileName(destDir + THUMBS_CACHE_FILE);
		if (fileIcons.size() > 10000000) {
			fileIcons.remove();
		} else if (fileIcons.open(QIODevice::WriteOnly)) {
			QDataStream out(&fileIcons);
			out << m_thumbs;
			fileIcons.close();
		}
	}
}
/*
void IconCache::loadThumbs(QModelIndexList indexes, FsModel *model)
{
	// Types that should be thumbnailed
	QStringList files, types;
	types << "jpg" << "jpeg" << "png" << "bmp" << "ico" << "svg" << "gif";

	// Remember files with valid suffix
	foreach (QModelIndex item, indexes) {
		QString suffix = QFileInfo(model->fileName(item)).suffix();
		if (types.contains(suffix, Qt::CaseInsensitive)) {
			files.append(model->filePath(item));
		}
	}

	// Loads thumbnails from cache
	if (files.count()) {
		if (m_thumbs.count() == 0) {
			QFile fileIcons(m_cacheDir + THUMBS_CACHE_FILE);
			if (fileIcons.open(QIODevice::ReadOnly)) {
				QDataStream out(&fileIcons);
				out >> m_thumbs;
				fileIcons.close();
				m_thumbCount = m_thumbs.count();
			}
		}
		foreach (QString item, files) {
			if (!m_thumbs.contains(item)) m_thumbs.insert(item, buildThumb(item));
		//	emit thumbUpdate(FsModel::index(item));
		}
	}
}*/

QByteArray IconCache::buildThumb(QString item)
{
	// Thumbnail image
	QImage theThumb, background;
	QImageReader pic(item);
	int w = pic.size().width();
	int h = pic.size().height();

	// Background
	background = QImage(128, 128, QImage::Format_RGB32);
	background.fill(QApplication::palette().color(QPalette::Base).rgb());

	// Scale image and create its shadow template (background.png)
	if (w > 128 || h > 128) {
		pic.setScaledSize(QSize(123, 93));
		QImage temp = pic.read();
		theThumb.load(":/background");
		QPainter painter(&theThumb);
		painter.drawImage(QPoint(0, 0), temp);
	} else {
		pic.setScaledSize(QSize(64, 64));
		theThumb = pic.read();
	}

	// Draw thumbnail picture
	QPainter painter(&background);
	painter.drawImage(QPoint((123 - theThumb.width()) / 2,
							 (115 - theThumb.height()) / 2), theThumb);

	// Write it to buffer
	QBuffer buffer;
	QImageWriter writer(&buffer, "jpg");
	writer.setQuality(50);
	writer.write(background);
	return buffer.buffer();
}
//==============================================================================================================================

void IconCache::drawSymbolicLink(QIcon &icon)
{
	int z = m_cfg->iconSize;
	QPixmap pix = icon.pixmap(z, z);
	QPainter painter(&pix);
	int x = z - m_symbolLink.width();
	int y = z - m_symbolLink.height();
	painter.drawPixmap(x, y, m_symbolLink);
	icon = QIcon(pix);
}

QVariant IconCache::findIcon(const QString &path)
{
	QFileInfo info(path);
	QIcon icon;
	QString name;
	bool isSymLink = info.isSymLink();

	if (info.isDir()) {
	//	name = info.fileName();
	//	if (m_folderIcons.contains(name)) {
	//		return m_folderIcons.value(name);
	//	}
		QString home = QDir::homePath();
	//	qDebug() << "findIcon():" << path << name << home;
		if (path == home) {
			m_loader->findMimeIcon(icon, "user-home");
		} else {
			m_loader->findMimeIcon(icon, "folder");
		}
		if (icon.isNull()) icon = m_loader->icon("folder");
		if (isSymLink) drawSymbolicLink(icon);
		return icon;
	//	return m_iconFactory->icon(info);
	}

	if (m_showThumbs) {
		if (m_thumbs.contains(info.filePath())) {
			QPixmap pic;
			pic.loadFromData(m_thumbs.value(info.filePath()));
			return QIcon(pic);
		}
	}

	QString suffix = info.suffix();

	if (m_mimeIcons.contains(suffix)) {
		icon = m_mimeIcons.value(suffix);
		if (isSymLink) drawSymbolicLink(icon);
		return icon;
	}

	if (suffix.isEmpty())
	{
		suffix = info.isExecutable() ? "exec" : "none";
		if (m_mimeIcons.contains(suffix)) {
			icon = m_mimeIcons.value(suffix);
			if (isSymLink) drawSymbolicLink(icon);
			return icon;
		}
		icon = m_loader->icon(info.isExecutable() ? "application-x-executable"
										   : "text-x-generic");
	} else {
		if (m_mimeGlob.count() == 0) loadMimeTypes();

		QString tmp;
		QChar sep('-');
		int n;
		name = m_mimeGlob.value(suffix.toLower());			// try name as it is
	//	qDebug() << "\nfindIcon().0:" << path << "(" << suffix.toLower() << ")(" << name << ")";

		if (name.isEmpty()) goto text;
		if (m_loader->findMimeIcon(icon, name)) goto done;

		if (name.endsWith(".ms-word")) {
			tmp = "x-office-document";
	//		qDebug() << "findIcon().1:" << tmp;
			if (m_loader->findMimeIcon(icon, tmp)) goto done;
		}

		if (!name.startsWith("application-")) {
			n = name.indexOf(sep);
			if (n > 0) {
				tmp = name.left(n);
	//			qDebug() << "findIcon().2:" << tmp;
				if (m_loader->findMimeIcon(icon, tmp)) goto done;

				tmp = tmp + "-x-generic";
	//			qDebug() << "findIcon().3:" << tmp;
				if (m_loader->findMimeIcon(icon, tmp)) goto done;
			}
		}
/*
		tmp = m_mime->iconName(info);
		qDebug() << "findIcon().3:" << name;
		if (m_loader->getMimeIcon(icon, name)) goto done;

		tmp = m_mime->genericIconName(info);
		qDebug() << "findIcon().4:" << name;
		if (m_loader->getMimeIcon(icon, name)) goto done;
*/
		name = m_mimeGeneric.value(name);				// try matching generic icon
	//	qDebug() << "findIcon().4:" << name;
		if (name.isEmpty()) goto text;
		if (m_loader->findMimeIcon(icon, name)) goto done;

		name = name.split("-").at(0) + "-x-generic";	// last resort try adding "-x-generic" to base type
	//	qDebug() << "findIcon().5:" << name;
		if (name.isEmpty()) goto text;
		if (m_loader->findMimeIcon(icon, name)) goto done;
text:
		icon = m_loader->icon("text-x-generic");
	//	qDebug() << "findIcon().6:" << "text-x-generic";
	}

done:
	m_mimeIcons.insert(suffix, icon);
	if (isSymLink) drawSymbolicLink(icon);
	return icon;
}

void IconCache::loadMimeTypes()
{
	QFile mimeInfo("/usr/share/mime/globs");				// Open file with mime/suffix associations
	if (mimeInfo.open(QIODevice::ReadOnly)) {
		QTextStream out(&mimeInfo);
		do {
			QStringList line = out.readLine().split(":");
			if (line.count() == 2) {
				QString suffix = line.at(1);
				suffix.remove("*.");
				QString mimeName = line.at(0);
				mimeName.replace("/", "-");
				m_mimeGlob.insert(suffix, mimeName);
			}
		} while (!out.atEnd());
		mimeInfo.close();
	}

	mimeInfo.setFileName("/usr/share/mime/generic-icons");	// Open file with mime/generic-mime associations
	if (mimeInfo.open(QIODevice::ReadOnly)) {
		QTextStream out(&mimeInfo);
		do {
			QStringList line = out.readLine().split(":");
			if (line.count() == 2) {
				QString mimeName = line.at(0);
				mimeName.replace("/", "-");
				QString icon = line.at(1);
				m_mimeGeneric.insert(mimeName, icon);
			}
		} while (!out.atEnd());
		mimeInfo.close();
	}
}
//==============================================================================================================================

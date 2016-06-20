#include <QFile>
#include <QDebug>

#include "yeicontheme.h"
//======================================================================================================================

IconPath::IconPath(QByteArray path, int size, Type type, QByteArray context)
	: m_path(path)
	, m_size(size)
	, m_type(type)
	, m_context(context)
{
}

IconPath::Type IconPath::decodeType(QByteArray data)
{
	static const QByteArray sFixed     = "Fixed";
	static const QByteArray sScalable  = "Scalable";
	static const QByteArray sThreshold = "Threshold";

	if (data == sFixed    ) return IconPath::Fixed;
	if (data == sScalable ) return IconPath::Scalable;
	if (data == sThreshold) return IconPath::Threshold;

	return IconPath::Threshold;
}

//======================================================================================================================
// class IconTheme
//======================================================================================================================

#define TAG_INHERITS    "Inherits="
#define TAG_DIRECTORIES "Directories="
#define TAG_SIZE        "Size="
#define TAG_TYPE        "Type="
#define TAG_CONTEXT     "Context="

IconTheme::IconTheme(IconTheme *inheritor)
	: m_inheritor(inheritor)
{
	m_exts  << "png" << "svg" << "xpm";
}

IconTheme::~IconTheme()
{
	clear();
}

void IconTheme::clear()
{
	foreach (IconTheme *item, m_parentList) { delete item; }
	m_parentList.clear();
	m_items.clear();
	m_name.clear();
	m_path.clear();
}
//======================================================================================================================

QString IconTheme::themePath(const QString &iconTheme)
{
	return QString("/usr/share/icons/%1").arg(iconTheme);
}

bool IconTheme::isValid(const QString &iconTheme)
{
	QByteArray data;
	return readIndexData(data, iconTheme);
}

bool IconTheme::readIndexData(QByteArray &data, const QString &iconTheme)
{
	static const char *tagDir = TAG_DIRECTORIES;

	QString indexPath = QString("%1/index.theme").arg(IconTheme::themePath(iconTheme));
	QFile indexFile(indexPath);

	if (!indexFile.exists()) {
	//	qDebug() << "IconTheme::readIndexData(): Not Found:" << indexPath;
		return false;
	}

	if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
	//	qDebug() << "IconTheme::readIndexData(): Failed to open:" << indexPath;
		return false;
	}

	data = indexFile.readAll();
	indexFile.close();

	return data.indexOf(tagDir) > 0;
}
//======================================================================================================================

void IconTheme::setupPathList(const QString &themeName, int size)
{
	clear();

	QByteArray data;
	if (!readIndexData(data, themeName)) {
		qDebug("IconTheme::setupPathList(): Failed to read index-data for theme `%s'.", qPrintable(themeName));
		return;
	}

	m_name = themeName;
	m_path = IconTheme::themePath(themeName);
	parsePathList(data, size);
}
//======================================================================================================================

static bool readLine(QByteArray &result, const QByteArray &data, int from, const char *tag, int &lastPos)
{
	static const char tagEnd = '\n';

	int pos = data.indexOf(tag, from);
	if (pos < 0) return false;

	pos += strlen(tag);
	int end = data.indexOf(tagEnd, pos);
	if (end <= pos) return false;

	result = data.mid(pos, end - pos);
	lastPos = end;
	return true;
}

bool IconTheme::isTheme(const QString &name) const
{
	if (m_name == name) {
	//	qDebug("IconTheme::isTheme(): %s", qPrintable(name));
	//	const IconTheme *inheritor = this;
	//	while (inheritor->m_inheritor != NULL) {
	//		inheritor = inheritor->m_inheritor;
	//		qDebug("IconTheme::isTheme(): inheritor=%s", qPrintable(inheritor->m_name));
	//	}
		return true;
	}

	foreach (IconTheme *parent, m_parentList) {
		bool ok = parent->isTheme(name);
		if (ok) return true;
	}

	return false;
}

bool IconTheme::exists(const QString &themeName) const
{
	const IconTheme *inheritor = this;
	while (inheritor->m_inheritor != NULL) inheritor = inheritor->m_inheritor;

	return inheritor->isTheme(themeName);
}

void IconTheme::parsePathList(const QByteArray &data, int requireSize)
{
	static const char *tagParents = TAG_INHERITS;
	static const char *tagDir     = TAG_DIRECTORIES;
	static const char *tagSize    = TAG_SIZE;
	static const char *tagType    = TAG_TYPE;
	static const char *tagContext = TAG_CONTEXT;
	static const char  separator  = ',';

	int from = 0, dummy;
	QByteArray parents, dirs;

	if (readLine(parents, data, from, tagParents, dummy)) {
		QList<QByteArray> parentList = parents.split(separator);
	//	qDebug("\nIconTheme::parsePathList(): %s.parentList=%s", qPrintable(m_name), parents.constData());
		foreach (const QByteArray &parent, parentList) {
			QString theme = QString::fromUtf8(parent);
			if (!exists(theme)) {
				if (isValid(theme)) {
				//	qDebug("IconTheme::parsePathList(): %s.addParent(%s)", qPrintable(m_name), parent.constData());
					IconTheme *item = new IconTheme(this);
					m_parentList.append(item);
					item->setupPathList(theme, requireSize);	// after: m_parentList.append() - avoid duplicating
				}
			}
		}
	}

	if (!readLine(dirs, data, from, tagDir, from)) {
		qDebug("IconTheme::parsePathList(): Failed to read tag `Directories'.");
		return;
	}

	QList<QByteArray> pathList = dirs.split(separator);
	foreach (const QByteArray &path, pathList) {
		QByteArray group = '[' + path + ']';
		if (data.indexOf(group, from) > 0) {
			QByteArray sizeStr, typeStr, context;
			int sizeFrom = from;
			int typeFrom = from;
			int contextFrom = from;
			readLine(sizeStr, data, from, tagSize, sizeFrom);

			int size = sizeStr.toInt();
			if (size > 0) {
				readLine(typeStr, data, from, tagType, typeFrom);
				readLine(context, data, from, tagContext, contextFrom);
				IconPath::Type type = IconPath::decodeType(typeStr);
				if (size == requireSize || type == IconPath::Scalable) {
					m_items.append(IconPath(path, size, type, context));
				//	qDebug("\nIconTheme::parsePathList(): group=%s", group.constData());
				//	qDebug("IconTheme::parsePathList(): size=%s, tag=`%s'", sizeStr.constData(), tagSize);
				//	qDebug("IconTheme::parsePathList(): type=%s, tag=`%s'", typeStr.constData(), tagType);
				//	qDebug("IconTheme::parsePathList(): context=%s, tag=`%s'", context.constData(), tagContext);
				}
			}

			from = qMax(from, sizeFrom);
			from = qMax(from, typeFrom);
			from = qMax(from, contextFrom);
		}
	}
}
//======================================================================================================================

void IconTheme::resizeIcon(QIcon &icon, int size)
{
	QPixmap pixmap = icon.pixmap(size, size);
	icon = QIcon(pixmap);
}

bool IconTheme::findIcon(QIcon &icon, int size, const QString &name, int mode)
{
	Q_UNUSED(mode);

	foreach (const IconPath &item, m_items) {
		if (item.size() == size) {
			QString path = QString("%1/%2").arg(m_path).arg(item.path());
			if (findIconFromPath(icon, name, path)) return true;
		}
	}

	foreach (const IconPath &item, m_items) {
		if (item.type() == IconPath::Scalable) {
			QString path = QString("%1/%2").arg(m_path).arg(item.path());
			if (findIconFromPath(icon, name, path)) {
				resizeIcon(icon, size);
				return true;
			}
		}
	}

	foreach (IconTheme *parent, m_parentList) {
		bool ok = parent->findIcon(icon, size, name, mode);
		if (ok) return true;
	}

	return false;
}

bool IconTheme::findIconFromPath(QIcon &icon, const QString &name, const QString &path)
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
//======================================================================================================================

#include <QDir>
#include <QDebug>

#include <QMimeDatabase>
#include <QMimeType>

#include "yemime.h"
#include "yedesktopfile.h"
#include "yeproperties.h"
#include "yefileutils.h"
//==============================================================================================================================

Mime::Mime(QObject *parent)
	: QObject(parent)
	, m_db(new QMimeDatabase)
{
	defaultsFileName = "/.local/share/applications/mimeapps.list";
	defaults = new Properties();
}

Mime::~Mime()
{
	delete defaults;
	delete m_db;
}
//==============================================================================================================================

QString Mime::getMimeType(const QString &fileName) const
{
	QMimeType mime = m_db->mimeTypeForFile(fileName);
//	qDebug() << "getMimeType" << mime.name();
	return mime.name();
}

QString Mime::genericIconName(const QFileInfo &fileInfo) const
{
	QMimeType mime = m_db->mimeTypeForFile(fileInfo);
	return mime.genericIconName();
}

QString Mime::iconName(const QFileInfo &fileInfo) const
{
	QMimeType mime = m_db->mimeTypeForFile(fileInfo);
	return mime.iconName();
}
//==============================================================================================================================

void Mime::loadDefaults()
{
	defaults->load(QDir::homePath() + defaultsFileName, "Default Applications");
	defaultsChanged = false;
}

QStringList Mime::getMimeTypes() const
{
	// Check whether file with mime descriptions exists
	QFile file("/usr/share/mime/types");
	if (!file.exists()) {
		return QStringList();
	}

	// Try to open file
	if (!file.open(QFile::ReadOnly)) {
		return QStringList();
	}

	// Read mime types
	QStringList result;
	QTextStream stream(&file);
	while (!stream.atEnd()) {
		result.append(stream.readLine());
	}
	file.close();
	return result;
}

void Mime::setDefaultsFileName(const QString &fileName)
{
	this->defaultsFileName = fileName;
	loadDefaults();
}

QString Mime::getDefaultsFileName() const
{
	return defaultsFileName;
}

void Mime::generateDefaults()
{
	// Load list of applications
	QList<DesktopFile> apps = FileUtils::getApplications();
	QStringList names;

	// Find defaults; for each application...
	// ------------------------------------------------------------------------
	foreach (DesktopFile a, apps) {

		// For each mime of current application...
		QStringList mimes = a.getMimeType();
		foreach (QString mime, mimes) {

			// Current app name
			QString name = a.getPureFileName() + ".desktop";
			names.append(name);

			// If current mime is not mentioned in the list of defaults, add it
			// together with current application and continue
			if (!defaults->contains(mime)) {
				defaults->set(mime, name);
				defaultsChanged = true;
				continue;
			}

			// Retrieve list of default applications for current mime, if it does
			// not contain current application, add this application to list
			QStringList appNames = defaults->value(mime).toString().split(";");
			if (!appNames.contains(name)) {
				appNames.append(name);
				defaults->set(mime, appNames.join(";"));
				defaultsChanged = true;
			}
		}
	}

	// Delete dead defaults (non existing apps)
	// ------------------------------------------------------------------------
	foreach (QString key, defaults->getKeys()) {
		QStringList tmpNames1 = defaults->value(key).toString().split(";");
		QStringList tmpNames2 = QStringList();
		foreach (QString name, tmpNames1) {
			if (names.contains(name)) {
				tmpNames2.append(name);
			}
		}
		if (tmpNames1.size() != tmpNames2.size()) {
			defaults->set(key, tmpNames2.join(";"));
			defaultsChanged = true;
		}
	}

	// Save defaults if changed
	saveDefaults();
}

void Mime::setDefault(const QString &mime, const QStringList &apps)
{
	QString value = apps.join(";");
	if (value.compare(defaults->value(mime, "").toString()) != 0) {
		defaults->set(mime, value);
		defaultsChanged = true;
	}
}

QStringList Mime::getDefault(const QString &mime) const
{
	return defaults->value(mime).toString().split(";");
}

void Mime::saveDefaults()
{
	if (defaultsChanged) {
		defaults->save(QDir::homePath() + defaultsFileName, "Default Applications");
		defaultsChanged = false;
	}
}
//==============================================================================================================================

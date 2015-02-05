#include <sys/vfs.h>
//#include <fcntl.h>

#include <QDirIterator>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

#include "yefileutils.h"
#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
//==============================================================================================================================

bool FileUtils::isDir(const QString &path)
{
	return QDir(path).exists();
}

bool FileUtils::isHidden(const QString &path)
{
	QChar sep = '/';
	QString tmp = path;
	QFileInfo info;

	while (true) {
		info.setFile(tmp);
	//	qDebug() << "FileUtils::isHidden" << info.isHidden() << tmp;
		if (info.isHidden()) return true;

		int n = tmp.lastIndexOf(sep);
		if (n < 1) break;

		tmp = tmp.left(n);
	}

	return false;
}

bool FileUtils::removeRecurse(const QString &path, const QString &name)
{

	// File location
	QString url = path + QDir::separator() + name;

	// Check whether file or directory exists
	QFileInfo file(url);
	if (!file.exists()) {
		return false;
	}

	// List of files that will be deleted
	QStringList files;

	// If given file is a directory, collect all children of given directory
	if (file.isDir()) {
		QDirIterator it(url, QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot
		                | QDir::Hidden, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			files.prepend(it.next());
		}
	}

	// Append given file to the list of files and delete all
	files.append(url);
	foreach (QString file, files) {
		QFile(file).remove();
	}
	return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Collects all file names in given path (recursive)
 * @param path path
 * @param parent parent path
 * @param list resulting list of files
 */
void FileUtils::recurseFolder(const QString &path, const QString &parent, QStringList *list)
{

	// Get all files in this path
	QDir dir(path);
	QStringList files = dir.entryList(QDir::AllEntries | QDir::Files
	                                  | QDir::NoDotAndDotDot | QDir::Hidden);

	// Go through all files in current directory
	for (int i = 0; i < files.count(); i++) {

		// If current file is folder perform this method again. Otherwise add file
		// to list of results
		QString current = parent + QDir::separator() + files.at(i);
		if (QFileInfo(files.at(i)).isDir()) {
			recurseFolder(files.at(i), current, list);
		} else list->append(current);
	}
}
//---------------------------------------------------------------------------

/**
 * @brief Returns size of all given files/dirs (including nested files/dirs)
 * @param files
 * @return total size
 */
qint64 FileUtils::totalSize(const QList<QUrl> &files)
{
	qint64 total = 1;
	foreach (QUrl url, files) {
		QFileInfo file = url.path();
		if (file.isFile()) total += file.size();
		else {
			QDirIterator it(url.path(), QDir::AllEntries | QDir::System
			                | QDir::NoDotAndDotDot | QDir::NoSymLinks
			                | QDir::Hidden, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				it.next();
				total += it.fileInfo().size();
			}
		}
	}
	return total;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns names of available applications
 * @return application name list
 */
QStringList FileUtils::getApplicationNames()
{
	QStringList appNames;
	QDirIterator it("/usr/share/applications", QStringList("*.desktop"),
	                QDir::Files | QDir::NoDotAndDotDot,
	                QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		appNames.append(it.fileName());
	}
	return appNames;
}
//---------------------------------------------------------------------------

QList<DesktopFile> FileUtils::getApplications()
{
	QList<DesktopFile> apps;
	QDirIterator it("/usr/share/applications", QStringList("*.desktop"),
	                QDir::Files | QDir::NoDotAndDotDot,
	                QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		apps.append(DesktopFile(it.filePath()));
	}
	return apps;
}

void FileUtils::getApplications(QList<DesktopFile *> &result)
{
	QDirIterator it("/usr/share/applications", QStringList("*.desktop"),
					QDir::Files | QDir::NoDotAndDotDot,
					QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		result.append(new DesktopFile(it.filePath()));
	}
}
//---------------------------------------------------------------------------

/**
 * @brief Returns real suffix for given file
 * @param name
 * @return suffix
 */
QString FileUtils::getRealSuffix(const QString &name)
{

	// Strip version suffix
	QStringList tmp = name.split(".");
	bool ok;
	while (tmp.size() > 1) {
		tmp.last().toInt(&ok);
		if (!ok) {
			return tmp.last();
		}
		tmp.removeLast();
	}
	return "";
}
//---------------------------------------------------------------------------

QString FileUtils::getTitleFromPath(const QString &path)
{
	int n = path.length();
	int i = path.lastIndexOf('/') + 1;

	if (i < n) return path.mid(i);
	if (i == 1) return "/";

	return QObject::tr("Untitled");
}
//==============================================================================================================================

QString FileUtils::formatSize(qint64 num)
{
	QString total;
	const qint64 kb = 1024;
	const qint64 mb = 1024 * kb;
	const qint64 gb = 1024 * mb;
	const qint64 tb = 1024 * gb;

	if      (num >= tb) total = QString("%1TB").arg(QString::number(qreal(num) / tb, 'f', 2));
	else if (num >= gb) total = QString("%1GB").arg(QString::number(qreal(num) / gb, 'f', 2));
	else if (num >= mb) total = QString("%1MB").arg(QString::number(qreal(num) / mb, 'f', 1));
	else if (num >= kb) total = QString("%1KB").arg(QString::number(qreal(num) / kb, 'f', 1));
	else if (num >=  1) total = QString("%1bytes").arg(num);
	else                total = QString("%1byte").arg(num);

	return total;
}

QString FileUtils::getDriveInfo(QString path)
{
	struct statfs info;
	statfs(path.toLocal8Bit(), &info);

	if (info.f_blocks == 0) return "";

	return QString("%1  /  %2  (%3%)")
		   .arg(formatSize((qint64)(info.f_blocks - info.f_bavail) * info.f_bsize))
	       .arg(formatSize((qint64) info.f_blocks * info.f_bsize))
	       .arg((info.f_blocks - info.f_bavail) * 100 / info.f_blocks);
}
//==============================================================================================================================

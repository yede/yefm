#ifndef YE_FILEUTILS_H
#define YE_FILEUTILS_H

#include <QFileInfo>
//==============================================================================================================================

class DesktopFile;

class FileUtils
{
public:
	static bool isDir(const QString &path);
	static bool isHidden(const QString &path);

	static inline QString getFileName(const QString &path);

	static QString getUniqueFileName(const QString &destFolder, const QString &fileName);
	static QString buildRenamePath(const QString &destFolder, const QString &fileName);
	static bool duplicate(const QFileInfo &src, const QString &destFolder);

	static bool removeRecurse(const QString &filePath);
	static void recurseFolder(const QString &path, const QString &parent, QStringList *list);

	static qint64 totalSize(const QList<QUrl> &files);
	static QStringList getApplicationNames();

	static QList<DesktopFile> getApplications();
	static void getApplications(QList<DesktopFile *> &result);

	static QString getRealSuffix(const QString &name);
	static QString getTitleFromPath(const QString &path);

	static QString formatSize(qint64);
	static QString getDriveInfo(QString);
};
//==============================================================================================================================

inline QString FileUtils::getFileName(const QString &path)
{
	QFileInfo fileInfo(path);
	return fileInfo.isRoot() ? QString("/") : fileInfo.fileName();
}
//==============================================================================================================================

#endif // YE_FILEUTILS_H

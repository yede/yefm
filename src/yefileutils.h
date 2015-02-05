#ifndef YE_FILEUTILS_H
#define YE_FILEUTILS_H

#include <QObject>
#include <QFileInfo>
#include <QIcon>
#include "yeprogressdlg.h"
#include "yedesktopfile.h"
#include "yeproperties.h"
//==============================================================================================================================

class ProgressWatcher;

class FileUtils
{
public:
	static bool isDir(const QString &path);
	static bool isHidden(const QString &path);
	static bool removeRecurse(const QString &path, const QString &name);
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

#endif // YE_FILEUTILS_H

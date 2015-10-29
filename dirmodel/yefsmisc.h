#ifndef YE_FS_MISC_H
#define YE_FS_MISC_H

#include <QFileInfoList>
#include <QString>
#include <QHash>
#include <QList>
#include <QDir>
//==============================================================================================================================

class FsNode;

class FsMisc
{
public:
	static void clearNodes(QHash<QString, FsNode *> &nodes);
	static void clearNodes(QList<FsNode *> &nodes);

	static void sortFileList(QFileInfoList &ret, QDir::SortFlags sortFlags, const QFileInfoList &src);

	static int compare(const QString &s1, const QString &s2, int len);
	static int compareFileName(const QString &s1, const QString &s2);
};

#endif

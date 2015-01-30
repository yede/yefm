#ifndef YE_FSMODELITEM_H
#define YE_FSMODELITEM_H

#include <QDir>
#include <QFileIconProvider>
#include <QDateTime>
//==============================================================================================================================

#define SEPARATOR QString("/")

class FsModelItem
{
public:
	FsModelItem(const QFileInfo &fileInfo, FsModelItem *parent);
	~FsModelItem();

	FsModelItem *childAt(int position);
	FsModelItem *parent();
	FsModelItem *matchPath(const QStringList &path, int startIndex = 0);

	QList<FsModelItem *> children();

	int childCount() const;
	int childNumber() const;
	bool hasChild(QString);

	QString absoluteFilePath() const;
	QString fileName() const;

	QFileInfo fileInfo() const;

	void refreshFileInfo();
	void addChild(FsModelItem *child);
	void removeChild(FsModelItem *child);
	void changeName(QString);
	void clearAll();

private:
	friend class FsModel;

	FsModelItem *m_parent;
	QFileInfo m_fileInfo;
	bool m_walked;
	bool m_dirty;
	bool m_watched;

	QList<FsModelItem *> m_children;
	QString m_absFilePath;

	QString m_mimeType;
	QString m_permissions;
};


#endif // YE_FSMODELITEM_H

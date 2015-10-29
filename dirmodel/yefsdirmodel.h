#ifndef YE_FS_DIRMODEL_H
#define YE_FS_DIRMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QMimeData>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
//==============================================================================================================================

struct FsFetchResult;

class FsNode;
class FsThread;
class FsWatcher;

class FsDirModel : public QAbstractItemModel
{
	Q_OBJECT
public:

	enum DragMode {
		DM_UNKNOWN = 0,
		DM_COPY,
		DM_MOVE,
		DM_LINK
	};

	FsDirModel(QObject *parent = 0);
	~FsDirModel();

	inline FsNode *getNodeByPath(const QString &filePath) const;
	inline FsNode *getNode(const QString &fileName) const;
	inline FsNode *getNode(const QModelIndex &index) const;
	inline FsNode *getVisibleNode(const QString &fileName) const;

	QModelIndex index(const QString &path) const;
	QModelIndex index(FsNode *node) const;
	QModelIndex index(int row, int col, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &index) const;

	int count() const { return m_visibleChildren.size(); }
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	bool canFetchMore(const QModelIndex &parent) const;
	void fetchMore(const QModelIndex &parent);

	Qt::DropActions supportedDropActions() const;
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;

	bool      isDir(const QModelIndex &index) const;
	qint64    size(const QModelIndex &index) const;
	QFileInfo fileInfo(const QModelIndex &index) const;
	QString   fileName(const QModelIndex &index) const;
	QString   filePath(const QModelIndex &index) const;

	void setRootPath(const QString &directoryPath);
	void resetModel();
	void invalidate();

	void createFile();
	void createFolder();

	const char    *name()                               const { return m_name; }
	const QString &rootPath()                           const { return m_rootPath; }
	const QString  getNodePath(const QString &fileName) const { return m_rootPath + '/' + fileName; }

	inline bool isValid()    const { return QDir(m_rootPath).exists(); }
	inline bool isReadable() const { return isValid() && QFileInfo(m_rootPath).isReadable(); }
	inline bool isWritable() const { return isValid() && QFileInfo(m_rootPath).isWritable(); }

protected:
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

signals:
	void goParent();
	void invalidated();
	void rootPathChanged(const QModelIndex &index);
	void edit(const QModelIndex &index);
	void select(const QModelIndex &index, bool ensureVisible = false);

private slots:
	void populate(FsFetchResult *data, int mode);

private:
	friend class FsThread;
	friend class FsWatcher;

	void populateFirstRet(FsFetchResult *data);
	void populateContinue(FsFetchResult *data);
	void tryGoUp();
	void insertNode(const QString &fileName);
	void deleteNode(const QString &fileName);
	void updateNode(const QString &fileName);
	void renameNode(const QString &fromName, const QString &toName);

private:
	FsThread  *m_thread;
	FsWatcher *m_watcher;
	QHash<QString, FsNode *> m_children;
	QList<FsNode *>          m_visibleChildren;

	char    m_name[16];
	QString m_rootPath;
	QString m_editFileName;
};
//==============================================================================================================================

inline FsNode *FsDirModel::getNode(const QModelIndex &index) const
{
	return index.isValid() ? static_cast<FsNode *>(index.internalPointer()) : NULL;
}

inline FsNode *FsDirModel::getNode(const QString &fileName) const
{
	return m_children.value(fileName, NULL);
}

inline FsNode *FsDirModel::getNodeByPath(const QString &filePath) const
{
	return (m_rootPath.isEmpty() || !filePath.startsWith(m_rootPath)) ? NULL : getNode(filePath.mid(m_rootPath.length() + 1));
}

inline FsNode *FsDirModel::getVisibleNode(const QString &fileName) const
{
	FsNode *node = m_children.value(fileName, NULL);
	return (m_visibleChildren.indexOf(node) < 0) ? NULL : node;
}
//==============================================================================================================================

#endif // YE_FS_DIRMODEL_H

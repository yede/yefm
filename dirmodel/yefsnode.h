#ifndef YE_FS_NODE_H
#define YE_FS_NODE_H

#include <QFileInfo>
#include <QDateTime>
#include <QStringList>
#include <QBitArray>
//==============================================================================================================================

#define cFilePath(node)   qPrintable(node->filePath())
#define cFileName(node)   qPrintable(node->fileName())
#define cPath(node)       qPrintable(node->path())


class FsNode
{
public:
	FsNode(const QFileInfo &fileInfo);
	FsNode();
	~FsNode();

	void refresh();
	void clear();
	void clearMimeType();
	void setFileName(const QString &fileName);
	void setFilePath(const QString &filePath);

	QString          filePath()       { return m_fileInfo.filePath(); }
	QString          fileName()       { return m_fileInfo.isRoot() ? sep : m_fileInfo.fileName(); }
	const QFileInfo &fileInfo() const { return m_fileInfo; }
	const QString   &getMimeType();
	const QString   &getPermissions();
	const QString    getDataTime();
	const QString    getFilePath(const QString &newName);

	inline qint64 size() const { return m_fileInfo.size(); }

	inline bool isRoot()       const { return m_fileInfo.isRoot(); }
	inline bool isDir()        const { return m_fileInfo.isDir(); }
	inline bool isFile()       const { return m_fileInfo.isFile(); }

	inline bool isSystem()     const { return !m_fileInfo.isDir() && !m_fileInfo.isFile(); }
	inline bool isHidden()     const { return m_fileInfo.isHidden(); }
	inline bool isSymLink()    const { return m_fileInfo.isSymLink(); }

	inline bool isReadable()   const { return m_fileInfo.isReadable(); }
	inline bool isWritable()   const { return m_fileInfo.isWritable(); }
	inline bool isExecutable() const { return m_fileInfo.isExecutable(); }

	bool isVisible() const { return getBool(bitVisible); }
	bool isCrawled() const { return getBool(bitCrawled); }
	bool isDirty()   const { return getBool(bitDirty);   }

	void setVisible(bool val) { setBool(bitVisible, val); }
	void setCrawled(bool val) { setBool(bitCrawled, val); }
	void setDirty  (bool val) { setBool(bitDirty,   val); }

	static const QString sep;

private:
	enum Bit {
		bitVisible, bitCrawled, bitDirty
	};

	bool getBool(Bit bit)     const { return (m_bools & (1 << bit)) != 0; }
	void setBool(Bit bit, bool val) { m_bools = ((0xffff ^ (1 << bit)) & m_bools) | (val ? (1 << bit) : 0); }

private:
	QFileInfo m_fileInfo;
	quint32   m_bools;

	QString m_mimeType;
	QString m_permissions;
};

#endif	// YE_FS_NODE_H

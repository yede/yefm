#ifndef YE_PASTEITEM_H
#define YE_PASTEITEM_H

#include <QStringList>
#include <QFileInfo>
//==============================================================================================================================

namespace PasteConflict {
	enum { None, Dir2Dir, Dir2File, File2Dir, File2File };
}

namespace PasteSolution {
	enum { None,
		   Skip,	// skip source file/folder
		   Rename,
		   Merge,	// for both folder only. copy/move items inside source folder into destination folder
		   Replace	// replace destination with source
		 };
}

namespace PasteCol {
	enum {
		Percent,
		Conflict,
		Solution,
		SrcName,
		DestName,
		ColumnCount
	};
}
//==============================================================================================================================

class PasteTask;

class PasteItem
{
public:
	PasteItem(bool isFolder, PasteTask *task, PasteItem *parent);
	~PasteItem();

	void clear();
	void addChild(PasteItem *item);
	void removeChild(PasteItem *item);

	QString getConflictText();
	QString getSolutionText();
	QString getPercentText();

public:

	PasteItem *parent()       const { return m_parent; }
	PasteItem *child(int row) const;
	bool       hasChildren()  const { return m_children.size() > 0; }
	bool       hasConflict()  const;
	bool       isFolder()     const { return m_isFolder; }
	QList<PasteItem*> &children()   { return m_children; }

	const QStringList &errors()   const { return m_errors; }
	bool               hasError() const { return !m_errors.isEmpty(); }
	void addErrorInfo(int err, const QString &text);

	int row() const;
	int rowCount() const { return m_children.size(); }

	int        conflict()     const { return m_conflict; }
	int        solution()     const { return m_solution; }

	void setConflict(int value)    { m_conflict = value; }
	void setSolution(int value);

	qint64     totalBytes()   const { return m_totalBytes; }
	qint64     transBytes()   const { return m_transBytes; }

	void setTotalBytes(qint64 val) { m_totalBytes = val; }
	void setTransBytes(qint64 val) { m_transBytes = val; }
	void addTransBytes(qint64 val) { m_transBytes += val; }
	void setTransDone()            { m_transBytes = m_totalBytes; }

	const QString &srcName()  const { return m_srcName; }
	const QString &srcPath()  const { return m_srcPath; }

	void setSrcName (const QString &name) { m_srcName  = name; }
	void setSrcPath (const QString &path) { m_srcPath  = path; }

	const QString  destPath()   const { return m_destFolder + "/" + destName(); }
	const QString &destFolder() const { return m_destFolder; }
	const QString &destName()   const
	{
		if (m_solution == PasteSolution::Rename) return m_destName.isEmpty() ? m_srcName : m_destName;
		return m_srcName;
	}

	void setDestName(const QString &name)   { m_destName   = name; }
	void setDestFolder(const QString &path) { m_destFolder = path; }

private:
	PasteTask *m_task;
	PasteItem *m_parent;

	qint64 m_totalBytes;
	qint64 m_transBytes;

	int  m_conflict;	// PasteConflict
	int  m_solution;	// PasteSolution

	bool m_isFolder;

	QString m_srcName;
	QString m_srcPath;

	QString m_destName;
	QString m_destFolder;

	QList<PasteItem*> m_children;
	QStringList m_errors;
};
//==============================================================================================================================

#endif // YE_PASTEITEM_H

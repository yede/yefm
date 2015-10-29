#ifndef YE_PASTETHREAD_H
#define YE_PASTETHREAD_H

#include <QThread>
#include <QStringList>
//==============================================================================================================================

namespace PasteError {
	enum { NoError, Aborted, UnWritable, MakeFolder, FileCopy };
}
//==============================================================================================================================

class PasteTask;
class PasteItem;

class PasteThread : public QThread
{
	Q_OBJECT
public:
	explicit PasteThread(PasteTask *parent);
	virtual ~PasteThread();

	void abort();

signals:
	void totalBytesChanged(qint64 total);
	void updateProgress(PasteItem *item, qint64 totalProgress);
	void pasteFinished(int error, const QStringList &selFiles);

public slots:

protected:
	void run();

private:
	void sumTotalBytes();
	void copyFile(const QString &srcPath, const QString &destPath, PasteItem *item);
	void copyFolder(const QString &srcPath, const QString &destPath, PasteItem *item);
	void copyItem(PasteItem *item);

	bool isAborted() const { return m_abort || m_stop; }

private:
	PasteTask *m_task;
	PasteItem *m_root;

	qint64 m_totalBytes;
	qint64 m_totalProgress;
	QStringList m_selFiles;		// a list of new files, we can select them later
	QString m_destFolder;

	int  m_type;
	int  m_errorCount;
	bool m_abort;
	bool m_stop;

	bool dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;
};

#endif // YE_PASTETHREAD_H

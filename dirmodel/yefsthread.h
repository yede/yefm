#ifndef YE_FS_THREAD_H
#define YE_FS_THREAD_H

#include <QThread>
#include <QFileInfoList>
#include <QMutex>
#include <QWaitCondition>
#include <QAtomicInt>
//==============================================================================================================================

class FsNode;
class FsDirModel;

struct FsFetchResult
{
	FsFetchResult(const QString &path): directoryPath(path) {}

	QString         directoryPath;
	QList<FsNode *> nodes;
	int             fileCount;
	int             populated;
};

namespace FsPopulateMode {
	enum { FirstRet, Continue };
}
//==============================================================================================================================

class FsThread : public QThread
{
	Q_OBJECT
public:
	FsThread(FsDirModel *parent);
	~FsThread();

	void ensureStop();
	void fetchChildren(const QString &directoryPath);
	void fetchMore();
	void setPopulated(FsFetchResult *data);

	bool isAborted() const { return m_abort; }

private:
	enum {
		ppNone,
		ppWait,		// thread waiting for model populating
		ppDone		// model finished populating, can populate another nodes
	};

	FsFetchResult *takeBuffer(int fileCount, int populated);
	void putBuffer(FsFetchResult *data);
	void clearRets();

	void waitAndPopulate(FsFetchResult *&data, int populated);
	void fetch();
	void dumpChildren(const QFileInfoList &list);
	void dumpStepInfo(int index, const char *msg, int size, int total);

	void waitInternal(unsigned long timeout);
	void waitForCall();
	void beginTask();

protected:
	void run();

signals:
	void populate(FsFetchResult *data, int mode);

public slots:

private:
	FsDirModel *m_engine;

	bool m_abort;
	bool m_sleeping;
	bool dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;

	QAtomicInt             m_populated;
	QMutex                 m_mutex;
	QWaitCondition         m_cond;

	QMutex                 m_dataMutex;
	QList<FsFetchResult *> m_dataList;

	QString m_directoryPath;
};
//==============================================================================================================================

#endif	// YE_FS_THREAD_H

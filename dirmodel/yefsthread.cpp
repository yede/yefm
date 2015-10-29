#include <QDirIterator>
#include <QDir>
#include <QDebug>

#include "yefsdirmodel.h"
#include "yefsthread.h"
#include "yefsnode.h"
#include "yefsmisc.h"

#include "yefsmisc.h"
//==============================================================================================================================

FsThread::FsThread(FsDirModel *parent)
	: QThread(parent)
	, m_engine(parent)
{
	connect(this, SIGNAL(populate(FsFetchResult*,int)), parent, SLOT(populate(FsFetchResult*,int)));
}

FsThread::~FsThread()
{
	clearRets();
}
//==============================================================================================================================

void FsThread::clearRets()
{
	foreach (FsFetchResult *data, m_dataList) {
		FsMisc::clearNodes(data->nodes);
		delete data;
	}
	m_dataList.clear();
}

FsFetchResult *FsThread::takeBuffer(int fileCount, int populated)
{
	FsFetchResult *data = NULL;

	m_dataMutex.lock();
	if (m_dataList.size() > 0) data = m_dataList.takeAt(0);
	m_dataMutex.unlock();

	if (data == NULL) {
		data = new FsFetchResult(m_directoryPath);
	} else {
		data->directoryPath = m_directoryPath;
	}
	data->fileCount = fileCount;
	data->populated = populated;

	return data;
}

void FsThread::putBuffer(FsFetchResult *data)
{
	m_dataMutex.lock();
	m_dataList.append(data);
	m_dataMutex.unlock();
}
//==============================================================================================================================

void FsThread::dumpChildren(const QFileInfoList &list)
{
	QStringList files;
	foreach (const QFileInfo &info, list) files.append(info.fileName());
	qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
	qDebug() << "FsModelThread::dumpChildren()" << m_engine->name();
	qDebug() << "directoryPath:" << m_directoryPath;
	qDebug() << "nodes=" << files;
}

void FsThread::dumpStepInfo(int index, const char *msg, int size, int total)
{
	QString time = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
	qDebug("fetch(): %s [%d] %s %s, size=%d, total=%d, directoryPath=\"%s\"",
		   m_engine->name(), index, qPrintable(time), msg, size, total, qPrintable(m_directoryPath));
}

void FsThread::setPopulated(FsFetchResult *data)
{
	data->nodes.clear();
	putBuffer(data);
	m_populated.fetchAndStoreOrdered(ppDone);
	m_cond.wakeAll();
}

void FsThread::waitAndPopulate(FsFetchResult *&data, int populated)
{
	if (m_populated == ppWait) {
		while (!m_abort) {
			if (m_populated != ppWait) {
			//	qDebug("EXIT ... %s waitAndPopulate() loop", m_engine->name());
				waitInternal(200);		// 120: lite stuck, 160: not bad, 200: good
				break;
			}
			waitInternal(30);
		}
	}

	if (!m_abort) {
		m_populated.fetchAndStoreOrdered(ppWait);
		int mode = (populated > 0) ? FsPopulateMode::Continue : FsPopulateMode::FirstRet;
		emit populate(data, mode);
		data = NULL;				// data has been delivered
	}
}

static void getFileInfoList(QFileInfoList &ret, const QString &directoryPath, QDir::Filters filters, QDir::SortFlags sortFlags)
{
	QDirIterator it(directoryPath, filters);
	QFileInfoList tmp;

	while (it.hasNext()) {
		it.next();
		tmp.append(it.fileInfo());
	}

	FsMisc::sortFileList(ret, sortFlags, tmp);
}

void FsThread::fetch()
{
//	qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//	int i = 0;
//	dumpStepInfo(i++, "start", 0, 0);

	QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
	QDir::SortFlags flags = QDir::Name | QDir::DirsFirst | QDir::IgnoreCase;// | QDir::LocaleAware;

	QFileInfoList list;
	getFileInfoList(list, m_directoryPath, filters, flags);
//	dumpChildren(list);

	int fileCount = list.size();
	int populated = 0;
	int size  = 0;
	int step  = 400;
	FsFetchResult *data = takeBuffer(fileCount, populated);
//	dumpStepInfo(i++, "iterated", 0, fileCount);

	foreach (const QFileInfo &fileInfo, list) {
		if (m_abort) break;

		FsNode *node = new FsNode(fileInfo);
		data->nodes.append(node);

		size++;
		if (size >= step) {
		//	dumpStepInfo(i++, "step", size, populated + size);
			waitAndPopulate(data, populated);
			if (m_abort) break;

			populated += size;
			size = 0;
			data = takeBuffer(fileCount, populated);
		}
	}

	if (!m_abort) {
	//	dumpStepInfo(i++, "done", size, populated + size);
		waitAndPopulate(data, populated);
		return;
	}

	if (data != NULL) {			// delivered?
		FsMisc::clearNodes(data->nodes);
		putBuffer(data);
	}
}

void FsThread::run()
{
	m_abort = m_sleeping = false;
	m_populated.fetchAndStoreOrdered(ppNone);

	fetch();
}
//==============================================================================================================================

void FsThread::fetchChildren(const QString &directoryPath)
{
	m_directoryPath = directoryPath;
	beginTask();
}

void FsThread::fetchMore()
{

}
//==============================================================================================================================

void FsThread::beginTask()
{
	ensureStop();
	start();
}

void FsThread::waitForCall()
{
	waitInternal(ULONG_MAX);
}

void FsThread::waitInternal(unsigned long timeout)
{
	QMutexLocker lock(&m_mutex);
//	Q_UNUSED(lock);
	m_sleeping = true;
	m_cond.wait(&m_mutex, timeout);		// ULONG_MAX
	m_sleeping = false;
}

void FsThread::ensureStop()
{
	m_abort = true;
//	int i = 0;
	while (isRunning()) {
//		qDebug() << "FsModelThread::waitStopped(): this->isRunning()" << i++;
		if (m_sleeping) m_cond.wakeAll();
		msleep(10);
	}
}
//==============================================================================================================================

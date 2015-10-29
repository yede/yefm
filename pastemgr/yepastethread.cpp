#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>

#include "yepastethread.h"
#include "yepastetask.h"
#include "yepasteitem.h"
#include "yepastemgr.h"

#include "yefileutils.h"
#include "yeapp.h"
//==============================================================================================================================

#define use_dbg 0

#if use_dbg
# define dbg(...) qDebug(__VA_ARGS__)
#else
# define dbg(...)
#endif //USE_DEBUG
//==============================================================================================================================

PasteThread::PasteThread(PasteTask *parent)
	: QThread(parent)
	, m_task(parent)
	, m_root(parent->m_root)
{
	connect(this, SIGNAL(totalBytesChanged(qint64)), m_task, SLOT(totalBytesChanged(qint64)));
	connect(this, SIGNAL(updateProgress(PasteItem*,qint64)), m_task, SLOT(updateProgress(PasteItem*,qint64)));
	connect(this, SIGNAL(pasteFinished(int,QStringList)), m_task, SLOT(pasteFinished(int,QStringList)));
}

PasteThread::~PasteThread()
{
}

void PasteThread::abort()
{
	m_abort = true;
}
//==============================================================================================================================

void PasteThread::sumTotalBytes()
{
	qint64 total = 0;

	foreach (PasteItem *item, m_root->children()) {
		if (item->solution() == PasteSolution::Skip) continue;

		QFileInfo file = QFileInfo(item->srcPath());
		qint64 itemBytes = file.size();

		if (file.isDir()) {
			QDirIterator it(item->srcPath(),
							QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden,
							QDirIterator::Subdirectories);
			while (it.hasNext()) {
				it.next();
				itemBytes += it.fileInfo().size();
			}
		}

		item->setTotalBytes(itemBytes);
		total += itemBytes;
	}
	dbg("PasteThread::sumTotalBytes(): total=%lld", total);

	m_totalBytes = total;
	m_root->setTotalBytes(total);
	emit totalBytesChanged(total);
}
//==============================================================================================================================
#define BUF_LEN       61440
#define UPDATE_STEP   20480000

void PasteThread::copyFile(const QString &srcPath, const QString &destPath, PasteItem *item)
{
	dbg("PasteThread::copyFile(): src=\"%s\", dest=\"%s\"", qPrintable(srcPath), qPrintable(destPath));

	QFile in(srcPath);
	QFile out(destPath);

	if (out.exists()) {				// auto rename
		QFileInfo dest(destPath);
		QString newPath = FileUtils::buildRenamePath(dest.path(), dest.fileName());
		out.setFileName(newPath);
		dbg("PasteThread::copyFile(): destFolder exists! srcFile=\"%s\", destPath=\"%s\", newPath=\"%s\"",
			qPrintable(srcPath), qPrintable(destPath), qPrintable(newPath));
	}

	in.open(QFile::ReadOnly);
	out.open(QFile::WriteOnly);

	char   block[BUF_LEN];
	qint64 total = in.size();
	qint64 steps = UPDATE_STEP;		// updateProgress on each 20MB (30~40MB/s)
	qint64 bytes = 0;

	while (!in.atEnd()) {
		if (isAborted()) break;

		qint64 inBytes = in.read(block, sizeof(block));
		out.write(block, inBytes);
		bytes += inBytes;

		if (bytes >= steps) {
			item->addTransBytes(bytes);
			m_totalProgress += bytes;
			emit updateProgress(item, m_totalProgress);
			bytes = 0;
		//	dbg() << "PasteThread::copyFile(): progress=" << m_itemProgress << m_totalProgress;
		}
	}

	out.close();
	in.close();

	if (isAborted()) return;

	item->addTransBytes(bytes);
	m_totalProgress += bytes;
	emit updateProgress(item, m_totalProgress);

	dbg("PasteThread::copyFile(): progress=(%lld:%lld:%lld:%lld), in:out=(%lld:%lld)",
		item->transBytes(), item->totalBytes(), m_totalProgress, m_totalBytes, total, out.size());

	if (out.size() != total) {
		dbg("PasteThread::copyFile(): out.size(%lld) != total(%lld)", out.size(), total);
		QString info = QString("[ERROR %1] File: \"%2\"").arg(PasteError::FileCopy).arg(item->srcPath());
		item->addErrorInfo(PasteError::FileCopy, info);
		m_errorCount ++;
	}

	if (m_type == PasteType::Move && m_errorCount == 0) {
		bool ok = QFile::remove(srcPath);
		if (!ok) m_errorCount ++;
		dbg("PasteThread::copyFile(): remove \"%s\" %s", qPrintable(srcPath), (ok ? "SUCCESS" : "FAILED"));
	}
}

//==============================================================================================================================

void PasteThread::copyFolder(const QString &srcPath, const QString &destPath, PasteItem *item)
{
	dbg("PasteThread::copyFolder(): src=\"%s\", dest=\"%s\"", qPrintable(srcPath), qPrintable(destPath));

	QFileInfo dest(destPath);
	QString destName = dest.fileName();
	QString baseFolder = dest.path();

	QDir destDir(baseFolder);
	if (!destDir.exists(destName)) {
		destDir.mkdir(destName);
		if (!destDir.exists(destName)) {
			QString info = QString("[ERROR %1] Failed to make folder: \"%2/%3\"")
						   .arg(PasteError::MakeFolder)
						   .arg(baseFolder)
						   .arg(destName);
			item->addErrorInfo(PasteError::MakeFolder, info);
			m_errorCount ++;
			m_stop  = true;
			dbg("PasteThread::copyFolder(): mkdir ERROR, destPath=\"%s\"", qPrintable(destPath));
			return;
		}
		dbg("PasteThread::copyFolder(): mkdir destPath=\"%s\"", qPrintable(destPath));
	}
	//--------------------------------------------------------------------------------------------------------------------------

	QDir srcDir(srcPath);
	QStringList files = srcDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);
	int cnt = files.count();
	destDir.cd(destName);

	for (int i = 0; i < cnt; i++) {
		if (isAborted()) return;

		QString srcPath = srcDir.path() + "/" + files[i];
		QString destPath = destDir.path() + "/" + files[i];
		dbg("PasteThread::copyFolder(): (dirs) srcPath=%s, destPath=%s", qPrintable(srcPath), qPrintable(destPath));
		copyFolder(srcPath, destPath, item);
	}

	if (isAborted()) return;
	//--------------------------------------------------------------------------------------------------------------------------

	files.clear();
	files = srcDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);
	cnt = files.count();

	for (int i = 0; i < cnt; i++) {
		QString srcPath = srcDir.path() + "/" + files[i];
		QString destPath = destDir.path() + "/" + files[i];
		dbg("PasteThread::copyFolder(): (files) srcPath=%s, destPath=%s", qPrintable(srcPath), qPrintable(destPath));
		copyFile(srcPath, destPath, item);

		if (isAborted()) return;
	}
	//--------------------------------------------------------------------------------------------------------------------------

	if (!item->hasError()) {
		int folderSize = QFileInfo(srcPath).size();
		item->addTransBytes(folderSize);
		m_totalProgress += folderSize;
		emit updateProgress(item, m_totalProgress);
	}
	//--------------------------------------------------------------------------------------------------------------------------

	if (m_type == PasteType::Move && m_errorCount == 0) {
		bool ok = QDir().rmdir(srcPath);
		if (!ok) m_errorCount ++;
		dbg("PasteThread::copyFolder(): rmdir=%d, srcPath=%s", ok, qPrintable(srcPath));
	}
}
//==============================================================================================================================

void PasteThread::copyItem(PasteItem *item)
{
	dbg("\nPasteThread::copyItem(): item.srcPath=%s", qPrintable(item->srcPath()));

	QString destPath = item->destFolder() + "/" + item->destName();
	QFileInfo dest(destPath);

	if (dest.exists()) {
		switch (item->solution()) {
			case PasteSolution::Skip:
				dbg("PasteThread::copyItem(): SKIP item.srcPath=%s", qPrintable(item->srcPath()));
				return;

			case PasteSolution::Merge:
				Q_ASSERT(dest.isDir());
				if (!item->isFolder()) destPath = FileUtils::buildRenamePath(destPath, dest.fileName());	// file into folder
				break;

			case PasteSolution::Rename:
				destPath = FileUtils::buildRenamePath(dest.path(), dest.fileName());
				break;

			case PasteSolution::Replace:
				FileUtils::removeRecurse(destPath);
				break;
		}
		dbg("PasteThread::copyItem(): destPath=%s", qPrintable(destPath));
	}
	//--------------------------------------------------------------------------------------------------------------------------

	qint64 totalBackup = m_totalProgress;

	if (m_type == PasteType::Move && QFile(item->srcPath()).rename(destPath)) {
		dbg("PasteThread::copyItem(): rename \"%s\" to \"%s\"", qPrintable(item->srcPath()), qPrintable(destPath));
	} else if (item->isFolder()) {
		copyFolder(item->srcPath(), destPath, item);
	} else {
		copyFile(item->srcPath(), destPath, item);
	}
	dbg("\nPasteThread::copyItem(): trans=%lld, total=%lld, srcPath=%s",
		item->transBytes(), item->totalBytes(), qPrintable(item->srcPath()));

	if (!isAborted() && item->transBytes() != item->totalBytes()) {
		item->setTransDone();
		m_totalProgress = totalBackup + item->totalBytes();
		emit updateProgress(item, m_totalProgress);
	}
	m_selFiles.append(destPath);
}
//==============================================================================================================================

void PasteThread::run()
{
	dbg("\nPasteThread::run().begin");
	m_totalBytes = m_totalProgress = 0;
	m_abort = m_stop = false;
	m_errorCount = 0;

	m_selFiles.clear();
	m_type = m_task->type();
	m_destFolder = m_task->destFolder();

	sumTotalBytes();

	QList<PasteItem*> &children = m_root->children();
	bool err = false;

	foreach (PasteItem *item, children) {
		if (item->solution() == PasteSolution::Skip) continue;

		copyItem(item);
		err = isAborted();
		if (err) break;
	}

	if (m_abort) m_errorCount ++;
	emit pasteFinished(m_errorCount, m_selFiles);
	dbg("PasteThread::run().end. err=%d, errCount=%d", err, m_errorCount);
}
//==============================================================================================================================

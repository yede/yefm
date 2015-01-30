#include <QFileInfo>
#include <QDirIterator>
#include <QMessageBox>
#include <QAbstractButton>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QFile>
# include <QtConcurrent>
#else
# include <QFSFileEngine>
# include <QtCore>
#endif

#include "yepastemgr.h"
#include "yeprogressdlg.h"
#include "yefswidget.h"
#include "yefsmodel.h"
#include "yeapp.h"
//==============================================================================================================================

PasteMgr::PasteMgr(FsModel *model, FsWidget *view, QObject *parent)
	: QObject(parent)
	, m_progressDlg(0)
	, m_view(view)
	, m_model(model)
{
	m_progressDlg = new ProgressDlg();
	connect(this, SIGNAL(updateCopyProgress(qint64, qint64, QString)),
			m_progressDlg, SLOT(update(qint64, qint64, QString)));
	connect(this, SIGNAL(pasteFinished(int, const QStringList &, const QStringList &)),
			this, SLOT(onPasteFinished(int, const QStringList &, const QStringList &)));
}
//==============================================================================================================================

namespace ErrCode {
	enum { NoErr, UnWritable, Cancelled, SomeError, FullError };
}
//==============================================================================================================================

static QString getErrMsg(const QStringList &errs)
{
	QString msg = QObject::tr("Paste failed.");
	if (errs.count()) {
		msg += "\n\n<b>" + QObject::tr("Failed items:") + "</b>";
		foreach (QString err, errs) { msg += "\n" + err; }
	}
	return msg;
}

void PasteMgr::onPasteFinished(int errCode, const QStringList &errs, const QStringList &files)
{
	m_progressDlg->close();
	delete m_progressDlg;

	if (files.count()) {
	//	if (QFile(Cfg::tmpFile()).exists())
	//		QApplication::clipboard()->clear();
		if (m_view != NULL) m_view->selectFiles(files);
		m_model->clearCutItems();
		QString root = m_model->rootPath();
		m_model->setRootPath("");	//changing rootPath forces reread, updates file sizes
		m_model->setRootPath(root);
	}

	bool hasMsg = false;
	QString title, text;

	switch (errCode) {
		case ErrCode::UnWritable:
			hasMsg = true;
			title = tr("Paste failed");
			text = tr("Can not write to destination");
			break;

		case ErrCode::Cancelled:
			hasMsg = errs.count() > 0;
			title = tr("Cancelled");
			text = getErrMsg(errs);
			break;

		case ErrCode::SomeError:
		case ErrCode::FullError:
			hasMsg = true;
			title = tr("Paste failed");
			text = getErrMsg(errs);
	}
	if (hasMsg) App::message(title, text);

	this->deleteLater();
}
//==============================================================================================================================

bool PasteMgr::pasteFile(QList<QUrl> files, QString newPath, QStringList cutList)
{
	QString title = (cutList.count() == 0) ? "Copying..." : "Moving...";
	m_progressDlg->setTitle(title);

	QStringList newFiles, errs;

	if (!QFileInfo(newPath).isWritable() || newPath == QDir(files.at(0).toLocalFile()).path()) {     //quit if folder not writable
		emit pasteFinished(ErrCode::UnWritable, errs, newFiles);
		return 0;
	}

	//get total size in bytes
	qint64 total = 1;
	foreach(QUrl url, files) {
		QFileInfo file = url.path();
		if (file.isFile()) total += file.size();
		else {
			QDirIterator it(url.path(), QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				it.next();
				total += it.fileInfo().size();
			}
		}
	}

	int code = ErrCode::NoErr;
	int erCnt = 0;
	int count = files.count();

	//main loop
	for (int i = 0; i < count; ++i) {
		if (m_progressDlg->result() == 1) {      //cancelled
			emit pasteFinished(ErrCode::Cancelled, errs, newFiles);
			return 1;
		}

		QFileInfo temp(files.at(i).toLocalFile());
		QString destName = temp.fileName();

		if (temp.path() == newPath) {       // only do 'Copy(x) of' if same folder
			int num = 1;
			while (QFile(newPath + "/" + destName).exists()) {
				destName = QString("Copy (%1) of %2").arg(num).arg(temp.fileName());
				num++;
			}
		}

		QString destFile = newPath + "/" + destName;
		QFileInfo dName(destFile);
		bool ok = true;

		if (!dName.exists() || dName.isDir()) {
			newFiles.append(destFile);                  //keep a list of new files so we can select them later

			if (cutList.contains(temp.filePath())) {            //cut action
				if (temp.isFile()) { //files

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
					QFile file(temp.filePath());
#else
					QFSFileEngine file(temp.filePath());
#endif
					if (!file.rename(destFile))             //rename will fail if across different filesystem, so use copy/remove method
						ok = cutCopyFile(temp.filePath(), destFile, total, true);
				}
				else {
					ok = QFile(temp.filePath()).rename(destFile);
					if (!ok) { //file exists or move folder between different filesystems, so use copy/remove method
						if (temp.isDir()) {
							ok = true;
							copyFolder(temp.filePath(), destFile, total, true);
							m_model->clearCutItems();
						}
						//file already exists, don't do anything
					}
				}
			}
			else {
				if (temp.isDir())
					copyFolder(temp.filePath(), destFile, total, false);
				else
					ok = cutCopyFile(temp.filePath(), destFile, total, false);
			}
		}

		if (!ok) {
			erCnt++;
			errs << destName;
		}
	}

	if      (erCnt == 0)     code = ErrCode::NoErr;
	else if (erCnt == count) code = ErrCode::FullError;
	else if (erCnt <  count) code = ErrCode::SomeError;

	emit pasteFinished(code, errs, newFiles);

	return 1;
}
//==============================================================================================================================

bool PasteMgr::copyFolder(QString sourceFolder, QString destFolder, qint64 total, bool cut)
{
	QDir sourceDir(sourceFolder);
	QDir destDir(QFileInfo(destFolder).path());
	QString folderName = QFileInfo(destFolder).fileName();

	bool ok = true;

	if (!QFileInfo(destFolder).exists()) destDir.mkdir(folderName);
	destDir = QDir(destFolder);

	QStringList files = sourceDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);

	for (int i = 0; i < files.count(); i++) {
		QString srcName = sourceDir.path() + "/" + files[i];
		QString destName = destDir.path() + "/" + files[i];
		if (!cutCopyFile(srcName, destName, total, cut)) ok = false;    //don't remove source folder if all files not cut

		if (m_progressDlg->result() == 1) return 0;                          //cancelled
	}

	files.clear();
	files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);

	for (int i = 0; i < files.count(); i++) {
		if (m_progressDlg->result() == 1) return 0;              //cancelled

		QString srcName = sourceDir.path() + "/" + files[i];
		QString destName = destDir.path() + "/" + files[i];
		copyFolder(srcName, destName, total, cut);
	}

	//remove source folder if all files moved ok
	if (cut && ok) sourceDir.rmdir(sourceFolder);
	return ok;
}
//==============================================================================================================================

bool PasteMgr::cutCopyFile(QString source, QString dest, qint64 totalSize, bool cut)
{
	QFile in(source);
	QFile out(dest);

	if (out.exists()) return 1; //file exists, don't do anything

	if (dest.length() > 50) dest = "/.../" + dest.split("/").last();

	in.open(QFile::ReadOnly);
	out.open(QFile::WriteOnly);

	char block[4096];
	qint64 total = in.size();
	qint64 steps = total >> 7;        //shift right 7, same as divide 128, much faster
	qint64 interTotal = 0;

	while (!in.atEnd()) {
		if (m_progressDlg->result() == 1) break;                //cancelled

		qint64 inBytes = in.read(block, sizeof(block));
		out.write(block, inBytes);
		interTotal += inBytes;

		if (interTotal > steps) {
			emit updateCopyProgress(interTotal, totalSize, dest);
			interTotal = 0;
		}
	}

	emit updateCopyProgress(interTotal, totalSize, dest);

	out.close();
	in.close();

	if (out.size() != total) return 0;
	if (cut) QFile::remove(source); //if file is cut remove the source

	return 1;
}
//==============================================================================================================================

static bool isValidPaste(const QString &destPath, const QList<QUrl> &srcFiles)
{
	QFileInfo info(destPath);
	if (!info.exists() || !info.isDir()) {
		App::message(QObject::tr("Folder of destination is not exists."));
		return false;
	}

	QString path;
	QString file;
	QChar sep('/');
	int pos;

	foreach(QUrl url, srcFiles) {
		path = url.path();
		info.setFile(path);

		if (!info.exists()) {
			App::message(QObject::tr("No paste available!"), QObject::tr("Some file no longer exists!"));
			return false;
		}

		if (info.isDir()) {
			if (!info.isSymLink() && path == destPath) {
				qDebug() << "PasteMgr: drop folder to itself.";
				return false;
			}
		}

		pos = path.lastIndexOf(sep) + 1;
		file = destPath + sep + path.mid(pos);
		if (file == path) {
			qDebug() << "PasteMgr: drop file/folder to same folder.";
			return false;
		}
	}

	return true;
}

static void recurseFolder(QString path, QString parent, QStringList *list)
{
	QDir dir(path);
	QStringList files = dir.entryList(QDir::AllEntries | QDir::Files |
									  QDir::NoDotAndDotDot | QDir::Hidden);
	for (int i = 0; i < files.count(); i++) {
		if (QFileInfo(files.at(i)).isDir())
			recurseFolder(files.at(i), parent + "/" + files.at(i), list);
		else list->append(parent + "/" + files.at(i));
	}
}

void PasteMgr::exec(const QString &destPath, const QMimeData *srcData, const QStringList &cutList,
					FsModel *model, FsWidget *view)
{
	QList<QUrl> files = srcData->urls();
	if (files.size() < 1) return;
	if (!isValidPaste(destPath, files)) return;

	int replace = 0;
	QStringList completeList;
	QString baseName = QFileInfo(files.at(0).toLocalFile()).path();

	if (destPath != baseName) {          //only if not in same directory, otherwise we will do 'Copy(x) of'
		foreach(QUrl file, files) {
			QFileInfo temp(file.toLocalFile());
			if (temp.isDir() && QFileInfo(destPath + "/" + temp.fileName()).exists()) { //merge or replace?
				QMessageBox message(QMessageBox::Question,
									"Existing folder",
									QString("<b>%1</b><p>Already exists!<p>What do you want to do?")
									.arg(destPath + "/" + temp.fileName()),
									QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

				message.button(QMessageBox::Yes)->setText("Merge");
				message.button(QMessageBox::No)->setText("Replace");

				int merge = message.exec();
				if (merge == QMessageBox::Cancel) return;
				if (merge == QMessageBox::Yes) recurseFolder(temp.filePath(), temp.fileName(), &completeList);
				else model->remove(model->index(destPath + "/" + temp.fileName()));
			}
			else completeList.append(temp.fileName());
		}

		foreach(QString file, completeList) {
			QFileInfo temp(destPath + "/" + file);
			if (temp.exists()) {
				QFileInfo orig(baseName + "/" + file);
				if (replace != QMessageBox::YesToAll && replace != QMessageBox::NoToAll)
					replace = QMessageBox::question(0, "Replace",
													QString("Do you want to replace:<p><b>%1</p><p>Modified: %2<br>Size: %3 bytes</p><p>with:<p><b>%4</p><p>Modified: %5<br>Size: %6 bytes</p>")
													.arg(temp.filePath()).arg(temp.lastModified().toString()).arg(temp.size())
													.arg(orig.filePath()).arg(orig.lastModified().toString()).arg(orig.size()),
													QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);

				if (replace == QMessageBox::Cancel) return;
				if (replace == QMessageBox::Yes || replace == QMessageBox::YesToAll)
					model->remove(model->index(temp.filePath()));
			}
		}
	}

	if (view != NULL) {
		if (view->workPath() != destPath) view = NULL;
	}
	PasteMgr *mgr = new PasteMgr(model, view);
	QtConcurrent::run(mgr, &PasteMgr::pasteFile, files, destPath, cutList);
}
//==============================================================================================================================

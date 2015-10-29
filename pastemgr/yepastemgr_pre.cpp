#include <QFileInfo>
#include <QDirIterator>
#include <QMessageBox>
#include <QAbstractButton>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QtConcurrent>
#else
# include <QtConcurrentRun>
#endif

#include "yepastemgr.h"
#include "yepastethread.h"
#include "yeprogressdlg.h"
#include "yefswidget.h"
#include "yefsmodel.h"
#include "yefileutils.h"
#include "yeapp.h"
//==============================================================================================================================

int PasteMgr::askForReplacing(const QFileInfo &f1, const QFileInfo &f2)
{
	QString tx = tr("Do you want to replace:"
					"<p><b>%1</b></p>"
					"<p>Modified: %2<br>Size: %3 bytes</p>"
					"<p>with:</p>"
					"<p><b>%4</b></p>"
					"<p>Modified: %5<br>Size: %6 bytes</p>")
				 .arg(f1.filePath()).arg(f1.lastModified().toString()).arg(f1.size())
				 .arg(f2.filePath()).arg(f2.lastModified().toString()).arg(f2.size());

	return QMessageBox::question(0, tr("Replace"), tx,
								 QMessageBox::Yes | QMessageBox::YesToAll |
								 QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
}

namespace PasteKind {
	enum { Dir2Dir, Dir2File, File2Dir, File2File };
}

namespace PasteExec {
	enum { None, Merge, Replace, Cancel };
}

int PasteMgr::getPasteMode(const QFileInfo &src, const QFileInfo &dest)
{
	if (src.isDir())
		return dest.isDir() ? PasteKind::Dir2Dir : PasteKind::Dir2File;
	return dest.isDir() ? PasteKind::File2Dir : PasteKind::File2File;
}

int PasteMgr::askForMerging(const QString &destPath, const QFileInfo &src)
{
	QString msg = tr("<b>%1</b><p>Already exists!<p>What do you want to do?")
				  .arg(destPath + QDir::separator() + src.fileName());
	QMessageBox message(QMessageBox::Question, tr("Existing folder"), msg,
						QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	message.button(QMessageBox::Yes)->setText(tr("Merge"));
	message.button(QMessageBox::No)->setText(tr("Replace"));

	return message.exec();
}

void PasteMgr::pasteLauncher(const QString &destPath, const QMimeData *srcData, const QStringList &cutList, FsWidget *view)
{
	Q_ASSERT(view != NULL);

	QList<QUrl> srcUrls = srcData->urls();
	if (srcUrls.size() < 1) return;
	if (!isValidPaste(destPath, srcUrls)) return;
	//--------------------------------------------------------------------------------------------------------------------------

	int replace = 0;
	QStringList mergeList, replaceList, completeList, removeDirs, removeFiles;
	QString srcPath = QFileInfo(srcUrls.at(0).toLocalFile()).path();

	if (destPath != srcPath)          // only if not in same directory, otherwise we will do 'Copy(x) of'
	{
		foreach (QUrl srcUrl, srcUrls)
		{
			QFileInfo src(srcUrl.toLocalFile());
			QFileInfo dest(destPath + "/" + src.fileName());

			if (dest.exists()) {
				int exec = PasteExec::None;
				int mode = getPasteMode(src, dest);
				switch (mode) {
					case PasteKind::Dir2Dir  : exec = prePasteDir2Dir  (src, dest, &replaceList, &mergeList); break;
					case PasteKind::Dir2File : exec = prePasteDir2File (src, dest); break;
					case PasteKind::File2Dir : exec = prePasteFile2Dir (src, dest); break;
					case PasteKind::File2File: exec = prePasteFile2File(src, dest); break;
				}

				if (exec == PasteExec::Cancel) return;
			}
		}

		foreach (QString file, completeList)
		{
			QFileInfo dest(destPath + "/" + file);
			if (dest.exists())
			{
				QFileInfo src(srcPath + "/" + file);
				if (replace != QMessageBox::YesToAll && replace != QMessageBox::NoToAll)
					replace = askForReplacing(dest, src);
				if (replace == QMessageBox::Cancel) return;

				if (replace == QMessageBox::YesToAll || replace == QMessageBox::Yes) {
					//model->remove(model->index(dest.filePath()));
					removeFiles.append(dest.filePath());
				}
			}
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------

	foreach (QString filePath, removeFiles) {
		//model->remove(model->index(filePath));
		QFile(filePath).remove();
	}
	foreach (QString dirName, removeDirs) {
		//model->remove(model->index(destPath + "/" + dirName));
		FileUtils::removeRecurse(destPath, dirName);
	}
	//--------------------------------------------------------------------------------------------------------------------------

	PasteMgr *mgr = new PasteMgr(view);
	QtConcurrent::run(mgr, &PasteMgr::pasteFiles, srcUrls, destPath, cutList);
}
//==============================================================================================================================

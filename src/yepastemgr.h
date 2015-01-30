#ifndef YE_PASTEMGR_H
#define YE_PASTEMGR_H

#include <QObject>
#include <QUrl>
#include <QStringList>
#include <QMimeData>
//==============================================================================================================================

class ProgressDlg;
class FsWidget;
class FsModel;

class PasteMgr : public QObject
{
    Q_OBJECT
public:
	explicit PasteMgr(FsModel *model, FsWidget *view, QObject *parent = 0);

	static void exec(const QString &destPath, const QMimeData *srcData, const QStringList &cutList,
					 FsModel *model, FsWidget *view);

signals:
	void updateCopyProgress(qint64, qint64, QString);
	void pasteFinished(int, const QStringList &, const QStringList &);

public slots:
	void onPasteFinished(int errCode, const QStringList &errs, const QStringList &files);
	bool pasteFile(QList<QUrl> files, QString newPath, QStringList cutList);

private:
	bool copyFolder(QString, QString, qint64, bool);
	bool cutCopyFile(QString source, QString dest, qint64 totalSize, bool cut);

private:
	ProgressDlg *m_progressDlg;
	FsWidget    *m_view;
	FsModel     *m_model;
};

#endif // YE_PASTEMGR_H

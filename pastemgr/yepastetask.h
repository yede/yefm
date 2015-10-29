#ifndef YE_PASTETASK_H
#define YE_PASTETASK_H

#include <QWidget>
#include <QLineEdit>
#include <QFileInfo>
#include <QStringList>
#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QModelIndex>
#include <QDateTime>
//==============================================================================================================================

class PasteMgr;
class PasteItem;
class PasteThread;
class PasteModel;
class PasteView;
class PasteTaskButton;

class PasteTask : public QWidget
{
	Q_OBJECT
public:
	PasteTask(const QStringList &srcFiles, const QString &destFolder, int type, QWidget *view, PasteMgr *mgr);
	~PasteTask();

	bool prepare();
	bool canStart() const;
	bool hasConflict() const;

	void startThread();
	void closeTask();
	void clear();
	void setCurrentItem(PasteItem *item);
	void setCurrentIndex(const QModelIndex &index);

	const QStringList &srcFiles()   const { return m_srcFiles; }
	const QString     &destFolder() const { return m_destFolder; }
	int                type()       const { return m_type; }
	bool isBusy() const;

	QString taskName();

	PasteMgr          *mgr()    const { return m_mgr; }
	PasteTaskButton   *button() const { return m_button; }

	const QColor &buttonColor() const { return m_buttonColor; }
	const QColor &activeColor() const { return m_activeColor; }

signals:
	void selectLater(const QStringList &selFiles);

public slots:
	void onSolution(bool checked);
	void startTask(bool checked);
	void abortTask(bool checked);
	void closeTask(bool checked);
	void closeAndHide();

	void totalBytesChanged(qint64 total);
	void updateProgress(PasteItem *item, qint64 totalProgress);
	void pasteFinished(int errorCount, const QStringList &selFiles);

private:
	int getConflict(const QFileInfo &src, const QFileInfo &dest);

	void disableSolutions();
	void enableSolutions(int conflict);
	void uncheckSolutions(int checkedSolution);
	void checkSolution(int solution);

	void addItems(const QStringList &srcFiles, const QString &destFolder, PasteItem *parent);
	void setupView();
	void setThreadState(int stat);
	void showTransferInfo(qint64 totalProgress);

private:
	friend class PasteThread;

	PasteMgr    *m_mgr;
	PasteThread *m_thread;
	int          m_threadStat;
	PasteItem   *m_root;
	PasteModel  *m_model;
	PasteView   *m_view;
	int          m_type;			// PasteMode: Copy, Move, Copy-folder-structure
	bool         m_busy;
	bool         m_show;

	bool dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;

	QTime        m_remainingTimer;
	qint64       m_totalBytes;

	QColor m_buttonColor;
	QColor m_activeColor;

	QStringList  m_srcFiles;
	QString      m_destFolder;

	QIcon m_iconSelected;
	QIcon m_iconUnSelect;

	PasteTaskButton *m_button;
	QLabel *m_lbSrcFolder;
	QLabel *m_lbDstFolder;
	QLabel *m_lbTransferInfo;

	QLabel *m_lbOperType;
	QLabel *m_lbConflict;
	QWidget     *m_frmSolution;
	QToolButton *m_tbSkip;
	QToolButton *m_tbRename;
	QToolButton *m_tbMerge;
	QToolButton *m_tbReplace;

	QPushButton *m_btCancel;
	QPushButton *m_btAbort;
	QPushButton *m_btClose;
	QPushButton *m_btStart;
};
//==============================================================================================================================

#endif // YE_PASTETASK_H

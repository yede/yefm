#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

#include "yepastetask.h"
#include "yepastetaskbutton.h"
#include "yepasteitem.h"
#include "yepastethread.h"
#include "yepastemodel.h"
#include "yepasteview.h"
#include "yepastemgr.h"
#include "yefileutils.h"

#include "yeapp.h"
#include "yeiconloader.h"
//==============================================================================================================================

namespace PasteTaskThreadStat {
	enum { Prepare, Copying, Finished };
}

PasteTask::PasteTask(const QStringList &srcFiles, const QString &destFolder, int type, QWidget *view, PasteMgr *mgr)
	: QWidget(NULL)
	, m_mgr(mgr)
	, m_thread(NULL)
	, m_threadStat(PasteTaskThreadStat::Prepare)
	, m_root(NULL)
	, m_model(NULL)
	, m_view(NULL)
	, m_type(type)
	, m_busy(false)
	, m_show(false)
	, m_ready(false)
	, m_srcFiles(srcFiles)
	, m_destFolder(destFolder)
	, m_iconSelected(":/option-selected")
	, m_iconUnSelect(":/option-unselect")
{
	m_root   = new PasteItem(true, NULL, NULL);
	m_button = new PasteTaskButton(this, mgr);

	m_buttonColor = palette().color(QPalette::Window);	// Light > Midlight > Button > Mid > Dark > Shadow
	m_activeColor = palette().color(QPalette::Button);

	connect(this, SIGNAL(selectLater(QStringList)), view, SLOT(selectLater(QStringList)));
}

PasteTask::~PasteTask()
{
	delete m_root;
}

void PasteTask::clear()
{
	m_root->clear();
}

void PasteTask::closeTask()
{
	m_mgr->removeTask(this);
	m_button->deleteLater();
	this->deleteLater();
}
//==============================================================================================================================

void PasteTask::setupView()
{
	if (m_ready) return;

	m_view = new PasteView(this);
	m_model = new PasteModel(m_view, m_root);

	connect(m_mgr, SIGNAL(iconThemeChanged()), m_view, SLOT(onIconThemeChanged()));
	//--------------------------------------------------------------------------------------------------------------------------

	QLabel *lbSrc = new QLabel(tr("<b>From Folder</b>"));
	QLabel *lbDst = new QLabel(tr("<b>To Folder</b>"));
//	QLabel *lbTransferInfo = new QLabel(tr("Transfer:"));

	lbSrc->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	lbDst->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
//	lbTransferInfo->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

	m_lbSrcFolder = new QLabel;
	m_lbSrcFolder->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_lbDstFolder = new QLabel;
	m_lbDstFolder->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_lbOperType = new QLabel;
	m_lbOperType->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_lbTransferInfo = new QLabel;
	m_lbTransferInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_lbTransferInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//	QSpacerItem *sp1 = new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Preferred);
//	QSpacerItem *sp2 = new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding);

	m_btCancel = new QPushButton(QIcon(":/close"), tr("Cancel"));
	m_btClose  = new QPushButton(QIcon(":/close"), tr("Close"));
	m_btAbort  = new QPushButton(IconLoader::icon("abort"), tr("Abort"));
	m_btStart  = new QPushButton(IconLoader::icon("start"), tr("Start"));

	connect(m_btCancel, SIGNAL(clicked(bool)), this, SLOT(closeTask(bool)));
	connect(m_btClose , SIGNAL(clicked(bool)), this, SLOT(closeTask(bool)));
	connect(m_btAbort , SIGNAL(clicked(bool)), this, SLOT(abortTask(bool)));
	connect(m_btStart , SIGNAL(clicked(bool)), this, SLOT(startTask(bool)));

	QVBoxLayout *btBox = new QVBoxLayout;
	btBox->setContentsMargins(0, 0, 0, 0);
	btBox->setSpacing(0);
	btBox->addWidget(m_btCancel);
	btBox->addWidget(m_btClose);
	btBox->addWidget(m_btAbort);
//	btBox->addSpacerItem(sp2);
	btBox->addWidget(m_btStart);

	QGridLayout *grid = new QGridLayout;
	grid->setContentsMargins(0, 6, 0, 6);
	grid->setHorizontalSpacing(6);
	grid->setVerticalSpacing(6);

	grid->addWidget(lbSrc,            0, 0);
	grid->addWidget(m_lbSrcFolder,    0, 1);
//	grid->addLayout(btBox,            0, 2, 4, 1);
	grid->addWidget(lbDst,            1, 0);
	grid->addWidget(m_lbDstFolder,    1, 1);
//	grid->addItem(  sp1,              2, 0, 1, 4);
	grid->addWidget(m_lbOperType,     2, 0);
	grid->addWidget(m_lbTransferInfo, 2, 1);

	QHBoxLayout *topBox = new QHBoxLayout;
	topBox->setContentsMargins(0, 0, 0, 0);
	topBox->setSpacing(0);
	topBox->addLayout(grid);
	topBox->addLayout(btBox);
	//--------------------------------------------------------------------------------------------------------------------------

	QLabel *lbConflict = new QLabel(tr("<b>Conflict</b>"));
	QLabel *lbSolution = new QLabel(tr("<b>Solution</b>"));

	lbConflict->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	lbSolution->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_lbConflict = new QLabel;
	m_lbConflict->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_tbSkip = new QToolButton();
	m_tbMerge = new QToolButton();
	m_tbRename = new QToolButton();
	m_tbReplace  = new QToolButton();

	m_tbSkip->setText(tr("Skip"));
	m_tbMerge->setText(tr("Merge"));
	m_tbRename->setText(tr("Rename"));
	m_tbReplace->setText(tr("Replace"));

	m_tbSkip->setIcon(m_iconUnSelect);
	m_tbMerge->setIcon(m_iconUnSelect);
	m_tbRename->setIcon(m_iconUnSelect);
	m_tbReplace->setIcon(m_iconUnSelect);

	m_tbSkip->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	m_tbMerge->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	m_tbRename->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	m_tbReplace->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	connect(m_tbSkip, SIGNAL(clicked(bool)), this, SLOT(onSolution(bool)));
	connect(m_tbMerge, SIGNAL(clicked(bool)), this, SLOT(onSolution(bool)));
	connect(m_tbRename, SIGNAL(clicked(bool)), this, SLOT(onSolution(bool)));
	connect(m_tbReplace, SIGNAL(clicked(bool)), this, SLOT(onSolution(bool)));

	QSpacerItem *sp3 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred);
//	QSpacerItem *sp4 = new QSpacerItem(6, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);

	m_frmSolution = new QWidget;
//	m_frmSolution->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	m_frmSolution->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QHBoxLayout *solutionBox = new QHBoxLayout(m_frmSolution);
	solutionBox->setContentsMargins(2, 1, 2, 1);
	solutionBox->setSpacing(3);
	solutionBox->addWidget(m_tbSkip);
	solutionBox->addWidget(m_tbRename);
	solutionBox->addWidget(m_tbMerge);
	solutionBox->addWidget(m_tbReplace);
	solutionBox->addSpacerItem(sp3);

	QGridLayout *grid2 = new QGridLayout;
	grid2->setContentsMargins(0, 0, 0, 0);
	grid2->setHorizontalSpacing(6);
	grid2->setVerticalSpacing(4);

	grid2->addWidget(lbConflict,    0, 0);
	grid2->addWidget(m_lbConflict,  0, 1);
//	grid2->addItem(  sp4,           0, 2, 2, 1);
//	grid2->addWidget(m_lbOperType,  0, 3, 2, 1);
	grid2->addWidget(lbSolution,    1, 0);
	grid2->addWidget(m_frmSolution, 1, 1);
	//--------------------------------------------------------------------------------------------------------------------------

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(6);
	vbox->addLayout(topBox);
	vbox->addWidget(m_view);
	vbox->addLayout(grid2);
	//--------------------------------------------------------------------------------------------------------------------------

	Q_ASSERT(m_srcFiles.size() > 0);

	m_view->updateColumeWidth();
	m_lbSrcFolder->setText(QFileInfo(m_srcFiles.at(0)).path());
	m_lbDstFolder->setText(m_destFolder);

	switch (m_type) {
		case PasteType::Copy: m_lbOperType->setText(tr("<b>Copy</b>")); break;
		case PasteType::Move: m_lbOperType->setText(tr("<b>Move</b>")); break;
	}

	m_ready = true;
}
//==============================================================================================================================

QString PasteTask::taskName()
{
	Q_ASSERT(m_srcFiles.size() > 0);

	return QFileInfo(m_srcFiles.at(0)).fileName();
}

void PasteTask::setCurrentIndex(const QModelIndex &index)
{
	PasteItem *item = m_model->itemAt(index);
	setCurrentItem(item);
}

void PasteTask::setCurrentItem(PasteItem *item)
{
	if (item == NULL || item == m_root) {
		m_lbConflict->clear();
		uncheckSolutions(PasteSolution::None);
		disableSolutions();
		return;
	}

	m_lbConflict->setText(item->getConflictText());
	enableSolutions(item->conflict());
	checkSolution(item->solution());
}

void PasteTask::onSolution(bool )
{
	PasteItem *item = m_view->currentItem();
	if (item == NULL) return;

	QToolButton *tb = qobject_cast<QToolButton*>(sender());
	int solution;

	if      (tb == m_tbSkip   ) solution = PasteSolution::Skip;
	else if (tb == m_tbMerge  ) solution = PasteSolution::Merge;
	else if (tb == m_tbRename ) solution = PasteSolution::Rename;
	else if (tb == m_tbReplace) solution = PasteSolution::Replace;
	else                        solution = PasteSolution::None;

	if (item->solution() == solution) {
		return;
	}

	checkSolution(solution);
	item->setSolution(solution);
	m_model->updateItem(item, PasteCol::Solution);
	m_model->updateItem(item, PasteCol::DestName);
}

void PasteTask::disableSolutions()
{
	m_tbSkip->setEnabled(false);
	m_tbMerge->setEnabled(false);
	m_tbRename->setEnabled(false);
	m_tbReplace->setEnabled(false);
}

void PasteTask::enableSolutions(int conflict)
{
	disableSolutions();

	switch (conflict) {
		case PasteConflict::Dir2Dir  : m_tbMerge->setEnabled(true);
		case PasteConflict::File2Dir :
		case PasteConflict::Dir2File :
		case PasteConflict::File2File:
			m_tbSkip->setEnabled(true);
			m_tbRename->setEnabled(true);
			m_tbReplace->setEnabled(true);
	}
}

void PasteTask::uncheckSolutions(int checkedSolution)
{
	if (checkedSolution != PasteSolution::Skip) m_tbSkip->setIcon(m_iconUnSelect);
	if (checkedSolution != PasteSolution::Merge) m_tbMerge->setIcon(m_iconUnSelect);
	if (checkedSolution != PasteSolution::Rename) m_tbRename->setIcon(m_iconUnSelect);
	if (checkedSolution != PasteSolution::Replace) m_tbReplace->setIcon(m_iconUnSelect);
}

void PasteTask::checkSolution(int solution)
{
	uncheckSolutions(solution);

	if      (solution == PasteSolution::Skip) m_tbSkip->setIcon(m_iconSelected);
	else if (solution == PasteSolution::Merge) m_tbMerge->setIcon(m_iconSelected);
	else if (solution == PasteSolution::Rename) m_tbRename->setIcon(m_iconSelected);
	else if (solution == PasteSolution::Replace) m_tbReplace->setIcon(m_iconSelected);
}
//==============================================================================================================================

void PasteTask::closeAndHide()
{
	m_mgr->hidePad();

	closeTask();
}

void PasteTask::closeTask(bool )
{
	closeAndHide();
}

void PasteTask::abortTask(bool )
{
//	qDebug() << "abort task";
	if (m_thread == NULL) return;

	m_thread->abort();
}

void PasteTask::startTask(bool )
{
	if (canStart()) {
		startThread();
	} else {
		App::message(tr("Unsolved conflict found."), m_mgr);
	}
}
//==============================================================================================================================

int PasteTask::getConflict(const QFileInfo &src, const QFileInfo &dest)
{
	if (src.isDir()) return dest.isDir() ? PasteConflict::Dir2Dir : PasteConflict::Dir2File;
	return dest.isDir() ? PasteConflict::File2Dir : PasteConflict::File2File;
}

void PasteTask::addItems(const QStringList &srcFiles, const QString &destFolder, PasteItem *parent)
{
	foreach (QString srcFile, srcFiles)
	{
		QFileInfo src(srcFile);
		QFileInfo dest(destFolder + "/" + src.fileName());

		PasteItem *item = new PasteItem(src.isDir(), this, parent);
		item->setSrcName(src.fileName());
		item->setSrcPath(src.filePath());
		item->setDestFolder(destFolder);

		parent->addChild(item);

		if (dest.exists()) {
			int conflict = getConflict(src, dest);
			item->setConflict(conflict);
		}
	}
}

bool PasteTask::hasConflict() const
{
	foreach (PasteItem *item, m_root->children()) {
		if (item->conflict() != PasteConflict::None) return true;
	}
	return false;
}

bool PasteTask::canStart() const
{
	foreach (PasteItem *item, m_root->children()) {
		if (item->conflict() == PasteConflict::None) continue;
		if (item->solution() == PasteSolution::None) return false;
	}
	return true;
}
//==============================================================================================================================

void PasteTask::showTransferInfo(qint64 totalProgress)
{
	int elapsed = m_remainingTimer.elapsed();							// msec
	if (elapsed == 0) return;

	float transferRate = (float) totalProgress * 1000.0 / elapsed;		// bytes per second
	float transferRateMB = transferRate / 1000000;						// convert to megabytes
	if (transferRate < 0.0001 || m_totalBytes == 0) return;

	int remaining = (m_totalBytes - totalProgress) / transferRate;
	int percent = totalProgress * 100.0 / m_totalBytes;

	QString time = remaining < 60 ? tr("%1 seconds").arg(remaining)
								  : tr("%1 min %2 sec").arg(remaining / 60).arg(remaining % 60);
	m_lbTransferInfo->setText(tr("%1% finished, speed %2 MB/s, remaining %3")
							  .arg(percent).arg(transferRateMB, 0, 'f', 1).arg(time));
	if (!m_show) {
		m_show = true;
		if (remaining > 1) m_mgr->showPad();
	}
}

void PasteTask::setThreadState(int stat)
{
	switch (stat) {
		case PasteTaskThreadStat::Prepare:
			m_btStart->setEnabled(true);
			m_btAbort->hide();
			m_btClose->hide();
			m_btCancel->setVisible(true);
			m_frmSolution->setEnabled(true);
			break;

		case PasteTaskThreadStat::Copying:
			m_btStart->setEnabled(false);
			m_btCancel->hide();
			m_btClose->hide();
			m_btAbort->setVisible(true);
			m_frmSolution->setEnabled(false);
			break;

		case PasteTaskThreadStat::Finished:
			m_btStart->setEnabled(false);
			m_btAbort->hide();
			m_btCancel->hide();
			m_btClose->setVisible(true);
			m_frmSolution->setEnabled(false);
			break;
	}

	m_threadStat = stat;
}

bool PasteTask::prepare()
{
	Q_ASSERT(m_srcFiles.size() > 0);

	addItems(m_srcFiles, m_destFolder, m_root);
	bool ok = m_root->rowCount() > 0;
	if (ok) {
		setupView();
		setThreadState(PasteTaskThreadStat::Prepare);
	}

	return ok;
}

void PasteTask::startThread()
{
	Q_ASSERT (m_thread == NULL);

	setThreadState(PasteTaskThreadStat::Copying);
	m_remainingTimer.start();
//	m_elapsedTime = 0;
	m_totalBytes = 0;
	m_busy = true;

	m_thread = new PasteThread(this);
	m_thread->start();
}
//==============================================================================================================================

void PasteTask::pasteFinished(int errorCount, const QStringList &selFiles)
{
	m_busy = false;
	setThreadState(PasteTaskThreadStat::Finished);

	if (selFiles.size() > 0) {
		emit selectLater(selFiles);
	}

	if (errorCount == 0) {
		showTransferInfo(m_totalBytes);
		QTimer::singleShot(3000, this, SLOT(closeAndHide()));
	}
}

void PasteTask::updateProgress(PasteItem *item, qint64 totalProgress)
{
	m_model->updateItem(item, PasteCol::Percent);
	showTransferInfo(totalProgress);
}

void PasteTask::totalBytesChanged(qint64 total)
{
	m_totalBytes = total;
}
//==============================================================================================================================

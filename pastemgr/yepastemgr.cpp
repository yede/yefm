#include <QUrl>
#include <QFileInfo>
#include <QDirIterator>
#include <QMessageBox>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QPainter>
#include <QDebug>

#include "yepastemgr.h"
#include "yepastetaskbutton.h"
#include "yepastetask.h"
#include "yepasteitem.h"

#include "yefileutils.h"
#include "yetoolitem.h"
#include "yetoolbar.h"

#include "yeapp.h"
#include "yeiconloader.h"
//==============================================================================================================================

PasteMgr::PasteMgr(QWidget *parent)
	: QDialog(parent)
{
	m_instance = this;
	setWindowTitle(tr("Paste Manager"));

//	int btPad = 4;
	int margin = 6;
	int barHeight = PasteTaskButton::buttonHeight();
	//--------------------------------------------------------------------------------------------------------------------------

	m_toolbar = new ToolBar;
	m_toolbar->setIconSize(16, 16);
	m_toolbar->setBasePads(0, 0, 0, 0);
	m_toolbar->setItemPads(3, 3, 3, 3);
	m_toolbar->setItemSpacing(0);
	m_toolbar->setupLayout();
	m_toolbar->setupTailLayout();

	m_toolbar->setFixedHeight(barHeight);
	m_toolbar->itemLayout()->setContentsMargins(0, 0, 0, 0);
	m_toolbar->tailLayout()->setContentsMargins(0, 0, 2, 0);

	m_toolbar->addTailIcon(QIcon(":/help-16"), tr("Help..."), this, SLOT(onHelp()));
	m_toolbar->addTailIcon(QIcon(":/down-16"), tr("Hide window"), this, SLOT(hidePad()));

	QPalette p = this->palette();
	m_lineColor = p.color(QPalette::Window).darker(120);
	p.setBrush(QPalette::Window, QBrush(m_lineColor));

	QWidget *line = new QWidget;
	line->setPalette(p);
	line->setAutoFillBackground(true);
	line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	line->setFixedHeight(1);

	QWidget *bar = new QWidget;
	bar->setAutoFillBackground(true);
	bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	bar->setFixedHeight(barHeight + 1);

	QVBoxLayout *barBox = new QVBoxLayout(bar);
	barBox->setContentsMargins(0, 0, 0, 1);
	barBox->setSpacing(0);
	barBox->addWidget(m_toolbar);
	barBox->addWidget(line);
	//--------------------------------------------------------------------------------------------------------------------------

	QVBoxLayout *stack = new QVBoxLayout;
	stack->setContentsMargins(margin, margin, margin, margin);

	m_stack = new QStackedLayout(stack);

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
	vbox->addWidget(bar);
	vbox->addLayout(stack);
}

PasteMgr::~PasteMgr()
{
	m_instance = NULL;
}
//==============================================================================================================================

void PasteMgr::setCurrentTask(PasteTask *task)
{
	int pos = m_stack->indexOf(task);
	if (pos >= 0) {
		int i = m_stack->count();
		while (i > 0) {
			i--;
			PasteTask *temp = qobject_cast<PasteTask*>(m_stack->widget(i));
			if (temp) temp->button()->setActive(i == pos);
		}
		m_stack->setCurrentIndex(pos);
	}
}

void PasteMgr::addTask(PasteTask *task)
{
	Q_ASSERT (task != NULL);

	m_toolbar->itemLayout()->addWidget(task->button());
	m_stack->addWidget(task);
}

void PasteMgr::removeTask(PasteTask *task)
{
	Q_ASSERT (task != NULL);

	m_toolbar->itemLayout()->removeWidget(task->button());
	m_stack->removeWidget(task);
}

void PasteMgr::showPad()
{
	static bool starting = true;

	if (starting) {
		starting = false;
		int w = 600;
		int h = 400;
		resize(w, h);
	}

	if (this->isMinimized()) {
		this->showNormal();
	} else {
		show();
	}
}

void PasteMgr::hidePad()
{
	showMinimized();
}

void PasteMgr::closeEvent(QCloseEvent *event)
{
	event->ignore();
	QTimer::singleShot(40, this, SLOT(hidePad()));
}
//==============================================================================================================================

bool PasteMgr::isBusy() const
{
	int i = m_stack->count();
	while (i > 0) {
		i--;
		PasteTask *task = qobject_cast<PasteTask*>(m_stack->widget(i));
		if (task && task->isBusy()) return true;
	}
	return false;
}

bool PasteMgr::isValidPaste(const QStringList &srcFiles, const QString &destFolder)
{
	if (srcFiles.size() < 1) return false;

	PasteMgr *mgr = PasteMgr::instance();

	QFileInfo dest(destFolder);
	if (!dest.exists() || !dest.isDir()) {
		App::message(tr("Destination folder not exists."), mgr);
		return false;
	}

	QChar sep('/');

	foreach (QString srcPath, srcFiles) {
		QFileInfo src(srcPath);
		if (!src.exists()) {
			App::message(tr("Some source files no longer exists."), mgr);
			return false;
		}

		if (src.isDir()) {
			if (!src.isSymLink() && srcPath == destFolder) {
				qDebug() << "PasteMgr: drop folder to itself.";
				return false;
			}
		}

		QString destPath = destFolder + sep + src.fileName();
		if (destPath == srcPath) {
			qDebug() << "PasteMgr: drop file/folder to same folder.";
			return false;
		}
	}

	return true;
}

void PasteMgr::launcher(const QMimeData *srcData, const QString &destFolder, int type, QWidget *view)
{
	Q_ASSERT(view != NULL);

	QList<QUrl> srcUrls = srcData->urls();
	QStringList srcFiles;
	foreach (QUrl url, srcUrls) srcFiles.append(url.toLocalFile());

	if (!isValidPaste(srcFiles, destFolder)) return;

	PasteMgr *mgr = PasteMgr::instance();

	PasteTask *task = new PasteTask(srcFiles, destFolder, type, view, mgr);
	if (!task->prepare()) {
		delete task;
		return;
	}

	mgr->addTask(task);
	mgr->setCurrentTask(task);

	if (task->hasConflict()) {
		mgr->showPad();
	} else {
		task->startThread();
	}
}
//==============================================================================================================================

void PasteMgr::onHelp()
{
	qDebug() << "help";
}

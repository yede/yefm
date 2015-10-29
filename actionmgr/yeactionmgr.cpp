#include <stdio.h>

#include <QContextMenuEvent>
#include <QProcess>
#include <QMenu>
#include <QDebug>

#include "yeactionmgr.h"
#include "yedefaultactions.h"
#include "yecustomactions.h"
#include "yemimeactions.h"

#include "yefswidgetdelegate.h"
#include "yeshortcuts.h"
#include "yeappcfg.h"
//==============================================================================================================================

ActionMgr::ActionMgr(QObject *parent)
	: QObject(parent)
	, m_defaultActions(NULL)
	, m_customActions(NULL)
	, m_mimeActions(NULL)
	, m_ctx(NULL)
{
}

ActionMgr::~ActionMgr()
{
	clear();
}

void ActionMgr::clear()
{
	if (m_defaultActions) { delete m_defaultActions; m_defaultActions = NULL; }
	if (m_customActions ) { delete m_customActions;  m_customActions  = NULL; }
	if (m_mimeActions   ) { delete m_mimeActions;    m_mimeActions    = NULL; }
}

void ActionMgr::resetCustomActions()
{
	if (m_customActions) m_customActions->reset();
}

DefaultActions *ActionMgr::getDefaultActions()
{
	if (m_defaultActions == NULL) m_defaultActions = new DefaultActions(this);
	return m_defaultActions;
}

CustomActions *ActionMgr::getCustomActions()
{
	if (m_customActions == NULL) m_customActions = new CustomActions(this);
	return m_customActions;
}

MimeActions *ActionMgr::getMimeActions()
{
	if (m_mimeActions == NULL) m_mimeActions = new MimeActions(this);
	return m_mimeActions;
}
//==============================================================================================================================

void ActionMgr::dispatchDefaultAction(int actionId)
{
	if (m_ctx != NULL) m_ctx->dispatchDefaultAction(actionId);
}

int ActionMgr::getSelectedFiles(QStringList &files, QString &workDir) const
{
	return m_ctx == NULL ? 0 : m_ctx->getSelectedFiles(files, workDir);
}

void ActionMgr::openFiles(QString exe, const QStringList &files, const QString &workDir, QObject *processOwner)
{
	QStringList split = exe.split(" ");
	if (split.size() < 1) return;

	QStringList args;
	QString name = split.takeAt(0);
	bool ok = false;

	foreach (QString arg, split) {
		QString tmp = arg.toLower();
		if (tmp == "%f" || tmp == "%u") {
			if (!ok) {
				args.append(files);
				ok = true;
			}
		} else {
			args.append(arg);
		}
	}

	if (!ok) {
		args.append(files);
	}

	if (AppCfg::instance()->dumpOpenFilesCmd) {
		printf("\n================================ open-with:\n");
		printf("exec: \"%s\"\n", qPrintable(name));
		for (int i = 0; i < args.size(); i++)
			printf("arg%d: \"%s\"\n", i, qPrintable(args.at(i)));
		printf("================================ end.\n");
	}

	QProcess *myProcess = new QProcess(processOwner);
	myProcess->startDetached(name, args, workDir);
	myProcess->waitForFinished(1000);
}

void ActionMgr::openWith()
{
	QString workDir;
	QStringList files;
	int cnt = getSelectedFiles(files, workDir);
	if (cnt < 1) return;

	QAction* action = dynamic_cast<QAction*>(sender());
	if (action) {
		openFiles(action->data().toString(), files, workDir, this);
	}
}
//==============================================================================================================================

bool ActionMgr::handleKeyAction(FsWidgetDelegate_key &d)
{
	ShortcutMgr *mgr = ShortcutMgr::instance();
	ShortcutGroup *grp = mgr->findGroup(DefaultActions::groupName());

	if (grp) {
		ShortcutItem *item = grp->find(d.shortcut());
		if (item) {
			int actionId = item->actionId;
			d.handleAction(actionId);
			return true;
		}
	}

	return false;
}
//==============================================================================================================================
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

void ActionMgr::showContextMenu(FsWidgetDelegate_ctx &d)
{
	QString path = d.workPath();
	QFileInfo info = QFileInfo(path);
	if (!info.exists()) return;
	//--------------------------------------------------------------------------------------------------------------------------

	QModelIndex index = d.getIndex();		// the index will become currentIndex
	if (!index.isValid()) {
		d.clearSelection();							// it would not clear in this case
		d.clearCurrentIndex();						//
	}
	//--------------------------------------------------------------------------------------------------------------------------

	bool isDirWritable = info.isWritable();
	bool hasSel = d.hasSelection();

	bool hasCurrFile = index.isValid();
	bool canPaste = QApplication::clipboard()->mimeData()->hasUrls();
	bool canRun = false;
	bool isWritable = false;

	if (hasCurrFile) {
		info = d.getFileInfo(index);
		canRun = info.isExecutable() && !info.isDir();
		isWritable = info.isWritable();
		canPaste = canPaste && info.isDir() && isWritable;
	} else {
		canPaste = canPaste && isDirWritable;
	}
	//--------------------------------------------------------------------------------------------------------------------------

	DefaultActions *defs = getDefaultActions();
	CustomActions  *cust = getCustomActions();
	MimeActions    *mime = getMimeActions();

	defs->createActions();
	cust->createActions();
	//--------------------------------------------------------------------------------------------------------------------------

	defs->m_actionNewFile->setEnabled(isDirWritable);
	defs->m_actionNewFolder->setEnabled(isDirWritable);
	defs->m_actionCut->setEnabled(isWritable && isDirWritable && hasSel);
	defs->m_actionCopy->setEnabled(hasSel);
	defs->m_actionPaste->setEnabled(canPaste);
	defs->m_actionDelete->setEnabled(isWritable && isDirWritable);
	defs->m_actionRename->setEnabled(isWritable);
	defs->m_actionProperties->setEnabled(true);
	//--------------------------------------------------------------------------------------------------------------------------

	QMenu menu;

	if (hasCurrFile) {
		if (canRun) {
			menu.addAction(defs->m_actionActivate);
			menu.addSeparator();
		}
		mime->addActions(menu, info);
		cust->addActions(menu, info);
		menu.addAction(defs->m_actionCut);
		menu.addAction(defs->m_actionCopy);
		menu.addAction(defs->m_actionPaste);
		menu.addSeparator();
		menu.addAction(defs->m_actionDelete);
		menu.addSeparator();
		menu.addAction(defs->m_actionRename);
	} else {
		cust->addActions(menu, info);
		menu.addAction(defs->m_actionNewFile);
		menu.addAction(defs->m_actionNewFolder);
		menu.addSeparator();
		menu.addAction(defs->m_actionPaste);
		menu.addSeparator();
	}

	menu.addAction(defs->m_actionProperties);
//	menu.addAction(m_actionTest);

	m_ctx = &d;
	menu.exec(d.event()->globalPos());
	m_ctx = NULL;
}

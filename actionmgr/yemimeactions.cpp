#include <QMenu>
#include <QDebug>

#include "yemimeactions.h"
#include "yeactionmgr.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
#include "yeappdelegate.h"

#include "yemime.h"
#include "yefileutils.h"
#include "yedesktopfile.h"
//==============================================================================================================================

MimeActions::MimeActions(QObject *parent)
	: QObject(parent)
	, m_mime(Mime::instance())
	, m_ready(false)
{
}

MimeActions::~MimeActions()
{
}
//==============================================================================================================================

void MimeActions::selectApp()
{
	QString workDir;
	QStringList files;

	ActionMgr *m = ActionMgr::instance();
	if (m->getSelectedFiles(files, workDir) < 1) return;

	QString appName;
	QString mimeType = m_mime->getMimeType(files.at(0));

	AppDelegate *a = AppDelegate::instance();
	if (!a->showMimeDlg(appName, mimeType)) return;

	DesktopFile df = DesktopFile("/usr/share/applications/" + appName);
	m->openFiles(df.getExec(), files, workDir, this);
}

QAction *MimeActions::createAction(const QString &appName, QObject *parent, bool isOpenWith)
{
	DesktopFile df = DesktopFile("/usr/share/applications/" + appName);
	QString name = df.getName();
	QString exec = df.getExec();

	if (name.isEmpty() || exec.isEmpty()) {
		return NULL;
	}

	if (isOpenWith)
		name = tr("Open with %1").arg(name);

	QAction *action = new QAction(IconLoader::appIcon(df), name, parent);
	action->setData(exec);

	ActionMgr *m = ActionMgr::instance();
	connect(action, SIGNAL(triggered()), m, SLOT(openWith()));

	return action;
}

void MimeActions::addActions(QMenu &menu, const QFileInfo &current)
{
	if (current.isDir()) return;

	QMenu *mimeMenu = new QMenu(&menu);

	QString mime = m_mime->getMimeType(current.filePath());
	QStringList appNames = m_mime->getDefault(mime);

	QAction *selectAppAct = new QAction(tr("Select... (mime-type: %1)").arg(mime), mimeMenu);
	selectAppAct->setStatusTip(tr("Select application for opening the file"));
	connect(selectAppAct, SIGNAL(triggered()), this, SLOT(selectApp()));

	QAction *a;
	int      n = 0;

	foreach (QString appName, appNames) {
		if (appName.isEmpty()) continue;

		a = createAction(appName, mimeMenu, n == 0);
		if (a == NULL) continue;

		if (n == 0) {
			menu.addAction(a);
		} else {
			mimeMenu->addAction(a);
		}

		n++;
	}

	if (n > 1) mimeMenu->addSeparator();
	mimeMenu->addAction(selectAppAct);
	mimeMenu->setTitle(n > 0 ? tr("More") : tr("Open with"));

	a = menu.addMenu(mimeMenu);
	a->setIcon(IconLoader::icon("document-open"));
	menu.addSeparator();
//	qDebug() << a->parent() << mimeMenu->parent() << mimeMenu << &menu;
}
//==============================================================================================================================

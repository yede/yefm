#include <QContextMenuEvent>
#include <QClipboard>
#include <QProcess>
#include <QMimeData>
#include <QDebug>

#include "yefsactions.h"
#include "yefsactionsdlg.h"
#include "yefswidget.h"

#include "yeapplication.h"
#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"

#include "yemime.h"
#include "yefileutils.h"
//==============================================================================================================================

FsActions::FsActions(YeApplication *app, QObject *parent)
	: QObject(parent)
	, m_app(app)
	, m_mime(app->mime())
	, m_view(NULL)
	, m_defaultActionsReady(false)
	, m_customActionsReady(false)
{
	connect(app, SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));
}

FsActions::~FsActions()
{
	if (m_customActions.size() > 0) clearCustomActions();
}
//==============================================================================================================================

void FsActions::clearCustomActions()
{
	QHash<QAction *, CustomActionData>::iterator i = m_customActions.begin();
	while (i != m_customActions.end()) {
		QAction *action = i.key();
		delete action;
		++i;
	}
	m_customActions.clear();
}

void FsActions::createCustomActions()
{
	if (m_customActions.size() > 0) clearCustomActions();

	QStringList items;
	FsActionsDlg::loadCustomActions(items);

	QString name, exec, icon, types;
	int kind, match;
	QAction *action;
	QStringList list;

	foreach (QString item, items) {
		if (FsActionsDlg::decodeCustomItem(kind, name, exec, icon, match, types, item)) {
			action = FsActionsDlg::isDesktopApp(kind) ? createCustomAction(name)
													  : createCustomAction(name, exec, icon);
			if (action != NULL) {
				FsActionsDlg::decodeCustomTypes(list, types);
				m_customActions.insert(action, CustomActionData(match, list));
			}
		}
	}

	m_customActionsReady = true;
}
//==============================================================================================================================

bool FsActions::hasDuplicateAction(QMenu &menu, const QAction *action)
{
	QList<QAction *> list = menu.actions();
	QString exec = action->data().toString();

	foreach (QAction *item, list) {
		if (item->data().toString() == exec) return true;
	}

	return false;
}

bool FsActions::addCustomAction(QMenu &menu, QAction *action)
{
	if (hasDuplicateAction(menu, action)) return false;

	menu.addAction(action);
	return true;
}

void FsActions::addCustomActions(QMenu &menu, const QFileInfo &current)
{
	bool flag = false;
	bool isDir = current.isDir();
	QString ext = isDir ? QString() : current.suffix().toLower();
	QHash<QAction *, CustomActionData>::const_iterator i = m_customActions.constBegin();

	while (i != m_customActions.constEnd())
	{
		const CustomActionData &d = i.value();
	//	qDebug() << "FsActions::addCustomActions" << d.type << d.types << ext;

		if (isDir) {
			if (FsActionsDlg::matchDir(d.match) || FsActionsDlg::matchAny(d.match)) {
				if (addCustomAction(menu, i.key())) flag = true;
			}
		} else {
			if (FsActionsDlg::matchFile(d.types, ext) || FsActionsDlg::matchAnyFile(d.match) || FsActionsDlg::matchAny(d.match)) {
				if (addCustomAction(menu, i.key())) flag = true;
			}
		}

		++i;
	}

	if (flag) {
		menu.addSeparator();
	}
}
//==============================================================================================================================

QAction *FsActions::createCustomAction(const QString &appName)
{
	DesktopFile df = DesktopFile("/usr/share/applications/" + appName + ".desktop");
	QString name = df.getName();
	QString exec = df.getExec();

	if (name.isEmpty() || exec.isEmpty()) {
		return NULL;
	}

	QAction *action = new QAction(R::appIcon(df), name, this);
	action->setData(exec);
	connect(action, SIGNAL(triggered()), SLOT(openWithApp()));

	return action;
}

QAction *FsActions::createCustomAction(const QString &title, const QString &exec, const QString &icon)
{
	if (title.isEmpty() || exec.isEmpty()) return NULL;

	QAction *action = new QAction(R::appIcon(icon), title, this);
	action->setData(exec);
	connect(action, SIGNAL(triggered()), SLOT(openWithApp()));

	return action;
}

void FsActions::openFiles(QString exe, const QStringList &files, const QString &workDir, QObject *processOwner)
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
//	qDebug() << name << args;

	QProcess *myProcess = new QProcess(processOwner);
	myProcess->startDetached(name, args, workDir);
	myProcess->waitForFinished(1000);
}
//==============================================================================================================================

void FsActions::openWithApp()
{
	QString workDir;
	QStringList files;
	if (m_view == NULL || m_view->getSelectedFiles(files, workDir) < 1) return;

	QAction* action = dynamic_cast<QAction*>(sender());
	if (action) {
		openFiles(action->data().toString(), files, workDir, this);
	}
}

void FsActions::selectMimeApp()
{
	QString workDir;
	QStringList files;
	if (m_view == NULL || m_view->getSelectedFiles(files, workDir) < 1) return;

	QString appName;
	QString mimeType = m_mime->getMimeType(files.at(0));
	if (!m_app->showMimeDlg(appName, mimeType)) return;

	DesktopFile df = DesktopFile("/usr/share/applications/" + appName);
	openFiles(df.getExec(), files, workDir, this);
}

QAction *FsActions::createMimeAction(const QString &appName, QObject *parent, bool isOpenWith)
{
	DesktopFile df = DesktopFile("/usr/share/applications/" + appName);
	QString name = df.getName();
	QString exec = df.getExec();

	if (name.isEmpty() || exec.isEmpty()) {
		return NULL;
	}

	if (isOpenWith)
		name = tr("Open with %1").arg(name);

	QAction *action = new QAction(R::appIcon(df), name, parent);
	action->setData(exec);
	connect(action, SIGNAL(triggered()), SLOT(openWithApp()));

	return action;
}

void FsActions::addMimeActions(QMenu &menu, const QFileInfo &current)
{
	if (current.isDir()) return;

	QMenu *mimeMenu = new QMenu(&menu);

	QString mime = m_mime->getMimeType(current.filePath());
	QStringList appNames = m_mime->getDefault(mime);

	QAction *selectAppAct = new QAction(tr("Select... (mime-type: %1)").arg(mime), mimeMenu);
	selectAppAct->setStatusTip(tr("Select application for opening the file"));
	connect(selectAppAct, SIGNAL(triggered()), this, SLOT(selectMimeApp()));

	QAction *a;
	int      n = 0;

	foreach (QString appName, appNames) {
		if (appName.isEmpty()) continue;

		a = createMimeAction(appName, mimeMenu, n == 0);
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
	a->setIcon(R::icon("document-open"));
	menu.addSeparator();
//	qDebug() << a->parent() << mimeMenu->parent() << mimeMenu << &menu;
}
//==============================================================================================================================

void FsActions::onIconThemeChanged()
{
	updateIconTheme();
}

void FsActions::updateIconTheme()
{
	if (!m_defaultActionsReady) return;

	m_actionNewFile->setIcon(R::menuIcon("document-new"));
	m_actionNewFolder->setIcon(R::menuIcon("folder-new"));
	m_actionRun->setIcon(R::menuIcon("system-run"));
	m_actionCut->setIcon(R::menuIcon("edit-cut"));
	m_actionCopy->setIcon(R::menuIcon("edit-copy"));
	m_actionPaste->setIcon(R::menuIcon("edit-paste"));
	m_actionDelete->setIcon(R::menuIcon("edit-delete"));
	m_actionRename->setIcon(R::menuIcon("edit-rename"));
	m_actionProperties->setIcon(R::menuIcon("document-properties"));
	m_actionTest->setIcon(R::menuIcon("edit-find"));
}
//==============================================================================================================================

void FsActions::createDefaultActions()
{
	YeApplication &a = *m_app;

	m_actionNewFile = new QAction(tr("New file"), this);
	m_actionNewFolder = new QAction(tr("New folder"), this);
	m_actionRun = new QAction(tr("Run"), this);
	m_actionCut = new QAction(tr("Cut"), this);
	m_actionCopy = new QAction(tr("Copy"), this);
	m_actionPaste = new QAction(tr("Paste"), this);
	m_actionDelete = new QAction(tr("Delete"), this);
	m_actionRename = new QAction(tr("Rename"), this);
	m_actionProperties = new QAction(tr("Properties"), this);
	m_actionTest = new QAction(tr("test..."), this);

	m_actionCut->setShortcut(a.getFileViewShortcut("Cut"));
	m_actionCopy->setShortcut(a.getFileViewShortcut("Copy"));
	m_actionPaste->setShortcut(a.getFileViewShortcut("Paste"));
	m_actionDelete->setShortcut(a.getFileViewShortcut("Delete"));
	m_actionRename->setShortcut(a.getFileViewShortcut("Rename"));
//	m_actionProperties->setShortcut(a.getFileViewShortcut("Properties"));

	connect(m_actionNewFile, SIGNAL(triggered()), this, SLOT(onContextMenuNewFile()));
	connect(m_actionNewFolder, SIGNAL(triggered()), this, SLOT(onContextMenuNewFolder()));
	connect(m_actionRun, SIGNAL(triggered()), this, SLOT(onContextMenuRun()));
	connect(m_actionCut, SIGNAL(triggered()), this, SLOT(onContextMenuCut()));
	connect(m_actionCopy, SIGNAL(triggered()), this, SLOT(onContextMenuCopy()));
	connect(m_actionPaste, SIGNAL(triggered()), this, SLOT(onContextMenuPaste()));
	connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(onContextMenuDelete()));
	connect(m_actionRename, SIGNAL(triggered()), this, SLOT(onContextMenuRename()));
	connect(m_actionProperties, SIGNAL(triggered()), this, SLOT(onContextMenuProperties()));
	connect(m_actionTest, SIGNAL(triggered()), this, SLOT(onContextMenuTest()));

	m_defaultActionsReady = true;
	updateIconTheme();
}

void FsActions::onContextMenuNewFile()    { m_view->handleNewFile();    }
void FsActions::onContextMenuNewFolder()  { m_view->handleNewFolder();  }
void FsActions::onContextMenuRun()        { m_view->handleRun();        }
void FsActions::onContextMenuCut()        { m_view->handleCut();        }
void FsActions::onContextMenuCopy()       { m_view->handleCopy();       }
void FsActions::onContextMenuPaste()      { m_view->handlePaste();      }
void FsActions::onContextMenuDelete()     { m_view->handleDelete();     }
void FsActions::onContextMenuRename()     { m_view->handleRename();     }
void FsActions::onContextMenuProperties() { m_view->handleProperties(); }
void FsActions::onContextMenuTest()       { m_view->handleTest();       }
//==============================================================================================================================

void FsActions::doShowContextMenu(QContextMenuEvent *event)
{
	QFileInfo info = QFileInfo(m_view->workPath());
	if (!info.exists()) return;
	//--------------------------------------------------------------------------------------------------------------------------

	QModelIndex index = m_view->getIndexAt(event->pos());	// the index will become currentIndex
	if (!index.isValid()) {
		m_view->clearSelection();							// it would not clear in this case
		m_view->clearCurrentIndex();						//
	}
	//--------------------------------------------------------------------------------------------------------------------------

	bool isDirWritable = info.isWritable();
	bool hasSel = m_view->selCount() > 0;

	bool hasCurrFile = index.isValid();
	bool canPaste = QApplication::clipboard()->mimeData()->hasUrls();
	bool canRun = false;
	bool isWritable = false;

	if (hasCurrFile) {
		info = m_view->getFileInfo(index);
		canRun = info.isExecutable() && !info.isDir();
		isWritable = info.isWritable();
		canPaste = canPaste && info.isDir() && isWritable;
	} else {
		canPaste = canPaste && isDirWritable;
	}
	//--------------------------------------------------------------------------------------------------------------------------

	if (!m_defaultActionsReady) createDefaultActions();
	if (!m_customActionsReady)  createCustomActions();
	//--------------------------------------------------------------------------------------------------------------------------

	m_actionNewFile->setEnabled(isDirWritable);
	m_actionNewFolder->setEnabled(isDirWritable);
	m_actionCut->setEnabled(isWritable && isDirWritable && hasSel);
	m_actionCopy->setEnabled(hasSel);
	m_actionPaste->setEnabled(canPaste);
	m_actionDelete->setEnabled(isWritable && isDirWritable);
	m_actionRename->setEnabled(isWritable);
	m_actionProperties->setEnabled(true);
	//--------------------------------------------------------------------------------------------------------------------------

	QMenu menu;

	if (hasCurrFile) {
		if (canRun) {
			menu.addAction(m_actionRun);
			menu.addSeparator();
		}
		addMimeActions(menu, info);
		addCustomActions(menu, info);
		menu.addAction(m_actionCut);
		menu.addAction(m_actionCopy);
		menu.addAction(m_actionPaste);
		menu.addSeparator();
		menu.addAction(m_actionDelete);
		menu.addSeparator();
		menu.addAction(m_actionRename);
	} else {
		addCustomActions(menu, info);
		menu.addAction(m_actionNewFile);
		menu.addAction(m_actionNewFolder);
		menu.addSeparator();
		menu.addAction(m_actionPaste);
		menu.addSeparator();
	}

	menu.addAction(m_actionProperties);
//	menu.addAction(m_actionTest);

	menu.exec(event->globalPos());
}

void FsActions::showContextMenu(FsWidget *view, QContextMenuEvent *event)
{
	Q_ASSERT(view != NULL);

	m_view = view;
	doShowContextMenu(event);
	m_view = NULL;
}
//==============================================================================================================================

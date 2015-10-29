#include <QMetaObject>
#include <QDebug>

#include "yedefaultactions.h"
#include "yeactionmgr.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
#include "yeshortcuts.h"

#include "yemime.h"
#include "yefileutils.h"
#include "yedesktopfile.h"
//==============================================================================================================================

DefaultActions::DefaultActions(QObject *parent)
	: QObject(parent)
	, m_ready(false)
{
}

DefaultActions::~DefaultActions()
{
}
//==============================================================================================================================

QString DefaultActions::groupName()
{
	return staticMetaObject.className();
}

void DefaultActions::setupShortcuts()
{
	ShortcutMgr *m = ShortcutMgr::instance();
	ShortcutGroup *grp = m->addGroup(DefaultActions::groupName());

	grp->add("Return", "Activate",   DefaultAction::Activate);
	grp->add("Alt+Up", "GoParent",   DefaultAction::GoParent);
	grp->add("Ctrl+N", "NewFile",    DefaultAction::NewFile);
	grp->add("Ctrl+D", "NewFolder",  DefaultAction::NewFolder);
	grp->add("Ctrl+X", "Cut",        DefaultAction::Cut);
	grp->add("Ctrl+C", "Copy",       DefaultAction::Copy);
	grp->add("Ctrl+V", "Paste",      DefaultAction::Paste);
	grp->add("Del",    "Delete",     DefaultAction::Delete);
	grp->add("F2",     "Rename",     DefaultAction::Rename);
	grp->add("F8",     "Properties", DefaultAction::Properties);
	grp->add("F9",     "Testing",    DefaultAction::Testing);
}
//==============================================================================================================================

void DefaultActions::updateIconTheme()
{
	if (!m_ready) return;

	m_actionNewFile->setIcon(IconLoader::menuIcon("document-new"));
	m_actionNewFolder->setIcon(IconLoader::menuIcon("folder-new"));
	m_actionActivate->setIcon(IconLoader::menuIcon("system-run"));
	m_actionCut->setIcon(IconLoader::menuIcon("edit-cut"));
	m_actionCopy->setIcon(IconLoader::menuIcon("edit-copy"));
	m_actionPaste->setIcon(IconLoader::menuIcon("edit-paste"));
	m_actionDelete->setIcon(IconLoader::menuIcon("edit-delete"));
	m_actionRename->setIcon(IconLoader::menuIcon("edit-rename"));
	m_actionProperties->setIcon(IconLoader::menuIcon("document-properties"));
	m_actionTest->setIcon(IconLoader::menuIcon("edit-find"));
}

void DefaultActions::createActions()
{
	if (m_ready) return;

	m_actionActivate   = newAction(tr("Run"),        DefaultAction::Activate);
	m_actionNewFile    = newAction(tr("New file"),   DefaultAction::NewFile);
	m_actionNewFolder  = newAction(tr("New folder"), DefaultAction::NewFolder);
	m_actionCut        = newAction(tr("Cut"),        DefaultAction::Cut);
	m_actionCopy       = newAction(tr("Copy"),       DefaultAction::Copy);
	m_actionPaste      = newAction(tr("Paste"),      DefaultAction::Paste);
	m_actionDelete     = newAction(tr("Delete"),     DefaultAction::Delete);
	m_actionRename     = newAction(tr("Rename"),     DefaultAction::Rename);
	m_actionProperties = newAction(tr("Properties"), DefaultAction::Properties);
	m_actionTest       = newAction(tr("test..."),    DefaultAction::Testing);

	m_ready = true;
	updateIconTheme();
	connect(App::app(), SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
}

QAction *DefaultActions::newAction(const QString &label, int actionId)
{
	QAction *act = new QAction(label, this);
	act->setData(actionId);

	ShortcutMgr *m = ShortcutMgr::instance();
	ShortcutGroup *grp = m->findGroup(DefaultActions::groupName());
	if (grp) {
		ShortcutItem *item = grp->find(actionId);
		if (item) act->setShortcut(item->keySequence());
	}

	connect(act, SIGNAL(triggered()), this, SLOT(triggered()));

	return act;
}

void DefaultActions::triggered()
{
	QAction *act = qobject_cast<QAction*>(sender());
	int actionId = act->data().toInt();

	ActionMgr *m = ActionMgr::instance();
	m->dispatchDefaultAction(actionId);
}
//==============================================================================================================================

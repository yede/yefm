#include <stdio.h>

#include "yeshortcuts.h"
//==============================================================================================================================

ShortcutItem::ShortcutItem(const QString &shortcut, const QString &text, int action)
{
	init(shortcut, text, action);
}

void ShortcutItem::init(const QString &shortcut, const QString &text, int action)
{
	this->shortcut = shortcut;
	this->text     = text;
	this->actionId = action;
}

//==============================================================================================================================
// class ShortcutGroup
//==============================================================================================================================

ShortcutGroup::ShortcutGroup(const QString &groupName)
	: m_groupName(groupName)
{}

ShortcutGroup::~ShortcutGroup()
{
	if (!m_items.isEmpty()) clear();
}

void ShortcutGroup::clear()
{
	foreach (ShortcutItem *item, m_items) { delete item; }
	m_items.clear();
	m_index.clear();
}

void ShortcutGroup::dump()
{
	qDebug("\ngroupName: %s", qPrintable(m_groupName));
	foreach (ShortcutItem *item, m_items) {
		qDebug("text: %s, shortcut: %s, actionId: %d",
			   qPrintable(item->text), qPrintable(item->shortcut), item->actionId);
	}
}

ShortcutItem *ShortcutGroup::add(const QString &shortcut, const QString &text, int actionId)
{
	Q_ASSERT(actionId >= 0);

	ShortcutItem *item = find(shortcut);

	if (item == NULL) {
		item = new ShortcutItem(shortcut, text, actionId);
		m_items.insert(shortcut, item);
		m_index.insert(actionId, item);
	} else {
		item->init(shortcut, text, actionId);
	}

	return item;
}

//==============================================================================================================================
// class ShortcutMgr
//==============================================================================================================================

ShortcutMgr::ShortcutMgr()
{
}

ShortcutMgr::~ShortcutMgr()
{
	if (!m_groups.isEmpty()) clear();
}

void ShortcutMgr::clear()
{
	foreach (ShortcutGroup *grp, m_groups) { delete grp; }
	m_groups.clear();
}

void ShortcutMgr::dump()
{
	qDebug("group count: %d", m_groups.size());
	foreach (ShortcutGroup *grp, m_groups) { grp->dump(); }
}
//==============================================================================================================================

ShortcutGroup *ShortcutMgr::addGroup(const QString &groupName)
{
	ShortcutGroup *grp = findGroup(groupName);
	if (grp == NULL) {
		grp = new ShortcutGroup(groupName);
		m_groups.append(grp);
	}
	return grp;
}

ShortcutGroup *ShortcutMgr::findGroup(const QString &groupName)
{
	foreach (ShortcutGroup *grp, m_groups) {
		if (grp->groupName() == groupName) return grp;
	}
	return NULL;
}
//==============================================================================================================================

int ShortcutMgr::findActionId(const QString &groupName, QKeyEvent *event)
{
	QString shortcut = ShortcutMgr::shortcut(event);
	return shortcut.isEmpty() ? -1 : findActionId(groupName, shortcut);
}

int ShortcutMgr::findActionId(const QString &groupName, const QString &shortcut)
{
	ShortcutGroup *grp = findGroup(groupName);
	if (grp != NULL) {
		ShortcutItem *item = grp->find(shortcut);
		if (item != NULL) return item->actionId;
	}

	return -1;
}

QString ShortcutMgr::findShortcut(const QString &groupName, int actionId)
{
	ShortcutGroup *grp = findGroup(groupName);
	if (grp != NULL) {
		ShortcutItem *item = grp->find(actionId);
		if (item != NULL) return item->shortcut;
	}

	return QString();
}
//==============================================================================================================================

void ShortcutMgr::dump(int key, Qt::KeyboardModifiers mod)
{
	const char *str = "";

	if (key == Qt::Key_Control) str = "Key_Control";
	if (key == Qt::Key_Shift  ) str = "Key_Shift";
	if (key == Qt::Key_Alt    ) str = "Key_Alt";
	if (key == Qt::Key_Meta   ) str = "Key_Meta";
	if (key == Qt::Key_AltGr  ) str = "Key_AltGr";
	if (key == Qt::Key_unknown) str = "Key_unknown";

	int shift = (mod & Qt::ShiftModifier)   ? 1 : 0;
	int ctrl  = (mod & Qt::ControlModifier) ? 1 : 0;
	int alt   = (mod & Qt::AltModifier)     ? 1 : 0;

	if (strlen(str) > 0)
		printf("key: 0x%x (%s), Shift: %d, Ctrl: %d, Alt: %d\n", key, str, shift, ctrl, alt);
	else
		printf("key: 0x%x, Shift: %d, Ctrl: %d, Alt: %d\n", key, shift, ctrl, alt);
}

void ShortcutMgr::dump(QKeyEvent *event)
{
	int key = event->key();
	Qt::KeyboardModifiers mod = event->modifiers();
	dump(key, mod);
}

QString ShortcutMgr::shortcut(QKeyEvent *event)
{
	int key = event->key();
	Qt::KeyboardModifiers mod = event->modifiers();
	return shortcut(key, mod);
}

QString ShortcutMgr::shortcut(int key, Qt::KeyboardModifiers mod)
{
//	if ((key == Qt::Key_Enter || key == Qt::Key_Return) && mod == Qt::NoModifier) {
	//	onItemActivated(m_widget->getCurrentIndex());
	//	event->accept();
//		return QString();
//	}

	if (key == Qt::Key_Control ||
		key == Qt::Key_Shift   ||
		key == Qt::Key_Alt     ||
		key == Qt::Key_Meta    ||
		key == Qt::Key_AltGr   ||
		key == Qt::Key_unknown)
	{
		return QString();
	}

	if (mod & Qt::ShiftModifier)   key += Qt::SHIFT;
	if (mod & Qt::ControlModifier) key += Qt::CTRL;
	if (mod & Qt::AltModifier)     key += Qt::ALT;

	return QKeySequence(key).toString(QKeySequence::NativeText);
//	return QKeySequence(key).toString(QKeySequence::PortableText);
}
//==============================================================================================================================

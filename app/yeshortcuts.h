#ifndef YE_SHORTCUTS_H
#define YE_SHORTCUTS_H

#include <QString>
#include <QHash>
#include <QList>
#include <QKeyEvent>
#include <QKeySequence>
//==============================================================================================================================

struct ShortcutItem
{
	ShortcutItem(const QString &shortcut, const QString &text, int action);

	void init(const QString &shortcut, const QString &text, int action);

	QKeySequence keySequence() const { return QKeySequence(shortcut); }

	QString shortcut;
	QString text;
	int     actionId;
};
//==============================================================================================================================

class ShortcutGroup
{
public:
	ShortcutGroup(const QString &groupName);
	~ShortcutGroup();

	void clear();
	void dump();

	ShortcutItem *add(const QString &shortcut, const QString &text, int actionId);
	ShortcutItem *find(const QString &shortcut) { return m_items.value(shortcut, NULL); }
	ShortcutItem *find(int actionId)            { return m_index.value(actionId, NULL); }

	const QString                 &groupName() const { return m_groupName; }
	QHash<QString, ShortcutItem*> &items()           { return m_items; }

private:
	QString                       m_groupName;
	QHash<QString, ShortcutItem*> m_items;
	QHash<int,     ShortcutItem*> m_index;
};
//==============================================================================================================================

class ShortcutMgr
{
public:
	ShortcutMgr();
	~ShortcutMgr();

	void clear();
	void dump();

	ShortcutGroup *addGroup(const QString &groupName);
	ShortcutGroup *findGroup(const QString &groupName);

	/**
	 * @return -1 if not found
	 */
	int     findActionId(const QString &groupName, QKeyEvent *event);
	int     findActionId(const QString &groupName, const QString &shortcut);
	QString findShortcut(const QString &groupName, int actionId);

	static void dump(QKeyEvent *event);
	static void dump(int key, Qt::KeyboardModifiers mod);

	static QString shortcut(QKeyEvent *event);
	static QString shortcut(int key, Qt::KeyboardModifiers mod);

	static ShortcutMgr *instance() { return m_instance; }
	static void createInstance()   { if (!m_instance) m_instance = new ShortcutMgr; }
	static void deleteInstance()   { delete m_instance; m_instance = NULL; }

private:
	QList<ShortcutGroup*> m_groups;

	static ShortcutMgr   *m_instance;
};
//==============================================================================================================================

#endif

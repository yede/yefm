#include <QSettings>
#include <QMenu>
#include <QDebug>

#include "yecustomactiondefines.h"
#include "yecustomactions.h"
#include "yeactionmgr.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"

#include "yemime.h"
#include "yefileutils.h"
#include "yedesktopfile.h"
//==============================================================================================================================

CustomActions::CustomActions(QObject *parent)
	: QObject(parent)
	, m_ready(false)
{
}

CustomActions::~CustomActions()
{
	if (m_items.size() > 0) clear();
}
//==============================================================================================================================

void CustomActions::load(QStringList &result)
{
	QSettings s(App::getCustomActionsFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	result = s.value("items").toStringList();
	s.endGroup();
}

void CustomActions::save(const QStringList &items)
{
	QSettings s(App::getCustomActionsFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	s.setValue("items", items);
	s.endGroup();
}
//==============================================================================================================================

void CustomActions::decodeTypes(QStringList &result, QString &types)
{
	QChar sep = QChar(',');
	result = types.split(sep);
}

bool CustomActions::decode(int &kind, QString &name, QString &exec, QString &icon,
						   int &match, QString &types, const QString &item)
{
	QChar sep = QChar('|');

	int i = item.indexOf(sep, 0);      if (i <  0) return false;
	int j = item.indexOf(sep, i + 1);  if (j <= i) return false;
	int k = item.indexOf(sep, j + 1);  if (k <= j) return false;

	name  = item.left(i);
	exec  = item.mid(i + 1, j - i - 1);
	icon  = item.mid(j + 1, k - j - 1);
	types = item.mid(k + 1);

	kind  = decodeKind(name);
	match = decodeMatch(types);

	return true;
}
//==============================================================================================================================

void CustomActions::encode(QString &item, int kind, const QString &name, const QString &exec, const QString &icon,
						   int match, const QString &types)
{
	QChar sep = QChar('|');
	QString tmp = types;
	insertTypeTag(tmp, match);
	item = (kind == FsActionKind::DesktopApp) ? QString(APP_TAG) : QString(EXE_TAG);
	item.append(name + sep + exec + sep + icon + sep + tmp);
}
//==============================================================================================================================

bool CustomActions::matchFile(const QStringList &types, const QString &ext)
{
	int cnt = types.size();
	for (int i = 0; i < cnt; i++) {
		const QString &type = types.at(i);
		if (type == ext) return true;
	}
	return false;
}

bool CustomActions::matchDir(int match)     { return match == FsActionMatch::Folder;   }
bool CustomActions::matchAny(int match)     { return match == FsActionMatch::Any;      }
bool CustomActions::matchAnyFile(int match) { return match == FsActionMatch::AnyFile;  }
bool CustomActions::isDesktopApp(int kind)  { return kind  == FsActionKind::DesktopApp; }

int CustomActions::decodeKind(QString &name)
{
	int n = QString(APP_TAG).length();
	QString kind = name.left(n);
	name = name.mid(n);

	return kind == QString(APP_TAG) ? FsActionKind::DesktopApp : FsActionKind::DirectExec;
}

int CustomActions::decodeMatch(QString &types)
{
	int n = QString(FILE_TAG).length();
	QString match = types.left(n);
	types = types.mid(n);

	if (match == QString(FOLDER_TAG))   return FsActionMatch::Folder;
	if (match == QString(ANY_FILE_TAG)) return FsActionMatch::AnyFile;
	if (match == QString(ANY_TAG))      return FsActionMatch::Any;

	return FsActionMatch::File;
}

void CustomActions::insertTypeTag(QString &types, int match)
{
	switch (match) {
		case FsActionMatch::Folder : types.prepend(QString(FOLDER_TAG  )); break;
		case FsActionMatch::File   : types.prepend(QString(FILE_TAG    )); break;
		case FsActionMatch::AnyFile: types.prepend(QString(ANY_FILE_TAG)); break;
		case FsActionMatch::Any    : types.prepend(QString(ANY_TAG     )); break;
	}
}
//==============================================================================================================================

QAction *CustomActions::createAction(const QString &appName)
{
	DesktopFile df = DesktopFile("/usr/share/applications/" + appName + ".desktop");
	QString name = df.getName();
	QString exec = df.getExec();

	if (name.isEmpty() || exec.isEmpty()) {
		return NULL;
	}

	QAction *action = new QAction(IconLoader::appIcon(df), name, this);
	action->setData(exec);

	ActionMgr *m = ActionMgr::instance();
	connect(action, SIGNAL(triggered()), m, SLOT(openWith()));

	return action;
}

QAction *CustomActions::createAction(const QString &title, const QString &exec, const QString &icon)
{
	if (title.isEmpty() || exec.isEmpty()) return NULL;

	QAction *action = new QAction(IconLoader::appIcon(icon), title, this);
	action->setData(exec);

	ActionMgr *m = ActionMgr::instance();
	connect(action, SIGNAL(triggered()), m, SLOT(openWith()));

	return action;
}

void CustomActions::createActions()
{
	if (m_ready) return;
	if (m_items.size() > 0) clear();

	QStringList items;
	load(items);

	QString name, exec, icon, types;
	int kind, match;
	QAction *action;
	QStringList list;

	foreach (QString item, items) {
		if (decode(kind, name, exec, icon, match, types, item)) {
			action = isDesktopApp(kind) ? createAction(name)
										: createAction(name, exec, icon);
			if (action != NULL) {
				decodeTypes(list, types);
				m_items.insert(action, UsrAction(match, list));
			}
		}
	}

	m_ready = true;
}

void CustomActions::clear()
{
	QHash<QAction*, UsrAction>::iterator i = m_items.begin();
	while (i != m_items.end()) {
		QAction *action = i.key();
		delete action;
		++i;
	}
	m_items.clear();
}
//==============================================================================================================================

bool CustomActions::hasDuplicateAction(QMenu &menu, const QAction *action)
{
	QList<QAction *> list = menu.actions();
	QString exec = action->data().toString();

	foreach (QAction *item, list) {
		if (item->data().toString() == exec) return true;
	}
	return false;
}

bool CustomActions::addAction(QMenu &menu, QAction *action)
{
	if (hasDuplicateAction(menu, action)) return false;

	menu.addAction(action);
	return true;
}

void CustomActions::addActions(QMenu &menu, const QFileInfo &fileInfo)
{
	bool flag = false;
	bool isDir = fileInfo.isDir();
	QString ext = isDir ? QString() : fileInfo.suffix().toLower();
	QHash<QAction*, UsrAction>::const_iterator i = m_items.constBegin();

	while (i != m_items.constEnd())
	{
		const UsrAction &d = i.value();
	//	qDebug() << "FsActions::addCustomActions" << d.type << d.types << ext;

		if (isDir) {
			if (matchDir(d.match) || matchAny(d.match)) {
				if (addAction(menu, i.key())) flag = true;
			}
		} else {
			if (matchFile(d.types, ext) || matchAnyFile(d.match) || matchAny(d.match)) {
				if (addAction(menu, i.key())) flag = true;
			}
		}

		++i;
	}

	if (flag) {
		menu.addSeparator();
	}
}
//==============================================================================================================================

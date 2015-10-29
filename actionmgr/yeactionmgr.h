#ifndef YE_ACTION_MGR_H
#define YE_ACTION_MGR_H

#include <QObject>
//==============================================================================================================================

class FsWidgetDelegate_key;
class FsWidgetDelegate_ctx;

class DefaultActions;
class CustomActions;
class MimeActions;

class ActionMgr : public QObject
{
	Q_OBJECT
public:
	explicit ActionMgr(QObject *parent = 0);
	virtual ~ActionMgr();

	void clear();
	void resetCustomActions();

	void showContextMenu(FsWidgetDelegate_ctx &d);
	bool handleKeyAction(FsWidgetDelegate_key &d);

	int getSelectedFiles(QStringList &files, QString &workDir) const;
	void openFiles(QString exe, const QStringList &files, const QString &workDir, QObject *processOwner);
	void dispatchDefaultAction(int actionId);

	static DefaultActions *defaultActions() { return m_instance->getDefaultActions(); }
	static CustomActions  *customActions()  { return m_instance->getCustomActions(); }
	static MimeActions    *mimeActions()    { return m_instance->getMimeActions(); }

	static ActionMgr *instance() { return m_instance; }
	static void createInstance() { if (!m_instance) m_instance = new ActionMgr; }
	static void deleteInstance() { delete m_instance; m_instance = NULL; }

private:
	DefaultActions *getDefaultActions();
	CustomActions  *getCustomActions();
	MimeActions    *getMimeActions();

signals:

public slots:
	void openWith();

private:
	DefaultActions *m_defaultActions;
	CustomActions  *m_customActions;
	MimeActions    *m_mimeActions;

	FsWidgetDelegate_ctx *m_ctx;

	static ActionMgr *m_instance;
};

#endif // YE_ACTION_MGR_H

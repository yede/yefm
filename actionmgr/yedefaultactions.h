#ifndef YE_DEFAULT_ACTIONS_H
#define YE_DEFAULT_ACTIONS_H

#include <QObject>
#include <QAction>
#include <QHash>
//==============================================================================================================================

namespace DefaultAction {
	enum {
		Activate,	// enter folder, run program file, open non-program file
		GoParent,	// go to parent folder
		NewFile,	// create new file
		NewFolder,	// create new folder
		Cut,
		Copy,
		Paste,
		Delete,
		Rename,
		Properties,
		Testing
	};
}

class DefaultActions : public QObject
{
	Q_OBJECT
public:
	explicit DefaultActions(QObject *parent = 0);
	~DefaultActions();

	void createActions();

	bool isReady() const { return m_ready; }

	static QString groupName();
	static void setupShortcuts();

private:
	QAction *newAction(const QString &label, int actionId);

signals:
	void activated(int actionId);

public slots:
	void updateIconTheme();
	void triggered();

private:
	friend class ActionMgr;

	bool m_ready;
	bool m_dummy_1, m_dummy_2, m_dummy_3, m_dummy_4, m_dummy_5, m_dummy_6, m_dummy_7;

	QAction *m_actionNewFile;
	QAction *m_actionNewFolder;
	QAction *m_actionActivate;
	QAction *m_actionCut;
	QAction *m_actionCopy;
	QAction *m_actionPaste;
	QAction *m_actionDelete;
	QAction *m_actionRename;
	QAction *m_actionProperties;
	QAction *m_actionTest;
};
//==============================================================================================================================

#endif // YE_DEFAULT_ACTIONS_H

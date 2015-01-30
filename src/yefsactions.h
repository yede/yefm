#ifndef FSACTIONS_H
#define FSACTIONS_H

#include <QObject>
#include <QAction>
//==============================================================================================================================

class QFileInfo;

class YeApplication;
class Mime;
class FsWidget;

class FsActions : public QObject
{
	Q_OBJECT
public:
	explicit FsActions(YeApplication *app, QObject *parent = 0);
	~FsActions();

	void showContextMenu(FsWidget *view, QContextMenuEvent *event);

	void resetCustomActions() { m_customActionsReady = false; }

private:
	void updateIconTheme();
	void clearCustomActions();
	void createDefaultActions();
	void createCustomActions();
	void openFiles(QString exe, const QStringList &files, const QString &workDir, QObject *processOwner = 0);
	void doShowContextMenu(QContextMenuEvent *event);

	bool hasDuplicateAction(QMenu &menu, const QAction *action);
	bool addCustomAction(QMenu &menu, QAction *action);
	void addCustomActions(QMenu &menu, const QFileInfo &current);
	void addMimeActions(QMenu &menu, const QFileInfo &current);

	QAction *createCustomAction(const QString &appName);
	QAction *createCustomAction(const QString &title, const QString &exec, const QString &icon);
	QAction *createMimeAction(const QString &appName, QObject *parent, bool isOpenWith);

signals:

public slots:
	void selectMimeApp();
	void openWithApp();
	void onIconThemeChanged();

	void onContextMenuNewFile();
	void onContextMenuNewFolder();
	void onContextMenuRun();
	void onContextMenuCut();
	void onContextMenuCopy();
	void onContextMenuPaste();
	void onContextMenuDelete();
	void onContextMenuRename();
	void onContextMenuProperties();
	void onContextMenuTest();

private:
	YeApplication *m_app;
	Mime          *m_mime;
	FsWidget      *m_view;
	bool           m_defaultActionsReady;
	bool           m_customActionsReady;

	QAction *m_actionNewFile;
	QAction *m_actionNewFolder;
	QAction *m_actionRun;
	QAction *m_actionCut;
	QAction *m_actionCopy;
	QAction *m_actionPaste;
	QAction *m_actionDelete;
	QAction *m_actionRename;
	QAction *m_actionProperties;
	QAction *m_actionTest;

private:
	struct CustomActionData {
		CustomActionData(int match, const QStringList &types)
			: match(match), types(types)
		{}

		int         match;
		QStringList types;
	};

	QHash<QAction *, CustomActionData> m_customActions;
};

#endif // FSACTIONS_H

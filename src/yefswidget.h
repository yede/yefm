#ifndef YE_FSWIDGET_H
#define YE_FSWIDGET_H

#include <QModelIndex>
#include <QWidget>
//==============================================================================================================================

namespace FsWidgetMode {
	enum { List, Tree };
}

namespace FsHoverArea {
	enum { None, Icon, Text };
}

class QStackedLayout;
class QItemSelectionModel;
class QItemSelection;
class QFileInfo;

class YeApplication;
class YeMainWindow;
class YeFilePane;
class FsListView;
class FsTreeView;
class TreeNode;
class FsModel;
class FsListModel;
class FsHandler;

class FsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FsWidget(YeFilePane *pane, TreeNode *tab, QWidget *parent = 0);
	~FsWidget();

	void updateColSizes();
	void saveColSizes();

	void goUp();
	void refresh();

	int  getHoverArea() const;
	bool canHoverSelect(bool isDir);
	void clearHoverState();

	void selectFile(const QString &path, bool ensureVisible = false);
	void selectFiles(const QStringList &paths);
	void clearSelection();
	void clearCurrentIndex();

	void setDockPane(YeFilePane *newPane);
	void setViewMode(int mode);
	void setWorkPath(const QString &path);
	void setHiddenVisible(bool visible);
	void setCurrentIndex(const QModelIndex &index);

	bool getCurrentFilePath(QString &result) const;
	bool getCurrentFileInfo(QFileInfo &result) const;
	int  getSelectedFiles(QStringList &files, QString &workDir) const;
	int  getSelectedFiles(QStringList &result) const;
	int  selCount() const;
	bool hasSelection() const;

	QModelIndex     getIndexAt(const QPoint &p) const;
	QModelIndex     getCurrentIndex()           const;
	QModelIndexList getSelection()              const;
	QModelIndexList getSourceSelection()        const;
	QString         getPastePath()              const;
	QFileInfo       getFileInfo(const QModelIndex &index) const;

	YeApplication *app()  const { return m_app; }
	YeFilePane    *pane() const { return m_pane; }
	TreeNode      *tab()  const { return m_tab; }

	int                  viewMode()  const { return m_viewMode; }
	QString              workPath()  const { return m_workPath; }
	FsModel             *model()     const { return m_model; }
	FsListModel         *sortModel() const { return m_sortModel; }
	QItemSelectionModel *seleModel() const { return m_seleModel; }

	FsHandler    *handler()  const { return m_handler; }
	FsTreeView   *fileTree() const { return m_fileTree; }
	FsListView   *fileList() const { return m_fileList; }

private:
	void createFileTreeView();
	void doSetWorkPath(const QString &path);

signals:

public slots:
	void updateIconTheme();
	void updateSettings();
	void updateHiddenState(bool visible);

	void handleNewFile();
	void handleNewFolder();
	void handleRun();
	void handleCut();
	void handleCopy();
	void handlePaste();
	void handleDelete();
	void handleRename();
	void handleProperties();
	void handleTest();

private:
	YeApplication *m_app;
	YeFilePane    *m_pane;
	TreeNode      *m_tab;
	bool           m_hiddenVisible;
	bool           m_pathIsHidden;
	FsTreeView    *m_fileTree;
	FsListView    *m_fileList;
	int            m_viewMode;

	FsHandler *m_handler;

	FsModel             *m_model;
	FsListModel         *m_sortModel;
	QItemSelectionModel *m_seleModel;
	QItemSelectionModel *m_idleModel;
	QString              m_workPath;

	QStackedLayout *m_fileStack;
//	QStackedWidget *m_toolStack;

//	FsListView *m_seleList;

private:

	struct History {
		History(const QString &path, int pos, const QString &file)
			: scrollPos(pos), selectedFile(file), workPath(path) {}

		void update(int pos, const QString &file) { scrollPos = pos; selectedFile = file; }

		int     scrollPos;
		QString selectedFile;
		QString workPath;
	};

	QList<History*> m_history;

	History *findHistory(const QString &path);
	History *takeHistory(const QString &path);
	void clearHistory();
	void saveHistory();
	void loadHistory();
};

#endif // YE_FSWIDGET_H

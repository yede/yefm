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

class FilePaneDelegate;
class FsListView;
class FsTreeView;
class TreeNode;
class FsModel;
class FsSortModel;
class FsHandler;

class FsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FsWidget(TreeNode *tab, QWidget *parent = 0);
	~FsWidget();

	void updateColSizes();
	void saveColSizes();

	int  getHoverArea() const;
	bool canHoverSelect(bool isDir);
	void clearHoverState();
	void showTooltip(const QString &tips);
	void validatePath();

	void selectFile(const QString &path, bool ensureVisible = false);
	void selectFiles(const QStringList &paths);
	void clearSelection();
	void clearCurrentIndex();

	void dispatchDefaultAction(int actionId);

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
	bool isHiddenVisible() const;

	QModelIndex     getIndexAt(const QPoint &p) const;
	QModelIndex     getCurrentIndex()           const;
	QModelIndexList getSelection()              const;
	QModelIndexList getSourceSelection()        const;
	QString         getPastePath()              const;
	QFileInfo       getFileInfo(const QModelIndex &index) const;

	void setPane(FilePaneDelegate *pane);
	void setEditing(bool v)        { m_editing = v; }
	FilePaneDelegate *pane() const { return m_pane; }
	TreeNode         *tab()  const { return m_tab;  }

	int                  viewMode()  const { return m_viewMode; }
	QString              workPath()  const { return m_workPath; }
	FsModel             *model()     const { return m_model; }
	FsSortModel         *sortModel() const { return m_sortModel; }
	QItemSelectionModel *seleModel() const { return m_seleModel; }

	FsHandler  *handler()  const { return m_handler; }
	FsTreeView *fileTree() const { return m_fileTree; }
	FsListView *fileList() const { return m_fileList; }

private:
	void createFileTreeView();
	void doSetWorkPath(const QString &path);
	void selectSibling(int row, int col, const QModelIndex &sibling);
	void selectSorted(const QModelIndex &index, bool ensureVisible = false);

signals:

public slots:
	void updateIconTheme();
	void updateSettings();
	void updateHiddenState(bool visible);

	void refresh();
	void invalidated();
	void edit(const QModelIndex &index);
	void select(const QModelIndex &index, bool ensureVisible = false);

	void rootPathChanged(const QString &path);
	void rootPathChanged(const QModelIndex &index);
	void selectLater(const QStringList &selFiles);

	void handleGoParent();

public:
	void handleActivate();
	void handleNewFile();
	void handleNewFolder();
	void handleCut();
	void handleCopy();
	void handlePaste();
	void handleDelete();
	void handleRename();
	void handleProperties();
	void handleTest();

private:
	FilePaneDelegate *m_pane;
	TreeNode         *m_tab;
	bool              m_hiddenVisible;
	bool              m_pathIsHidden;
	bool              m_editing, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;
	FsTreeView       *m_fileTree;
	FsListView       *m_fileList;
	int               m_viewMode;

	FsHandler *m_handler;

	FsModel             *m_model;
	FsSortModel         *m_sortModel;
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

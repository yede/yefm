#ifndef YE_FILEPANE_DELEGATES_H
#define YE_FILEPANE_DELEGATES_H

#include <QFileInfo>
#include <QModelIndex>
#include <QContextMenuEvent>
//==============================================================================================================================

class FilePane;
class FsWidget;
class TreeNode;

typedef int  (FilePane::*_index)           () const;
typedef int  (FilePane::*_currentViewMode) () const;
typedef bool (FilePane::*_isHiddenVisible) () const;
typedef void (FilePane::*_showPath)  (FsWidget *widget, const QString &path);
typedef void (FilePane::*_updateTab) (TreeNode *node, const QString &name, const QString &path);

//==============================================================================================================================

class FilePaneDelegate
{
public:
	FilePaneDelegate(FilePane *pane): m_pane(pane) {}

	void showPath(FsWidget *widget, const QString &path);
	void updateTab(TreeNode *node, const QString &name, const QString &path);

	int       index()           const;
	int       currentViewMode() const;
	bool      isHiddenVisible() const;

	FilePane *pane() const { return m_pane; }

private:
	friend class FilePane;

	FilePane *m_pane;
	_index    m_index;
	_currentViewMode m_currentViewMode;
	_isHiddenVisible m_isHiddenVisible;
	_showPath  m_showPath;
	_updateTab m_updateTab;
};
//==============================================================================================================================

#endif // YE_FILEPANE_DELEGATES_H

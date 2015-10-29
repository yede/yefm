
#include <QDebug>

#include "yefilepanedelegate.h"
//==============================================================================================================================

int FilePaneDelegate::index() const
{
	return (m_pane->*m_index)();
}

int FilePaneDelegate::currentViewMode() const
{
	return (m_pane->*m_currentViewMode)();
}

bool FilePaneDelegate::isHiddenVisible() const
{
	return (m_pane->*m_isHiddenVisible)();
}

void FilePaneDelegate::showPath(FsWidget *widget, const QString &path)
{
	(m_pane->*m_showPath)(widget, path);
}

void FilePaneDelegate::updateTab(TreeNode *node, const QString &name, const QString &path)
{
	(m_pane->*m_updateTab)(node, name, path);
}

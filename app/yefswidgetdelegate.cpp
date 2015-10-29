
#include <QDebug>

#include "yefswidgetdelegate.h"
//==============================================================================================================================

void FsWidgetDelegate_key::handleAction(int action)
{
	(m_view->*m_handler)(action);
}

//==============================================================================================================================
// class FsWidgetDelegate_ctx
//==============================================================================================================================

QString FsWidgetDelegate_ctx::workPath() const
{
	return (m_view->*m_workPath)();
}

QModelIndex FsWidgetDelegate_ctx::getIndex() const
{
	QPoint pos = m_event->pos();
	return (m_view->*m_getIndexAt)(pos);
}

QFileInfo FsWidgetDelegate_ctx::getFileInfo(const QModelIndex &index) const
{
	return (m_view->*m_getFileInfo)(index);
}

int FsWidgetDelegate_ctx::getSelectedFiles(QStringList &files, QString &workDir) const
{
	return (m_view->*m_getSelectedFiles)(files, workDir);
}

bool FsWidgetDelegate_ctx::hasSelection() const
{
	return (m_view->*m_hasSelection)();
}

void FsWidgetDelegate_ctx::dispatchDefaultAction(int actionId)
{
	(m_view->*m_dispatchDefaultAction)(actionId);
}

void FsWidgetDelegate_ctx::clearSelection()
{
	(m_view->*m_clearSelection)();
}

void FsWidgetDelegate_ctx::clearCurrentIndex()
{
	(m_view->*m_clearCurrentIndex)();
}

#ifndef YE_FS_WIDGET_DELEGATE_H
#define YE_FS_WIDGET_DELEGATE_H

#include <QFileInfo>
#include <QModelIndex>
#include <QContextMenuEvent>
//==============================================================================================================================

class FsWidget;

typedef QString     (FsWidget::*_workPath)     ()                         const;
typedef QModelIndex (FsWidget::*_getIndexAt)   (const QPoint &pos)        const;
typedef QFileInfo   (FsWidget::*_getFileInfo)  (const QModelIndex &index) const;
typedef bool        (FsWidget::*_hasSelection) ()                         const;
typedef int         (FsWidget::*_getSelectedFiles) (QStringList &files, QString &workDir) const;

typedef void (FsWidget::*_dispatchDefaultAction) (int actionId);

typedef void (FsWidget::*_clearSelection)    ();
typedef void (FsWidget::*_clearCurrentIndex) ();

typedef void (FsWidget::*_handleAction) (int action);
//==============================================================================================================================

class FsWidgetDelegate_key
{
public:
	void handleAction(int action);

	const QString shortcut() const { return m_shortcut; }

private:
	friend class FsHandler;

	_handleAction m_handler;

	FsWidget *m_view;
	QString m_shortcut;
};
//==============================================================================================================================

class FsWidgetDelegate_ctx
{
public:
	QString     workPath() const;
	QModelIndex getIndex() const;
	QFileInfo   getFileInfo(const QModelIndex &index) const;
	int  getSelectedFiles(QStringList &files, QString &workDir) const;
	bool hasSelection() const;
	void dispatchDefaultAction(int actionId);
	void clearSelection();
	void clearCurrentIndex();

	QContextMenuEvent *event() const { return m_event; }

private:
	friend class FsHandler;

	_workPath              m_workPath;
	_getIndexAt            m_getIndexAt;
	_getFileInfo           m_getFileInfo;
	_hasSelection          m_hasSelection;
	_getSelectedFiles      m_getSelectedFiles;
	_dispatchDefaultAction m_dispatchDefaultAction;

	_clearSelection    m_clearSelection;
	_clearCurrentIndex m_clearCurrentIndex;

	FsWidget          *m_view;
	QContextMenuEvent *m_event;
};
//==============================================================================================================================

#endif // YE_FS_WIDGET_DELEGATE_H

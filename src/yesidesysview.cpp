#include <QDebug>

#include "yefilepane.h"
#include "yefswidget.h"
#include "yesidesysview.h"
#include "yefsmodel.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
//==============================================================================================================================

bool SideSysProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	QFileSystemModel *model = qobject_cast<QFileSystemModel *>(sourceModel());

	return model->isDir(index);
}

//==============================================================================================================================
// class SideSysView
//==============================================================================================================================

SideSysView::SideSysView(YeFilePane *pane, QWidget *parent)
	: QTreeView(parent)
	, m_app(pane->app())
	, m_pane(pane)
	, m_model(NULL)
{
	updateIconTheme();
	connect(m_app, SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
}

SideSysView::~SideSysView()
{
}

void SideSysView::lateStart()
{
	setHeaderHidden(true);
	setUniformRowHeights(true);
//	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
//	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setDragDropMode(QAbstractItemView::DragDrop);
	setDefaultDropAction(Qt::MoveAction);
	setDropIndicatorShown(true);
	setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
	setMouseTracking(true);

	m_model = new FsModel(m_app);
	m_model->setRootPath("/");

	m_proxy = new SideSysProxyModel();
	m_proxy->setSourceModel(m_model);
	m_proxy->sort(0);

	QModelIndex index = m_model->index("/");
	QModelIndex dest = m_proxy->mapFromSource(index);

	setModel(m_proxy);
	setRootIndex(dest);

	m_seleModel = this->selectionModel();

	setColumnHidden(1, true);
	setColumnHidden(2, true);
	setColumnHidden(3, true);
	setColumnHidden(4, true);

	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
}

void SideSysView::updateIconTheme()
{
	int sz = R::data().iconSize;
	setIconSize(QSize(sz, sz));

	if (m_model != NULL) {
		m_model->setRootPath("");	//changing rootPath forces reread, updates file sizes
		m_model->setRootPath("/");
		m_model->clearCutItems();
	}
}
//==============================================================================================================================

void SideSysView::updateWorkPath()
{
	if (m_model == NULL) lateStart();

//	qDebug() << "updateWorkPath().1";
	FsWidget *w = m_pane->currentView();
	if (w == NULL) return;

	QString path = w->workPath();
//	qDebug() << "updateWorkPath().2" << path;
//	if (path == m_workPath) return;

	QModelIndex index = m_model->index(path);
//	qDebug() << "updateWorkPath().3" << index.isValid();
	if (!index.isValid()) return;

	QModelIndex dest = m_proxy->mapFromSource(index);
//	setRootIndex(dest);
	setCurrentIndex(dest);
	m_seleModel->select(dest, QItemSelectionModel::Select);
	scrollTo(dest, QAbstractItemView::EnsureVisible);

	m_workPath = path;
//	qDebug() << "updateWorkPath().4" << path;
}

void SideSysView::onClicked(const QModelIndex &index)
{
	if (!index.isValid()) return;

	QModelIndex src = m_proxy->mapToSource(index);
	QFileInfo info = m_model->fileInfo(src);
	if (info.isDir()) emit itemClicked(info.filePath());
}

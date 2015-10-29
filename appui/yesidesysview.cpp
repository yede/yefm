
#include <QDebug>

#include "yefilepane.h"
#include "yesidesysview.h"

#include "yeapp.h"
#include "yeappcfg.h"
//==============================================================================================================================

SideSysProxyModel::SideSysProxyModel(QFileSystemModel *parent)
	: QSortFilterProxyModel(parent)
	, m_model(parent)
{
	setSourceModel(m_model);
}

bool SideSysProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex index = m_model->index(sourceRow, 0, sourceParent);
	return m_model->isDir(index);
}

//==============================================================================================================================
// class SideSysView
//==============================================================================================================================

SideSysView::SideSysView(QWidget *parent)
	: QTreeView(parent)
	, m_model(NULL)
{
	updateIconTheme();
	connect(App::app(), SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
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

	m_model = new QFileSystemModel(this);
	m_model->setRootPath("/");

	m_proxy = new SideSysProxyModel(m_model);
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
	int sz = AppCfg::instance()->iconSize;
	setIconSize(QSize(sz, sz));

	if (m_model != NULL) {
		m_model->setRootPath("");	//changing rootPath forces reread, updates file sizes
		m_model->setRootPath("/");
	//	m_model->clearCutItems();
	}
}
//==============================================================================================================================

void SideSysView::updateWorkPath()
{
	if (m_model == NULL) lateStart();
/*
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
*/
}

void SideSysView::onClicked(const QModelIndex &index)
{
	if (!index.isValid()) return;

	QModelIndex src = m_proxy->mapToSource(index);
	QFileInfo info = m_model->fileInfo(src);
	if (info.isDir()) emit itemClicked(info.filePath());
}

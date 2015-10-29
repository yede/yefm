#include <QHeaderView>
#include <QMouseEvent>

#include "yepasteview.h"
#include "yepastemodel.h"
#include "yepasteitem.h"
#include "yepastetask.h"

#include "yeiconloader.h"
#include "yeappcfg.h"
//==============================================================================================================================

PasteViewDelegate::PasteViewDelegate(PasteView *view)
	: QItemDelegate(view)
	, m_rowHeight(17)
{
}

void PasteViewDelegate::updateRowHeight()
{
	m_rowHeight = AppCfg::instance()->iconSize + 2;
}

QSize PasteViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QSize size = QItemDelegate::sizeHint(option, index);

	return QSize(size.width(), m_rowHeight);
}

//==============================================================================================================================
// class PasteView
//==============================================================================================================================

PasteView::PasteView(PasteTask *task, QWidget *parent)
	: QTreeView(parent)
	, m_task(task)
	, m_model(NULL)
{
	setRootIsDecorated(false);
	setItemsExpandable(false);
	setUniformRowHeights(true);
	setAlternatingRowColors(true);
//	setSelectionMode(QAbstractItemView::ExtendedSelection);
//	setSelectionBehavior(SelectRows);
	setSelectionMode(SingleSelection);

	m_delegate = new PasteViewDelegate(this);
	setItemDelegate(m_delegate);

	updateIconTheme();	// after: setItemDelegate()

	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
}

void PasteView::setPasteModel(PasteModel *model)
{
	m_model = model;
	setModel(model);
}

void PasteView::updateColumeWidth()
{
	int w0 = 40;
	int w1 = 60;
	int w2 = 68;
	int w3 = (width() - w0 - w1 - w2) >> 1;

//	resizeColumnToContents(0);
	this->setColumnWidth(PasteCol::Percent , w0);
	this->setColumnWidth(PasteCol::Conflict, w1);
	this->setColumnWidth(PasteCol::Solution, w2);
	this->setColumnWidth(PasteCol::SrcName , w3);
	this->header()->setStretchLastSection(true);
}

void PasteView::updateIconTheme()
{
	m_delegate->updateRowHeight();
	int sz = AppCfg::instance()->iconSize;
	setIconSize(QSize(sz, sz));
}

void PasteView::onIconThemeChanged()
{
	updateIconTheme();
	if (m_model) {
		m_model->refresh();
	}
}

void PasteView::onClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
}

void PasteView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		QModelIndex index = indexAt(event->pos());
		int row = index.row();
		int col = PasteCol::ColumnCount - 1;

		QModelIndex left = m_model->index(row, 0, index.parent());
		QModelIndex right = m_model->index(row, col, index.parent());
		QItemSelection sel(left, right);

		selectionModel()->clearSelection();
		selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current);
		selectionModel()->select(sel, QItemSelectionModel::Select);
	}
}

void PasteView::resizeEvent(QResizeEvent *)
{
	updateColumeWidth();
}

void PasteView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	QModelIndexList list = selected.indexes();
	PasteItem *item = NULL;

	if (list.size() > 0) {
		QModelIndex index = list.at(0);
		item = m_model->itemAt(index);
	}
	m_task->setCurrentItem(item);
}

PasteItem *PasteView::currentItem() const
{
	QModelIndexList list = this->selectedIndexes();
	if (list.size() < 1) return NULL;

	QModelIndex index = list.at(0);
	return m_model->itemAt(index);
}

#include <QDebug>

#include "yepastemodel.h"
#include "yepasteview.h"
#include "yepasteitem.h"
#include "yeiconloader.h"
#include "yeiconcache.h"
//==============================================================================================================================

PasteModel::PasteModel(PasteView *view, PasteItem *root, QObject *parent)
	: QAbstractItemModel(parent)
	, m_cache(IconCache::instance())
	, m_root(root)
	, m_view(view)
{
	view->setPasteModel(this);
}

PasteModel::~PasteModel()
{
}
//==============================================================================================================================

void PasteModel::refresh()
{
	beginResetModel();
	endResetModel();
}

void PasteModel::updateItem(PasteItem *item, int col)
{
	Q_ASSERT(item != NULL);

	QModelIndex index = this->index(item->row(), col, QModelIndex());
	m_view->update(index);
}
//==============================================================================================================================

PasteItem *PasteModel::itemAt(const QModelIndex &index) const
{
	if (index.isValid()) {
		PasteItem *item = static_cast<PasteItem *>(index.internalPointer());
		Q_ASSERT(item != NULL);
		return item;
	}
	return m_root;
}

QModelIndex PasteModel::index(int row, int column, const QModelIndex &parent) const
{
	PasteItem *p = itemAt(parent);
	PasteItem *child = p->child(row);

	return (child == NULL) ? QModelIndex() : createIndex(row, column, child);
}

QModelIndex PasteModel::parent(const QModelIndex &index) const
{
	PasteItem *p = itemAt(index)->parent();

	return (p == m_root || p == NULL) ? QModelIndex() : createIndex(p->row(), index.column(), p);
}
//==============================================================================================================================

int PasteModel::rowCount(const QModelIndex &parent) const
{
	return itemAt(parent)->rowCount();
}

int PasteModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return PasteCol::ColumnCount;
}

QVariant PasteModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	PasteItem *item = itemAt(index);

	switch (role) {
		case Qt::DisplayRole:
			switch (index.column()) {
				case PasteCol::Percent : return item->getPercentText();
				case PasteCol::Conflict: return QVariant();
				case PasteCol::Solution: return item->getSolutionText();
				case PasteCol::SrcName : return item->srcName();
				case PasteCol::DestName: return item->conflict() ? item->destName() : QVariant();
			}
			break;

		case Qt::DecorationRole:
			switch (index.column()) {
				case PasteCol::Conflict: return item->hasConflict() ? IconLoader::icon("error") : QVariant();
				case PasteCol::Solution: return QVariant();
				case PasteCol::SrcName : return m_cache->findIcon(item->srcPath());
				case PasteCol::DestName: return item->conflict() ? m_cache->findIcon(item->destPath()) : QVariant();
			}
			break;
	}

	return QVariant();
}

QVariant PasteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
		switch (section) {
			case PasteCol::Percent : return QString("%");
			case PasteCol::Conflict: return tr("Conflict");
			case PasteCol::Solution: return tr("Solution");
			case PasteCol::SrcName : return tr("Source File Name");
			case PasteCol::DestName: return tr("File in Conflict");
		}

	return QAbstractItemModel::headerData(section, orientation, role);
}

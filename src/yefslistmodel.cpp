#include <QDateTime>

#include "yefslistmodel.h"
#include "yefsmodel.h"
#include "yefscache.h"
//==============================================================================================================================

FsListModel::FsListModel(FsModel *source, QObject *parent)
	: QSortFilterProxyModel(parent)
	, m_source(source)
{
	setSourceModel(source);
	setSortCaseSensitivity(Qt::CaseInsensitive);
}

FsListModel::~FsListModel()
{
}
//==============================================================================================================================

bool FsListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(this->filterRegExp().isEmpty()) return true;

	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
//	myModel* fileModel = qobject_cast<myModel*>(sourceModel());

	if (m_source->fileInfo(index0).isHidden()) return false;

	return true;
}

//---------------------------------------------------------------------------------
bool FsListModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
//	myModel* m_source = dynamic_cast<myModel*>(sourceModel());

	if ((m_source->isDir(left) && !m_source->isDir(right)))
		return sortOrder() == Qt::AscendingOrder;
	else if(!m_source->isDir(left) && m_source->isDir(right))
		return sortOrder() == Qt::DescendingOrder;

	if(left.column() == 1)          //size
	{
		if (m_source->size(left) > m_source->size(right)) return true;
		else return false;
	}
	else
	if(left.column() == 3)          //date
	{
		if(m_source->fileInfo(left).lastModified() > m_source->fileInfo(right).lastModified()) return true;
		else return false;
	}

	return QSortFilterProxyModel::lessThan(left,right);
}

#include "yefstreemodel.h"
#include "yefsmodel.h"
//==============================================================================================================================

FsTreeModel::FsTreeModel(FsModel *source, QObject *parent)
	: QSortFilterProxyModel(parent)
	, m_source(source)
{

}

FsTreeModel::~FsTreeModel()
{

}
//==============================================================================================================================

bool FsTreeModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
//	myModel* fileModel = qobject_cast<myModel*>(sourceModel());

	if (m_source->isDir(index0)) {
		if(this->filterRegExp().isEmpty() || m_source->fileInfo(index0).isHidden() == 0) return true;
	}

	return false;
}

#ifndef FSTREEMODEL_H
#define FSTREEMODEL_H

#include <QSortFilterProxyModel>
//==============================================================================================================================

class FsModel;

class FsTreeModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit FsTreeModel(FsModel *source, QObject *parent = 0);
	~FsTreeModel();

protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
	FsModel *m_source;
};

#endif // FSTREEMODEL_H

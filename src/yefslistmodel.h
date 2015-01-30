#ifndef YE_FSLISTMODEL_H
#define YE_FSLISTMODEL_H

#include <QSortFilterProxyModel>
//==============================================================================================================================

class FsModel;

class FsListModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit FsListModel(FsModel *source, QObject *parent = 0);
	~FsListModel();

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

signals:

public slots:

private:
	FsModel *m_source;
};

#endif // YE_FSLISTMODEL_H

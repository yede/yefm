#ifndef YE_FS_SORT_MODEL_H
#define YE_FS_SORT_MODEL_H

#include <QSortFilterProxyModel>
//==============================================================================================================================

class FsModel;

class FsSortModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit FsSortModel(FsModel *source, QObject *parent = 0);
	~FsSortModel();

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

signals:

public slots:

private:
	FsModel *m_source;
};

#endif // YE_FS_SORT_MODEL_H

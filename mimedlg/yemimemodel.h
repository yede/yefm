#ifndef YE_MIMEMODEL_H
#define YE_MIMEMODEL_H

#include <QSortFilterProxyModel>
#include <QIcon>
//==============================================================================================================================

class Mime;
class MimeItem;

class MimeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit MimeModel(QObject *parent = 0);
	~MimeModel();

	void loadMimes();
	bool saveMimes();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
//	bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
//	bool canFetchMore(const QModelIndex &parent) const;
//	void fetchMore(const QModelIndex &parent);
	MimeItem *getItem(const QModelIndex &index) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(MimeItem *item, int column) const;
	QModelIndex parent(const QModelIndex &index) const;

	bool isRoot(MimeItem *item) const { return item == m_root; }
	bool isLoaded() const { return m_loaded; }

protected:
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
	void loadItems();

signals:

public slots:
	void updateIconTheme();

private:
	Mime *m_mime;
	bool m_loaded;
	MimeItem *m_root;
	QIcon m_defaultIcon;
};
//==============================================================================================================================

class MimeFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit MimeFilterModel(MimeModel *source, QObject *parent = 0);
	~MimeFilterModel();

	void setMimePattern(const QString &pattern);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
//	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
	bool filterMimes(MimeItem *item) const;

signals:

public slots:

private:
	MimeModel *m_source;
	int m_mode;
	QStringList m_mimes;
};
//==============================================================================================================================

#endif // YE_MIMEMODEL_H

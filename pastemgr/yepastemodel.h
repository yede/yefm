#ifndef YE_PASTEMODEL_H
#define YE_PASTEMODEL_H

#include <QAbstractItemModel>
//==============================================================================================================================

class IconCache;
class PasteItem;
class PasteView;

class PasteModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit PasteModel(PasteView *view, PasteItem *root, QObject *parent = 0);
	~PasteModel();

	void refresh();
	void updateItem(PasteItem *item, int col);

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	PasteItem *itemAt(const QModelIndex &index) const;

signals:

public slots:

private:
	IconCache *m_cache;
	PasteItem *m_root;
	PasteView *m_view;
};

#endif // YE_PASTEMODEL_H

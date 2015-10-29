#ifndef YE_MIMEVIEW_H
#define YE_MIMEVIEW_H

#include <QItemDelegate>
#include <QTreeView>
//==============================================================================================================================

class MimeViewDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	MimeViewDelegate(QObject *parent): QItemDelegate(parent) {}

	void updateRowHeight();
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
	int m_rowHeight;
};
//==============================================================================================================================

class MimeItem;
class MimeModel;
class MimeFilterModel;

class MimeView : public QTreeView
{
	Q_OBJECT
public:
	explicit MimeView(QWidget *parent = 0);
	~MimeView();

	void loadMimes();
	bool saveMimes();

	void updateApps(MimeItem *item);
	void setFilter(const QString &pattern);

	bool isLoaded() const;
	MimeItem *currentItem() const;

private:
	MimeItem *getItem(const QModelIndex &mapped) const;

signals:
	void currentItemChanged(MimeItem *current, MimeItem *previous);

public slots:
	void updateIconTheme();
	void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
	MimeModel        *m_model;
	MimeFilterModel  *m_filter;
	MimeViewDelegate *m_delegate;
};

#endif // YE_MIMEVIEW_H

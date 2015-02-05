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

class YeApplication;
class MimeModel;
class MimeItem;

class MimeView : public QTreeView
{
	Q_OBJECT
public:
	explicit MimeView(YeApplication *app, QWidget *parent = 0);
	~MimeView();

	void loadMimes();
	bool saveMimes();

	void updateApps(MimeItem *item);

	bool isLoaded() const;
	MimeItem *currentItem() const;

private:

signals:
	void currentItemChanged(MimeItem *current, MimeItem *previous);

public slots:
	void updateIconTheme();
	void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
	YeApplication *m_app;
	MimeModel *m_model;
	MimeViewDelegate *m_delegate;
};

#endif // YE_MIMEVIEW_H

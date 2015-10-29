#ifndef YE_PASTEVIEW_H
#define YE_PASTEVIEW_H

#include <QItemDelegate>
#include <QTreeView>
//==============================================================================================================================

class PasteView;

class PasteViewDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	PasteViewDelegate(PasteView *view);

	void updateRowHeight();
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
	int m_rowHeight;
};
//==============================================================================================================================

class PasteTask;
class PasteItem;
class PasteModel;

class PasteView : public QTreeView
{
	Q_OBJECT
public:
	explicit PasteView(PasteTask *task, QWidget *parent = 0);

	void setPasteModel(PasteModel *model);
	void updateColumeWidth();

	PasteItem *currentItem() const;

private:
	void updateIconTheme();

protected:
	void mousePressEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);

signals:

public slots:
	void onIconThemeChanged();
	void onClicked(const QModelIndex &index);

protected slots:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
	PasteTask *m_task;
	PasteModel *m_model;

	PasteViewDelegate *m_delegate;
};

#endif // YE_PASTEVIEW_H

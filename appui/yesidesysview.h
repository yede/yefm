#ifndef SIDESYSVIEW_H
#define SIDESYSVIEW_H

#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
//==============================================================================================================================

class SideSysProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SideSysProxyModel(QFileSystemModel *parent);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
	QFileSystemModel *m_model;
};
//==============================================================================================================================

class SideSysView : public QTreeView
{
	Q_OBJECT
public:
	explicit SideSysView(QWidget *parent = 0);
	~SideSysView();

	void updateWorkPath();

private:
	void lateStart();

signals:
	void itemClicked(const QString &path);

public slots:
	void updateIconTheme();
	void onClicked(const QModelIndex &index);

private:
	QFileSystemModel *m_model;
	SideSysProxyModel *m_proxy;
	QItemSelectionModel *m_seleModel;
	QString m_workPath;
};

#endif // SIDESYSVIEW_H

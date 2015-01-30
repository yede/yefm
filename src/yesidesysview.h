#ifndef SIDESYSVIEW_H
#define SIDESYSVIEW_H

#include <QTreeView>
#include <QSortFilterProxyModel>
//==============================================================================================================================

class SideSysProxyModel : public QSortFilterProxyModel
{
protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};
//==============================================================================================================================

class YeApplication;
class YeFilePane;
class FsModel;
class TreeNode;

class SideSysView : public QTreeView
{
	Q_OBJECT
public:
	explicit SideSysView(YeFilePane *pane, QWidget *parent = 0);
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
	YeApplication *m_app;
	YeFilePane *m_pane;
	FsModel *m_model;

	SideSysProxyModel *m_proxy;
	QItemSelectionModel *m_seleModel;
	QString m_workPath;
};

#endif // SIDESYSVIEW_H

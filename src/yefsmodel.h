#ifndef YE_FSMODEL_H
#define YE_FSMODEL_H

#include <QFileSystemModel>
#include <QModelIndex>
#include <QMimeData>
#include <QStringList>
//==============================================================================================================================

class YeApplication;
class FsCache;
class FsWidget;
class FsTreeView;

class FsModel : public QFileSystemModel
{
	Q_OBJECT

public:
	FsModel(YeApplication *app, QObject *parent = 0);
	~FsModel();

//	QStringList	    mimeTypes() const;

	void setFsWidget(FsWidget *widget) { m_widget = widget; }
	void setMode(bool);
	void setHiddenVisible(bool visible);

	bool remove(const QModelIndex &index) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	int columnCount(const QModelIndex &parent) const;

	QString tempFile() const;
	void addCutItems(QStringList);
	void clearCutItems();

	void handleCut();
	void handleCopy();
	void handlePaste();
	void handleDelete();

	void execPaste(const QString &destPath, const QMimeData *srcData, const QStringList &cutList);

	static void initColWidths(QList<int> &cols);
	static void updateColSizes(YeApplication *app, FsTreeView *p);
	static void saveColSizes(YeApplication *app, FsTreeView *p);

signals:

public slots:

protected:
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
	YeApplication *m_app;
	FsCache       *m_cache;
	FsWidget      *m_widget;
	bool           m_showThumbs;
	QStringList    m_cutItems;
};

#endif // YE_FSMODEL_H

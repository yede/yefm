#ifndef YE_FSMODEL_H
#define YE_FSMODEL_H

#include <QModelIndex>
#include <QMimeData>
#include <QStringList>
//==============================================================================================================================
#define USE_qsys 0
#define USE_node 1

#if USE_qsys
# include <QFileSystemModel>
#endif

#if USE_node
# include "yefsdirmodel.h"
#endif
//==============================================================================================================================

class Style;
class IconCache;
class FsWidget;
class FsTreeView;

class FsModel : public
#if USE_qsys
				QFileSystemModel
#endif
#if USE_node
				FsDirModel
#endif
{
	Q_OBJECT

public:
	FsModel(FsWidget *widget, QObject *parent = 0);
	~FsModel();

//	QStringList	    mimeTypes() const;

	bool remove(const QModelIndex &index) const;

	void setMode(bool);
	void setHiddenVisible(bool visible);

	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	int columnCount(const QModelIndex &parent) const;

	void handleCut();
	void handleCopy();
	void handlePaste();
	void handleDelete();

	void startPaste(const QMimeData *srcData, const QString &destFolder, int pasteType);

	static void initColWidths(QList<int> &cols);
	static void updateColSizes(FsTreeView *p);
	static void saveColSizes(FsTreeView *p);

	static QString tempFile();
	static void setCutItems(const QStringList &files);
	static void clearCutItems();
	static bool isCutAction();

	FsWidget *widget() const { return m_widget; }

private:
	static QStringList m_cutItems;

signals:

public slots:

protected:
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
	FsWidget  *m_widget;
	Style   *m_style;
	IconCache *m_cache;
	bool       m_showThumbs;
	bool dummy_1, dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;
};

#endif // YE_FSMODEL_H

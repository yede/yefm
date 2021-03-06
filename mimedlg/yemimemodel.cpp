#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QDebug>

#include "yemimemodel.h"
#include "yemimeitem.h"
#include "yemimeview.h"
#include "yemime.h"

#include "yeiconloader.h"
//==============================================================================================================================

MimeModel::MimeModel(QObject *parent)
	: QAbstractItemModel(parent)
	, m_mime(Mime::instance())
	, m_loaded(false)
{
	m_root = new MimeItem(MimeItemType::Root, "root", NULL);
}

MimeModel::~MimeModel()
{
	delete m_root;
}

void MimeModel::updateIconTheme()
{
	if (!IconLoader::findMimeIcon(m_defaultIcon, "unknown")) {
		m_defaultIcon = IconLoader::defaultIcon("unknown");
	}

	if (m_loaded) {
	//	int sz = Config::instance()->iconSize;
	//	setIconSize(QSize(sz, sz));
	}
}
//==============================================================================================================================

static bool getMimeTypes(QStringList &result)
{
	// Check whether file with mime descriptions exists
	QFile file("/usr/share/mime/types");
	if (!file.exists()) {
		return false;
	}

	// Try to open file
	if (!file.open(QFile::ReadOnly)) {
		return false;
	}

	// Read mime types
	QTextStream stream(&file);
	while (!stream.atEnd()) {
		result.append(stream.readLine());
	}
	file.close();
	return true;
}

void MimeModel::loadItems()
{
	QStringList mimes;
	if (!getMimeTypes(mimes)) return;

	int n;
	QString cate, name, apps;
	QChar sep = QChar('/');
	MimeItem *cateItem, *item;
	QMap<QString, MimeItem *> categories;				// Mime cathegories and their icons
	QList<MimeItem *> cateItems;

	foreach (QString mime, mimes) {						// Load mime settings

		if (mime.startsWith("inode"))     continue;		// Skip all 'inode' nodes including 'inode/directory'
		if (mime.startsWith("x-content")) continue;		// Skip all 'x-content'
		if (mime.startsWith("message"))   continue;		// Skip all 'message' nodes

		n = mime.indexOf(sep);
		if (n < 0) continue;

		cate = mime.left(n);
		name = mime.mid(n + 1);
		if (name.isEmpty() || cate.isEmpty()) continue;

		cateItem = categories.value(cate, NULL);
		if (cateItem == NULL) {
			cateItem = new MimeItem(MimeItemType::Cate, cate, NULL);
			cateItems.append(cateItem);
			categories.insert(cate, cateItem);
		}

		apps = m_mime->getDefault(mime).join(";");
		item = cateItem->addChild(MimeItemType::Mime, name);
		item->apps = apps.remove(".desktop");
	}

	n = cateItems.size();
	beginInsertRows(QModelIndex(), 0, n - 1);
	for (int i = 0; i < n; i++) {
		item = cateItems.at(i);
		m_root->addChild(item);
	}
	endInsertRows();
}

void MimeModel::loadMimes()
{
	if (m_loaded) return;

//	QTime time;
//	time.start();
	loadItems();
//	qDebug() << time.elapsed();

	m_loaded = true;
}

bool MimeModel::saveMimes()
{
	int cnt = m_root->rowCount();
	QChar sep = QChar('/');

	for (int r = 0; r < cnt; ++r)
	{
		MimeItem *cate = m_root->child(r);
		int mimeCount = cate->rowCount();
		for (int m = 0; m < mimeCount; m++)
		{
			QStringList apps;
			MimeItem *item = cate->child(m);	// mime item
			if (!item->apps.isEmpty())
			{
				apps = item->apps.split(";");
				int appCount = apps.size();
				for (int a = 0; a < appCount; a++) {
					apps[a] = apps[a] + ".desktop";
				}
			}
			QString mime = cate->name + sep + item->name;
			m_mime->setDefault(mime, apps);
		}
	}

	m_mime->saveDefaults();

	return true;
}
//==============================================================================================================================

Qt::ItemFlags MimeModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int MimeModel::rowCount(const QModelIndex &parent) const
{
	MimeItem *p = getItem(parent);
	return p->rowCount();
}

int MimeModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
/*	MimeItem *p = getItem(parent);
	switch (p->type) {
		case MimeItemType::Root: return 2;
		case MimeItemType::Cate: return 2;
	}*/
	return 2;
}
/*
bool MimeModel::hasChildren(const QModelIndex &parent) const
{
	return rowCount(parent) > 0;
}

bool MimeModel::canFetchMore(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return false;
}

void MimeModel::fetchMore(const QModelIndex &parent)
{
	Q_UNUSED(parent);
}*/
//==============================================================================================================================

MimeItem *MimeModel::getItem(const QModelIndex &index) const
{
	MimeItem *item = index.isValid() ? static_cast<MimeItem *>(index.internalPointer()) : m_root;
	Q_ASSERT(item != NULL);

	return item;
}

QModelIndex MimeModel::index(int row, int column, const QModelIndex &parent) const
{
	MimeItem *p = getItem(parent);
	MimeItem *child = p->child(row);

	return (child == NULL) ? QModelIndex() : createIndex(row, column, child);
}

QModelIndex MimeModel::index(MimeItem *item, int column) const
{
	return (item == NULL) ? QModelIndex() : createIndex(item->row(), column, item);
}

QModelIndex MimeModel::parent(const QModelIndex &index) const
{
	MimeItem *p = getItem(index)->parent;

	return (p == m_root) ? QModelIndex() : createIndex(p->row(), 0, p);
}
//==============================================================================================================================

QVariant MimeModel::data(const QModelIndex &index, int role) const
{
	MimeItem *p = getItem(index);

	switch (role) {
		case Qt::DisplayRole   : return index.column() == 0 ? p->name : p->apps;
		case Qt::DecorationRole: {
			if (p->type != MimeItemType::Mime || index.column() > 0) return QVariant();
			QIcon icon;
			if (p->getIcon(icon)) return icon;
			return m_defaultIcon;
		}
	}

	return QVariant();
}

QVariant MimeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);

	if (role == Qt::DisplayRole)
		switch (section) {
			case 0: return tr("Mime");
			case 1: return tr("Application");
		}

	return QVariant();
}

//==============================================================================================================================
// class MimeFilterModel
//==============================================================================================================================

namespace MimeFilterMode {
	enum { None, Mime, Cate };
}

MimeFilterModel::MimeFilterModel(MimeModel *source, QObject *parent)
	: QSortFilterProxyModel(parent)
	, m_source(source)
	, m_mode(MimeFilterMode::None)
{
	setSourceModel(source);
	setSortCaseSensitivity(Qt::CaseInsensitive);
}

MimeFilterModel::~MimeFilterModel()
{
}
//==============================================================================================================================

void MimeFilterModel::setMimePattern(const QString &pattern)
{
	m_mimes.clear();
	QStringList mimes = pattern.split(',');
	int n = mimes.size();

	for (int i = 0; i < n; i++) {
		QString str = mimes.at(i).trimmed();
		if (!str.isEmpty()) m_mimes.append(str);
	}

	m_mode = MimeFilterMode::Mime;		// round 1, we select all categories and expected mime-items
	invalidateFilter();

	m_mode = MimeFilterMode::Cate;		// round 2, we skip all empty categories, keep expected mime-items
	invalidateFilter();

	m_mode = MimeFilterMode::None;
}

bool MimeFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if (m_mode == MimeFilterMode::None) return true;
	if (m_mimes.isEmpty()) return true;
//	qDebug() << "filterAcceptsRow" << QTime::currentTime();

	QModelIndex source = m_source->index(sourceRow, 0, sourceParent);
	MimeItem *item = m_source->getItem(source);

	if (m_mode == MimeFilterMode::Mime) {
		if (item->type != MimeItemType::Mime) return true;
		return filterMimes(item);
	}

	if (m_mode == MimeFilterMode::Cate) {
		if (item->type == MimeItemType::Mime) return filterMimes(item);
		if (item->type != MimeItemType::Cate) return true;
		QModelIndex mapped = this->mapFromSource(source);
		return hasChildren(mapped);
	}

	return false;
}

bool MimeFilterModel::filterMimes(MimeItem *item) const
{
	int n = m_mimes.size();
	for (int i = 0; i < n; i++) {
		if (item->name.contains(m_mimes.at(i))) return true;
	}
	return false;
}
//==============================================================================================================================

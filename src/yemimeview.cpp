#include <QDebug>

#include "yemimemodel.h"
#include "yemimeitem.h"
#include "yemimeview.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
//==============================================================================================================================

void MimeViewDelegate::updateRowHeight()
{
	m_rowHeight = R::data().iconSize + 2;
}

QSize MimeViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QSize size = QItemDelegate::sizeHint(option, index);
	return QSize(size.width(), m_rowHeight);
}

//==============================================================================================================================
// class MimeView
//==============================================================================================================================

MimeView::MimeView(YeApplication *app, QWidget *parent)
	: QTreeView(parent)
	, m_app(app)
	, m_model(NULL)
{
	setAlternatingRowColors(true);
	setRootIsDecorated(true);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	m_model = new MimeModel(m_app);
	m_filter = new MimeFilterModel(m_model);
	setModel(m_filter);

	setColumnWidth(0, 200);
//	setColumnWidth(1, 300);

	m_delegate = new MimeViewDelegate(this);
	setItemDelegate(m_delegate);

	updateIconTheme();	// after: setItemDelegate()
	connect(app, SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
}

MimeView::~MimeView()
{
	delete m_model;
}

void MimeView::updateIconTheme()
{
	m_delegate->updateRowHeight();
	int sz = R::data().iconSize;
	setIconSize(QSize(sz, sz));

	m_model->updateIconTheme();
}
//==============================================================================================================================

bool MimeView::isLoaded() const
{
	return m_model->isLoaded();
}

void MimeView::loadMimes()
{
	m_model->loadMimes();
}

bool MimeView::saveMimes()
{
	if (m_model != NULL) return m_model->saveMimes();
	return false;
}

void MimeView::updateApps(MimeItem *item)
{
	if (item == NULL || item->type != MimeItemType::Mime) return;

	QModelIndex source = m_model->index(item, 1);
	QModelIndex mapped = m_filter->mapFromSource(source);
	update(mapped);
}

void MimeView::setFilter(const QString &pattern)
{
	m_filter->setMimePattern(pattern);
//	this->expandAll();
}
//==============================================================================================================================

MimeItem *MimeView::getItem(const QModelIndex &mapped) const
{
	QModelIndex source = m_filter->mapToSource(mapped);
	MimeItem *item = m_model->getItem(source);
	if (m_model->isRoot(item)) item = NULL;

	return item;
}

MimeItem *MimeView::currentItem() const
{
	QModelIndex index = currentIndex();

	return getItem(index);
}

void MimeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (m_model == NULL) return;

	MimeItem *curr = getItem(current);
	MimeItem *prev = getItem(previous);

	emit currentItemChanged(curr, prev);
}

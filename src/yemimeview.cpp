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
	setModel(m_model);

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

	QModelIndex index = m_model->index(item, 1);
	update(index);
}
//==============================================================================================================================

MimeItem *MimeView::currentItem() const
{
	QModelIndex index = currentIndex();
	MimeItem *curr = m_model->getItem(index);
	if (m_model->isRoot(curr)) curr = NULL;
	return curr;
}

void MimeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (m_model == NULL) return;

	MimeItem *curr = m_model->getItem(current);
	if (m_model->isRoot(curr)) curr = NULL;

	MimeItem *prev = m_model->getItem(previous);
	if (m_model->isRoot(prev)) prev = NULL;

	emit currentItemChanged(curr, prev);
}

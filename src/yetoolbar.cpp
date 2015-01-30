#include <QPainter>
#include <QFrame>
#include <QDebug>

#include "yetoolbar.h"
#include "yetoolitem.h"
//==============================================================================================================================

ToolBar::ToolBar(Qt::Orientation orientation, QWidget *parent)
	: QWidget(parent)
	, m_orientation(orientation)
	, m_iconSize(16, 16)
	, m_basePads(0, 0, 0, 0)
	, m_itemPads(2, 2, 2, 2)
	, m_itemSpacing(2)
	, m_separatorSize(8)
	, m_hasSpacer(true)
	, m_baseSpacer(NULL)
	, m_baseLayout(NULL)
	, m_itemLayout(NULL)
	, m_tailLayout(NULL)
{
}

ToolBar::~ToolBar()
{
}
//==============================================================================================================================

void ToolBar::setupLayout()
{
	if (m_baseLayout != NULL) return;

	updateMinimumSize();

	if (isHorizontal()) {
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	} else {
		setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	}

	QBoxLayout::Direction direction = this->direction();

	m_itemLayout = new QBoxLayout(direction);
	m_itemLayout->setContentsMargins(0, 0, 0, 0);
	m_itemLayout->setSpacing(m_itemSpacing);

	m_baseLayout = new QBoxLayout(direction, this);
	m_baseLayout->setContentsMargins(m_basePads);
	m_baseLayout->setSpacing(0);
	m_baseLayout->addLayout(m_itemLayout);

	if (m_hasSpacer) {
		m_baseSpacer = isHorizontal() ? new QSpacerItem(0, 10, QSizePolicy::Expanding, QSizePolicy::Preferred)
									  : new QSpacerItem(10, 0, QSizePolicy::Preferred, QSizePolicy::Expanding);
		m_baseLayout->addSpacerItem(m_baseSpacer);
	}
}

void ToolBar::setupTailLayout()
{
	if (m_tailLayout != NULL) return;

	QBoxLayout::Direction direction = this->direction();

	m_tailLayout = new QBoxLayout(direction);
	m_tailLayout->setContentsMargins(0, 0, 0, 0);
	m_tailLayout->setSpacing(m_itemSpacing);

	m_baseLayout->addLayout(m_tailLayout);
}
//==============================================================================================================================

void ToolBar::updateIconSize(int w, int h)
{
	m_iconSize = QSize(w, h);
	updateMinimumSize();
}

void ToolBar::updateMinimumSize()
{
	setMinimumWidth(m_iconSize.width() + m_itemPads.left() + m_itemPads.right() + m_basePads.left() + m_basePads.right());
	setMinimumHeight(m_iconSize.height() + m_itemPads.top() + m_itemPads.bottom() + m_basePads.top() + m_basePads.bottom());
}

void ToolBar::updateOrientation()
{
	qDebug() << "TODO: ToolBar::updateOrientation: reset Orientation of each item.";
}

void ToolBar::updateLayout()
{
	if (m_baseLayout == NULL) {
		setupLayout();
		return;
	}

	updateMinimumSize();

	if (m_itemLayout->count() > 0) {
		updateOrientation();
	}

	if (m_baseSpacer != NULL) {
		m_baseLayout->removeItem(m_baseSpacer);
		delete m_baseSpacer;
		m_baseSpacer = NULL;
	}

	QBoxLayout::Direction direction = this->direction();

	if (m_hasSpacer) {
		m_baseSpacer = isHorizontal() ? new QSpacerItem(0, 10, QSizePolicy::Expanding, QSizePolicy::Preferred)
									  : new QSpacerItem(10, 0, QSizePolicy::Preferred, QSizePolicy::Expanding);
		m_baseLayout->addSpacerItem(m_baseSpacer);
	}

	m_baseLayout->setDirection(direction);
	m_baseLayout->setSpacing(0);
	m_baseLayout->setContentsMargins(m_basePads);

	m_itemLayout->setDirection(direction);
	m_itemLayout->setSpacing(m_itemSpacing);
	m_itemLayout->setContentsMargins(0, 0, 0, 0);

	Qt::Alignment align = alignment();
	int cnt = m_itemLayout->count();

	for (int i = 0; i < cnt; i++) {
		QLayoutItem *item = m_itemLayout->itemAt(i);
		item->setAlignment(align);
		//QWidget *wid = item->widget(); // TODO: handle separator
	}

	if (m_tailLayout != NULL) {
		m_tailLayout->setDirection(direction);
		m_tailLayout->setContentsMargins(0, 0, 0, 0);
		m_tailLayout->setSpacing(m_itemSpacing);

		cnt = m_tailLayout->count();

		for (int i = 0; i < cnt; i++) {
			QLayoutItem *item = m_tailLayout->itemAt(i);
			item->setAlignment(align);
			//QWidget *wid = item->widget(); // TODO: handle separator
		}
	}
}
//==============================================================================================================================

void ToolBar::setIconSize(int w, int h)
{
	m_iconSize = QSize(w, h);
}

void ToolBar::setBasePads(int left, int top, int right, int bottom)
{
	m_basePads.setLeft(left);
	m_basePads.setTop(top);
	m_basePads.setRight(right);
	m_basePads.setBottom(bottom);
}

void ToolBar::setItemPads(int left, int top, int right, int bottom)
{
	m_itemPads.setLeft(left);
	m_itemPads.setTop(top);
	m_itemPads.setRight(right);
	m_itemPads.setBottom(bottom);
}

void ToolBar::setItemSpacing(int spacing)
{
	m_itemSpacing = spacing;
}
//==============================================================================================================================

void ToolBar::addItem(QWidget *item, int stretch, Qt::Alignment alignment)
{
	m_itemLayout->addWidget(item, stretch, alignment);
}

void ToolBar::addSeparator(int size)                          { doAddSeparator(m_itemLayout, size); }
void ToolBar::addItem(QWidget *item, int stretch)             { doInsertItem(m_itemLayout, item, -1, stretch); }
void ToolBar::insertItem(QWidget *item, int pos, int stretch) { doInsertItem(m_itemLayout, item, pos, stretch); }
void ToolBar::removeItem(QWidget *item)                       { doRemoveItem(m_itemLayout, item); }

ToolList *ToolBar::addToolList()
{
	ToolList *item = new ToolList(this);
	addItem(item);
	return item;
}

ToolIcon *ToolBar::addToolIcon()
{
	ToolIcon *item = new ToolIcon(this);
	addItem(item);
	return item;
}

ToolIcon *ToolBar::addToolIcon(const QIcon &icon)
{
	ToolIcon *item = new ToolIcon(icon, this);
	addItem(item);
	return item;
}

ToolIcon *ToolBar::addToolIcon(const QIcon &icon, const QString &tips)
{
	ToolIcon *item = addToolIcon(icon);
	item->setToolTip(tips);
	return item;
}

ToolIcon *ToolBar::addToolIcon(const QIcon &icon, const QObject *receiver, const char *method)
{
	ToolIcon *item = addToolIcon(icon);
	connect(item, SIGNAL(clicked()), receiver, method);
	return item;
}

ToolIcon *ToolBar::addToolIcon(const QIcon &icon, const QString &tips, const QObject *receiver, const char *method)
{
	ToolIcon *item = addToolIcon(icon, receiver, method);
	item->setToolTip(tips);
	return item;
}

ToolIcon *ToolBar::addToolIcon(const QString &tips)
{
	ToolIcon *item = addToolIcon();
	item->setToolTip(tips);
	return item;
}

ToolIcon *ToolBar::addToolIcon(const QString &tips, const QObject *receiver, const char *method)
{
	ToolIcon *item = addToolIcon(tips);
	connect(item, SIGNAL(clicked()), receiver, method);
	return item;
}
//==============================================================================================================================

void ToolBar::doAddSeparator(QBoxLayout *box, int size)
{
	QFrame *sp = new QFrame();
//	sp->setUserData(...);		// TODO: set user-data for updateLayout()

	if (size < 1) size = m_separatorSize;

	if (isHorizontal()) {
		sp->setFrameStyle(QFrame::VLine | QFrame::Sunken);
		sp->setFixedWidth(size);
		sp->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	} else {
		sp->setFrameStyle(QFrame::HLine | QFrame::Sunken);
		sp->setFixedHeight(size);
		sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	}

	box->addWidget(sp);
}

void ToolBar::doInsertItem(QBoxLayout *box, QWidget *item, int pos, int stretch)
{
	Qt::Alignment align = alignment();
	box->insertWidget(pos, item, stretch, align);
}

void ToolBar::doRemoveItem(QBoxLayout *box, QWidget *item)
{
	box->removeWidget(item);
}
//==============================================================================================================================

void ToolBar::addTailItem(QWidget *item, int stretch, Qt::Alignment alignment)
{
	m_tailLayout->addWidget(item, stretch, alignment);
}

void ToolBar::addTailSeparator(int size)                          { doAddSeparator(m_tailLayout, size); }
void ToolBar::addTailItem(QWidget *item, int stretch)             { doInsertItem(m_tailLayout, item, -1, stretch); }
void ToolBar::insertTailItem(QWidget *item, int pos, int stretch) { doInsertItem(m_tailLayout, item, pos, stretch); }
void ToolBar::removeTailItem(QWidget *item)                       { doRemoveItem(m_tailLayout, item); }

ToolIcon *ToolBar::addTailIcon()
{
	ToolIcon *item = new ToolIcon(this);
	addTailItem(item);
	return item;
}

ToolIcon *ToolBar::addTailIcon(const QIcon &icon)
{
	ToolIcon *item = new ToolIcon(icon, this);
	addTailItem(item);
	return item;
}

ToolIcon *ToolBar::addTailIcon(const QIcon &icon, const QString &tips)
{
	ToolIcon *item = addTailIcon(icon);
	item->setToolTip(tips);
	return item;
}

ToolIcon *ToolBar::addTailIcon(const QIcon &icon, const QObject *receiver, const char *method)
{
	ToolIcon *item = addTailIcon(icon);
	connect(item, SIGNAL(clicked()), receiver, method);
	return item;
}

ToolIcon *ToolBar::addTailIcon(const QIcon &icon, const QString &tips, const QObject *receiver, const char *method)
{
	ToolIcon *item = addTailIcon(icon, receiver, method);
	item->setToolTip(tips);
	return item;
}

ToolIcon *ToolBar::addTailIcon(const QString &tips)
{
	ToolIcon *item = addTailIcon();
	item->setToolTip(tips);
	return item;
}

ToolIcon *ToolBar::addTailIcon(const QString &tips, const QObject *receiver, const char *method)
{
	ToolIcon *item = addTailIcon(tips);
	connect(item, SIGNAL(clicked()), receiver, method);
	return item;
}

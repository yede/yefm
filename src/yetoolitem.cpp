#include <QStylePainter>
#include <QStyleOption>
#include <QStyleOptionToolButton>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QDebug>

#include "yetoolbar.h"
#include "yetoolitem.h"
//==============================================================================================================================
// class ToolItem
//==============================================================================================================================

ToolItem::ToolItem(QWidget *parent)
	: QWidget(parent)
	, m_toolBar(NULL)
{
	init();
}

ToolItem::ToolItem(ToolBar *toolBar)
	: QWidget(NULL)
	, m_toolBar(toolBar)
{
	init();
}

QSize ToolItem::sizeHint() const
{
	return m_defaultSize;
}

void ToolItem::init()
{
	m_timer = NULL;
	m_menu = NULL;
	m_defaultSize = QSize(16, 16);
	m_autoCheck = false;
	m_checked = false;
	m_hovering = false;
	m_pressed = false;
	m_popup = false;

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QAction *ToolItem::addMenuItem(const QIcon &icon, const QString &title,
							   const QObject *receiver, const char *method)
{
	QAction *act = insertMenuItem(0, title, receiver, method);
	act->setIcon(icon);
	return act;
}

QAction *ToolItem::addMenuItem(const QString &title,
							   const QObject *receiver, const char *method)
{
	return insertMenuItem(0, title, receiver, method);
}

QAction *ToolItem::insertMenuItem(QAction *before, const QString &title,
								  const QObject *receiver, const char *method)
{
	if (!m_menu) createMenu();
	QAction *act = new QAction(title, this);
	connect(act, SIGNAL(triggered()), receiver, method/*SLOT(method())*/);
	m_menu->insertAction(before, act);
	return act;
}

void ToolItem::addMenuSeparator()
{
	if (!m_menu) createMenu();
	m_menu->addSeparator();
}

QMenu *ToolItem::createMenu()
{
	if (!m_menu) {
		QMenu *menu = new QMenu(this);
		setMenu(menu);
	}
	return m_menu;
}

void ToolItem::setMenu(QMenu *menu)
{
	if (m_menu == menu) return;

	if (m_menu != NULL && m_menu->parent() == this) delete m_menu;
	m_menu = menu;
	if (menu == NULL) return;

	if (m_timer == NULL) {
		m_timer = new QTimer(this);
		m_timer->setSingleShot(true);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	}
	connect(menu, SIGNAL(aboutToHide()), this, SLOT(aboutToHide()));
}

void ToolItem::showMenu(QMenu *menu)
{
	if (!menu) return;

	menu->adjustSize();	// for first showing ONLY

	QPoint p(0, this->height() - 1);
	p = this->mapToGlobal(p);
	if (p.x() + menu->width() > QApplication::desktop()->width())
		p.setX(QApplication::desktop()->width() - menu->width());
	if (p.y() + menu->height() > QApplication::desktop()->height())
		p.setY(p.y() - menu->height() - this->height());

	m_popup = true;
	menu->popup(p);
}

void ToolItem::aboutToHide()
{
	m_hovering = false;
	m_pressed = false;
	if (m_timer != NULL) {		// when m_menu is showing, and mouse is on this toolitem,
		m_timer->start(40);		// after mouse button pressed by hand,
	} else {					// aboutToHide() run before mousePressedEvent().
		m_popup = false;		// the result is you can never hide the popup menu
		update();				// unless moving mouse cursor out of this toolitem and click.
	}
}

void ToolItem::onTimeout()
{
	m_popup = false;
	update();
}
//==============================================================================================================================

void ToolItem::mousePressEvent(QMouseEvent *event)
{
//	QWidget::mousePressEvent(event);
	Q_UNUSED(event);

	m_pressed = true;
	if (m_menu == NULL) {
		update();
		return;
	}

	if (m_timer != NULL && m_timer->isActive()) {	// before
		m_timer->stop();
	}

	if (m_popup) {
		m_menu->hide();
		m_popup = false;
	} else {
		showMenu(m_menu);
	}
	update();
}

void ToolItem::mouseReleaseEvent(QMouseEvent *event)
{
	m_pressed = false;
	bool flag = event->x() >= 0 && event->x() < width() &&
				event->y() >= 0 && event->y() < height();
	if (flag && m_autoCheck) m_checked = !m_checked;
	update();
	if (flag && !m_menu) emit clicked();
}

void ToolItem::enterEvent(QEvent *event)
{
	Q_UNUSED(event);
	m_hovering = true;
	update();
}

void ToolItem::leaveEvent(QEvent *event)
{
	Q_UNUSED(event);
	m_hovering = false;
//	m_pressed = false;
	update();
}

void ToolItem::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	bool isOn = m_pressed || m_checked;
	if (isOn || m_hovering)
	{
		QStylePainter painter(this);
		QStyleOptionToolButton opt;
		opt.initFrom(this);

		opt.state = QStyle::State_Enabled;
		if (isOn && m_hovering) {
			opt.state |= QStyle::State_On | QStyle::State_MouseOver;
		} else {
			if (m_hovering) opt.state |= QStyle::State_MouseOver | QStyle::State_Raised;
			else if (isOn)  opt.state |= QStyle::State_On | QStyle::State_AutoRaise;
		}
		opt.arrowType = Qt::NoArrow;
		opt.direction = Qt::LeftToRight;
		opt.features = QStyleOptionToolButton::None;
	//	opt.palette = qApp->palette();
	//	opt.rect = emailButtonRect(option.rect);
		opt.toolButtonStyle = Qt::ToolButtonIconOnly;
		opt.subControls = QStyle::SC_ToolButton;
	//	opt.activeSubControls = QStyle::SC_None;

		painter.drawComplexControl(QStyle::CC_ToolButton, opt);
	}
}

//==============================================================================================================================
// class ToolIcon
//==============================================================================================================================

ToolIcon::ToolIcon(ToolBar *parent)
	: ToolItem(parent)
{
	updateContent();
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

ToolIcon::ToolIcon(const QIcon &icon, ToolBar *parent)
	: ToolItem(parent)
{
	setIcon(icon);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ToolIcon::setIcon(const QIcon &icon)
{
	m_pixmap = icon.pixmap(m_toolBar->iconSize());
	updateContent();
}

void ToolIcon::updateContent()
{
	m_iconRect = QRect(m_toolBar->itemPads().left(), m_toolBar->itemPads().top(),
					   m_toolBar->iconSize().width(), m_toolBar->iconSize().height());

	const QMargins &pads = m_toolBar->itemPads();
	int w = m_toolBar->iconSize().width() + pads.left() + pads.right();
	int h = m_toolBar->iconSize().height() + pads.top() + pads.bottom();

	setFixedSize(w, h);
}
//==============================================================================================================================

void ToolIcon::paintEvent(QPaintEvent *event)
{
	ToolItem::paintEvent(event);

	QPainter painter(this);

	if (m_pressed || m_checked) {
		QRect r = m_iconRect;
		r.moveLeft(r.x() + 1);
		r.moveTop(r.y() + 1);
		painter.drawPixmap(r, m_pixmap);
	} else {
		painter.drawPixmap(m_iconRect, m_pixmap);
	}
}

//==============================================================================================================================
// class ToolList
//==============================================================================================================================

struct ToolListItemData
{
	ToolListItemData(void *_item, const QString &label, void *_group)
		: itemKey(_item), groupKey(_group), itemLabel(label), action(0)
	{}
	void    *itemKey;
	void    *groupKey;
	QString  itemLabel;
	QAction *action;
};
//==============================================================================================================================

ToolList::ToolList(ToolBar *parent)
	: ToolItem(parent)
	, m_currentItemIndex(-1)
	, m_modus(ToolListInsertModus::GroupTailing)
{
	int w = 99999;
	int h = parent->iconSize().height() + parent->itemPads().top() + parent->itemPads().bottom();
//	setMinimumSize(w, h);
//	setMaximumSize(w, h);
	setDefaultSize(w, h);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

ToolList::~ToolList()
{
	clear();
}
//==============================================================================================================================

ToolListItemData *ToolList::currentItem() const
{
	return (m_currentItemIndex < 0) ? 0 : m_items.at(m_currentItemIndex);
}

void *ToolList::currentGroupKey() const
{
	ToolListItemData *item = currentItem();
	return item ? item->groupKey : 0;
}

void *ToolList::currentItemKey() const
{
	ToolListItemData *item = currentItem();
	return item ? item->itemKey : 0;
}

void ToolList::setCurrentItem(void *itemKey)
{
	int i = -1;
	findItem(i, itemKey);
	if (i >= 0) setCurrentItem(i);
}

void ToolList::setCurrentItem(int index)
{
	if (index >= m_items.count()) index = m_items.count() - 1;
	else if (index < 0 && m_items.count() > 0) index = 0;
	else if (index < 0 && index != -1) index = -1;

	m_currentItemIndex = index;
	QString tips;
	if (index >= 0) {
		ToolListItemData *item = m_items.at(index);
		tips = getGroupLabel(item->groupKey);
		tips = tips.isEmpty() ? item->itemLabel
							  : item->itemLabel + "\n" + tips;
	}
	this->setToolTip(tips);
	update();
}

void ToolList::setInsertModus(int modus)
{
	if (modus < 0 || modus > ToolListInsertModus::LastPosition) return;
	m_modus = modus;
}
//==============================================================================================================================

void ToolList::onItemClicked()
{
	QObject *obj = sender();
	QAction *act = qobject_cast<QAction *>(obj);
	if (act) {
		int i = -1;
		ToolListItemData *item = findActionItem(i, act);
		if (item) emit itemClicked(i, item->itemKey, item->groupKey);
	}
}

ToolListItemData *ToolList::findActionItem(int &retIndex, QAction *action)
{
	int i = m_items.count() - 1;
	while (i >= 0) {
		ToolListItemData *item = m_items.at(i);
		if (item->action == action) { retIndex = i; return item; }
		i--;
	}
	return 0;
}

ToolListItemData *ToolList::findItem(int &retIndex, void *itemKey)
{
	int i = m_items.count() - 1;
	while (i >= 0) {
		ToolListItemData *item = m_items.at(i);
		if (item->itemKey == itemKey) { retIndex = i; return item; }
		i--;
	}
	return 0;
}

int ToolList::getGroupTail(void *groupKey)
{
	int cnt = m_items.count();
	int i = cnt - 1;
	while (i >= 0) {
		if (m_items.at(i)->groupKey == groupKey) { i++; return i; }
		i--;
	}
	return cnt;
}

QAction *ToolList::addItem(void *itemKey, const QString &label, void *groupKey)
{
	if (!itemKey) return 0;

	ToolListItemData *item = new ToolListItemData(itemKey, label, groupKey);
	int cnt = m_items.count();
	int i = cnt;

	switch (m_modus) {
		case ToolListInsertModus::AfterCurrent: i = m_currentItemIndex + 1; break;
		case ToolListInsertModus::GroupTailing: i = getGroupTail(groupKey); break;
		case ToolListInsertModus::LastPosition: i = cnt;                    break;
	}
	m_items.insert(i, item);
	if (m_currentItemIndex < 0) setCurrentItem(i);

	cnt++;
	i++;
	QAction *before = (i < cnt) ? m_items.at(i)->action : 0;
	QAction *act = insertMenuItem(before, label, this, SLOT(onItemClicked()));
	item->action = act;
	return act;
}

void ToolList::setItemLabel(void *itemKey, const QString &label)
{
	if (!itemKey) return;

	int i = 0;
	ToolListItemData *item = findItem(i, itemKey);
	if (item) {
		item->itemLabel = label;
		if (item->action) item->action->setText(label);
	//	if (m_currentItemIndex == i)
			setCurrentItem(i);
	}
}

void ToolList::removeItem(void *itemKey)
{
//	qDebug() << "ToolList::removeItem.1" << m_items.count();
	if (!itemKey) return;

	foreach (ToolListItemData *item, m_items) {
		if (item->itemKey == itemKey) {
			m_items.removeOne(item);
			deleteItem (item);
			break;
		}
	}
	if (m_currentItemIndex >= m_items.size()) {
		m_currentItemIndex = m_items.size() - 1;
	}
	update();
//	qDebug() << "ToolList::removeItem.2" << m_items.count();
}

void ToolList::deleteItem(ToolListItemData *item)
{
	if (item->action) delete item->action;
	delete item;
}
//==============================================================================================================================

QString ToolList::getGroupLabel(void *key)
{
	QHash<void *, QString>::iterator it = m_groups.find(key);
	return (it == m_groups.end()) ? QString() : it.value();
}

void ToolList::addGroup(void *groupKey, const QString &label)
{
	m_groups.insert(groupKey, label);
}

void ToolList::setGroupLabel(void *groupKey, const QString &label)
{
	QHash<void *, QString>::iterator i = m_groups.find(groupKey);
	if (i != m_groups.end()) i.value() = label;
}

void ToolList::removeGroup(void *groupKey)
{
	if (!groupKey) return;

	int i = m_items.size();
	while (i > 0) {
		i--;
		ToolListItemData *item = m_items.at(i);
		if (item->groupKey == groupKey) {
			m_items.removeAt(i);
			deleteItem (item);
		}
	}
	m_groups.remove(groupKey);

	if (m_currentItemIndex >= m_items.size())
		m_currentItemIndex = m_items.size() - 1;
	update();
}

void ToolList::clear()
{
	foreach (ToolListItemData *item, m_items) { delete item; }
	m_items.clear();
	m_groups.clear();
	m_currentItemIndex = -1;
	update();
}
//==============================================================================================================================

void ToolList::paintEvent(QPaintEvent *event)
{
	ToolItem::paintEvent(event);

	QRect r = this->rect();
	int pad = 4;
	r.setX(pad);
	r.setWidth(r.width() - pad - pad);

	QPainter painter(this);
	QFontMetrics fm = painter.fontMetrics();
	ToolListItemData *item = currentItem();
	QString str = item ? item->itemLabel : QString();
	QString elidedText = fm.elidedText(str, Qt::ElideRight, r.width(), Qt::TextShowMnemonic);
	painter.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
}

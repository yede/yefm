#include <QDebug>

#include "yeudevview.h"
#include "yeudevnodeitem.h"
#include "yeudev.h"
#include "yeudevnode.h"
#include "yeudevutil.h"
//==============================================================================================================================

UDevView::UDevView(UDev *udev, QWidget *parent)
	: QGraphicsView(parent)
	, m_udev(udev)
	, m_margins(4, 4, 4, 4)
	, m_spacing(4)
{
	m_scene = new QGraphicsScene;
	setScene(m_scene);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);

	m_bg = new UDevNodeBg;
	m_bg->setSize(120, 120);
	m_scene->addItem(m_bg);

	m_buttonColor = palette().color(QPalette::Button);

	udev->addView(this);
	connect(udev, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
}

UDevView::~UDevView()
{
	clearIdles();
}

void UDevView::clearIdles()
{
	foreach (UDevNodeItem *item, m_idles) delete item;
	m_idles.clear();
}

UDevNodeItem *UDevView::addItem(UDevNode *node)
{
	UDevNodeItem *item;
	if (m_idles.size() > 0) {
		item = m_idles.takeAt(0);
		item->setParentItem(m_bg);
	} else {
		item = new UDevNodeItem(this);
	}
	item->setNode(node);
	return item;
}

void UDevView::removeItem(UDevNode *node)
{
	UDevNodeItem *item = findItem(node);
	if (item) {
		item->setParentItem(NULL);
		m_scene->removeItem(item);
	//	delete item;
		m_idles.append(item);
	}
}

void UDevView::updateItem(UDevNode *node)
{
	UDevNodeItem *item = findItem(node);
	if (item) item->update();
}

UDevNodeItem *UDevView::findItem(UDevNode *node)
{
	foreach (QGraphicsItem *child, m_bg->childItems()) {
		UDevNodeItem *item = static_cast<UDevNodeItem *>(child);
		if (item->node() == node) return item;
	}
	return NULL;
}

void UDevView::dataChanged()
{
	this->resize(parentWidget()->width(), this->height());
	int x = m_margins.left();
	int y = m_margins.top();
	int w = this->width();
//	qDebug() << "UDevWidget::dataChanged()" << w;

	foreach (UDevNode *node, m_udev->nodes()) {
		UDevNodeItem *item = addItem(node);
		item->setPos(x, y);
		item->updateLayout();
		y += item->height() + m_spacing;
	}

	m_bg->setSize(w, y);
}

void UDevView::updateLayout()
{
	int x = m_margins.left();
	int y = m_margins.top();
	int w = this->width();

	foreach (QGraphicsItem *child, m_bg->childItems()) {
		UDevNodeItem *item = static_cast<UDevNodeItem *>(child);
		item->setPos(x, y);
		item->updateLayout();
		y += item->height() + m_spacing;
	}

	m_bg->setSize(w, y);
}

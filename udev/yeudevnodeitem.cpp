#include <QPainter>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QMouseEvent>
#include <QDebug>

#include "yeudevnodeitem.h"
#include "yeudevview.h"
#include "yeudev.h"
#include "yeudevnode.h"
#include "yeudevutil.h"
//==============================================================================================================================

#define PAD  4
#define V_SP 4
#define H_SP 6
#define LINE 14

UDevNodeItem::UDevNodeItem(UDevView *host)
	: QGraphicsItem(host->bgItem())
	, m_host(host)
	, m_node(NULL)
	, m_w(80)
	, m_h(80)
{
}

UDevNodeItem::~UDevNodeItem()
{
}

void UDevNodeItem::setNode(UDevNode *node)
{
	m_node = node;
	if (m_node == NULL) return;

	int n = 2;
	if (!node->label().isEmpty()) n++;
	if (!node->fsType().isEmpty()) n++;
//	if (!node->uuid().isEmpty()) n++;

	m_h = n * LINE + (n - 1) * V_SP + (PAD << 1);
}

void UDevNodeItem::updateLayout()
{
	m_w = m_host->itemWidth();
	update();
}
//==============================================================================================================================

void UDevNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
//	qDebug() << "hoverEnterEvent" << m_hovering;
	Q_UNUSED(event);
	m_hover = true;
}

void UDevNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
//	qDebug() << "hoverLeaveEvent" << m_hovering;
	Q_UNUSED(event);
	m_hover = false;
}

void UDevNodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
}

void UDevNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (isUnderMouse()) {
		if (event->button() == Qt::LeftButton) {
			// FIXME: Workaround.
			// For some weird reason, if clicked() function is called directly, and menu is opened,
			// this item will receive hover enter event on menu close. But it shouldn't (mouse is outside).
			// Probably somehow related to taking a mouse grab when one is already active.
		//	QTimer::singleShot(1, this, SLOT(clicked()));
			if (m_node->isMounted()) {
				emit m_host->deviceClicked(m_node->mountPoint());
			}
		}
		if (event->button() == Qt::RightButton) {
		//	m_panelWindow->showPanelContextMenu(m_position + QPoint(static_cast<int>(event->pos().x()), static_cast<int>(event->pos().y())));
		}
	}
}
//==============================================================================================================================

void UDevNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (m_node == NULL) return;

	QRectF r = this->boundingRect();
	QColor bg = m_host->buttonColor();
	painter->fillRect(r, bg);

	int txFlags = Qt::AlignLeft | Qt::AlignVCenter;
	int x = PAD;
	int y = PAD;
	int w = m_w - (PAD << 1);
	int h = LINE;

	const QPixmap &pixmap = m_host->udev()->pixmap(m_node);
	painter->drawPixmap(x, y, pixmap);

	if (m_node->isMounted()) {
		QColor led(255, 0, 0);
	//	painter->setPen(Qt::green);
		painter->fillRect(x, y + pixmap.height() + 2, pixmap.width(), 4, led);
	}

	x += pixmap.width() + H_SP;
	painter->setPen(Qt::black);
	painter->drawText(x, y, w, h, txFlags, m_node->devicePath());

	if (!m_node->fsType().isEmpty()) {
		y += h + V_SP;
		painter->drawText(x, y, w, h, txFlags, m_node->fsType());
	}

	if (!m_node->label().isEmpty()) {
		y += h + V_SP;
		painter->drawText(x, y, w, h, txFlags, m_node->label());
	}

//	if (!m_node->uuid().isEmpty()) {
//		y += h + SP;
//		painter->drawText(x, y, w, h, txFlags, m_node->uuid());
//	}

	y += h + V_SP;
	painter->drawText(x, y, w, h, txFlags, m_node->sizeText());

/*
	QStyleOptionButton options;
	options.rect = rect();
	options.palette = palette();
	options.text = m_shortText;

	options.features |= QStyleOptionButton::Flat;
	options.state = QStyle::State_Enabled;

	if (m_active || m_pressed) {
		options.state |= QStyle::State_Sunken;
	} else {
		options.state |= QStyle::State_Raised;
	}

	if (m_hover) {
		options.state |= QStyle::State_MouseOver;
	}

	painter.drawControl(QStyle::CE_PushButton, options);

	if (!(m_active || m_pressed) && m_hover) {
		painter.setPen(m_hover ? m_hoverColor : m_otherColor);

		int x1 = rect().x();
		int y1 = rect().y();
		int x2 = x1 + rect().width() - 1;
		int y2 = y1 + rect().height() - 1;
		int xr = 2;
		int yr = 2;

		painter.drawLine(x1 + xr, y1, x2 - xr, y1);		// top line
		painter.drawLine(x1 + xr, y2, x2 - xr, y2);		// bottom line

		painter.drawLine(x1, y1 + yr, x1, y2 - yr);		// left line
		painter.drawLine(x2, y1 + yr, x2, y2 - yr);		// right line

		painter.drawLine(x1, y1 + yr, x1 + xr, y1);		// top-left corner
		painter.drawLine(x2 - xr, y1, x2, y1 + yr);		// top-right corner

		painter.drawLine(x1, y2 - yr, x1 + xr, y2);		// bottom-left corner
		painter.drawLine(x2 - xr, y2, x2, y2 - yr);		// bottom-right corner
	}
*/
}

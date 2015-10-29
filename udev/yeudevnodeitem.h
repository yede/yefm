#ifndef YE_UDEV_NODE_ITEM_H
#define YE_UDEV_NODE_ITEM_H

#include <QGraphicsItem>
//==============================================================================================================================

class UDev;
class UDevNode;
class UDevView;

class UDevNodeItem : public QGraphicsItem
{
public:
	explicit UDevNodeItem(UDevView *host);
	~UDevNodeItem();

	void setNode(UDevNode *node);
	void updateLayout();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

	int width()  const { return m_w; }
	int height() const { return m_h; }

	void set_w(int w) { m_w = w; }
	void set_h(int h) { m_h = h; }
	void setSize(int w, int h) { m_w = w; m_h = h; }

	QRectF boundingRect() const { return QRectF(0.0, 0.0, m_w, m_h); }

	UDevNode *node() const { return m_node; }

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:

private:
	UDevView *m_host;
	UDevNode   *m_node;
	int  m_w, m_h;
	bool m_active;
	bool m_hover;
	bool m_pressed;
	QColor m_hoverColor;
	QColor m_otherColor;
};

#endif

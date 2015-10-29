#ifndef YE_UDEV_WIDGET_H
#define YE_UDEV_WIDGET_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QList>
//==============================================================================================================================

class UDevNodeBg: public QGraphicsItem
{
public:
	UDevNodeBg(QGraphicsItem *parent = 0)
		: QGraphicsItem(parent)
	{
		setZValue(-10.0);
		setAcceptedMouseButtons(Qt::RightButton);
	}

	~UDevNodeBg()
	{
	}

	QRectF boundingRect() const
	{
		return QRectF(0.0, 0.0, m_w, m_h);
	}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
	{
		Q_UNUSED(painter);
		Q_UNUSED(option);
		Q_UNUSED(widget);
	}

	void set_w(int w) { m_w = w; }
	void set_h(int h) { m_h = h; }
	void setSize(int w, int h) { m_w = w; m_h = h; }

private:
	int  m_w, m_h;
};
//==============================================================================================================================

class UDev;
class UDevNode;
class UDevNodeItem;

class UDevView : public QGraphicsView
{
	Q_OBJECT
public:
	explicit UDevView(UDev *udev, QWidget *parent = 0);
	~UDevView();

	UDevNodeItem *addItem(UDevNode *node);
	void removeItem(UDevNode *node);
	void updateItem(UDevNode *node);
	void updateLayout();


	UDevNodeBg *bgItem() const { return m_bg; }
	int itemWidth() const { return width() - m_margins.left() - m_margins.right(); }

	UDev *udev() const { return m_udev; }

	const QColor &buttonColor() const { return m_buttonColor; }

private:
	UDevNodeItem *findItem(UDevNode *node);
	void clearIdles();

public slots:
	void dataChanged();

signals:
	void deviceClicked(const QString &path);

private:
	friend class UDevNodeItem;

	UDev           *m_udev;
	QGraphicsScene *m_scene;
	UDevNodeBg     *m_bg;
	QMargins        m_margins;
	int             m_spacing;

	QColor m_hoverColor;
	QColor m_buttonColor;

	QList<UDevNodeItem *> m_idles;
};

#endif

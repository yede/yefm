#include <QFontMetrics>
#include <QStylePainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QDebug>

#include "yepathbutton.h"
#include "yepathwidget.h"
//==============================================================================================================================

#define MAX_BUTTON_WIDTH 120
#define MIN_BUTTON_WIDTH 12
#define BUTTON_PADDING   12
//==============================================================================================================================

PathButton::PathButton(PathWidget *host, const QString &dir, bool active, QWidget *parent)
	: QWidget(parent)
	, m_host(host)
	, m_text(dir)
	, m_active(active)
	, m_hover(false)
	, m_pressed(false)
{
	QFontMetrics fm(this->font());
	int w = fm.width(dir);
	if (w > MAX_BUTTON_WIDTH) {
		m_width = MAX_BUTTON_WIDTH;
		m_shortText = fm.elidedText(dir, Qt::ElideRight, m_width, 0);
		setToolTip(m_shortText);
	} else {
		m_width = w < MIN_BUTTON_WIDTH ? MIN_BUTTON_WIDTH : w;
		m_shortText = dir;
	}
	m_width += BUTTON_PADDING;	// padding
	setFixedSize(m_width, m_host->buttonHeight());
	setCursor(Qt::PointingHandCursor);

	m_hoverColor = host->palette().color(QPalette::Midlight);	// Light > Midlight > Button > Mid > Dark > Shadow
	m_otherColor = host->palette().color(QPalette::Button);
}

PathButton::~PathButton()
{
}
//==============================================================================================================================

void PathButton::updateIconTheme()
{
	setFixedSize(m_width, m_host->buttonHeight());
	update();
}

QSize PathButton::sizeHint() const
{
	return QSize(m_width, m_host->buttonHeight());
}

void PathButton::setActive(bool flag)
{
	if (m_active == flag) return;

	m_active = flag;
	update();
}
//==============================================================================================================================

void PathButton::mouseMoveEvent(QMouseEvent *event)
{
	if (!m_pressed || !(event->buttons() & Qt::LeftButton))
		return;

}

void PathButton::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	m_pressed = true;
	update();
}

void PathButton::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	m_pressed = false;
	update();

	if (event->x() >= 0 && event->x() < width() && event->y() >= 0 && event->y() < height()) {
		m_host->changeWorkPath(this);
	}
}

void PathButton::enterEvent(QEvent *)
{
	m_hover = true;
	update();
}

void PathButton::leaveEvent(QEvent *)
{
	m_hover = false;
	update();
}
//==============================================================================================================================

void PathButton::paintEvent(QPaintEvent *)
{
	QStylePainter painter(this);
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
}

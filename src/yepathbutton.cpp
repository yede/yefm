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

	m_hoverColor = host->palette().color(QPalette::Light);	// Light > Midlight > Button > Mid > Dark > Shadow
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
		m_host->setWorkPath(this);
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
//	painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, m_shortText);

	if (!(m_active || m_pressed)) {
		painter.setPen(m_hover ? m_hoverColor : m_otherColor);
		int x = rect().x();
		int y = rect().y() + 1;
		int w = rect().width() - 1;
		int h = rect().height() - 3;
		painter.drawRoundedRect(x, y, w, h, 3.0, 3.0);
	}
}


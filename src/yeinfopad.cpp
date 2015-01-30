#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

#include "yeinfopad.h"
#include "yemainwindow.h"
//==============================================================================================================================

InfoPad::InfoPad(YeMainWindow *parent)
	: QWidget(parent)
	, m_win(parent)
{
	m_bgColor = QColor(128, 128, 192, 160);
	m_txColor = QColor(255, 255, 255);

	QFont ft = this->font();
	ft.setPixelSize(20);
	setFont(ft);

	m_timer.setSingleShot(true);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

InfoPad::~InfoPad()
{
}

void InfoPad::showMessage(const QString &text, const QString &title, int time)
{
	m_text = text;
	m_title = title;

	int w = m_win->width();
	int h = m_win->height();

	setGeometry(0, 0, w, h);
	show();
	update();
	m_timer.start(time);
}
//==============================================================================================================================

void InfoPad::onTimeout()
{
	hide();
}

void InfoPad::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		m_timer.stop();
		hide();
	}
}

void InfoPad::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	painter.fillRect(rect(), m_bgColor);
	painter.setPen(m_txColor);
	painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, m_text);
}

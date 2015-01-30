#include <QStylePainter>
#include <QStyleOption>
#include <QMouseEvent>

#include "yesplitterhandle.h"
#include "yesplitter.h"
//==============================================================================================================================

SplitterHandle::SplitterHandle(Splitter *parent)
	: QWidget(parent)
	, m_splitter(parent)
	, m_hover(false)
	, m_pressed(false)
	, m_startPos(0)
	, m_handleSize(4)
{
	setPalette(m_splitter->palette());
	setMouseTracking(true);
}

SplitterHandle::~SplitterHandle()
{
}

void SplitterHandle::init()
{
	if (m_splitter->isHorizontal()) {
		setCursor(Qt::SplitHCursor);
	//	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	} else {
		setCursor(Qt::SplitVCursor);
	//	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	}

	setMinimumSize(m_handleSize, m_handleSize);
}
//==============================================================================================================================

void SplitterHandle::mouseMoveEvent(QMouseEvent *event)
{
	if (!m_pressed || !(event->buttons() & Qt::LeftButton))
		return;

	bool flag = m_splitter->isHorizontal();
	int pos = flag ? event->globalX() : event->globalY();
	if (pos == m_startPos) return;

	int step = pos - m_startPos;
	int smax = flag ? (m_splitter->width() - 20) : (m_splitter->height() - 20);
	int smin = 4;
	int w = m_splitter->m_sideSize + step;

	if (w < smin) return;
	if (w > smax) return;

	m_splitter->moveHandle(step);
	m_startPos = pos;
}

void SplitterHandle::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;

	m_startPos = m_splitter->isHorizontal() ? event->globalX() : event->globalY();
	m_pressed = true;
}

void SplitterHandle::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) return;
	m_pressed = false;
}

void SplitterHandle::enterEvent(QEvent *)
{
	m_hover = true;
	update();
}

void SplitterHandle::leaveEvent(QEvent *)
{
	m_hover = false;
	update();
}
//==============================================================================================================================

void SplitterHandle::paintEvent(QPaintEvent *)
{
	QStylePainter painter(this);
	QStyleOption options;
	options.rect = rect();
	options.palette = palette();

	options.state = QStyle::State_Enabled;

	if (m_splitter->isHorizontal()) {
		options.state |= QStyle::State_Horizontal;
	}

	if (m_hover) {
		options.state |= QStyle::State_MouseOver;
	}

	painter.drawControl(QStyle::CE_Splitter, options);
}

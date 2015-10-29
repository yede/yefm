#include <QDebug>

#include "yesplitter.h"
#include "yesplitterhandle.h"
//==============================================================================================================================

Splitter::Splitter(QWidget *parent)
	: QWidget(parent)
	, m_client0(NULL)
	, m_client1(NULL)
	, m_direction(SplitterDirection::Left)
	, m_sideSize(10)
{
	m_handle = new SplitterHandle(this);
}

Splitter::~Splitter()
{
}
//==============================================================================================================================

bool Splitter::isHorizontal() const
{
	switch (m_direction) {
		case SplitterDirection::Top:
		case SplitterDirection::Bottom: return false;
	//	case SplitterDirection::Left:
	//	case SplitterDirection::Right:
	}

	return true;
}

int Splitter::handleSize() const
{
	return m_handle->handleSize();
}

void Splitter::setDirection(int dir, int sideSize)
{
	m_direction = dir;
	m_sideSize = sideSize;
	m_handle->init();
}

void Splitter::setClient(QWidget *c0, QWidget *c1)
{
	m_client0 = c0;
	m_client1 = c1;

	c0->setParent(this);
	c1->setParent(this);
}
//==============================================================================================================================

void Splitter::moveHandle(int step)
{
	switch (m_direction) {
		case SplitterDirection::Top:
		case SplitterDirection::Left:   m_sideSize += step; break;
		case SplitterDirection::Bottom:
		case SplitterDirection::Right:  m_sideSize -= step; break;
	}

	moveResize();
}

void Splitter::moveResize()
{
//	qDebug() << "Splitter::moveResize.0" << m_client0->geometry() << m_client1->geometry()
//			 << m_handle->isVisible() << m_client0->isVisible() << m_client1->isVisible();
	int x, y, w, h;

	if (!m_client0->isVisible() || !m_client1->isVisible()) {
		x = y = 0;
		w = width();
		h = qMax(height(), 120);
		if (m_handle->isVisible()) m_handle->hide();
		if (m_client0->isVisible()) { m_client0->setGeometry(x, y, w, h); m_sideSize = h; }
		if (m_client1->isVisible()) { m_client1->setGeometry(x, y, w, h); }
//		qDebug() << "Splitter::moveResize.1" << m_client0->geometry() << m_client1->geometry()
//				 << m_handle->isVisible() << m_client0->isVisible() << m_client1->isVisible();
		return;
	}

	if (!m_handle->isVisible()) {
		m_handle->setVisible(true);
	}

	if (isHorizontal()) {
		m_handle->setFixedSize(m_handle->m_handleSize, height());
	} else {
		m_handle->setFixedSize(width(), m_handle->m_handleSize);
	}

	switch (m_direction) {
		case SplitterDirection::Left:
			x = y = 0;
			w = m_sideSize;
			h = height();
			m_client0->setGeometry(x, y, w, h);

			x += w;
			m_handle->move(x, y);

			x += m_handle->width();
			w = width() - x;
			m_client1->setGeometry(x, y, w, h);
			break;

		case SplitterDirection::Right:
			x = y = 0;
			w = width() - m_handle->width() - m_sideSize;
			h = height();
			m_client1->setGeometry(x, y, w, h);

			x += w;
			m_handle->move(x, y);

			x += m_handle->width();
			w = m_sideSize;
			m_client0->setGeometry(x, y, w, h);
			break;

		case SplitterDirection::Top:
			x = y = 0;
			w = width();
			h = m_sideSize;
			m_client0->setGeometry(x, y, w, h);

			y += h;
			m_handle->move(x, y);

			y += m_handle->height();
			h = height() - y;
			m_client1->setGeometry(x, y, w, h);
			break;

		case SplitterDirection::Bottom:
		default:
			x = y = 0;
			w = width();
			h = height() - m_handle->height() - m_sideSize;
			m_client1->setGeometry(x, y, w, h);

			y += h;
			m_handle->move(x, y);

			y += m_handle->height();
			h = m_sideSize;
			m_client0->setGeometry(x, y, w, h);
	}
//	qDebug() << "Splitter::moveResize.2" << m_client0->geometry() << m_client1->geometry() << m_handle->isVisible();
}
//==============================================================================================================================

void Splitter::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
//	qDebug() << "Splitter::resizeEvent" << this->geometry();

	moveResize();
}

void Splitter::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);

	moveResize();
}

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

#include "yepastetaskbutton.h"
#include "yepastetask.h"
#include "yepastemgr.h"
//==============================================================================================================================

#define MAX_BUTTON_WIDTH 120
#define MIN_BUTTON_WIDTH 30
#define BUTTON_PADDING   12
//==============================================================================================================================

PasteTaskButton::PasteTaskButton(PasteTask *task, PasteMgr *mgr, QWidget *parent)
	: QWidget(parent)
	, m_task(task)
	, m_mgr(mgr)
	, m_active(false)
{
	m_text = task->taskName();

	QFontMetrics fm(this->font());
	int w = fm.width(m_text);
	if (w > MAX_BUTTON_WIDTH) {
		w = MAX_BUTTON_WIDTH;
		m_shortText = fm.elidedText(m_text, Qt::ElideRight, w, 0);
		setToolTip(m_shortText);
	} else {
		w = w < MIN_BUTTON_WIDTH ? MIN_BUTTON_WIDTH : w;
		m_shortText = m_text;
	}
	w += BUTTON_PADDING;				// padding
	setFixedSize(w, buttonHeight());
	setCursor(Qt::PointingHandCursor);
}

int PasteTaskButton::buttonHeight()
{
	return 26;
}

void PasteTaskButton::setActive(bool flag)
{
	m_active = flag;
	update();
}
//==============================================================================================================================

void PasteTaskButton::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		m_mgr->setCurrentTask(m_task);
	}
}

void PasteTaskButton::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	painter.fillRect(rect(), m_active ? m_task->activeColor() : m_task->buttonColor());
	painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, m_shortText);

	painter.setPen(m_task->mgr()->lineColor());
	painter.drawLine(rect().topRight(), rect().bottomRight());
}

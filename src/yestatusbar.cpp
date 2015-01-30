#include <QFontMetrics>
#include <QStylePainter>
#include <QStyleOption>
#include <QDebug>

#include "yestatusbar.h"
#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
#include "yefilepane.h"
#include "yestylesettings.h"
//==============================================================================================================================

StatusBar::StatusBar(YeMainWindow *parent)
	: QWidget(parent)
	, m_win(parent)
	, m_multiColor(false)
	, m_spacing(8)
{
	m_widths.reserve(32);
	m_timer.setSingleShot(true);

	updateSettings();

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	connect(m_win->app(), SIGNAL(settingsChanged()), this, SLOT(updateSettings()));
}

StatusBar::~StatusBar()
{

}

void StatusBar::showMessage(const QString &message, int pos, int time)
{
	m_multiColor = false;
	m_message = message;

	showMessage(pos, time);
}

void StatusBar::showMessage(const QStringList &messages, int pos, int time)
{
	m_multiColor = true;
	m_msgs = messages;

	int cnt = m_msgs.size();
	int n = m_widths.size();
	while (n < cnt) {
		m_widths.append(0);
		n++;
	}

	QFontMetrics fm(this->font());
	for (int i = 0; i < cnt; i++) {
		const QString msg = m_msgs.at(i);
		m_widths[i] = fm.width(msg);
	}

	showMessage(pos, time);
}

void StatusBar::showMessage(int pos, int time)
{
	YeFilePane *pane = (pos == 0) ? m_win->pane0() : m_win->pane1();

	int x = 0;
	int h = 22;
	int y = pane->y() + pane->height() - h;
	int w = pane->width();

	setGeometry(x, y, w, h);
	show();
	update();
	m_timer.start(time);
}

void StatusBar::onTimeout()
{
	hide();
}

void StatusBar::updateSettings()
{
	FmStyle::StatBarStyle &m = R::app()->fmStyle()->stStyle;

	m_bgColor = m.useWinBg ? m_win->palette().color(QPalette::Window) : m.bgColor;
	m_fgColor = m.fgColor;

	QFont ft = font();
	ft.setFamily(m.fontName);
	ft.setPixelSize(m.fontSize);
	setFont(ft);

	m_colors.clear();
	m_colors << m.pmsColor << m.usrColor << m.grpColor << m.sizeColor << m.dateColor << m.timeColor << m.fileColor;
}
//==============================================================================================================================

void StatusBar::enterEvent(QEvent *)
{
	m_timer.stop();
	hide();
}

void StatusBar::leaveEvent(QEvent *)
{
}

void StatusBar::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);

	hide();
}

void StatusBar::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	const QRect &r = rect();
	int x = r.x() + 8;
	int y = r.y();
	int w = r.width() - 16;
	int h = r.height();

	painter.fillRect(r, m_bgColor);

	if (m_multiColor) {
		int colorCount = m_colors.size();
		int cnt = m_msgs.size();
		QColor color;

		for (int i = 0; i < cnt; i++) {
			const QString &msg = m_msgs.at(i);
			int z = m_widths.at(i);
			color = (i < colorCount) ? m_colors.at(i) : m_fgColor;
			painter.setPen(color);
			painter.drawText(x, y, z, h, Qt::AlignLeft | Qt::AlignVCenter, msg);
			x += z + m_spacing;
		}

		x -= m_spacing;
		w = r.width() - 8;
		if (x > w) {
			x = w - 12;
			w = 20;
			painter.fillRect(x, y, w, h, m_bgColor);
			x ++;   painter.drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignVCenter, ".");
			x += 3; painter.drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignVCenter, ".");
			x += 3; painter.drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignVCenter, ".");
		}

	} else {
		painter.setPen(m_fgColor);
		painter.drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignVCenter, m_message);
	}
}

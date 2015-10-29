#include <QFontMetrics>
#include <QStylePainter>
#include <QStyleOption>
#include <QDebug>

#include "yestatusbar.h"
#include "yemainwindow.h"
#include "yeapplication.h"
#include "yefilepane.h"

#include "yeiconloader.h"
#include "yeappcfg.h"
#include "yeapp.h"
#include "yestyle.h"
//==============================================================================================================================

StatusBar::StatusBar(bool floating, MainWindow *parent)
	: QWidget(parent)
	, m_win(parent)
	, m_floating(floating)
	, m_multiColor(false)
	, m_slideShow(false)
	, m_slideStep(0)
	, m_spacing(8)
	, m_timer(NULL)
{
	m_widths.reserve(32);
	m_height = AppCfg::instance()->statusbarHeight;
	if (m_height < 12 || m_height > 40) m_height = 20;

	updateSettings();

	connect(App::app(), SIGNAL(settingsChanged()), this, SLOT(updateSettings()));

	if (floating) {
		setupTimer();
	} else {
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		setFixedHeight(m_height);
	}
}

StatusBar::~StatusBar()
{
}

void StatusBar::setupTimer()
{
	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void StatusBar::showMessage(const QString &message, int pos, int time)
{
	m_multiColor = false;
	m_message = message;

	showMessage(pos, time);
}
#define use_slide_show 0

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
	int totalWidth = 0;
	for (int i = 0; i < cnt; i++) {
		const QString msg = m_msgs.at(i);
		m_widths[i] = fm.width(msg);
		totalWidth += m_widths[i] + m_spacing;
	}

#if use_slide_show
	m_slideShow = totalWidth > width();
	m_slideStep = 0;
	if (m_slideShow && m_timer == NULL) setupTimer();
#endif

	showMessage(pos, time);
}

void StatusBar::showMessage(int pos, int time)
{
	if (m_timer != NULL && m_timer->isActive()) m_timer->stop();

	if (!m_floating) {
		update();
		return;
	}

	FilePane *pane = (pos == 0) ? m_win->pane0() : m_win->pane1();
	int x = 0;
	int h = m_height;
	int y = pane->y() + pane->height() - h;
	int w = pane->width();

	setGeometry(x, y, w, h);
	show();
	update();

	Q_ASSERT(m_timer != NULL);
	m_timer->start(time);
}

void StatusBar::onTimeout()
{
	if (m_slideShow) {
		update();
	} else {
		hide();
	}
}

void StatusBar::updateSettings()
{
	Style::StatBarStyle &m = Style::instance()->stStyle;

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
	if (m_floating) {
		Q_ASSERT(m_timer != NULL);
		m_timer->stop();
		hide();
	}
}

void StatusBar::leaveEvent(QEvent *)
{
}

void StatusBar::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);

	if (m_floating) {
		hide();
	}
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
		int pos = 0, cnt = m_msgs.size();
		QColor color;

		if (m_slideShow) {
			if (m_slideStep % 2) { pos = cnt - 1; } else { cnt--; }
			m_slideStep++;
			if (m_slideStep < 16) m_timer->start(1800);
		}

		for (int i = pos; i < cnt; i++) {
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

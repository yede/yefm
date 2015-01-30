#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"

#include "yesplitter.h"
#include "yefilepane.h"
#include "yestatusbar.h"
#include "yeinfopad.h"
#include "yefsmodel.h"
//==============================================================================================================================

YeMainWindow::YeMainWindow(YeApplication *app, QWidget *parent)
	: QMainWindow(parent)
	, m_app(app)
	, m_pane0(NULL)
	, m_ready(false)
{
}

YeMainWindow::~YeMainWindow()
{
}

void YeMainWindow::showStatusMessage(const QString &message, int pos, int time)
{
	m_statueBar->showMessage(message, pos, time);
}

void YeMainWindow::showStatusMessage(const QStringList &messages, int pos, int time)
{
	m_statueBar->showMessage(messages, pos, time);
}

void YeMainWindow::showPadMessage(const QString &text, const QString &title, int time)
{
	m_infoPad->showMessage(text, title, time);
}
//==============================================================================================================================

void YeMainWindow::setupWindow()
{
	QWidget *central = new QWidget;
	setCentralWidget(central);

	m_statueBar = new StatusBar(this);
	m_statueBar->hide();

	m_infoPad = new InfoPad(this);
	m_infoPad->hide();

	m_pane0 = new YeFilePane(this);
	m_pane1 = new YeFilePane(this);

	loadSessionData();
	int h = m_sessionData.geometry.height();

	m_splitter = new Splitter;
	m_splitter->setDirection(SplitterDirection::Top, h);
	m_splitter->setClient(m_pane0, m_pane1);

	QVBoxLayout *box = new QVBoxLayout(central);
	box->setSpacing(0);
	box->setContentsMargins(1, 0, 1, 0);
	box->addWidget(m_splitter);
}
//==============================================================================================================================

void YeMainWindow::startSession()
{
	initWindowGeometry();
	m_pane0->loadSessionTabs();
	m_pane1->loadSessionTabs();
	m_pane1->setVisible(false);
	show();
	m_ready = true;
}

void YeMainWindow::saveSessionData()
{
	SessionData &m = m_sessionData;
	QList<QVariant> cols;
	int cnt = m.cols.size();
	for (int i = 0; i < cnt; i++) {
		cols.append(m.cols.at(i));
	}

	QSettings s(App::getSessionFile(), QSettings::IniFormat);
	s.beginGroup("window");
	s.setValue("maximized", m.maximized);
	s.setValue("geometry", m.geometry);
	s.setValue("pane1Height", m.pane1Height);
	s.setValue("cols", cols);
	s.endGroup();

	m_pane0->saveSessionTabs();
	m_pane1->saveSessionTabs();
}

void YeMainWindow::loadSessionData()
{
	SessionData &m = m_sessionData;
	QList<QVariant> cols;

	QSettings s(App::getSessionFile(), QSettings::IniFormat);
	s.beginGroup("window");
	m.maximized = s.value("maximized", false).toBool();
	m.geometry = s.value("geometry", QRect(0, 0, 0, 0)).toRect();
	m.pane1Height = s.value("pane1Height", 0).toInt();
	cols = s.value("cols").toList();
	s.endGroup();

	if (m.geometry.width() < 10 || m.geometry.y() < 10) {
		QDesktopWidget *d = m_app->desktop();
		const QRect &r = d->availableGeometry();
		m.geometry = QRect(2, 22, r.width() >> 1, r.height() >> 1);
	}

	FsModel::initColWidths(m.cols);
	int cnt = m.cols.size();
	int max = cols.size();
	int i = 0;

	while (i < cnt && i < max) {
		m.cols[i] = cols.at(i).toInt();
		i++;
	}
}

void YeMainWindow::initWindowGeometry()
{
	SessionData &m = m_sessionData;
	setGeometry(m.geometry);
	updateGeometry();

	if (m.maximized) {
		Qt::WindowStates states = windowState() | Qt::WindowMaximized;
		setWindowState(states);
	}
}

void YeMainWindow::toggle2pane(bool active)
{
	QDesktopWidget *d = m_app->desktop();
	QRect a = d->availableGeometry();
	QRect f = frameGeometry();
	QRect m = geometry();
	int t = f.height() - m.height();

	int x = m.x();
	int y = m.y();
	int w = m.width();
	int h;

	if (active) {
		ensure2paneVisible();
		if (R::data().expand2paneWinHeight) {
			h = a.height() - t;
		} else {
			h = m_sessionData.pane1Height >= 200 ? m_sessionData.pane1Height : m_splitter->sideSize();
			h = qMin(a.height() - t, (m_splitter->sideSize() + h) + m_splitter->handleSize());
		}
	} else {
		h = m_splitter->sideSize();
	}

	setGeometry(x, y, w, h);	// exclude title-bar
	updateGeometry();
}

void YeMainWindow::on2pane(bool active)
{
	if (m_pane1->isVisible() == active) return;

	m_pane1->setVisible(active);

	if (isMaximized() || isFullScreen()) {
		if (active) ensure2paneVisible();
		m_splitter->moveResize();
	} else {
		toggle2pane(active);
	}
}

void YeMainWindow::ensure2paneVisible()
{
	int h = m_splitter->sideSize();
	if (h < 200) {
		m_splitter->setSideSize(200);
	} else {
		int a = m_app->desktop()->availableGeometry().height();
		if (a - h < 220) m_splitter->setSideSize(a - 220);
	}
}
//==============================================================================================================================

void YeMainWindow::traceWindowGeometry()
{
	if (!m_ready) return;

	m_sessionData.geometry = geometry();

	if (m_pane1->isVisible()) {
		if (!R::data().expand2paneWinHeight) m_sessionData.pane1Height = m_pane1->height();
		m_sessionData.geometry.setHeight(m_pane0->height());
	}
}

void YeMainWindow::moveEvent(QMoveEvent *)
{
	if (!m_sessionData.maximized) traceWindowGeometry();
}

void YeMainWindow::resizeEvent(QResizeEvent *)
{
	m_sessionData.maximized = isMaximized();
	if (!m_sessionData.maximized) traceWindowGeometry();

	if (m_statueBar->isVisible()) m_statueBar->hide();
	if (m_infoPad->isVisible())   m_infoPad->hide();
}

void YeMainWindow::closeEvent(QCloseEvent *event)
{
	saveSessionData();
	event->accept();
}

#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QTime>
#include <QDebug>

#include "yeapp.h"
#include "yeappcfg.h"
#include "yesessiondata.h"

#include "yeapplication.h"
#include "yemainwindow.h"

#include "yesplitter.h"
#include "yefilepane.h"
#include "yestatusbar.h"
#include "yeinfopad.h"
#include "yefsmodel.h"

#include "yepastemgr.h"
//==============================================================================================================================

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_app(Application::uiapp())
	, m_cfg(AppCfg::instance())
	, m_pane0(NULL)
	, m_statueBa2(NULL)
	, m_statusbarFloating(false)
	, m_ready(false)
{
	m_win = this;
}

MainWindow::~MainWindow()
{
	m_win = NULL;
}

void MainWindow::showStatusMessage(const QString &msg, int paneIndex, int time)
{
	StatusBar *bar = (m_statusbarFloating || paneIndex == 0) ? m_statueBar : m_statueBa2;
	Q_ASSERT(bar != NULL);
	bar->showMessage(msg, paneIndex, time);
}

void MainWindow::showStatusMessage(const QStringList &msgList, int paneIndex, int time)
{
	StatusBar *bar = (m_statusbarFloating || paneIndex == 0) ? m_statueBar : m_statueBa2;
	Q_ASSERT(bar != NULL);
	bar->showMessage(msgList, paneIndex, time);
}

void MainWindow::showPadMessage(const QString &text, const QString &title, int time)
{
	m_infoPad->showMessage(text, title, time);
}

void MainWindow::showPastePad()
{
	m_pasteMgr->showPad();
}
//==============================================================================================================================

void MainWindow::setupWindow()
{
	loadSessionData();

	SessionData *d = SessionData::instance();
	int h = d->geometry.height();

	QWidget *central = new QWidget;
	setCentralWidget(central);

	m_statusbarFloating = m_cfg->statusbarFloating;
	m_statueBar = new StatusBar(m_statusbarFloating, this);

	m_pasteMgr = new PasteMgr(NULL);
	m_pasteMgr->hide();
	connect(m_app, SIGNAL(iconThemeChanged()), m_pasteMgr, SIGNAL(iconThemeChanged()));

	m_infoPad = new InfoPad(this);
	m_infoPad->hide();

	if (m_statusbarFloating) {
		m_statueBar->hide();
		m_pane0 = new FilePane(NULL, 0);
		m_pane1 = new FilePane(NULL, 1);
	} else {
		m_statueBa2 = new StatusBar(m_statusbarFloating, this);
		m_pane0 = new FilePane(m_statueBar, 0);
		m_pane1 = new FilePane(m_statueBa2, 1);
	}

	m_splitter = new Splitter;
	m_splitter->setDirection(SplitterDirection::Top, h);
	m_splitter->setClient(m_pane0, m_pane1);

	QVBoxLayout *box = new QVBoxLayout(central);
	box->setSpacing(0);
	box->setContentsMargins(1, 0, 1, 0);
	box->addWidget(m_splitter);

	connect(m_app, SIGNAL(statusMessage(QString,int,int)), this, SLOT(showStatusMessage(QString,int,int)));
	connect(m_app, SIGNAL(statusMessage(QStringList,int,int)), this, SLOT(showStatusMessage(QStringList,int,int)));
	connect(m_app, SIGNAL(padMessage(QString,QString,int)), this, SLOT(showPadMessage(QString,QString,int)));
}
//==============================================================================================================================

void MainWindow::startSession(int &argc, char **argv)
{
	initWindowGeometry();

	QStringList paths;
	int i = 1;
	while (i < argc) {
		QString path = argv[i];
		paths.append(path);
		i++;
	}
	m_pane0->loadSessionTabs(paths);
	m_pane1->loadSessionTabs(QStringList());

	m_pane1->setVisible(false);
	showUI();
}

void MainWindow::saveSessionData()
{
	m_pane0->updateSessionData();
	m_pane1->updateSessionData();

	SessionData *d = SessionData::instance();
	QList<QVariant> cols;
	int cnt = d->cols.size();
	for (int i = 0; i < cnt; i++) {
		cols.append(d->cols.at(i));
	}

	QSettings s(App::getSessionFile(), QSettings::IniFormat);
	s.beginGroup("window");
	s.setValue("maximized", d->maximized);
	s.setValue("geometry" , d->geometry);
	s.setValue("pane1_height", d->pane1_height);
	s.setValue("side0_width" , d->side0_width);
	s.setValue("side1_width" , d->side1_width);
	s.setValue("tabs0_height", d->tabs0_height);
	s.setValue("tabs1_height", d->tabs1_height);
	s.setValue("cols", cols);
	s.endGroup();

	m_pane0->saveSessionTabs();
	m_pane1->saveSessionTabs();
}

void MainWindow::loadSessionData()
{
	SessionData *d = SessionData::instance();
	QList<QVariant> cols;

	QSettings s(App::getSessionFile(), QSettings::IniFormat);
	s.beginGroup("window");
	d->maximized    = s.value("maximized", false).toBool();
	d->geometry     = s.value("geometry", QRect(0, 0, 0, 0)).toRect();
	d->pane1_height = s.value("pane1_height", 0).toInt();
	d->side0_width  = s.value("side0_width" , 180).toInt();
	d->side1_width  = s.value("side1_width" , 180).toInt();
	d->tabs0_height = s.value("tabs0_height", 120).toInt();
	d->tabs1_height = s.value("tabs1_height", 120).toInt();
	cols = s.value("cols").toList();
	s.endGroup();

	if (d->geometry.width() < 10 || d->geometry.y() < 10) {
		QDesktopWidget *wid = m_app->desktop();
		const QRect &r = wid->availableGeometry();
		d->geometry = QRect(2, 22, r.width() >> 1, r.height() >> 1);
	}

	FsModel::initColWidths(d->cols);
	int cnt = d->cols.size();
	int max = cols.size();
	int i = 0;

	while (i < cnt && i < max) {
		d->cols[i] = cols.at(i).toInt();
		i++;
	}
}

void MainWindow::initWindowGeometry()
{
	SessionData *d = SessionData::instance();
	adjustSize();
	setGeometry(d->geometry);

	if (d->maximized) {
		Qt::WindowStates states = windowState() | Qt::WindowMaximized;
		setWindowState(states);
	}
//	qDebug() << "initWindowGeometry()";
}

void MainWindow::toggle2pane(bool active)
{
	SessionData *d = SessionData::instance();
	QDesktopWidget *wid = m_app->desktop();
	QRect a = wid->availableGeometry();
	QRect f = frameGeometry();

	int t = f.height() - d->geometry.height();

	int x = d->geometry.x();
	int y = d->geometry.y();
	int w = d->geometry.width();
	int h;

	if (active) {
		ensure2paneVisible();
		if (m_cfg->expand2paneWinHeight) {
			h = a.height() - t;
		} else {
			h = d->pane1_height >= 200 ? d->pane1_height : m_splitter->sideSize();
			h = qMin(a.height() - t, (m_splitter->sideSize() + h) + m_splitter->handleSize());
		}
	} else {
		h = m_splitter->sideSize();
	}

	setGeometry(x, y, w, h);	// exclude title-bar
}

void MainWindow::on2pane(bool active)
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

void MainWindow::ensure2paneVisible()
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

void MainWindow::openFolder(const QString &path)
{
	m_pane0->addTab(path, true);
	activateWindow();
}

void MainWindow::showUI()
{
	show();
//	qDebug() << "showUI()";

	QTimer::singleShot(4000, this, SLOT(onTimeout()));
}

void MainWindow::onTimeout()
{
	m_ready = true;
//	qDebug() << "onTimeout()" << m_ready << QTime::currentTime().toString("HH:mm:ss.zzz");
}

void MainWindow::traceWindowGeometry()
{
	SessionData *d = SessionData::instance();
	d->geometry = geometry();

	if (m_pane1->isVisible()) {
		if (!m_cfg->expand2paneWinHeight) d->pane1_height = m_pane1->height();
		d->geometry.setHeight(m_pane0->height());
	}
}

void MainWindow::moveEvent(QMoveEvent *)
{
//	qDebug() << "moveEvent()" << m_ready << QTime::currentTime().toString("HH:mm:ss.zzz") << m_sessionData.geometry << geometry();
	if (m_ready) {
		SessionData *d = SessionData::instance();
		if (!d->maximized) traceWindowGeometry();
	}
}

void MainWindow::resizeEvent(QResizeEvent *)
{
//	qDebug() << "resizeEvent()" << m_ready << QTime::currentTime().toString("HH:mm:ss.zzz") << m_sessionData.geometry << geometry();
	if (m_ready) {
		SessionData *d = SessionData::instance();
		d->maximized = isMaximized();
		if (!d->maximized) traceWindowGeometry();

		if (m_statusbarFloating && m_statueBar->isVisible()) m_statueBar->hide();
		if (m_infoPad->isVisible()) m_infoPad->hide();
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (m_pasteMgr->isBusy()) {
		m_pasteMgr->showPad();
		event->ignore();
		App::message(tr("Paste manager is busy."));
	} else {
		saveSessionData();
		event->accept();
		if (m_pasteMgr->parent() == NULL) delete m_pasteMgr;
	}
}

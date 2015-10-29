#ifndef YEMAINWINDOW_H
#define YEMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
//==============================================================================================================================

class Application;
class AppCfg;

class FilePane;
class Splitter;
class StatusBar;
class InfoPad;
class PasteMgr;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();

	void showPastePad();

	void setupWindow();
	void startSession(int &argc, char **argv);

	static MainWindow *win() { return m_win; }

	FilePane    *pane0()    const { return m_pane0; }
	FilePane    *pane1()    const { return m_pane1; }
	PasteMgr    *pastePad() const { return m_pasteMgr; }

protected:
	void moveEvent(QMoveEvent *event);
	void resizeEvent(QResizeEvent *event);
	void closeEvent(QCloseEvent *event);

private:
	void loadSessionData();
	void saveSessionData();
	void initWindowGeometry();
	void toggle2pane(bool active);
	void ensure2paneVisible();
	void traceWindowGeometry();
	void showUI();

signals:

public slots:
	void openFolder(const QString &path);
	void on2pane(bool active);
	void onTimeout();
	void showStatusMessage(const QString &msg, int paneIndex, int time);
	void showStatusMessage(const QStringList &msgList, int paneIndex, int time);
	void showPadMessage(const QString &text, const QString &title, int time);

private:
	Application *m_app;
	AppCfg      *m_cfg;
	FilePane    *m_pane0;
	StatusBar   *m_statueBa2;
	bool m_statusbarFloating;
	bool m_ready;

	Splitter   *m_splitter;
	FilePane   *m_pane1;
	StatusBar  *m_statueBar;
	InfoPad    *m_infoPad;
	PasteMgr   *m_pasteMgr;

	static MainWindow *m_win;
};

#endif // YEMAINWINDOW_H

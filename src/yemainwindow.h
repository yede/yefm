#ifndef YEMAINWINDOW_H
#define YEMAINWINDOW_H

#include <QMainWindow>
//==============================================================================================================================

struct SessionData {
	bool  maximized;
	QRect geometry;		// pane0
	int   pane1Height;
	QList<int> cols;
};

class YeApplication;
class YeFilePane;
class Splitter;
class StatusBar;
class InfoPad;

class YeMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit YeMainWindow(YeApplication *app, QWidget *parent = 0);
	virtual ~YeMainWindow();

	void showStatusMessage(const QString &message, int pos, int time);
	void showStatusMessage(const QStringList &messages, int pos, int time);
	void showPadMessage(const QString &text, const QString &title, int time);

	void setupWindow();
	void startSession();

	YeApplication *app()   const { return m_app; }
	YeFilePane    *pane0() const { return m_pane0; }
	YeFilePane    *pane1() const { return m_pane1; }

	SessionData   &sessionData() { return m_sessionData; }

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

signals:

public slots:
	void on2pane(bool active);

private:
	YeApplication *m_app;
	YeFilePane *m_pane0;
	bool m_ready;

	Splitter   *m_splitter;
	YeFilePane *m_pane1;
	StatusBar  *m_statueBar;
	InfoPad    *m_infoPad;

	SessionData m_sessionData;
};

#endif // YEMAINWINDOW_H

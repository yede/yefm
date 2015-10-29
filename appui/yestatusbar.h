#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>
#include <QTimer>
//==============================================================================================================================

class Application;
class MainWindow;

class StatusBar : public QWidget
{
	Q_OBJECT
public:
	explicit StatusBar(bool floating, MainWindow *parent);
	~StatusBar();

	void showMessage(const QString &message, int pos, int time);
	void showMessage(const QStringList &messages, int pos, int time);

private:
	void setupTimer();
	void showMessage(int pos, int time);

protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

signals:

public slots:
	void onTimeout();
	void updateSettings();

private:
	MainWindow *m_win;
	bool m_floating;
	bool m_multiColor;
	bool m_slideShow;
	int  m_slideStep;
	int  m_spacing;
	QTimer *m_timer;

	int m_height;
	QList<QColor> m_colors;
	QStringList   m_msgs;
	QList<int> m_widths;
	QString m_message;
	QColor m_bgColor;
	QColor m_fgColor;
};

#endif // STATUSBAR_H

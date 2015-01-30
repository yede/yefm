#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>
#include <QTimer>
//==============================================================================================================================

class YeApplication;
class YeMainWindow;

class StatusBar : public QWidget
{
	Q_OBJECT
public:
	explicit StatusBar(YeMainWindow *parent = 0);
	~StatusBar();

	void showMessage(const QString &message, int pos, int time);
	void showMessage(const QStringList &messages, int pos, int time);

private:
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
	YeMainWindow *m_win;
	bool m_multiColor;
	int m_spacing;
	QList<QColor> m_colors;
	QStringList   m_msgs;
	QList<int> m_widths;
	QString m_message;
	QColor m_bgColor;
	QColor m_fgColor;
	QTimer m_timer;
};

#endif // STATUSBAR_H

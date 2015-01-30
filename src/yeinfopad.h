#ifndef INFOPAD_H
#define INFOPAD_H

#include <QWidget>
#include <QTimer>
//==============================================================================================================================

class YeMainWindow;

class InfoPad : public QWidget
{
	Q_OBJECT
public:
	explicit InfoPad(YeMainWindow *parent = 0);
	~InfoPad();

	void showMessage(const QString &text, const QString &title, int time);

protected:
	void mousePressEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

signals:

public slots:
	void onTimeout();

private:
	YeMainWindow *m_win;
	QTimer  m_timer;
	QString m_text;
	QString m_title;
	QColor  m_bgColor;
	QColor  m_txColor;
};

#endif // INFOPAD_H

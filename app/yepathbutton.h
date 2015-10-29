#ifndef PATHBUTTON_H
#define PATHBUTTON_H

#include <QWidget>
//==============================================================================================================================

class PathWidget;

class PathButton : public QWidget
{
	Q_OBJECT
public:
	explicit PathButton(PathWidget *host, const QString &dir, bool active = false, QWidget *parent = 0);
	~PathButton();

	bool isActive() const { return m_active; }
	void setActive(bool flag);
	void updateIconTheme();

	QString dir() const { return m_text; }
	QSize sizeHint() const;

protected:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void paintEvent(QPaintEvent *event);

signals:

public slots:

private:
	PathWidget *m_host;
	QString m_text;
	bool m_active;
	bool m_hover;
	bool m_pressed;
	int  m_width;
	QString m_shortText;
	QColor m_hoverColor;
	QColor m_otherColor;
};

#endif // PATHBUTTON_H

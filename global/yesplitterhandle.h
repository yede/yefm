#ifndef SPLITTERHANDLE_H
#define SPLITTERHANDLE_H

#include <QWidget>
//==============================================================================================================================

class Splitter;

class SplitterHandle : public QWidget
{
	Q_OBJECT
public:
	explicit SplitterHandle(Splitter *parent);
	~SplitterHandle();

	int  handleSize() const { return m_handleSize; }

protected:
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

private:
	friend class Splitter;

	void init();

signals:

public slots:

private:
	Splitter *m_splitter;
	bool m_hover;
	bool m_pressed;
	int m_startPos;
	int m_handleSize;
};

#endif // SPLITTERHANDLE_H

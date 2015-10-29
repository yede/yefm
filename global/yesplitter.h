#ifndef SPLITTER_H
#define SPLITTER_H

#include <QWidget>
//==============================================================================================================================

namespace SplitterDirection {
	enum { Left, Right, Top, Bottom };
}

class SplitterHandle;

class Splitter : public QWidget
{
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = 0);
	~Splitter();

	void setDirection(int dir, int sideSize);
	void setClient(QWidget *c0, QWidget *c1);
	void moveResize();

	bool isHorizontal() const;
	int  handleSize() const;
	int  sideSize() const { return m_sideSize; }
	void setSideSize(int size) { m_sideSize = size; }

protected:
	void moveHandle(int step);
	void resizeEvent(QResizeEvent *event);
	void showEvent(QShowEvent *event);

signals:

public slots:

private:
	friend class SplitterHandle;
	SplitterHandle *m_handle;

	QWidget *m_client0;
	QWidget *m_client1;

	int m_direction;	// SplitterDirection
	int m_sideSize;		//

};

#endif // SPLITTER_H

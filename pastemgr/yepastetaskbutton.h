#ifndef PASTETASKBUTTON_H
#define PASTETASKBUTTON_H

#include <QWidget>
//==============================================================================================================================

class PasteTask;
class PasteMgr;

class PasteTaskButton : public QWidget
{
	Q_OBJECT
public:
	explicit PasteTaskButton(PasteTask *task, PasteMgr *mgr, QWidget *parent = 0);

	void setActive(bool flag);

	PasteTask *task() const { return m_task; }

	static int buttonHeight();

protected:
	void mousePressEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

signals:

public slots:

private:
	PasteTask *m_task;
	PasteMgr  *m_mgr;
	bool m_active;
	QString m_text, m_shortText;
};

#endif // PASTETASKBUTTON_H

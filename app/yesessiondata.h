#ifndef YE_SESSION_DATA_H
#define YE_SESSION_DATA_H

#include <QRect>
#include <QList>
//==============================================================================================================================

struct SessionData
{
	SessionData();

	bool  maximized;
	QRect geometry;		// pane0
	int   pane1_height;
	int   side0_width;
	int   tabs0_height;
	int   side1_width;
	int   tabs1_height;
	QList<int> cols;

	static SessionData *instance() { return m_instance; }
	static void createInstance()   { if (!m_instance) m_instance = new SessionData; }
	static void deleteInstance()   { delete m_instance; m_instance = NULL; }

private:
	static SessionData *m_instance;
};

#endif

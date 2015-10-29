#ifndef YE_MIME_ACTIONS_H
#define YE_MIME_ACTIONS_H

#include <QObject>
#include <QAction>
#include <QHash>
//==============================================================================================================================

class QFileInfo;
class Mime;

class MimeActions : public QObject
{
	Q_OBJECT
public:
	explicit MimeActions(QObject *parent = 0);
	~MimeActions();

	void addActions(QMenu &menu, const QFileInfo &current);

	bool isReady() const { return m_ready; }

private:
	QAction *createAction(const QString &appName, QObject *parent, bool isOpenWith);

signals:

public slots:
	void selectApp();

private:
	Mime *m_mime;
	bool  m_ready;
	bool  m_dummy_1, m_dummy_2, m_dummy_3, m_dummy_4, m_dummy_5, m_dummy_6, m_dummy_7;
};
//==============================================================================================================================

#endif // YE_MIME_ACTIONS_H

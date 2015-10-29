#ifndef YE_FS_WATCHER_H
#define YE_FS_WATCHER_H

#include <QObject>
#include <QSocketNotifier>
#include <QList>
#include <QHash>
#include <QTimer>
//==============================================================================================================================

struct inotify_event;

class FsNode;
class FsDirModel;

class FsWatcher : public QObject
{
	Q_OBJECT
public:
	FsWatcher(FsDirModel *parent);
	~FsWatcher();

	void registerWatcher(const QString &path);

private:
	void init();
	bool haltWarning() const;
	void unregisterWatcher();
	void removeWatcher(int watchId);

	void dumpEvent(const inotify_event *event);

signals:

private slots:
	void notify(int socket);
	void timeout();

private:
	FsDirModel *m_engine;

	bool m_halt;
	bool dummy_1, dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;

	int              m_notifyFD;
	QSocketNotifier *m_notifier;
	QTimer          *m_timer;
	int              m_watchId;
};

#endif	// YE_FS_WATCHER_H

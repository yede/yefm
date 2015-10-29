#ifndef YE_UDEV_MONITOR_H
#define YE_UDEV_MONITOR_H

#include <QThread>
//==============================================================================================================================

struct udev_monitor;
class UDev;

class UDevMonitor : public QThread
{
	Q_OBJECT
public:
	explicit UDevMonitor(UDev *parent);
	~UDevMonitor();

	void ensureStop();

private:
	bool runLoop();
	void handleMountEvent(int mountsFd);
	void handleUDevEvent(udev_monitor *mon);
	void emitQuitSignal();

protected:
	void run();

public slots:

signals:
	void udevActive(int action, const QByteArray &sysPath);
	void mountsChanged(const QStringList &devList);

private:
	UDev *m_udev;
	int   m_quitFd;
	bool  m_abort;
};
//==============================================================================================================================

#endif

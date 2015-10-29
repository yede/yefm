#ifndef YE_UDEV_H
#define YE_UDEV_H

#include <QObject>
#include <QPixmap>
#include <QList>
//==============================================================================================================================

struct udev;
struct udev_device;

class UDevNode;
class UDevMonitor;
class UDevView;

class UDev : public QObject
{
	Q_OBJECT
public:
	enum Action { actionNone, actionAdd, actionRemove, actionMove, actionChange };

	explicit UDev(QObject *parent = 0);
	~UDev();

	bool listDevices();

	void start();
	void stop();
	void addView(UDevView *view);

	const char *actionText(int action) const;
	QList<UDevNode *> nodes() { return m_nodes; }

	const QPixmap &pixmap(UDevNode *node) const;
	const QPixmap &pixUsb() const { return m_pixUsb; }
	const QPixmap &pixDsk() const { return m_pixDsk; }

	static UDev *instance()      { return m_instance; }
	static void createInstance() { if (!m_instance) m_instance = new UDev; }
	static void deleteInstance() { delete m_instance; m_instance = NULL; }

private:
	void setupParent(UDevNode *node, const QByteArray &sysPath);
	void getDevicePath(QString &devicePath, QString &parentPath, const QByteArray &sysPath);
	UDevNode *findNode(const QString &devPath);

	UDevNode *addNode(udev *_udev, const char *sysPath);
	void removeNode(UDevNode *node);
	void clearNodes();

	void createItems(UDevNode *node);
	void removeItems(UDevNode *node);
	void updateItems(UDevNode *node);
	void updateLayout();

public slots:
	void udevActive(int action, const QByteArray &sysPath);
	void mountsChanged(const QStringList &devList);

signals:
	void dataChanged();

private:
	UDevMonitor *m_monitor;
	QPixmap      m_pixDsk;
	QPixmap      m_pixUsb;

	udev *m_udev;

	QList<UDevView *> m_views;
	QList<UDevNode *> m_nodes;
	QList<UDevNode *> m_mounts;
	QList<UDevNode *> m_idles;

	static UDev *m_instance;
};

#endif

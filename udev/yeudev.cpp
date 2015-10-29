#include <stdio.h>
#include <libudev.h>

#include <QDebug>

#include "yeudevview.h"
#include "yeudev.h"
#include "yeudevmonitor.h"
#include "yeudevnode.h"
#include "yeudevutil.h"
//==============================================================================================================================
#define IMG_DSK "/home/nat/ydev/yede/projects/yefm-src/data/img-16/drive-harddisk-usb.png"
#define IMG_USB "/home/nat/ydev/yede/projects/yefm-src/data/img-16/usb-stick.png"

UDev::UDev(QObject *parent)
	: QObject(parent)
	, m_pixDsk(IMG_DSK)
	, m_pixUsb(IMG_USB)
{
	m_monitor = new UDevMonitor(this);
	m_udev = udev_new();
}

UDev::~UDev()
{
	udev_unref(m_udev);
	stop();
	clearNodes();
}

void UDev::start()
{
	if (m_nodes.isEmpty()) {
		listDevices();
	}
	m_monitor->start();
}

void UDev::stop()
{
	m_monitor->ensureStop();
}

void UDev::addView(UDevView *view)
{
	if (!m_views.contains(view)) m_views.append(view);
}
//==============================================================================================================================

const QPixmap &UDev::pixmap(UDevNode *node) const
{
	return node->mediaType() == UDevNode::mediaUsbStick ? m_pixUsb : m_pixDsk;
}

bool UDev::listDevices()
{
	if (!m_udev) {
		printf("UDev::listDevices(): Can't create udev\n");
		return false;
	}

	udev_enumerate *enumerate = udev_enumerate_new(m_udev);
	udev_enumerate_add_match_subsystem(enumerate, "block");
	udev_enumerate_scan_devices(enumerate);
	udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
	udev_list_entry *entry;

	udev_list_entry_foreach(entry, devices) {
		// Get the filename of the /sys entry for the device and create a udev_device object (dev) representing it
		const char *path = udev_list_entry_get_name(entry);
		addNode(m_udev, path);
	}

	udev_enumerate_unref(enumerate);
//	printf("UDev::listDevices(): done!\n");

	emit dataChanged();

	return true;
}

UDevNode *UDev::addNode(udev *_udev, const char *sysPath)
{
	if (_udev == NULL) return NULL;

	udev_device *dev = udev_device_new_from_syspath(_udev, sysPath);
//	printf("UDev::listDevices(): syspath: %s\n" , sysPath);

	UDevNode *node;
	if (m_idles.size() > 0) {
		node = m_idles.takeAt(0);
	} else {
		node = new UDevNode();
	}
	node->init(dev);
//	node->dump();

	if (node->isPartition()) setupParent(node, sysPath);
	m_nodes.append(node);

	udev_device_unref(dev);

	return node;
}

void UDev::removeNode(UDevNode *node)
{
	m_mounts.removeAll(node);
	m_nodes.removeAll(node);
//	delete node;
	m_idles.append(node);
}

void UDev::clearNodes()
{
	foreach (UDevNode *node, m_nodes) delete node;
	foreach (UDevNode *node, m_idles) delete node;
	m_nodes.clear();
	m_idles.clear();
	m_mounts.clear();
}

void UDev::setupParent(UDevNode *node, const QByteArray &sysPath)
{
	QString devPath, diskPath;
	getDevicePath(devPath, diskPath, sysPath);

	if (!diskPath.isEmpty()) {
		UDevNode *parent = findNode(diskPath);
		if (parent->isDisk()) node->setParent(parent);
	}
}

void UDev::getDevicePath(QString &devicePath, QString &parentPath, const QByteArray &sysPath)
{
	static const QByteArray block_tag("/block/");
	static const QChar sep('/');

	int i = sysPath.indexOf(block_tag);
	if (i < 0) return;

	QByteArray dev_str = sysPath.mid(i + block_tag.length());
	i = dev_str.indexOf(sep);
	if (i > 0) {
		parentPath = QString("/dev/") + dev_str.left(i);
		devicePath = QString("/dev/") + dev_str.mid(i + 1);
	} else {
		devicePath = QString("/dev/") + dev_str;
	}
}

UDevNode *UDev::findNode(const QString &devPath)
{
	for (int i = m_nodes.size() - 1; i >= 0; i--) {
		UDevNode *node = m_nodes.at(i);
		if (node->devicePath() == devPath) return node;
	}
	return NULL;
}
//==============================================================================================================================

const char *UDev::actionText(int action) const
{
	switch (action) {
		case actionAdd   : return "add";
		case actionRemove: return "remove";
		case actionMove  : return "move";
		case actionChange: return "change";
	}
	return "none";
}

void UDev::udevActive(int action, const QByteArray &sysPath)
{
	QString devicePath, parentPath;
	getDevicePath(devicePath, parentPath, sysPath);
//	printf("UDev::udevActive(): action: %s, devicePath: %s, parentPath: %s\n" , actionText(action), qPrintable(devicePath), qPrintable(parentPath));

	UDevNode *node = findNode(devicePath);

	if (action == actionAdd) {
		if (node == NULL) {
			node = addNode(m_udev, sysPath.constData());
			if (node != NULL) createItems(node);
		}
	}
	else if (action == actionRemove) {
		if (node != NULL) removeItems(node);
	}

	updateLayout();
}

void UDev::mountsChanged(const QStringList &devList)
{
	QList<UDevNode *> mounts;

	foreach (const QString &dev, devList) {
		QStringList list = dev.split(" ");
		list.removeAll("");
		if (list.size() != 6) continue;
	//	printf("\n");
	//	for (int i = 0; i < list.count(); i++) printf("UDev::mountsChanged(): [%d] %s\n", i, qPrintable(list.at(i)));
		UDevNode *node = findNode(list.at(0));
		if (node) {
			node->setMountInfo(list.at(1), list.at(3));
			mounts.append(node);
		}
	}

	foreach (UDevNode *node, m_mounts) {					// old-mounts
		if (!mounts.contains(node)) {
			node->clearMountInfo();
			updateItems(node);
		}
	}

	foreach (UDevNode *node, mounts) {						// new-mounts
		if (!m_mounts.contains(node)) updateItems(node);
	}

	m_mounts.clear();
	m_mounts = mounts;
}

void UDev::createItems(UDevNode *node)
{
	foreach (UDevView *view, m_views) { view->addItem(node); }
}

void UDev::removeItems(UDevNode *node)
{
	foreach (UDevView *view, m_views) { view->removeItem(node); }
	removeNode(node);
}

void UDev::updateItems(UDevNode *node)
{
	foreach (UDevView *view, m_views) { view->updateItem(node); }
}

void UDev::updateLayout()
{
	foreach (UDevView *view, m_views) { view->updateLayout(); }
}

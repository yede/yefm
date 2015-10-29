#include <stdio.h>
#include <libudev.h>

#include <QDebug>

#include "yeudevnode.h"
#include "yeudevutil.h"
#include "yeglobal.h"
//==============================================================================================================================

UDevNode::UDevNode()
	: m_parent(NULL)
	, m_size(0)
	, m_major(0)
	, m_minor(0)
{
}

UDevNode::~UDevNode()
{
}

QString UDevNode::getDevType() const
{
	if (m_devType == Part) return QObject::tr("Partition");
	if (m_devType == Disk) return QObject::tr("Disk");
	return QObject::tr("Unknown");
}

void UDevNode::setMountInfo(const QString &point, const QString &opts)
{
	m_mountPoint = point;
	m_mountOptions = opts;
}

void UDevNode::clearMountInfo()
{
	m_mountPoint.clear();
	m_mountOptions.clear();
}
//==============================================================================================================================

void UDevNode::init(udev_device *dev)
{
	m_parent = NULL;
	clearMountInfo();

	dev_t devnum = udev_device_get_devnum(dev);
	m_major = gnu_dev_major(devnum);
	m_minor = gnu_dev_minor(devnum);

	m_devicePath = udev_device_get_devnode(dev);

	const char *devType = udev_device_get_devtype(dev);
	if (strcmp(devType, "partition") == 0) m_devType = Part;
	else if (strcmp(devType, "disk") == 0) m_devType = Disk;
	else                                   m_devType = None;

	m_fsType = udev_device_get_property_value(dev, "ID_FS_TYPE");
	m_label  = udev_device_get_property_value(dev, "ID_FS_LABEL");
	m_uuid   = udev_device_get_property_value(dev, "ID_FS_UUID");

	const char *logical_block_size = udev_device_get_sysattr_value(dev, "queue/logical_block_size");
	const char *sz = udev_device_get_sysattr_value(dev, "size");
	int block_size = logical_block_size ? atoll(logical_block_size) : 512;
	qint64 sz_raw = sz ? atoll(sz) : 0;

	m_size = sz_raw * block_size;
	m_icon = UDevUtil::getIcon(dev);
	m_sizeText = G::formatSize(m_size);
	m_mediaType = m_icon.indexOf("usb") > 0 ? mediaUsbStick : mediaHardDrive;
}

void UDevNode::dump()
{
	qDebug("==================================");
	qDebug("Device: %s %d:%d (%s)", qPrintable(m_devicePath), m_major, m_minor, qPrintable(getDevType()));
	qDebug("FsType: %s", qPrintable(m_fsType));
	qDebug("Label : %s", qPrintable(m_label));
	qDebug("Uuid  : %s", qPrintable(m_uuid));
	qDebug("Size  : %s", qPrintable(m_sizeText));
	qDebug("Icon  : %s", qPrintable(m_icon));
}

#ifndef YE_UDEV_NODE_H
#define YE_UDEV_NODE_H

#include <QObject>
//==============================================================================================================================

struct udev_device;

class UDevNode
{
public:
	enum DevType { None, Disk, Part };
	enum MediaType { mediaUnknown, mediaHardDrive, mediaUsbStick, mediaOptical, mediaFdd };

	UDevNode(const QString &devicePath, UDevNode *parent);
	UDevNode();
	~UDevNode();

	void init(udev_device *dev);
	void dump();

	void setParent(UDevNode *parent) { m_parent = parent; }
	void setMountInfo(const QString &point, const QString &opts);
	void clearMountInfo();

	const QString &mountPoint()   const { return m_mountPoint; }
	const QString &mountOptions() const { return m_mountOptions; }

	QString getDevType()  const;
	bool    isPartition() const { return m_devType == Part; }
	bool    isDisk()      const { return m_devType == Disk; }
	bool    isMounted()   const { return !m_mountPoint.isEmpty(); }

	const QString &devicePath() const { return m_devicePath; }
	const QString &fsType()     const { return m_fsType; }
	const QString &label()      const { return m_label; }
	const QString &uuid()       const { return m_uuid; }
	const QString &icon()       const { return m_icon; }
	const QString &sizeText()   const { return m_sizeText; }
	int            mediaType()  const { return m_mediaType; }
	int            devType()    const { return m_devType; }
	qint64         devSize()    const { return m_size; }

private:

private:
	UDevNode *m_parent;
	qint64    m_size;
	quint32   m_major, m_minor;

	QString m_devicePath;
	QString m_mountPoint;
	QString m_mountOptions;

	DevType m_devType;
	QString m_fsType;
	QString m_label;
	QString m_uuid;
	QString m_icon;
	QString m_sizeText;

	MediaType m_mediaType;
};

#endif

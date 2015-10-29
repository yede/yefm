#ifndef YE_UDEV_UTIL_H
#define YE_UDEV_UTIL_H

#include <QObject>
//==============================================================================================================================

struct udev_device;

class UDevUtil
{
public:
	static const char *getIcon(udev_device *dev);
};

#endif

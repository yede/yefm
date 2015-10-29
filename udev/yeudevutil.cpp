#include <string.h>
#include <libudev.h>

#include "yeudevutil.h"
//==============================================================================================================================

/* how long to show notifications */
#define NOTIFICATION_TIMEOUT         10000

/* this icons shown */
#define ICON_DEVICE_MAPPER           "media-playlist-shuffle"
#define ICON_DRIVE_HARDDISK          "drive-harddisk"
#define ICON_DRIVE_HARDDISK_IEEE1394 "drive-harddisk-ieee1394"
#define ICON_DRIVE_HARDDISK_USB      "drive-harddisk-usb"
#define ICON_DRIVE_OPTICAL           "drive-optical"
#define ICON_DRIVE_MULTIDISK         "drive-multidisk"
#define ICON_LOOP                    "media-playlist-repeat"
#define ICON_MEDIA_FLASH             "media-flash"
#define ICON_MEDIA_FLOPPY            "media-floppy"
#define ICON_MEDIA_OPTICAL_CD_AUDIO  "media-optical-cd-audio"
#define ICON_MEDIA_REMOVABLE         "media-removable"
#define ICON_MEDIA_ZIP               "media-zip"
#define ICON_MULTIMEDIA_PLAYER       "multimedia-player"
#define ICON_NETWORK_SERVER          "network-server"
#define ICON_UNKNOWN                 "dialog-question"


const char *UDevUtil::getIcon(udev_device *dev)
{
	const char *icon = ICON_UNKNOWN;

	/* decide about what icon to use */
	const char *value = udev_device_get_property_value(dev, "ID_BUS");
	const char *device = udev_device_get_sysname(dev);
//	printf("ID_BUS: %s\n", value);		// usb, ata
//	printf("sysname: %s\n", device);	// sda, sda1

	if (udev_device_get_property_value(dev, "ID_CDROM") != NULL) { /* optical drive */
		if (udev_device_get_property_value(dev, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO") != NULL) {
			icon = ICON_MEDIA_OPTICAL_CD_AUDIO;
		} else {
			icon = ICON_DRIVE_OPTICAL;
		}
	} else if (udev_device_get_property_value(dev, "ID_DRIVE_FLOPPY") != NULL) { /* floppy drive */
		icon = ICON_MEDIA_FLOPPY;
	} else if (udev_device_get_property_value(dev, "ID_DRIVE_THUMB") != NULL) { /* thumb drive, e.g. USB flash */
		icon = ICON_MEDIA_REMOVABLE;
	} else if (udev_device_get_property_value(dev, "ID_DRIVE_FLASH_CF") != NULL ||
			udev_device_get_property_value(dev, "ID_DRIVE_FLASH_MS") != NULL ||
			udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SD") != NULL ||
			udev_device_get_property_value(dev, "ID_DRIVE_FLASH_SM") != NULL) { /* flash card reader */
				/* note that usb card reader are recognized as USB hard disk */
		icon = ICON_MEDIA_FLASH;
	} else if (udev_device_get_property_value(dev, "ID_DRIVE_FLOPPY_ZIP") != NULL) { /* ZIP drive */
		icon = ICON_MEDIA_ZIP;
	} else if (udev_device_get_property_value(dev, "ID_MEDIA_PLAYER") != NULL) { /* media player */
		icon = ICON_MULTIMEDIA_PLAYER;
	} else if (udev_device_get_property_value(dev, "DM_NAME") != NULL) { /* device mapper */
		icon = ICON_DEVICE_MAPPER;
	} else if (udev_device_get_property_value(dev, "MD_NAME") != NULL) { /* multi disk */
		icon = ICON_DRIVE_MULTIDISK;
	} else if (strncmp(device, "loop", 4) == 0 ||
			strncmp(device, "ram", 3) == 0) { /* loop & RAM */
		icon = ICON_LOOP;
	} else if (strncmp(device, "nbd", 3) == 0) { /* network block device */
		icon = ICON_NETWORK_SERVER;
	} else if (value != NULL) {
		if (strcmp(value, "ata") == 0 ||
				strcmp(value, "scsi") == 0) { /* internal (s)ata/scsi hard disk */
			icon = ICON_DRIVE_HARDDISK;
		} else if (strcmp(value, "usb") == 0) { /* USB hard disk */
			icon = ICON_DRIVE_HARDDISK_USB;
		} else if (strcmp(value, "ieee1394") == 0) { /* firewire hard disk */
			icon = ICON_DRIVE_HARDDISK_IEEE1394;
		}
	}

	return icon;
}

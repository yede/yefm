
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/eventfd.h>
#include <libudev.h>

#include "yeudevmonitor.h"
#include "yeudev.h"
//==============================================================================================================================

UDevMonitor::UDevMonitor(UDev *parent)
	: QThread(parent)
	, m_udev(parent)
	, m_quitFd(0)
	, m_abort(false)
{
	connect(this, SIGNAL(udevActive(int,QByteArray)), parent, SLOT(udevActive(int,QByteArray)));
	connect(this, SIGNAL(mountsChanged(QStringList)), parent, SLOT(mountsChanged(QStringList)));
}

UDevMonitor::~UDevMonitor()
{
	ensureStop();
}
//==============================================================================================================================

static int getAction(const char *actionStr)
{
	if (strcmp(actionStr, "add"   ) == 0) return UDev::actionAdd;
	if (strcmp(actionStr, "remove") == 0) return UDev::actionRemove;
	if (strcmp(actionStr, "move"  ) == 0) return UDev::actionMove;
	if (strcmp(actionStr, "change") == 0) return UDev::actionChange;

	return UDev::actionNone;	// we should never get here I think...
}

void UDevMonitor::handleUDevEvent(udev_monitor *mon)
{
	struct udev_device *dev = udev_monitor_receive_device(mon);
	if (dev) {
		int action = getAction(udev_device_get_action(dev));
		if (action == UDev::actionAdd || action == UDev::actionRemove) {
			QByteArray sysPath = udev_device_get_syspath(dev);
			emit udevActive(action, sysPath);
		}
		udev_device_unref(dev);
	} else {
		printf("UDevMonitor::handleUDevEvent(): No Device from receive_device(). An error occured.\n");
	}
}

void UDevMonitor::handleMountEvent(int mountsFd)
{
	QStringList devList;
	QByteArray fileData;
	int size = 4096, readin = 0;

	while (1) {
		lseek(mountsFd, 0, SEEK_SET);
		fileData.resize(size);
		readin = read(mountsFd, fileData.data(), size);
	//	printf("UDevMounts::runLoop(): bufSize=%d, readin=%d\n", size, readin);
		if (size > readin) break;
		size += 2048;
	}
	fileData.resize(readin);
//	printf("UDevMounts::runLoop(): size=%d, data=\n%s\n", fileData.size(), fileData.data());

	QList<QByteArray> lines = fileData.split('\n');
	foreach (QByteArray line, lines) {
		if (line.startsWith("/dev/")) devList.append(QString(line));
	}

	emit mountsChanged(devList);
}
//==============================================================================================================================

#define MOUNTS "/proc/mounts"

bool UDevMonitor::runLoop()
{
	bool result = false;
	udev *_udev = NULL;
	udev_monitor *mon = NULL;
	fd_set readFds, exceptFds;
	int monFd, maxFd;
	int mountsFd = m_quitFd = 0;
//	int loop = 0;

	_udev = udev_new();
	if (_udev == NULL) {
		printf("UDevMonitor::runLoop(): Can't create udev.\n");
		goto quit;
	}

	mon = udev_monitor_new_from_netlink(_udev, "udev");
	if (mon == NULL) {
		printf("UDevMonitor::runLoop(): Can't create monitor.\n");
		goto quit;
	}

	mountsFd = open(MOUNTS, O_RDONLY, 0);
	if (mountsFd < 0) {
		printf("UDevMonitor::runLoop(): Failed open file %s .\n", MOUNTS);
		goto quit;
	}

	m_quitFd = eventfd(0, EFD_NONBLOCK);
	if (m_quitFd < 0) {
		printf("UDevMonitor::runLoop(): Failed to create eventfd: (result fd: %d)", m_quitFd);
		goto quit;
	}

	udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
	udev_monitor_enable_receiving(mon);

	handleMountEvent(mountsFd);	// setup mounts infomation
	result = true;
	monFd  = udev_monitor_get_fd(mon);
	maxFd  = qMax(monFd, qMax(mountsFd, m_quitFd)) + 1;
//	printf("UDevMonitor::runLoop(): m_quitFd=%d, loop=%d\n", m_quitFd, fd);

	while (!m_abort) {
	//	printf("UDevMonitor::runLoop(): loop=%d\n", loop++);

		FD_ZERO(&readFds);
		FD_SET(monFd, &readFds);
		FD_SET(m_quitFd, &readFds);		// for waking up select()

		FD_ZERO(&exceptFds);
		FD_SET(mountsFd, &exceptFds);

		int ret = select(maxFd, &readFds, NULL, &exceptFds, NULL);
		if (m_abort) break;
		if (ret < 0) {
			perror("UDevMonitor::runLoop(): select error!\n");
			continue;
		}

		if (FD_ISSET(mountsFd, &exceptFds)) {
		//	printf("UDevMonitor::runLoop(): mounts changed!\n");
			handleMountEvent(mountsFd);
		}

		if (FD_ISSET(monFd, &readFds)) {
		//	printf("UDevMonitor::runLoop(): udev active!\n");
			handleUDevEvent(mon);
		}
	//	msleep(50);
	}
//	printf("UDevMonitor::runLoop(): quit\n");

quit:
	if (m_quitFd  > 0) close(m_quitFd);
	if (mountsFd  > 0) close(mountsFd);
	if (mon   != NULL) udev_monitor_unref(mon);
	if (_udev != NULL) udev_unref(_udev);

	return result;
}

void UDevMonitor::run()
{
//	qDebug("ThreadId=%p %s", QThread::currentThreadId(), __PRETTY_FUNCTION__);
	m_abort = false;

	runLoop();
}
//==============================================================================================================================

void UDevMonitor::emitQuitSignal()
{
	if (m_quitFd < 1) return;

	eventfd_t tmp = 1;
//	printf("UDevMonitor::emitQuitSignal(): %d\n", (int)tmp);

	eventfd_write(m_quitFd, tmp);
}

void UDevMonitor::ensureStop()
{
//	int loop = 0;
	m_abort = true;
	while (isRunning()) {
//		printf("UDevMonitor::ensureStop(): %d\n", loop++);
		emitQuitSignal();
		msleep(10);
	}
}
//==============================================================================================================================

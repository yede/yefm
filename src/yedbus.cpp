#include <stdio.h>

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDir>
#include <QDebug>

#include "yedbus.h"
#include "yeapp.h"
#include "yeapplication.h"
#include "yemainwindow.h"
#include "yefilepane.h"
//==============================================================================================================================

#define DBUS_APP_NAME     "yefm"
#define DBUS_SERVICE_NAME "com.yefm.DBus"
#define DBUS_SERVICE_PATH "/service"

QString DBus::appName()  { return QString(DBUS_APP_NAME); }
//==============================================================================================================================

DBus::DBus(YeApplication *app, QObject *parent)
	: QObject(parent)
	, m_app(app)
	, m_hasPriorInstance(false)
{
	startService();
}

DBus::~DBus()
{
	stopService();
}
//==============================================================================================================================

void DBus::openFolder(const QString &path)
{
	m_app->win()->pane0()->addTab(path, true);
//	qDebug() << "DBus: openFolder()" << path;
	m_app->win()->activateWindow();
}

void DBus::callPriorInstance(int &argc, char **argv)
{
	QDBusInterface iface(DBUS_SERVICE_NAME, DBUS_SERVICE_PATH, "", QDBusConnection::sessionBus());
	if (!iface.isValid()) {
		qDebug() << "DBus::callPriorInstance(): !iface.isValid()";
		return;
	}

	for (int i = 1; i < argc; i++) {
	//	qDebug() << "DBus::callPriorInstance()" << i << argv[i];
		QString path = argv[i];
		if (QDir(path).exists()) {
			QDBusMessage reply = iface.call("openFolder", path);
		//	qDebug() << "DBus::callPriorInstance(): err=" << reply.errorMessage() << path;
		}
	}
}
//==============================================================================================================================

void DBus::startService()
{
	QDBusConnection bus = QDBusConnection::sessionBus();	// qdbusviewer
	bool ok = bus.isConnected();

	if (!ok) {
		qDebug("Cannot connect to the D-Bus session bus.\n"
			   "To start it, run:\n"
			   "  dbus-launch --auto-syntax");
		return;
	}

	QString name = DBUS_SERVICE_NAME;
	ok = bus.registerService(name);
	if (!ok) {
		qDebug() << QString("The name \"%1\" is already registered by another application.").arg(name);
		m_hasPriorInstance = true;
		return;
	}

	ok = bus.registerObject(DBUS_SERVICE_PATH, this, QDBusConnection::ExportAllSlots);
	if (!ok) {
		qDebug() << bus.lastError().message();
		qDebug() << "Failed: bus.registerObject()";
	} else {
	//	qDebug() << "Done: bus.registerObject()";
	}
}

void DBus::stopService()
{

//	QDBusConnection bus = QDBusConnection::sessionBus();
//	bus.unregisterObject(SERVER_PATH);
//	bus.unregisterService(SERVER_NAME);

}

#ifndef DBUS_H
#define DBUS_H

#include <QObject>
#include <QHash>
//==============================================================================================================================

class YeApplication;

class DBus : public QObject
{
    Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "com.yefm.DBus.iface")

public:
	explicit DBus(YeApplication *app, QObject *parent = 0);
	~DBus();

	void callPriorInstance(int &argc, char **argv);

	bool hasPriorInstance() const { return m_hasPriorInstance; }
	static QString appName();

private:
	void startService();
	void stopService();

signals:

public slots:
	void openFolder(const QString &path);

private:
	YeApplication *m_app;
	bool m_hasPriorInstance;
};

#endif // DBUS_H

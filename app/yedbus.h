#ifndef YE_DBUS_H
#define YE_DBUS_H

#include <QObject>
#include <QHash>
//==============================================================================================================================

class DBus : public QObject
{
    Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "com.yefm.DBus.iface")

public:
	explicit DBus(QObject *parent = 0);
	~DBus();

	void callPriorInstance(int &argc, char **argv);

	bool hasPriorInstance() const { return m_hasPriorInstance; }
	static QString appName();

private:
	void startService();
	void stopService();

signals:
	void openFolder(const QString &path);

public slots:
	void callOpenFolder(const QString &path);

private:
	bool m_hasPriorInstance;
};

#endif // YE_DBUS_H

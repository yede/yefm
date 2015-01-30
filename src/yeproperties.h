#ifndef YE_PROPERTIES_H
#define YE_PROPERTIES_H

#include <QVariant>
#include <QObject>
#include <QMap>
//==============================================================================================================================

class Properties
{
public:
	explicit Properties(const QString &fileName = "", const QString &group = "");
	explicit Properties(const Properties &other);

	QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
	bool load(const QString &fileName, const QString &group = "");
	bool save(const QString &fileName, const QString &group = "");
	void set(const QString &key, const QVariant &value);
	bool contains(const QString &key) const;
	QStringList getKeys() const;

protected:
	QMap<QString, QVariant> data;
};

#endif // YE_PROPERTIES_H

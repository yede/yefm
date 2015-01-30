#include <QTextStream>
#include <QStringList>
#include <QFile>

#include "yeproperties.h"
//==============================================================================================================================

Properties::Properties(const QString &fileName, const QString &group)
{
	if (!fileName.isEmpty()) {
		load(fileName, group);
	}
}

Properties::Properties(const Properties &other)
{
	this->data = other.data;
}

bool Properties::load(const QString &fileName, const QString &group)
{

	// NOTE: This class is used for reading of property files instead of QSettings
	// class, which considers separator ';' as comment

	// Try open file
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}

	// Clear old data
	data.clear();

	// Indicator whether group was found or not, if name of group was not
	// specified, groupFound is always true
	bool groupFound = group.isEmpty();

	// Read propeties
	QTextStream in(&file);
	while (!in.atEnd()) {

		// Read new line
		QString line = in.readLine();

		// Skip empty line or line with invalid format
		if (line.trimmed().isEmpty()) {
			continue;
		}

		// Read group
		// NOTE: symbols '[' and ']' can be found not only in group names, but
		// only group can start with '['
		if (!group.isEmpty() && line.trimmed().startsWith("[")) {
			QString tmp = line.trimmed().replace("[", "").replace("]", "");
			groupFound = group.trimmed().compare(tmp) == 0;
		}

		// If we are in correct group and line contains assignment then read data
		if (groupFound && line.contains("=")) {
			QStringList tmp = line.split("=");
			data.insert(tmp.at(0), tmp.at(1));
		}
	}
	file.close();
	return true;
}

bool Properties::save(const QString &fileName, const QString &group)
{

	// Try open file
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}

	// Write group
	QTextStream out(&file);
	if (!group.isEmpty()) {
		out << "[" + group + "]\n";
	}

	// Write data
	foreach (QString key, data.keys()) {
		out << key << "=" << data.value(key).toString() << "\n";
	}

	// Exit
	file.close();
	return true;
}

bool Properties::contains(const QString &key) const
{
	return data.contains(key);
}

QVariant Properties::value(const QString &key, const QVariant &defaultValue)
{
	return data.value(key, defaultValue);
}

void Properties::set(const QString &key, const QVariant &value)
{
	if (data.contains(key)) {
		data.take(key);
	}
	data.insert(key, value);
}

QStringList Properties::getKeys() const
{
	return data.keys();
}

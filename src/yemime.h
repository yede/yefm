#ifndef YE_MIMEUTILS_H
#define YE_MIMEUTILS_H

#include <QMenu>
#include <QFileInfo>
#include "yeproperties.h"
//==============================================================================================================================

class QMimeDatabase;
class QMimeType;

class YeApplication;

class Mime : public QObject
{
	Q_OBJECT
public:
	explicit Mime(YeApplication *app, QObject *parent = 0);
	virtual ~Mime();

	void setDefaultsFileName(const QString &fileName);
	void setDefault(const QString &mime, const QStringList &apps);

	QStringList getDefault(const QString &mime) const;
	QStringList getMimeTypes() const;
	QString getDefaultsFileName() const;

	QString getMimeType(const QString &fileName) const;
	QString genericIconName(const QFileInfo &fileInfo) const;
	QString iconName(const QFileInfo &fileInfo) const;

public slots:
	void generateDefaults();
	void saveDefaults();
	void loadDefaults();

private:
	YeApplication *m_app;
	QMimeDatabase *m_db;
	bool defaultsChanged;
	QString defaultsFileName;
	Properties *defaults;
};

#endif // YE_MIMEUTILS_H

#ifndef YE_MIMEUTILS_H
#define YE_MIMEUTILS_H

#include <QObject>
#include <QFileInfo>
//==============================================================================================================================

class QMimeDatabase;
class QMimeType;
class Properties;

class Mime : public QObject
{
	Q_OBJECT
public:
	explicit Mime(QObject *parent = 0);
	virtual ~Mime();

	void setDefaultsFileName(const QString &fileName);
	void setDefault(const QString &mime, const QStringList &apps);

	QStringList getDefault(const QString &mime) const;
	QStringList getMimeTypes() const;
	QString getDefaultsFileName() const;

	QString getMimeType(const QString &fileName) const;
	QString genericIconName(const QFileInfo &fileInfo) const;
	QString iconName(const QFileInfo &fileInfo) const;

	static Mime *instance()      { return m_instance; }
	static void createInstance() { if (!m_instance) m_instance = new Mime; }
	static void deleteInstance() { delete m_instance; m_instance = NULL; }

public slots:
	void generateDefaults();
	void saveDefaults();
	void loadDefaults();

private:
	QMimeDatabase *m_db;
	bool           defaultsChanged;
	QString        defaultsFileName;
	Properties    *defaults;

	static Mime *m_instance;
};

#endif // YE_MIMEUTILS_H

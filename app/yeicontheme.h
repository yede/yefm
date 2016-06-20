#ifndef YE_ICON_THEME_H
#define YE_ICON_THEME_H

#include <QStringList>
#include <QPixmap>
#include <QIcon>
#include <QList>
//======================================================================================================================

class IconPath
{
public:
	enum Type { Threshold, Fixed, Scalable };

	IconPath(QByteArray path, int size, Type type, QByteArray context);

	static Type decodeType(QByteArray data);

	const QString &path()    const { return m_path; }
	const QString &context() const { return m_context; }
	int            size()    const { return m_size; }
	Type           type()    const { return m_type; }

private:
	QString m_path;
	int     m_size;
	Type    m_type;
	QString m_context;
};
//======================================================================================================================

class IconTheme
{
public:
	explicit IconTheme(IconTheme *inheritor = NULL);
	virtual ~IconTheme();

	void clear();
	void setupPathList(const QString &themeName, int size);

	bool findIcon(QIcon &icon, int size, const QString &name, int mode);

	static void resizeIcon(QIcon &icon, int size);
	static bool isValid(const QString &iconTheme);

private:
	static bool readIndexData(QByteArray &data, const QString &iconTheme);
	static QString themePath(const QString &iconTheme);

	void parsePathList(const QByteArray &data, int requireSize);
	bool findIconFromPath(QIcon &icon, const QString &name, const QString &path);

	bool isTheme(const QString &name) const;
	bool exists(const QString &themeName) const;

private:
	IconTheme        *m_inheritor;
	QString           m_name;
	QString           m_path;
	QStringList       m_exts;
	QList<IconPath>   m_items;
	QList<IconTheme*> m_parentList;
};
//======================================================================================================================

#endif // YE_ICON_THEME_H

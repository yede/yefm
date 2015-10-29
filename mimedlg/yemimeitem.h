#ifndef YE_MIMEITEM_H
#define YE_MIMEITEM_H

#include <QIcon>
#include <QString>
#include <QList>
//==============================================================================================================================

namespace MimeItemType {
	enum { Root, Cate, Mime };
}

class MimeItem
{
public:
	MimeItem(int type, const QString &name, MimeItem *parent);
	~MimeItem();

	MimeItem *addChild(int type, const QString &name);
	void addChild(MimeItem *item);

	MimeItem *child(int row) const;
	int row() const;
	int rowCount() const;
	int columnCount() const;
	bool getIcon(QIcon &result);

	int       type;
	MimeItem *parent;
	QString   name;
	QString   apps;

	QList<MimeItem *> children;

private:
	void searchGenericIcon();
	void searchMimeIcon();

	bool m_iconLoaded;
	QIcon m_icon;
};

#endif // YE_MIMEITEM_H

#include <QDebug>

#include "yemimeitem.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
//==============================================================================================================================

MimeItem::MimeItem(int type, const QString &name, MimeItem *parent)
	: type(type)
	, parent(parent)
	, name(name)
	, m_iconLoaded(false)
{
}

MimeItem::~MimeItem()
{
	qDeleteAll(children);
}
//==============================================================================================================================

MimeItem *MimeItem::addChild(int type, const QString &name)
{
	MimeItem *item = new MimeItem(type, name, this);
	children.append(item);
	return item;
}

void MimeItem::addChild(MimeItem *item)
{
	if (item == NULL) return;

	item->parent = this;
	children.append(item);
}

MimeItem *MimeItem::child(int row) const
{
	return (row < 0 || row >= children.size()) ? NULL : children.at(row);
}

int MimeItem::row() const
{
	return parent == NULL ? -1 : parent->children.indexOf(const_cast<MimeItem*>(this));
}

int MimeItem::rowCount() const
{
	return children.size();
}

int MimeItem::columnCount() const
{
	return type == MimeItemType::Mime ? 2 : 1;
}
//==============================================================================================================================

void MimeItem::searchGenericIcon()
{
	if (m_iconLoaded) return;

	m_iconLoaded = true;

	if (R::getMimeIcon(m_icon, name + "-generic")) return;

	R::getMimeIcon(m_icon, name + "-x-generic");
}

void MimeItem::searchMimeIcon()
{
	if (m_iconLoaded) return;

	m_iconLoaded = true;

	if (type != MimeItemType::Mime) return;
	if (parent == NULL) return;

	QString icon = parent->name + "-" + name;
	if (R::getMimeIcon(m_icon, icon)) return;

	if (parent->type == MimeItemType::Cate)
		parent->searchGenericIcon();
}

bool MimeItem::getIcon(QIcon &result)
{
	if (!m_iconLoaded) {
		switch (type) {
			case MimeItemType::Cate: searchGenericIcon(); break;
			case MimeItemType::Mime: searchMimeIcon();    break;
		}
	}

	if (!m_icon.isNull()) {
		result = m_icon;
		return true;
	}

	if (parent == NULL) {
		return false;
	}

	if (parent->m_icon.isNull()) {
		return false;
	}

	result = parent->m_icon;
	return true;
}

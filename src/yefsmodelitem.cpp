#include <QFileInfoList>

#include "yefsmodelitem.h"
//==============================================================================================================================

FsModelItem::FsModelItem(const QFileInfo &fileInfo, FsModelItem *parent)
{
	m_parent = parent;
	m_fileInfo = fileInfo;
	m_walked = false;
	m_dirty = false;
	m_watched = false;

	if (parent) {
		parent->addChild(this);
		m_absFilePath = fileInfo.filePath();
	} else {
		m_walked = true;
		m_absFilePath = "";
	}
}

FsModelItem::~FsModelItem()
{
	qDeleteAll(m_children);
}
//==============================================================================================================================

FsModelItem *FsModelItem::childAt(int position)
{
	return m_children.value(position, 0);
}

int FsModelItem::childCount() const
{
	if (m_walked) return m_children.count();
	return 1;
}

bool FsModelItem::hasChild(QString fileName)
{
	foreach (FsModelItem *item, m_children) {
		if (item->fileName() == fileName) return true;
	}
	return false;
}

int FsModelItem::childNumber() const
{
	if (m_parent) {
		return m_parent->m_children.indexOf(const_cast<FsModelItem *>(this));
	}

	return 0;
}

QList<FsModelItem *> FsModelItem::children()
{
	return m_children;
}

FsModelItem *FsModelItem::parent()
{
	return m_parent;
}

QString FsModelItem::absoluteFilePath()const
{
	return m_absFilePath;
}

QString FsModelItem::fileName() const
{
	return m_absFilePath == QString("/") ? QString("/") : m_fileInfo.fileName();
}

QFileInfo FsModelItem::fileInfo() const
{
	return m_fileInfo;
}

void FsModelItem::refreshFileInfo()
{
	m_fileInfo.refresh();
	m_permissions.clear();
	m_mimeType.clear();
}

void FsModelItem::addChild(FsModelItem *child)
{
	if (!m_children.contains(child))
		m_children.append(child);
}

void FsModelItem::removeChild(FsModelItem *child)
{
	m_children.removeOne(child);
	delete child;
}

void FsModelItem::clearAll()
{
	foreach (FsModelItem *child, m_children)
		delete child;
	m_children.clear();
	m_walked = 0;
}

void FsModelItem::changeName(QString newName)
{
	m_absFilePath = m_parent->absoluteFilePath() + SEPARATOR + newName;
	m_fileInfo.setFile(m_absFilePath);
	clearAll();
}

FsModelItem *FsModelItem::matchPath(const QStringList &path, int startIndex)
{
	QStringList temp = path;
	temp.replace(0, "/");
	temp.removeAll("");

	if (m_walked == 0) {  //not populated yet
		m_walked = true;
		QDir dir(this->absoluteFilePath());
		QFileInfoList all = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

		foreach (QFileInfo one, all)
			new FsModelItem(one, this);
	}

	foreach (FsModelItem *child, m_children) {
		QString match = temp.at(startIndex);

		if (child->fileName() == match) {
			if (startIndex + 1 == temp.count()) return child;
			else return child->matchPath(path, startIndex + 1);
		}
	}

	return 0;
}
//==============================================================================================================================

#include <QMessageBox>
#include <QAbstractButton>
#include <QDebug>

#include "yepasteitem.h"
#include "yepastetask.h"
#include "yepastemgr.h"
#include "yefileutils.h"
//==============================================================================================================================

PasteItem::PasteItem(bool isFolder, PasteTask *task, PasteItem *parent)
	: m_task(task)
	, m_parent(parent)
	, m_totalBytes(0)
	, m_transBytes(0)
	, m_conflict(PasteConflict::None)
	, m_solution(PasteSolution::None)
	, m_isFolder(isFolder)
{
}

PasteItem::~PasteItem()
{
	qDeleteAll(m_children);
}
//==============================================================================================================================

void PasteItem::clear()
{
	int i = m_children.size();
	while (i > 0) {
		i--;
		PasteItem *item = m_children.at(i);
		item->clear();
		delete item;
	}
	m_children.clear();
}

void PasteItem::addChild(PasteItem *item)
{
	if (item == NULL) return;
//	if (m_children.indexOf(item) >= 0) return;

	m_children.append(item);
	item->m_parent = this;
}

void PasteItem::removeChild(PasteItem *item)
{
	if (item == NULL || item->parent() != this) return;

	item->m_parent = NULL;
	m_children.removeOne(item);
}

PasteItem *PasteItem::child(int row) const
{
	if (row < 0 || row >= m_children.size()) return NULL;

	return m_children.at(row);
}

int PasteItem::row() const
{
	if (!m_parent) return -1;

	return m_parent->m_children.indexOf(const_cast<PasteItem*>(this));
}

bool PasteItem::hasConflict() const
{
	if (m_conflict == PasteConflict::None) return false;

	return true;
}

QString PasteItem::getConflictText()
{
	switch (m_conflict) {
		case PasteConflict::Dir2Dir  : return QObject::tr("Another folder already exists by this name.");
		case PasteConflict::Dir2File : return QObject::tr("A file with the same name already exists.");
		case PasteConflict::File2Dir : return QObject::tr("A folder with the same name already exists.");
		case PasteConflict::File2File: return QObject::tr("Another file already exists by this name.");
	}

	return QString();
}

QString PasteItem::getSolutionText()
{
	switch (m_solution) {
		case PasteSolution::Skip   : return QObject::tr("Skip");
		case PasteSolution::Rename : return QObject::tr("Rename");
		case PasteSolution::Merge  : return QObject::tr("Merge");
		case PasteSolution::Replace: return QObject::tr("Replace");
	}

	return QString();
}

QString PasteItem::getPercentText()
{
	if (m_totalBytes < 1 || m_transBytes < 1) return QString();

	int val = m_transBytes * 100.0 / m_totalBytes;
	return QString::number(val);
}

void PasteItem::addErrorInfo(int err, const QString &text)
{
	QString msg = QString("(%1)\n%2").arg(err).arg(text);
	m_errors.append(msg);
}

void PasteItem::setSolution(int value)
{
	if (value == PasteSolution::Rename && m_destName.isEmpty()) {
		QString path = FileUtils::buildRenamePath(m_destFolder, m_srcName);
		m_destName = QFileInfo(path).fileName();
	}
	m_solution = value;
}

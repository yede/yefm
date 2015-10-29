//#include <algorithm>
//#include <stdlib.h>

#include <QDateTime>
#include <QDebug>

#include "yefssortmodel.h"
#include "yefsmodel.h"
#include "yefswidget.h"

#include "yefsmisc.h"
//==============================================================================================================================

FsSortModel::FsSortModel(FsModel *source, QObject *parent)
	: QSortFilterProxyModel(parent)
	, m_source(source)
{
	setSortCaseSensitivity(Qt::CaseInsensitive);
//	setDynamicSortFilter(true);
//	sort(0);
	setSourceModel(source);
}

FsSortModel::~FsSortModel()
{
}
//==============================================================================================================================

bool FsSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
//	qDebug() << "filterAcceptsRow" << filterRegExp().isEmpty() << filterRegExp();

	FsWidget *widget = m_source->widget();
	if (widget != NULL && !widget->isHiddenVisible()) {
		QModelIndex index = m_source->index(sourceRow, 0, sourceParent);
		if (m_source->fileInfo(index).isHidden()) return false;
	}

	return true;
}

//---------------------------------------------------------------------------------
bool FsSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
//	qDebug() << "lessThan" << m_source->filePath(left) << m_source->filePath(right);

	if (m_source->isDir(left) && !m_source->isDir(right)) {
		return sortOrder() == Qt::AscendingOrder;
	}

	if (!m_source->isDir(left) && m_source->isDir(right)) {
		return sortOrder() == Qt::DescendingOrder;
	}

	if (left.column() == 1) {			// size
		return (m_source->size(left) > m_source->size(right));
	}

	if (left.column() == 3) {			// date
		return (m_source->fileInfo(left).lastModified() > m_source->fileInfo(right).lastModified());
	}

//	bool flag = QSortFilterProxyModel::lessThan(left, right);
//	bool flag = m_source->filePath(left).compare(m_source->filePath(right), Qt::CaseInsensitive) < 0;
//	qDebug() << "lessThan=" << flag << m_source->filePath(left) << m_source->filePath(right);
	bool flag = FsMisc::compareFileName(m_source->filePath(left), m_source->filePath(right)) < 0;

	return flag;
}
//==============================================================================================================================

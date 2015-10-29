#include <algorithm>
//#include <stdlib.h>

#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>

#include "yefsmisc.h"
//==============================================================================================================================

struct SortItem
{
	mutable QString filename_cache;
	mutable QString suffix_cache;
	QFileInfo item;
};

class SortItemComparator
{
	int m_sortFlags;
	Qt::CaseSensitivity m_cs;

public:
	SortItemComparator(int flags)
		: m_sortFlags(flags)
		, m_cs(flags & QDir::IgnoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive)
	{}
	bool operator()(const SortItem &, const SortItem &) const;
};
//==============================================================================================================================

bool SortItemComparator::operator()(const SortItem &n1, const SortItem &n2) const
{
	const SortItem *f1 = &n1;
	const SortItem *f2 = &n2;

	if ((m_sortFlags & QDir::DirsFirst) && (f1->item.isDir() != f2->item.isDir())) return  f1->item.isDir();
	if ((m_sortFlags & QDir::DirsLast ) && (f1->item.isDir() != f2->item.isDir())) return !f1->item.isDir();

	qint64 r = 0;
	int sortBy = (m_sortFlags & QDir::SortByMask) | (m_sortFlags & QDir::Type);

	switch (sortBy) {
		case QDir::Time: {
			QDateTime firstModified = f1->item.lastModified();
			QDateTime secondModified = f2->item.lastModified();

			// QDateTime by default will do all sorts of conversions on these to find timezones,
			// which is incredibly expensive. As we aren't presenting these to the user,
			// we don't care (at all) about the local timezone, so force them to UTC to avoid that conversion.
			firstModified.setTimeSpec(Qt::UTC);
			secondModified.setTimeSpec(Qt::UTC);

			r = firstModified.msecsTo(secondModified);
			break;
		}
		case QDir::Size:
			r = f2->item.size() - f1->item.size();
			break;
		case QDir::Type: {
		//	bool ic = qt_cmp_si_sort_flags & QDir::IgnoreCase;
		//	if (f1->suffix_cache.isNull()) f1->suffix_cache = ic ? f1->item.suffix().toLower() : f1->item.suffix();
		//	if (f2->suffix_cache.isNull()) f2->suffix_cache = ic ? f2->item.suffix().toLower() : f2->item.suffix();

			if (f1->suffix_cache.isNull()) f1->suffix_cache = f1->item.suffix().toLower();
			if (f2->suffix_cache.isNull()) f2->suffix_cache = f2->item.suffix().toLower();

			r = m_sortFlags & QDir::LocaleAware ? f1->suffix_cache.localeAwareCompare(f2->suffix_cache)
												: FsMisc::compareFileName(f1->filename_cache, f2->filename_cache);
			break;
		}
		default:
		;
	}

	if (r == 0 && sortBy != QDir::Unsorted) {
		// Still not sorted - sort by name
	//	bool ic = qt_cmp_si_sort_flags & QDir::IgnoreCase;
	//	if (f1->filename_cache.isNull()) f1->filename_cache = ic ? f1->item.fileName().toLower() : f1->item.fileName();
	//	if (f2->filename_cache.isNull()) f2->filename_cache = ic ? f2->item.fileName().toLower() : f2->item.fileName();

		if (f1->filename_cache.isNull()) f1->filename_cache = f1->item.fileName();
		if (f2->filename_cache.isNull()) f2->filename_cache = f2->item.fileName();

		r = m_sortFlags & QDir::LocaleAware ? f1->filename_cache.localeAwareCompare(f2->filename_cache)
											: FsMisc::compareFileName(f1->filename_cache, f2->filename_cache);
	}

	if (m_sortFlags & QDir::Reversed)
		return r > 0;

	return r < 0;
}
//==============================================================================================================================

void FsMisc::sortFileList(QFileInfoList &ret, QDir::SortFlags sortFlags, const QFileInfoList &src)
{
	int n = src.size();
	if (n > 0) {
		if (n == 1 || (sortFlags & QDir::SortByMask) == QDir::Unsorted) {
			ret = src;
		} else {
			QScopedArrayPointer<SortItem> si(new SortItem[n]);
			for (int i = 0; i < n; ++i) si[i].item = src.at(i);
			std::sort(si.data(), si.data() + n, SortItemComparator(sortFlags));
			for (int i = 0; i < n; ++i) ret.append(si[i].item);
		}
	}
}
//==============================================================================================================================

#include <sys/vfs.h>
//#include <fcntl.h>

#include <QDirIterator>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

#include "yeglobal.h"
//==============================================================================================================================

QString G::formatSize(qint64 num)
{
	QString total;
	const qint64 kb = 1024;
	const qint64 mb = 1024 * kb;
	const qint64 gb = 1024 * mb;
	const qint64 tb = 1024 * gb;

	if      (num >= tb) total = QString("%1TB").arg(QString::number(qreal(num) / tb, 'f', 2));
	else if (num >= gb) total = QString("%1GB").arg(QString::number(qreal(num) / gb, 'f', 2));
	else if (num >= mb) total = QString("%1MB").arg(QString::number(qreal(num) / mb, 'f', 1));
	else if (num >= kb) total = QString("%1KB").arg(QString::number(qreal(num) / kb, 'f', 1));
	else if (num >=  1) total = QString("%1bytes").arg(num);
	else                total = QString("%1byte").arg(num);

	return total;
}

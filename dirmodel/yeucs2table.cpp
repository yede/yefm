#include <stdlib.h>

#include <QTextCodec>
#include <QDebug>

#include "yeucs2table.h"
//==============================================================================================================================

Ucs2Table::Ucs2Table()
	: m_data(NULL)
	, m_fromUcs(FROM_UCS)
	, m_lastUcs(LAST_UCS)
{
	build();
}

Ucs2Table::~Ucs2Table()
{
	clear();
}
//==============================================================================================================================

static quint32 getGB18030(const QChar &c, QTextCodec *codec)
{
	QByteArray bytes = codec->fromUnicode(&c, 1);
	quint32 code = 0;

	for (int i = 0; i < bytes.length(); i++) {
		uchar c = bytes.at(i);
		code = (code << 8) | c;
	}

	return code;
}

void Ucs2Table::clear()
{
	if (m_data) {
		delete [] m_data;
		m_data = NULL;
	}
}

void Ucs2Table::build()
{
	if (m_data) return;

	QTextCodec *codec = QTextCodec::codecForName("GB18030");
	if (codec == NULL) {
		qDebug("Ucs2Table::build(): QTextCodec::codecForName(\"GB18030\") FAILED!");
		return;
	}

	int cnt = m_lastUcs - m_fromUcs + 1;
	m_data = new ushort[cnt];
	ushort *p = m_data;

	int i;
	QChar c;
	ushort u;
	quint32 code;

	for (i = 0, u = m_fromUcs; u <= m_lastUcs; i++, u++) {
		c = QChar(u);
		code = getGB18030(c, codec);
		if (code > 0xffff) {
			qDebug("Ucs2Table::build(): [%d:0x%04x] > 0xffff: ", i, code);
		}
		*p = code;
		p++;
	}
//	qDebug("Ucs2Table::build(): %d done.", i);
}

//==============================================================================================================================
/*
void cmp(const QString &s1, const QString &s2)
{
//	qDebug() << G::compareFileName(s1, s2) << s1 << s2;
	G::compareFileName(s1, s2);
}*/

static const char *printChar(char *buf, int bufLen, const QChar c)
{

	ushort u = c.unicode();
	if (u <= 32) {
		snprintf(buf, bufLen, "0x%02x", u);
	} else {
		snprintf(buf, bufLen, "0x%04x: %s", c.unicode(), QString(c).toUtf8().constData());
	}
	return buf;
}

static void dumpTestChar(const QChar c)
{
	char buf[32];
	qDebug("isSpace: %d, isDigit: %d, "
		   //	   "isNumber: %d, "
		   "isLetter: %d, isPunct: %d, isSymbol: %d, "
		   //	   "isMark: %d, isTitleCase: %d, isNonCharacter: %d, isSurrogate: %d "
		   "category: %2d, unicode: %5d, "
	   #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		   "script: %2d, "
	   #endif
		   "row: %3d, cell: %3d "
		   "--- [%s]",
		   c.isSpace(), c.isDigit(),
		   //	   c.isNumber(),
		   c.isLetter(), c.isPunct(), c.isSymbol(),
		   //	   c.isMark(), c.isTitleCase(), c.isNonCharacter(), c.isSurrogate(),
		   c.category(), c.unicode(),
	   #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		   c.script(),
	   #endif
		   c.row(), c.cell(),
		   printChar(buf, sizeof(buf), c)
		  );
}

void testGB18030()
{
	Ucs2Table *usc = Ucs2Table::instance();
	QString s = QString::fromUtf8("䎥");
//	qDebug() << s.length();
	ushort code = usc->gbkCode(s.at(0));
	qDebug("0x%08x [%s]", code, s.toUtf8().constData());
}

void Ucs2Table::testChars()
{
	testGB18030();
	qDebug() << "";
return;
	dumpTestChar(QChar());
	dumpTestChar(QString::fromUtf8(" ").at(0));
	dumpTestChar(QString::fromUtf8("　").at(0));
	dumpTestChar(QString::fromUtf8("\t").at(0));
	dumpTestChar(QString::fromUtf8("\r").at(0));
	dumpTestChar(QString::fromUtf8("\n").at(0));
	dumpTestChar(QString::fromUtf8("1").at(0));
	dumpTestChar(QString::fromUtf8("0").at(0));
	dumpTestChar(QString::fromUtf8("一").at(0));
	dumpTestChar(QString::fromUtf8("九").at(0));
	dumpTestChar(QString::fromUtf8("三").at(0));
	dumpTestChar(QString::fromUtf8("c").at(0));
	dumpTestChar(QString::fromUtf8("B").at(0));
	dumpTestChar(QString::fromUtf8("经").at(0));
	dumpTestChar(QString::fromUtf8("我").at(0));
	dumpTestChar(QString::fromUtf8("人").at(0));
	dumpTestChar(QString::fromUtf8("，").at(0));
	dumpTestChar(QString::fromUtf8("《").at(0));
	dumpTestChar(QString::fromUtf8("]").at(0));
	dumpTestChar(QString::fromUtf8("!").at(0));
	dumpTestChar(QString::fromUtf8(".").at(0));
	dumpTestChar(QString::fromUtf8("/").at(0));
	dumpTestChar(QString::fromUtf8("~").at(0));
	dumpTestChar(QString::fromUtf8("_").at(0));
	dumpTestChar(QString::fromUtf8("+").at(0));
	dumpTestChar(QString::fromUtf8("-").at(0));
	dumpTestChar(QString::fromUtf8("*").at(0));
	dumpTestChar(QString::fromUtf8("^").at(0));
	dumpTestChar(QString::fromUtf8("\"").at(0));
	dumpTestChar(QString::fromUtf8("|").at(0));
	dumpTestChar(QString::fromUtf8("◆").at(0));
	dumpTestChar(QString::fromUtf8("·").at(0));
}

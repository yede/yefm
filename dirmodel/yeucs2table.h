#ifndef YE_GLOBAL_UCS2TABLE_H
#define YE_GLOBAL_UCS2TABLE_H

#include <QChar>
//==============================================================================================================================

class Ucs2Table
{
public:
	Ucs2Table();
	~Ucs2Table();

	inline ushort fromUcs()               const { return m_fromUcs; }
	inline ushort lastUcs()               const { return m_lastUcs; }
	inline ushort gbkCode(const QChar &c) const;

	inline bool isCjk(ushort u)       const { return u >= m_fromUcs && u <= m_lastUcs; }
	inline bool isCjk(const QChar &c) const;

	static void testChars();

	static Ucs2Table *instance() { return m_instance; }
	static void createInstance() { if (!m_instance) m_instance = new Ucs2Table; }
	static void deleteInstance() { delete m_instance; m_instance = NULL; }

private:
	void build();
	void clear();

private:
	ushort *m_data;
	ushort  m_fromUcs;
	ushort  m_lastUcs;

	static Ucs2Table *m_instance;
};
//==============================================================================================================================
#define FROM_UCS 0x4e00
#define LAST_UCS 0x9fa5
/*
static inline bool isCjk(const QChar &c)
{
	ushort uc = c.unicode();
	return (uc >= FROM_UCS && uc <= LAST_UCS);
}*/

inline bool Ucs2Table::isCjk(const QChar &c) const
{
	ushort uc = c.unicode();
	return (uc >= m_fromUcs && uc <= m_lastUcs);
}

inline ushort Ucs2Table::gbkCode(const QChar &c) const
{
	ushort u = c.unicode();
	return u < m_fromUcs || u > m_lastUcs ? 0 : m_data[u - m_fromUcs];
}
//==============================================================================================================================

#endif

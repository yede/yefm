#include <emmintrin.h>
#include <x86intrin.h>

#include <QDebug>

#include "yefsmisc.h"
#include "yeucs2table.h"
//==============================================================================================================================

/*
// Unicode case-insensitive comparison
static int ucstricmp(const ushort *a, const ushort *ae, const ushort *b, const ushort *be)
{
	if (a == b)
		return (ae - be);
	if (a == 0)
		return 1;
	if (b == 0)
		return -1;

	const ushort *e = ae;
	if (be - b < ae - a)
		e = a + (be - b);

	uint alast = 0;
	uint blast = 0;
	while (a < e) {
//         qDebug() << hex << alast << blast;
//         qDebug() << hex << "*a=" << *a << "alast=" << alast << "folded=" << foldCase (*a, alast);
//         qDebug() << hex << "*b=" << *b << "blast=" << blast << "folded=" << foldCase (*b, blast);
		int diff = foldCase(*a, alast) - foldCase(*b, blast);
		if ((diff))
			return diff;
		++a;
		++b;
	}
	if (a == ae) {
		if (b == be)
			return 0;
		return -1;
	}
	return 1;
}

// Case-insensitive comparison between a Unicode string and a QLatin1String
static int ucstricmp(const ushort *a, const ushort *ae, const uchar *b, const uchar *be)
{
	if (a == 0) {
		if (b == 0)
			return 0;
		return 1;
	}
	if (b == 0)
		return -1;

	const ushort *e = ae;
	if (be - b < ae - a)
		e = a + (be - b);

	while (a < e) {
		int diff = foldCase(*a) - foldCase(*b);
		if ((diff))
			return diff;
		++a;
		++b;
	}
	if (a == ae) {
		if (b == be)
			return 0;
		return -1;
	}
	return 1;
}*/

// Unicode case-sensitive compare two same-sized strings
static int ucstrncmp(const QChar *a, const QChar *b, int l)
{
#if defined(__mips_dsp)
	if (l >= 8) {
		return qt_ucstrncmp_mips_dsp_asm(reinterpret_cast<const ushort *>(a),
		                                 reinterpret_cast<const ushort *>(b),
		                                 l);
	}
#endif // __mips_dsp
#ifdef __SSE2__
	const char *ptr = reinterpret_cast<const char *>(a);
	qptrdiff distance = reinterpret_cast<const char *>(b) - ptr;
	a += l & ~7;
	b += l & ~7;
	l &= 7;

	// we're going to read ptr[0..15] (16 bytes)
	for (; ptr + 15 < reinterpret_cast<const char *>(a); ptr += 16) {
		__m128i a_data = _mm_loadu_si128((__m128i *)ptr);
		__m128i b_data = _mm_loadu_si128((__m128i *)(ptr + distance));
		__m128i result = _mm_cmpeq_epi16(a_data, b_data);
		uint mask = ~_mm_movemask_epi8(result);
		if (ushort(mask)) {
			// found a different byte
			uint idx = uint(_bit_scan_forward(mask));
			return reinterpret_cast<const QChar *>(ptr + idx)->unicode()
			       - reinterpret_cast<const QChar *>(ptr + distance + idx)->unicode();
		}
	}
#  if defined(Q_COMPILER_LAMBDA) && !defined(__OPTIMIZE_SIZE__) && 0
	const auto &lambda = [ = ](int i) -> int {
		return reinterpret_cast<const QChar *>(ptr)[i].unicode()
		- reinterpret_cast<const QChar *>(ptr + distance)[i].unicode();
	};
	return UnrollTailLoop<7>::exec(l, 0, lambda, lambda);
#  endif
#endif
	if (!l)
		return 0;

	union {
		const QChar *w;
		const quint32 *d;
		quintptr value;
	} sa, sb;
	sa.w = a;
	sb.w = b;

	// check alignment
	if ((sa.value & 2) == (sb.value & 2)) {
		// both addresses have the same alignment
		if (sa.value & 2) {
			// both addresses are not aligned to 4-bytes boundaries
			// compare the first character
			if (*sa.w != *sb.w)
				return sa.w->unicode() - sb.w->unicode();
			--l;
			++sa.w;
			++sb.w;

			// now both addresses are 4-bytes aligned
		}

		// both addresses are 4-bytes aligned
		// do a fast 32-bit comparison
		const quint32 *e = sa.d + (l >> 1);
		for (; sa.d != e; ++sa.d, ++sb.d) {
			if (*sa.d != *sb.d) {
				if (*sa.w != *sb.w)
					return sa.w->unicode() - sb.w->unicode();
				return sa.w[1].unicode() - sb.w[1].unicode();
			}
		}

		// do we have a tail?
		return (l & 1) ? sa.w->unicode() - sb.w->unicode() : 0;
	} else {
		// one of the addresses isn't 4-byte aligned but the other is
		const QChar *e = sa.w + l;
		for (; sa.w != e; ++sa.w, ++sb.w) {
			if (*sa.w != *sb.w)
				return sa.w->unicode() - sb.w->unicode();
		}
	}
	return 0;
}
/*
static int ucstrncmp(const QChar *a, const uchar *c, int l)
{
	const ushort *uc = reinterpret_cast<const ushort *>(a);
	const ushort *e = uc + l;

#ifdef __SSE2__
	__m128i nullmask = _mm_setzero_si128();
	qptrdiff offset = 0;

	// we're going to read uc[offset..offset+15] (32 bytes)
	// and c[offset..offset+15] (16 bytes)
	for ( ; uc + offset + 15 < e; offset += 16) {
		// similar to fromLatin1_helper:
		// load 16 bytes of Latin 1 data
		__m128i chunk = _mm_loadu_si128((__m128i*)(c + offset));

#  ifdef __AVX2__
		// expand Latin 1 data via zero extension
		__m256i ldata = _mm256_cvtepu8_epi16(chunk);

		// load UTF-16 data and compare
		__m256i ucdata = _mm256_loadu_si256((__m256i*)(uc + offset));
		__m256i result = _mm256_cmpeq_epi16(ldata, ucdata);

		uint mask = ~_mm256_movemask_epi8(result);
#  else
		// expand via unpacking
		__m128i firstHalf = _mm_unpacklo_epi8(chunk, nullmask);
		__m128i secondHalf = _mm_unpackhi_epi8(chunk, nullmask);

		// load UTF-16 data and compare
		__m128i ucdata1 = _mm_loadu_si128((__m128i*)(uc + offset));
		__m128i ucdata2 = _mm_loadu_si128((__m128i*)(uc + offset + 8));
		__m128i result1 = _mm_cmpeq_epi16(firstHalf, ucdata1);
		__m128i result2 = _mm_cmpeq_epi16(secondHalf, ucdata2);

		uint mask = ~(_mm_movemask_epi8(result1) | _mm_movemask_epi8(result2) << 16);
#  endif
		if (mask) {
			// found a different character
			uint idx = uint(_bit_scan_forward(mask));
			return uc[offset + idx / 2] - c[offset + idx / 2];
		}
	}

#  ifdef Q_PROCESSOR_X86_64
	enum { MaxTailLength = 7 };
	// we'll read uc[offset..offset+7] (16 bytes) and c[offset..offset+7] (8 bytes)
	if (uc + offset + 7 < e) {
		// same, but we're using an 8-byte load
		__m128i chunk = _mm_cvtsi64_si128(*(long long *)(c + offset));
		__m128i secondHalf = _mm_unpacklo_epi8(chunk, nullmask);

		__m128i ucdata = _mm_loadu_si128((__m128i*)(uc + offset));
		__m128i result = _mm_cmpeq_epi16(secondHalf, ucdata);
		uint mask = ~_mm_movemask_epi8(result);
		if (ushort(mask)) {
			// found a different character
			uint idx = uint(_bit_scan_forward(mask));
			return uc[offset + idx / 2] - c[offset + idx / 2];
		}

		// still matched
		offset += 8;
	}
#  else
	// 32-bit, we can't do MOVQ to load 8 bytes
	enum { MaxTailLength = 15 };
#  endif

	// reset uc and c
	uc += offset;
	c += offset;

#  if defined(Q_COMPILER_LAMBDA) && !defined(__OPTIMIZE_SIZE__)
	const auto &lambda = [=](int i) { return uc[i] - ushort(c[i]); };
	return UnrollTailLoop<MaxTailLength>::exec(e - uc, 0, lambda, lambda);
#  endif
#endif

	while (uc < e) {
		int diff = *uc - *c;
		if (diff)
			return diff;
		uc++, c++;
	}

	return 0;
}

// Unicode case-insensitive compare two same-sized strings
static int ucstrnicmp(const ushort *a, const ushort *b, int l)
{
	return ucstricmp(a, a + l, b, b + l);
}

static int ucstrcmp(const QChar *a, int alen, const uchar *b, int blen)
{
	int l = qMin(alen, blen);
	int cmp = ucstrncmp(a, b, l);
	return cmp ? cmp : (alen-blen);
}
*/

// Unicode case-sensitive comparison
static int ucstrcmp(const QChar *a, int alen, const QChar *b, int blen)
{
	if (a == b && alen == blen)
		return 0;
	int l = qMin(alen, blen);
	int cmp = ucstrncmp(a, b, l);
	return cmp ? cmp : (alen - blen);
}

int FsMisc::compare(const QString &s1, const QString &s2, int len)
{
	return ucstrcmp(s1.constData(), len, s2.constData(), len);
}

//int G::compare(const QString &s1, const QString &s2)
//{
//	if (cs == Qt::CaseSensitive)
//		return ucstrcmp(s1.constData(), s1.length(), s2.constData(), s2.length());
//	return ucstricmp(s1.d->data(), s1.d->data() + s1.d->size, s2.d->data(), s2.d->data() + s2.d->size);
//}
//==============================================================================================================================

#define get_char(c, s, pos, len)   c = (pos < len) ? s.at(pos) : QChar();   pos++

int FsMisc::compareFileName(const QString &s1, const QString &s2)
{
	Ucs2Table *ucs = Ucs2Table::instance();

	static QChar zero = QChar('0');
	int pos1 = 0, len1 = s1.count(), result;
	int pos2 = 0, len2 = s2.count();
	bool f1, f2;								// f: flag
	QChar c1, c2;

	while (pos1 <= len1 && pos2 <= len2) {
		get_char(c1, s1, pos1, len1);
		get_char(c2, s2, pos2, len2);
		//----------------------------------------------------------------------------------------------------------------------

		f1 = c1.isNull();
		f2 = c2.isNull();
		if (f1 && !f2) return -1;
		if (f2 && !f1) return 1;
		if (f1 &&  f2) goto end;
		//----------------------------------------------------------------------------------------------------------------------

		f1 = c1.isSpace();
		f2 = c2.isSpace();
		if (f1 && !f2) return -1;
		if (f2 && !f1) return 1;
		if (f1 &&  f2) {
			if (c1 != c2) return c1.unicode() > c2.unicode() ? 1 : -1;
			continue;
		}
		//----------------------------------------------------------------------------------------------------------------------

		f1 = c1.isSymbol();
		f2 = c2.isSymbol();
		if (f1 && !f2) return -1;
		if (f2 && !f1) return 1;
		if (f1 &&  f2) {
			if (c1 != c2) return c1.unicode() > c2.unicode() ? 1 : -1;
			continue;
		}
		//----------------------------------------------------------------------------------------------------------------------

		f1 = c1.isPunct();
		f2 = c2.isPunct();
		if (f1 && !f2) return -1;
		if (f2 && !f1) return 1;
		if (f1 &&  f2) {
			if (c1 != c2) return c1.unicode() > c2.unicode() ? 1 : -1;
			continue;
		}
		//----------------------------------------------------------------------------------------------------------------------

		f1 = c1.isDigit();
		f2 = c2.isDigit();
		if (f1 && !f2) return -1;
		if (f2 && !f1) return 1;
		if (f1 &&  f2) {
			result = 0;
			while (c1 == zero) { get_char(c1, s1, pos1, len1); }
			while (c2 == zero) { get_char(c2, s2, pos2, len2); }

			for (;;) {				// make Natural number sort, like: 01.jpg, 2.jpg, 10.jpg, 100.jpg
				f1 = c1.isDigit();
				f2 = c2.isDigit();

				if (!f1 && !f2) break;
				if (!f1) return -1;
				if (!f2) return 1;

				if (result == 0) {
					if (c1 != c2) result = c1.unicode() > c2.unicode() ? 1 : -1;
				}

				get_char(c1, s1, pos1, len1);
				get_char(c2, s2, pos2, len2);
			}

			if (result != 0) return result;

			f1 = c1.isNull();
			f2 = c2.isNull();
			if (f1 && !f2) return -1;
			if (f2 && !f1) return 1;
			if (f1 &&  f2) goto end;
			continue;
		}
		//----------------------------------------------------------------------------------------------------------------------

		f1 = c1.isLetter();
		f2 = c2.isLetter();
		if (f1 && !f2) return -1;
		if (f2 && !f1) return 1;
		if (f1 &&  f2) {
			if (c1 != c2) {
				ushort u1 = c1.unicode();
				ushort u2 = c2.unicode();
				if (ucs->isCjk(u1) && ucs->isCjk(u2)) {
					ushort v1 = ucs->gbkCode(c1);
					ushort v2 = ucs->gbkCode(c2);
					if (v1 > 0 && v2 > 0) return v1 > v2 ? 1 : -1;	// sort by spell
				}
				ushort l1 = c1.toLower().unicode();
				ushort l2 = c2.toLower().unicode();
				if (l1 != l2) {
					return l1 > l2 ? 1 : -1;
				}
				return u1 > u2 ? 1 : -1;
			}
			continue;
		}
		//----------------------------------------------------------------------------------------------------------------------

		if (c1 != c2) return c1.unicode() > c2.unicode() ? 1 : -1;
	}

end:
	// The two strings are the same (02 == 2) so fall back to the normal sort
	return //QString::compare(s1, s2, cs);
	    ucstrcmp(s1.constData(), s1.length(), s2.constData(), s2.length());
}
//==============================================================================================================================

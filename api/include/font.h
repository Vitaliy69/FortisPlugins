
#ifndef __FONT_H__
#define __FONT_H__

typedef enum
{
	EXTFONT_8859_1,	// ISO/IEC 8859-1 [25] 		West European
	EXTFONT_8859_2,	// ISO/IEC 8859-2 [26] 		East European
	EXTFONT_8859_3,	// ISO/IEC 8859-3 [27] 		South European
	EXTFONT_8859_4,	// ISO/IEC 8859-4 [28] 		North and North-East European

	EXTFONT_8859_5,	// ISO/IEC 8859-5 [29] 		Latin/Cyrillic alphabet A.2
	EXTFONT_8859_6,	// ISO/IEC 8859-6 [30] 		Latin/Arabic alphabet A.3

	EXTFONT_8859_7,	// ISO/IEC 8859-7 [31] 		Latin/Greek alphabet A.4
	EXTFONT_8859_8,	// ISO/IEC 8859-8 [32] 		Latin/Hebrew alphabet A.5

	EXTFONT_8859_9,	// ISO/IEC 8859-9 [33] 		Latin alphabet No. 5 A.6, DEFAULT

	EXTFONT_8859_10,	// ISO/IEC 8859-10 [34] 		Latin alphabet No. 6 (North European) A.7
	EXTFONT_8859_11,	// ISO/IEC 8859-11 [35] 		Latin/Thai (draft only) A.8
	EXTFONT_8859_12,	// ISO/IEC 8859-12 (see bibliography) possibly reserved for Indian

	EXTFONT_8859_13,	// ISO/IEC 8859-13 [36] 		Latin alphabet No. 7 (Baltic) A.9

	EXTFONT_8859_14,	// ISO/IEC 8859-14 [37] 		Latin alphabet No. 8 (Celtic) A.10
	EXTFONT_8859_15,	// ISO/IEC 8859-15 [38] 		Latin alphabet No. 9 (West European) A.11
	EXTFONT_6937,

	EXTFONT_8859_End,
	EXTFONT_UNICODE = EXTFONT_8859_End,
	EXTFONT_EUCKR,
	EXTFONT_WIN1251,	// cyrillic russian
	EXTFONT_WIN1256,	// arabic
	EXTFONT_UTF8,		// UTF-8 encoding of ISO/IEC 10646-1 [18]

	N_ExtFont = 0xf0
} T_ExtFont;

T_ExtFont GetExtFont(const char *str);
const char *GetExtFontTypeStr(T_ExtFont extFont);

#endif // _FONT_H__


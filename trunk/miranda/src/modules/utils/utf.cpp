/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

 Copyright 2000 Alexandre Julliard of Wine project 
 (UTF-8 conversion routines)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

/* number of following bytes in sequence based on first byte value (for bytes above 0x7f) */
static const char utf8_length[128] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xa0-0xaf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xb0-0xbf */
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xc0-0xcf */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0xd0-0xdf */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xe0-0xef */
    3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0  /* 0xf0-0xff */
};

/* first byte mask depending on UTF-8 sequence length */
static const unsigned char utf8_mask[4] = { 0x7f, 0x1f, 0x0f, 0x07 };

/* minimum Unicode value depending on UTF-8 sequence length */
static const unsigned int utf8_minval[4] = { 0x0, 0x80, 0x800, 0x10000 };


/* get the next char value taking surrogates into account */
static unsigned int getSurrogateValue(const wchar_t *src, unsigned int srclen)
{
    if (src[0] >= 0xd800 && src[0] <= 0xdfff)  /* surrogate pair */
    {
        if (src[0] > 0xdbff || /* invalid high surrogate */
            srclen <= 1 ||     /* missing low surrogate */
            src[1] < 0xdc00 || src[1] > 0xdfff) /* invalid low surrogate */
            return 0;
        return 0x10000 + ((src[0] & 0x3ff) << 10) + (src[1] & 0x3ff);
    }
    return src[0];
}

/* query necessary dst length for src string */
static int Ucs2toUtf8Len(const wchar_t *src, unsigned int srclen)
{
    int len;
    unsigned int val;

    for (len = 0; srclen; srclen--, src++)
    {
        if (*src < 0x80)  /* 0x00-0x7f: 1 byte */
        {
            len++;
            continue;
        }
        if (*src < 0x800)  /* 0x80-0x7ff: 2 bytes */
        {
            len += 2;
            continue;
        }
        if (!(val = getSurrogateValue(src, srclen)))
        {
            return -2;
        }
        if (val < 0x10000)  /* 0x800-0xffff: 3 bytes */
            len += 3;
        else   /* 0x10000-0x10ffff: 4 bytes */
        {
            len += 4;
            src++;
            srclen--;
        }
    }
    return len;
}

/* wide char to UTF-8 string conversion */
/* return -1 on dst buffer overflow, -2 on invalid input char */
int Ucs2toUtf8(const wchar_t *src, int srclen, char *dst, int dstlen)
{
    int len;

    for (len = dstlen; srclen; srclen--, src++)
    {
        WCHAR ch = *src;
        unsigned int val;

        if (ch < 0x80)  /* 0x00-0x7f: 1 byte */
        {
            if (!len--) return -1;  /* overflow */
            *dst++ = ch;
            continue;
        }

        if (ch < 0x800)  /* 0x80-0x7ff: 2 bytes */
        {
            if ((len -= 2) < 0) return -1;  /* overflow */
            dst[1] = 0x80 | (ch & 0x3f);
            ch >>= 6;
            dst[0] = 0xc0 | ch;
            dst += 2;
            continue;
        }

        if (!(val = getSurrogateValue(src, srclen)))
        {
            return -2;
        }

        if (val < 0x10000)  /* 0x800-0xffff: 3 bytes */
        {
            if ((len -= 3) < 0) return -1;  /* overflow */
            dst[2] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[1] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[0] = 0xe0 | val;
            dst += 3;
        }
        else   /* 0x10000-0x10ffff: 4 bytes */
        {
            if ((len -= 4) < 0) return -1;  /* overflow */
            dst[3] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[2] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[1] = 0x80 | (val & 0x3f);
            val >>= 6;
            dst[0] = 0xf0 | val;
            dst += 4;
            src++;
            srclen--;
        }
    }
    return dstlen - len;
}

/* helper for the various utf8 mbstowcs functions */
static unsigned int decodeUtf8Char(unsigned char ch, const char **str, const char *strend)
{
    unsigned int len = utf8_length[ch-0x80];
    unsigned int res = ch & utf8_mask[len];
    const char *end = *str + len;

    if (end > strend) return ~0;
    switch(len)
    {
    case 3:
        if ((ch = end[-3] ^ 0x80) >= 0x40) break;
        res = (res << 6) | ch;
        (*str)++;
    case 2:
        if ((ch = end[-2] ^ 0x80) >= 0x40) break;
        res = (res << 6) | ch;
        (*str)++;
    case 1:
        if ((ch = end[-1] ^ 0x80) >= 0x40) break;
        res = (res << 6) | ch;
        (*str)++;
        if (res < utf8_minval[len]) break;
        return res;
    }
    return ~0;
}

/* query necessary dst length for src string */
static inline int Utf8toUcs2Len(const char *src, int srclen)
{
    int ret = 0;
    unsigned int res;
    const char *srcend = src + srclen;

    while (src < srcend)
    {
        unsigned char ch = *src++;
        if (ch < 0x80)  /* special fast case for 7-bit ASCII */
        {
            ret++;
            continue;
        }
        if ((res = decodeUtf8Char(ch, &src, srcend)) <= 0x10ffff)
        {
            if (res > 0xffff) ret++;
            ret++;
        }
        else return -2;  /* bad char */
        /* otherwise ignore it */
    }
    return ret;
}

/* UTF-8 to wide char string conversion */
/* return -1 on dst buffer overflow, -2 on invalid input char */
int Utf8toUcs2(const char *src, int srclen, wchar_t *dst, int dstlen)
{
    unsigned int res;
    const char *srcend = src + srclen;
    wchar_t *dstend = dst + dstlen;

    while ((dst < dstend) && (src < srcend))
    {
        unsigned char ch = *src++;
        if (ch < 0x80)  /* special fast case for 7-bit ASCII */
        {
            *dst++ = ch;
            continue;
        }
        if ((res = decodeUtf8Char(ch, &src, srcend)) <= 0xffff)
        {
            *dst++ = res;
        }
        else if (res <= 0x10ffff)  /* we need surrogates */
        {
            if (dst == dstend - 1) return -1;  /* overflow */
            res -= 0x10000;
            *dst++ = 0xd800 | (res >> 10);
            *dst++ = 0xdc00 | (res & 0x3ff);
        }
        else return -2;  /* bad char */
        /* otherwise ignore it */
    }
    if (src < srcend) return -1;  /* overflow */
    return dstlen - (dstend - dst);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Utf8Decode - converts UTF8-encoded string to the UCS2/MBCS format

char* Utf8DecodeCP(char* str, int codepage, wchar_t** ucs2)
{
	int len; 
    bool needs_free = false;
	wchar_t* tempBuf = NULL;

	if (str == NULL) 
		return NULL;

	len = (int)strlen(str);

	if (len < 2) 
	{
		if (ucs2 != NULL) 
		{
			*ucs2 = tempBuf = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t));
			MultiByteToWideChar(codepage, 0, str, len, tempBuf, len);
			tempBuf[len] = 0;
		}
		return str;
	}

	int destlen = Utf8toUcs2Len(str, len);
	if (destlen < 0) return NULL;

	if (ucs2 == NULL) 
	{
		__try
		{
			tempBuf = (wchar_t*)alloca((destlen + 1) * sizeof(wchar_t));
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			tempBuf = NULL;
			needs_free = true;
		}
	}

	if (tempBuf == NULL)
	{
		tempBuf = (wchar_t*)mir_alloc((destlen + 1) * sizeof(wchar_t));
		if (tempBuf == NULL) return NULL;
	}
	
	Utf8toUcs2(str, len, tempBuf, destlen); 
	tempBuf[destlen] = 0;
	WideCharToMultiByte(codepage, 0, tempBuf, -1, str, len, "?", NULL);

	if (ucs2)
		*ucs2 = tempBuf;
	else if (needs_free)
		mir_free(tempBuf);

	return str;
}

char* Utf8Decode(char* str, wchar_t** ucs2)
{
	return Utf8DecodeCP(str, LangPackGetDefaultCodePage(), ucs2);
}

wchar_t* Utf8DecodeUcs2(const char* str)
{
	size_t len;
	wchar_t* ucs2;

	if (str == NULL)
		return NULL;

	len = strlen(str);

	int destlen = Utf8toUcs2Len(str, len);
	if (destlen < 0) return NULL;

	ucs2 = (wchar_t*)mir_alloc((destlen + 1) * sizeof(wchar_t));
	if (ucs2 == NULL) return NULL;

	if (Utf8toUcs2(str, len, ucs2, destlen) >= 0)
	{
		ucs2[destlen] = 0;
		return ucs2;
	}

	mir_free(ucs2);

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Utf8Encode - converts MBCS string to the UTF8-encoded format

char* Utf8EncodeCP(const char* src, int codepage)
{
	int len; 
    bool needs_free = false;
	char* result = NULL;
	wchar_t* tempBuf;

	if (src == NULL)
		return NULL;

	len = (int)strlen(src);

	__try
	{
		tempBuf = (wchar_t*)alloca((len + 1) * sizeof(wchar_t));
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		tempBuf = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t));
		if (tempBuf == NULL) return NULL;
		needs_free = true;
	}

	len = MultiByteToWideChar(codepage, 0, src, -1, tempBuf, len);
	tempBuf[len] = 0;

	int destlen = Ucs2toUtf8Len(tempBuf, len);
	if (destlen >= 0)
	{
		result = (char*)mir_alloc(destlen + 1);
		if (result)
		{
			Ucs2toUtf8(tempBuf, len, result, destlen);
			result[destlen] = 0;
		}
	}

	if (needs_free)
		mir_free(tempBuf);

	return result;
}

char* Utf8Encode(const char* src)
{
	return Utf8EncodeCP(src, LangPackGetDefaultCodePage());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Utf8Encode - converts UCS2 string to the UTF8-encoded format

char* Utf8EncodeUcs2(const wchar_t* src)
{
	if (src == NULL)
		return NULL;

	size_t len = wcslen(src);

	int destlen = Ucs2toUtf8Len(src, len);
	if (destlen < 0) return NULL;

	char* result = (char*)mir_alloc(destlen + 1);
	if (result == NULL)
		return NULL;

	Ucs2toUtf8(src, len, result, destlen);
	result[destlen] = 0;

	return result;
}

/*
dbRW

Copyright (c) 2005-2009 Robert Rainwater

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
#include "dbrw.h"

void utf8_decode(char* str, wchar_t** ucs2) {
	int len;
	wchar_t* tempBuf;

	if (str==NULL)
		return;
	len = strlen(str);
	if (len<2) {
		if (ucs2!=NULL) {
			*ucs2 = (wchar_t*)dbrw_alloc((len+1)*sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, 0, str, len, *ucs2, len);
			(*ucs2)[len] = 0;
		}
		return;
	}
	tempBuf = (wchar_t*)alloca((len+1)*sizeof(wchar_t));
	{
		wchar_t* d = tempBuf;
		BYTE* s = ( BYTE* )str;

		while(*s) {
			if ((*s&0x80)==0) {
				*d++ = *s++;
				continue;
			}
			if ((s[0]&0xE0)==0xE0&&(s[1]&0xC0)==0x80&&(s[2]&0xC0 )==0x80) {
				*d++ = ((WORD)(s[0]&0x0F)<<12)+(WORD)((s[1]&0x3F)<<6)+(WORD)(s[2]&0x3F);
				s += 3;
				continue;
			}
			if ((s[0]&0xE0)==0xC0&&(s[1]&0xC0)==0x80) {
				*d++ = (WORD)((s[0]&0x1F )<<6)+(WORD)(s[1]&0x3F);
				s += 2;
				continue;
			}
			*d++ = *s++;
		}
		*d = 0;
	}
	if (ucs2!=NULL) {
		size_t fullLen = (len+1)*sizeof(wchar_t);

		*ucs2 = (wchar_t*)dbrw_alloc(fullLen);
		memcpy(*ucs2, tempBuf, fullLen);
	}
	WideCharToMultiByte(CP_ACP, 0, tempBuf, -1, str, len, NULL, NULL);
}

char* utf8_encode(const char* src) {
	size_t len;
	char* result;
	wchar_t* tempBuf;

	if (src==NULL)
		return NULL;
	len = strlen(src);
	result = (char*)dbrw_alloc(len*3+1);
	if (result==NULL)
		return NULL;
	tempBuf = (wchar_t*)alloca((len+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, src, -1, tempBuf, (int)len);
	tempBuf[len] = 0;
	{
		wchar_t* s = tempBuf;
		BYTE* d = (BYTE*)result;

		while(*s) {
			int U = *s++;

			if (U<0x80) {
				*d++ = (BYTE)U;
			}
			else if (U<0x800) {
				*d++ = 0xC0+((U>>6)&0x3F);
				*d++ = 0x80+(U&0x003F);
			}
			else {
				*d++ = 0xE0+(U>>12);
				*d++ = 0x80+((U>>6)&0x3F);
				*d++ = 0x80+(U&0x3F);
			}	
		}
		*d = 0;
	}
	return result;
}

char* utf8_encodeUsc2(const wchar_t* src) {
	size_t len = wcslen(src);
	char* result = (char*)dbrw_alloc(len*3+1);

	if (result==NULL)
		return NULL;
	{	
		const wchar_t* s = src;
		BYTE* d = (BYTE*)result;

		while(*s) {
			int U = *s++;

			if (U<0x80) {
				*d++ = (BYTE)U;
			}
			else if (U<0x800) {
				*d++ = 0xC0+((U>>6)&0x3F);
				*d++ = 0x80+(U&0x003F);
			}
			else {
				*d++ = 0xE0+(U>>12);
				*d++ = 0x80+((U>>6)&0x3F);
				*d++ = 0x80+(U&0x3F);
			}
		}
		*d = 0;
	}
	return result;
}
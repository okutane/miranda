/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

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

/**** memory.c *************************************************************************/

void*  mir_alloc( size_t );
void*  mir_calloc( size_t );
void*  mir_realloc( void* ptr, size_t );
void   mir_free( void* ptr );
char*  mir_strdup( const char* str );
WCHAR* mir_wstrdup( const WCHAR* str );

#if defined( _UNICODE )
	#define mir_tstrdup mir_wstrdup
#else
	#define mir_tstrdup mir_strdup
#endif

/**** utf.c ****************************************************************************/

void Utf8Decode( char* str, wchar_t** ucs2 );

/**** langpack.c ***********************************************************************/

int    LangPackGetDefaultCodePage();
TCHAR* LangPackPcharToTchar( const char* pszStr );
char*  LangPackTranslateString(const char *szEnglish, const int W);

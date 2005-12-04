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
#include "commonheaders.h"
#include "clc.h"
#include "../database/dblists.h"

SortedList* clistCache = NULL;

static int compareContacts( ClcCacheEntryBase* p1, ClcCacheEntryBase* p2 )
{
	return ( char* )p1->hContact - ( char* )p2->hContact;
}

void InitDisplayNameCache(void)
{
	clistCache = List_Create( 0, 50 );
	clistCache->sortFunc = compareContacts;
}

void FreeDisplayNameCache(void)
{
	if ( clistCache != NULL ) {
		int i;
		for ( i = 0; i < clistCache->realCount; i++) {
			cli.pfnFreeCacheItem(( ClcCacheEntryBase* )clistCache->items[i] );
			free( clistCache->items[i] );
		}

		List_Destroy( clistCache ); 
		free(clistCache);
		clistCache = NULL;
}	}

// default handlers for the cache item creation and destruction

ClcCacheEntryBase* fnCreateCacheItem( HANDLE hContact )
{
	ClcCacheEntryBase* p = ( ClcCacheEntryBase* )calloc( sizeof( ClcCacheEntryBase ), 1 );
	if ( p == NULL )
		return NULL;

	p->hContact = hContact;
	return p;
}

void fnCheckCacheItem( ClcCacheEntryBase* p )
{
	DBVARIANT dbv;
	if ( p->group == NULL ) {
		if ( !DBGetContactSettingTString( p->hContact, "CList", "Group", &dbv )) {
			p->group = _tcsdup( dbv.ptszVal );
			free( dbv.ptszVal );
		}
		else p->group = _tcsdup( _T("") );
	}

	if ( p->isHidden == -1 )
		p->isHidden = DBGetContactSettingByte( p->hContact, "CList", "Hidden", 0 );
}

void fnFreeCacheItem( ClcCacheEntryBase* p )
{
	if ( p->name ) { free( p->name ); p->name = NULL; }
	#if defined( _UNICODE )
		if ( p->szName ) { free( p->szName); p->szName = NULL; }
	#endif
	if ( p->group ) { free( p->group ); p->group = NULL; }
	p->isHidden = -1;
}

ClcCacheEntryBase* fnGetCacheEntry(HANDLE hContact)
{
	ClcCacheEntryBase* p;
	int idx;
	if ( !List_GetIndex( clistCache, &hContact, &idx ))
	{	p = cli.pfnCreateCacheItem( hContact );
		List_Insert( clistCache, p, idx );
	}
	else p = ( ClcCacheEntryBase* )clistCache->items[idx];

	cli.pfnCheckCacheItem( p );
	return p;
}

void fnInvalidateDisplayNameCacheEntry(HANDLE hContact)
{
	if (hContact == INVALID_HANDLE_VALUE) {
		FreeDisplayNameCache();
		InitDisplayNameCache();
		SendMessage(cli.hwndContactTree, CLM_AUTOREBUILD, 0, 0);
	}
	else {
		int idx;
		if ( List_GetIndex( clistCache, &hContact, &idx ))
			cli.pfnFreeCacheItem(( ClcCacheEntryBase* )clistCache->items[idx] );
}	}

TCHAR* fnGetContactDisplayName( HANDLE hContact, int mode )
{
	CONTACTINFO ci;
	TCHAR *buffer;
	ClcCacheEntryBase* cacheEntry = NULL;

	if ( mode & GCDNF_NOCACHE )
		mode &= ~GCDNF_NOCACHE;
	else if ( mode != GCDNF_NOMYHANDLE) {
		cacheEntry = cli.pfnGetCacheEntry( hContact );
		if ( cacheEntry->name )
			return cacheEntry->name;
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	if (ci.hContact == NULL)
		ci.szProto = "ICQ";
	ci.dwFlag = (mode == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY;
	#if defined( _UNICODE )
		ci.dwFlag += CNF_UNICODE;
	#endif
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (cacheEntry == NULL) {
				size_t len = _tcslen(ci.pszVal);
				buffer = (TCHAR*) malloc( sizeof( TCHAR )*( len+1 ));
				memcpy( buffer, ci.pszVal, len * sizeof( TCHAR ));
				buffer[ len ] = 0;
				free(ci.pszVal);
				return buffer;
			}
			else {
				cacheEntry->name = ci.pszVal;
				#if defined( _UNICODE )
					cacheEntry->szName = u2a( ci.pszVal );
				#endif
				return ci.pszVal;
		}	}

		if (ci.type == CNFT_DWORD) {
			if (cacheEntry == NULL) {
				buffer = (TCHAR*) malloc(15 * sizeof( TCHAR ));
				_ltot(ci.dVal, buffer, 10 );
				return buffer;
			}
			else {
				buffer = (TCHAR*) malloc(15 * sizeof( TCHAR ));
				_ltot(ci.dVal, buffer, 10 );
				cacheEntry->name = buffer;
				#if defined( _UNICODE )
					cacheEntry->szName = u2a( buffer );
				#else
				#endif
				return buffer;
	}	}	}

	CallContactService(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);
	buffer = TranslateT("(Unknown Contact)");
	return buffer;
}

int GetContactDisplayName(WPARAM wParam, LPARAM lParam)
{
	CONTACTINFO ci;
	ClcCacheEntryBase* cacheEntry = NULL;
	char *buffer;

	if ( lParam & GCDNF_UNICODE )
		return ( int )cli.pfnGetContactDisplayName(( HANDLE )wParam, lParam & ~GCDNF_UNICODE );

	if ((int) lParam != GCDNF_NOMYHANDLE) {
		cacheEntry = cli.pfnGetCacheEntry((HANDLE) wParam);
		#if defined( _UNICODE )
			if ( cacheEntry->szName )
				return (int)cacheEntry->szName;
		#else
			if ( cacheEntry->name )
				return (int)cacheEntry->name;
		#endif
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = (HANDLE) wParam;
	if (ci.hContact == NULL)
		ci.szProto = "ICQ";
	ci.dwFlag = (int) lParam == GCDNF_NOMYHANDLE ? CNF_DISPLAYNC : CNF_DISPLAY;
	#if defined( _UNICODE )
		ci.dwFlag += CNF_UNICODE;
	#endif
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (cacheEntry == NULL) {
				size_t len = _tcslen(ci.pszVal);
				#if defined( _UNICODE )
					buffer = u2a( ci.pszVal );
					free(ci.pszVal);
				#else
					buffer = ci.pszVal;
				#endif
				return (int) buffer;
			}
			else {
				cacheEntry->name = ci.pszVal;
				#if defined( _UNICODE )
					cacheEntry->szName = u2a( ci.pszVal );
					return (int)cacheEntry->szName;
				#else
					return (int)cacheEntry->name;
				#endif
			}
		}
		if (ci.type == CNFT_DWORD) {
			if (cacheEntry == NULL) {
				buffer = ( char* )malloc(15);
				ltoa(ci.dVal, buffer, 10 );
				return (int) buffer;
			}
			else {
				buffer = ( char* )malloc(15);
				ltoa(ci.dVal, buffer, 10 );
				#if defined( _UNICODE )
					cacheEntry->szName = buffer;
					cacheEntry->name = a2u( buffer );
				#else
					cacheEntry->name = buffer;
				#endif
				return (int) buffer;
	}	}	}

	CallContactService((HANDLE) wParam, PSS_GETINFO, SGIF_MINIMAL, 0);
	buffer = Translate("(Unknown Contact)");
	return (int) buffer;
}

int InvalidateDisplayName(WPARAM wParam, LPARAM lParam)
{
	cli.pfnInvalidateDisplayNameCacheEntry((HANDLE) wParam);
	return 0;
}

int ContactAdded(WPARAM wParam, LPARAM lParam)
{
	cli.pfnChangeContactIcon((HANDLE) wParam, cli.pfnIconFromStatusMode((char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0), ID_STATUS_OFFLINE), 1);
	cli.pfnSortContacts();
	return 0;
}

int ContactDeleted(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
	return 0;
}

int ContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	DBVARIANT dbv;

	// Early exit
	if ((HANDLE) wParam == NULL)
		return 0;

	dbv.pszVal = NULL;
	if (!DBGetContactSetting((HANDLE) wParam, "Protocol", "p", &dbv)) {
		if (!strcmp(cws->szModule, dbv.pszVal)) {
			cli.pfnInvalidateDisplayNameCacheEntry((HANDLE) wParam);
			if (!strcmp(cws->szSetting, "UIN") || !strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName")
				|| !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "e-mail")) {
					CallService(MS_CLUI_CONTACTRENAMED, wParam, 0);
				}
			else if (!strcmp(cws->szSetting, "Status")) {
				if (!DBGetContactSettingByte((HANDLE) wParam, "CList", "Hidden", 0)) {
					if (DBGetContactSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT)) {
						// User's state is changing, and we are hideOffline-ing
						if (cws->value.wVal == ID_STATUS_OFFLINE) {
							cli.pfnChangeContactIcon((HANDLE) wParam, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal), 0);
							CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
							free(dbv.pszVal);
							return 0;
						}
						cli.pfnChangeContactIcon((HANDLE) wParam, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal), 1);
					}
					cli.pfnChangeContactIcon((HANDLE) wParam, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal), 0);
				}
			}
			else {
				free(dbv.pszVal);
				return 0;
			}
			cli.pfnSortContacts();
		}
	}

	if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "Hidden")) {
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0) {
				char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
				cli.pfnChangeContactIcon((HANDLE) wParam,
					cli.pfnIconFromStatusMode(szProto,
					szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord((HANDLE) wParam, szProto, "Status",
					ID_STATUS_OFFLINE)), 1);
			}
			else
				CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
		}
		if (!strcmp(cws->szSetting, "MyHandle")) {
			cli.pfnInvalidateDisplayNameCacheEntry((HANDLE) wParam);
		}
	}

	if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			char *szProto;
			if (cws->value.type == DBVT_DELETED)
				szProto = NULL;
			else
				szProto = cws->value.pszVal;
			cli.pfnChangeContactIcon((HANDLE) wParam,
				cli.pfnIconFromStatusMode(szProto,
					szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord((HANDLE) wParam, szProto, "Status",
					ID_STATUS_OFFLINE)), 0);
		}
	}

	// Clean up
	if (dbv.pszVal)
		free(dbv.pszVal);

	return 0;

}

/*-----------------------------------------------------*/

char* u2a( wchar_t* src )
{
	int codepage = CallService( MS_LANGPACK_GETCODEPAGE, 0, 0 );

	int cbLen = WideCharToMultiByte( codepage, 0, src, -1, NULL, 0, NULL, NULL );
	char* result = ( char* )malloc( cbLen+1 );
	if ( result == NULL )
		return NULL;

	WideCharToMultiByte( codepage, 0, src, -1, result, cbLen, NULL, NULL );
	result[ cbLen ] = 0;
	return result;
}

wchar_t* a2u( char* src )
{
	int codepage = CallService( MS_LANGPACK_GETCODEPAGE, 0, 0 );

	int cbLen = MultiByteToWideChar( codepage, 0, src, -1, NULL, 0 );
	wchar_t* result = ( wchar_t* )malloc( sizeof( wchar_t )*(cbLen+1));
	if ( result == NULL )
		return NULL;

	MultiByteToWideChar( codepage, 0, src, -1, result, cbLen );
	result[ cbLen ] = 0;
	return result;
}

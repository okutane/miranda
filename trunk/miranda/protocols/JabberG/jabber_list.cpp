/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan
Copyright ( C ) 2007     Maxim Mluhov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"
#include "jabber_list.h"

void JabberMenuUpdateSrmmIcon(JABBER_LIST_ITEM *item);

/////////////////////////////////////////////////////////////////////////////////////////
// List item freeing

static void JabberListFreeResourceInternal( JABBER_RESOURCE_STATUS *r)
{
	if ( r->resourceName ) mir_free( r->resourceName );
	if ( r->statusMessage ) mir_free( r->statusMessage );
	if ( r->software ) mir_free( r->software );
	if ( r->version ) mir_free( r->version );
	if ( r->system ) mir_free( r->system );
	if ( r->szCapsNode ) mir_free( r->szCapsNode );
	if ( r->szCapsVer ) mir_free( r->szCapsVer );
	if ( r->szCapsExt ) mir_free( r->szCapsExt );
	if ( r->szRealJid ) mir_free( r->szRealJid );
}

static void JabberListFreeItemInternal( JABBER_LIST_ITEM *item )
{
	if ( item == NULL )
		return;

	if ( item->jid ) mir_free( item->jid );
	if ( item->nick ) mir_free( item->nick );

	JABBER_RESOURCE_STATUS* r = item->resource;
	for ( int i=0; i < item->resourceCount; i++, r++ )
		JabberListFreeResourceInternal( r );
	if ( item->resource ) mir_free( item->resource );

	JabberListFreeResourceInternal( &item->itemResource );
	
	if ( item->group ) mir_free( item->group );
	if ( item->photoFileName ) {
		DeleteFileA( item->photoFileName );
		mir_free( item->photoFileName );
	}
	if ( item->messageEventIdStr ) mir_free( item->messageEventIdStr );
	if ( item->name ) mir_free( item->name );
	if ( item->type ) mir_free( item->type );
	if ( item->service ) mir_free( item->service );
	if ( item->password ) mir_free( item->password );
	if ( item->list==LIST_ROSTER && item->ft ) delete item->ft;
	mir_free( item );
}

void CJabberProto::JabberListWipe( void )
{
	int i;

	EnterCriticalSection( &csLists );
	for( i=0; i < roster.getCount(); i++ )
		JabberListFreeItemInternal( roster[i] );

	roster.destroy();
	LeaveCriticalSection( &csLists );
}

int CJabberProto::JabberListExist( JABBER_LIST list, const TCHAR* jid )
{
	JABBER_LIST_ITEM tmp;
	tmp.list = list;
	tmp.jid  = (TCHAR*)jid;
	tmp.bUseResource = FALSE;

	EnterCriticalSection( &csLists );

	//fyr
	if ( list == LIST_ROSTER )
	{
		tmp.list = LIST_CHATROOM;
		int id = roster.getIndex( &tmp );
		if ( id != -1) 
			tmp.bUseResource = TRUE;
		tmp.list = list;
	}
	
	int idx = roster.getIndex( &tmp );

	if ( idx == -1 ) {
		LeaveCriticalSection( &csLists );
		return 0;
	}

	LeaveCriticalSection( &csLists );
	return idx+1;
}

JABBER_LIST_ITEM *CJabberProto::JabberListAdd( JABBER_LIST list, const TCHAR* jid )
{
	JABBER_LIST_ITEM* item;
	BOOL bUseResource=FALSE;
	EnterCriticalSection( &csLists );
	if (( item = JabberListGetItemPtr( list, jid )) != NULL ) {
		LeaveCriticalSection( &csLists );
		return item;
	}

	TCHAR *s = mir_tstrdup( jid );
	TCHAR *q = NULL;
	// strip resource name if any
	//fyr
	if ( !((list== LIST_ROSTER )  && JabberListExist(LIST_CHATROOM, jid))) { // but only if it is not chat room contact	
		if ( list != LIST_VCARD_TEMP ) {
			TCHAR *p;
			if (( p = _tcschr( s, '@' )) != NULL )
				if (( q = _tcschr( p, '/' )) != NULL )
					*q = '\0';
		}
	} else {
		bUseResource=TRUE;
	}
	
	if ( !bUseResource && list== LIST_ROSTER )
	{
		//if it is a chat room keep resource and made it resource sensitive
		if ( JabberChatRoomHContactFromJID( s ) )
		{
			if (q != NULL)	*q='/';
			bUseResource=TRUE;
		}
	}
	item = ( JABBER_LIST_ITEM* )mir_alloc( sizeof( JABBER_LIST_ITEM ));
	ZeroMemory( item, sizeof( JABBER_LIST_ITEM ));
	item->list = list;
	item->jid = s;
	item->itemResource.status = ID_STATUS_OFFLINE;
	item->resource = NULL;
	item->resourceMode = RSMODE_LASTSEEN;
	item->lastSeenResource = -1;
	item->manualResource = -1;
	item->bUseResource = bUseResource;

	roster.insert( item );
	LeaveCriticalSection( &csLists );

	JabberMenuUpdateSrmmIcon(item);
	return item;
}

void CJabberProto::JabberListRemove( JABBER_LIST list, const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	if ( i != 0 ) {
		JabberListFreeItemInternal( roster[ --i ] );
		roster.remove( i );
	}
	LeaveCriticalSection( &csLists );
}

void CJabberProto::JabberListRemoveList( JABBER_LIST list )
{
	int i = 0;
	while (( i=JabberListFindNext( list, i )) >= 0 )
		JabberListRemoveByIndex( i );
}

void CJabberProto::JabberListRemoveByIndex( int index )
{
	EnterCriticalSection( &csLists );
	if ( index >= 0 && index < roster.getCount() ) {
		JabberListFreeItemInternal( roster[index] );
		roster.remove( index );
	}
	LeaveCriticalSection( &csLists );
}

int CJabberProto::JabberListAddResource( JABBER_LIST list, const TCHAR* jid, int status, const TCHAR* statusMessage, char priority )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return 0;
	}

	JABBER_LIST_ITEM* LI = roster[i-1];
	int bIsNewResource = false, j;

	const TCHAR* p = _tcschr( jid, '@' );
	const TCHAR* q = _tcschr(( p == NULL ) ? jid : p, '/' );
	if ( q ) {
		const TCHAR* resource = q+1;
		if ( resource[0] ) {
			JABBER_RESOURCE_STATUS* r = LI->resource;
			for ( j=0; j < LI->resourceCount; j++, r++ ) {
				if ( !_tcscmp( r->resourceName, resource )) {
					// Already exist, update status and statusMessage
					r->status = status;
					replaceStr( r->statusMessage, statusMessage );
					r->priority = priority;
					break;
			}	}

			if ( j >= LI->resourceCount ) {
				// Not already exist, add new resource
				LI->resource = ( JABBER_RESOURCE_STATUS * ) mir_realloc( LI->resource, ( LI->resourceCount+1 )*sizeof( JABBER_RESOURCE_STATUS ));
				bIsNewResource = true;
				r = LI->resource + LI->resourceCount++;
				memset( r, 0, sizeof( JABBER_RESOURCE_STATUS ));
				r->status = status;
				r->affiliation = AFFILIATION_NONE;
				r->role = ROLE_NONE;
				r->resourceName = mir_tstrdup( resource );
				if ( statusMessage )
					r->statusMessage = mir_tstrdup( statusMessage );
				r->priority = priority;
		}	}
	}
	// No resource, update the main statusMessage
	else {
		LI->itemResource.status = status;
		replaceStr( LI->itemResource.statusMessage, statusMessage );
	}

	LeaveCriticalSection( &csLists );

	JabberMenuUpdateSrmmIcon(LI);
	return bIsNewResource;
}

void CJabberProto::JabberListRemoveResource( JABBER_LIST list, const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	JABBER_LIST_ITEM* LI = roster[i-1];
	if ( !i || LI == NULL ) {
		LeaveCriticalSection( &csLists );
		return;
	}

	const TCHAR* p = _tcschr( jid, '@' );
	const TCHAR* q = _tcschr(( p == NULL ) ? jid : p, '/' );
	if ( q ) {
		const TCHAR* resource = q+1;
		if ( resource[0] ) {
			JABBER_RESOURCE_STATUS* r = LI->resource;
			int j;
			for ( j=0; j < LI->resourceCount; j++, r++ ) {
				if ( !_tcsicmp( r->resourceName, resource ))
					break;
			}
			if ( j < LI->resourceCount ) {
				// Found last seen resource ID to be removed
				if ( LI->lastSeenResource == j )
					LI->lastSeenResource = -1;
				else if ( LI->lastSeenResource > j )
					LI->lastSeenResource--;
				// update manually selected resource ID
				if (LI->resourceMode == RSMODE_MANUAL)
				{
					if ( LI->manualResource == j )
					{
						LI->resourceMode = RSMODE_LASTSEEN;
						LI->manualResource = -1;
					} else if ( LI->manualResource > j )
						LI->manualResource--;
				}

				// Update MirVer due to possible resource changes
				JabberUpdateMirVer(LI);

				JabberListFreeResourceInternal( r );

				if ( LI->resourceCount-- == 1 ) {
					mir_free( r );
					LI->resource = NULL;
				}
				else {
					memmove( r, r+1, ( LI->resourceCount-j )*sizeof( JABBER_RESOURCE_STATUS ));
					LI->resource = ( JABBER_RESOURCE_STATUS* )mir_realloc( LI->resource, LI->resourceCount*sizeof( JABBER_RESOURCE_STATUS ));
	}	}	}	}

	LeaveCriticalSection( &csLists );

	JabberMenuUpdateSrmmIcon(LI);
}

TCHAR* CJabberProto::JabberListGetBestResourceNamePtr( const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( LIST_ROSTER, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return NULL;
	}

	TCHAR* res = NULL;

	JABBER_LIST_ITEM* LI = roster[i-1];
	if ( LI->resourceCount > 1 ) {
		if ( LI->resourceMode == RSMODE_LASTSEEN && LI->lastSeenResource>=0 && LI->lastSeenResource < LI->resourceCount )
			res = LI->resource[ LI->lastSeenResource ].resourceName;
		else if (LI->resourceMode == RSMODE_MANUAL && LI->manualResource>=0 && LI->manualResource < LI->resourceCount )
			res = LI->resource[ LI->manualResource ].resourceName;
		else {
			int nBestPos = -1, nBestPri = -200, j;
			for ( j = 0; j < LI->resourceCount; j++ ) {
				if ( LI->resource[ j ].priority > nBestPri ) {
					nBestPri = LI->resource[ j ].priority;
					nBestPos = j;
				}
			}
			if ( nBestPos != -1 )
				res = LI->resource[ nBestPos ].resourceName;
		}
	}

	if ( !res && LI->resource)
		res = LI->resource[0].resourceName;

	LeaveCriticalSection( &csLists );
	return res;
}

TCHAR* CJabberProto::JabberListGetBestClientResourceNamePtr( const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( LIST_ROSTER, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return NULL;
	}

	JABBER_LIST_ITEM* LI = roster[i-1];
	TCHAR* res = JabberListGetBestResourceNamePtr( jid );
	if ( res == NULL ) {
		JABBER_RESOURCE_STATUS* r = LI->resource;
		int status = ID_STATUS_OFFLINE;
		res = NULL;
		for ( i=0; i < LI->resourceCount; i++ ) {
			int s = r[i].status;
			BOOL foundBetter = FALSE;
			switch ( s ) {
			case ID_STATUS_FREECHAT:
				foundBetter = TRUE;
				break;
			case ID_STATUS_ONLINE:
				if ( status != ID_STATUS_FREECHAT )
					foundBetter = TRUE;
				break;
			case ID_STATUS_DND:
				if ( status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE )
					foundBetter = TRUE;
				break;
			case ID_STATUS_AWAY:
				if ( status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND )
					foundBetter = TRUE;
				break;
			case ID_STATUS_NA:
				if ( status != ID_STATUS_FREECHAT && status != ID_STATUS_ONLINE && status != ID_STATUS_DND && status != ID_STATUS_AWAY )
					foundBetter = TRUE;
				break;
			}
			if ( foundBetter ) {
				res = r[i].resourceName;
				status = s;
	}	}	}

	LeaveCriticalSection( &csLists );
	return res;
}

int CJabberProto::JabberListFindNext( JABBER_LIST list, int fromOffset )
{
	EnterCriticalSection( &csLists );
	int i = ( fromOffset >= 0 ) ? fromOffset : 0;
	for( ; i<roster.getCount(); i++ )
		if ( roster[i]->list == list ) {
		  	LeaveCriticalSection( &csLists );
			return i;
		}
	LeaveCriticalSection( &csLists );
	return -1;
}

JABBER_LIST_ITEM *CJabberProto::JabberListGetItemPtr( JABBER_LIST list, const TCHAR* jid )
{
	EnterCriticalSection( &csLists );
	int i = JabberListExist( list, jid );
	if ( !i ) {
		LeaveCriticalSection( &csLists );
		return NULL;
	}
	i--;
	LeaveCriticalSection( &csLists );
	return roster[i];
}

JABBER_LIST_ITEM *CJabberProto::JabberListGetItemPtrFromIndex( int index )
{
	EnterCriticalSection( &csLists );
	if ( index >= 0 && index < roster.getCount() ) {
		LeaveCriticalSection( &csLists );
		return roster[index];
	}
	LeaveCriticalSection( &csLists );
	return NULL;
}

BOOL CJabberProto::JabberListLock()
{
	EnterCriticalSection( &csLists );
	return TRUE;
}

BOOL CJabberProto::JabberListUnlock()
{
	LeaveCriticalSection( &csLists );
	return TRUE;
}

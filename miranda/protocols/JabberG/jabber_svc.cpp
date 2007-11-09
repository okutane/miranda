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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_svc.cpp,v $
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"

#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "resource.h"
#include "jabber_list.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "m_file.h"
#include "m_addcontact.h"
#include "jabber_disco.h"
#include "sdk/m_proto_listeningto.h"

extern LIST<void> arServices;

////////////////////////////////////////////////////////////////////////////////////////
// JabberAddToList - adds a contact to the contact list

HANDLE AddToListByJID( const TCHAR* newJid, DWORD flags )
{
	HANDLE hContact;
	TCHAR* jid, *nick;

	JabberLog( "AddToListByJID jid = " TCHAR_STR_PARAM, newJid );

	if (( hContact=JabberHContactFromJID( newJid )) == NULL ) {
		// not already there: add
		jid = mir_tstrdup( newJid );
		JabberLog( "Add new jid to contact jid = " TCHAR_STR_PARAM, jid );
		hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_ADD, 0, 0 );
		JCallService( MS_PROTO_ADDTOCONTACT, ( WPARAM ) hContact, ( LPARAM )jabberProtoName );
		JSetStringT( hContact, "jid", jid );
		if (( nick=JabberNickFromJID( newJid )) == NULL )
			nick = mir_tstrdup( newJid );
//		JSetStringT( hContact, "Nick", nick );
		mir_free( nick );
		mir_free( jid );

		// Note that by removing or disable the "NotOnList" will trigger
		// the plugin to add a particular contact to the roster list.
		// See DBSettingChanged hook at the bottom part of this source file.
		// But the add module will delete "NotOnList". So we will not do it here.
		// Also because we need "MyHandle" and "Group" info, which are set after
		// PS_ADDTOLIST is called but before the add dialog issue deletion of
		// "NotOnList".
		// If temporary add, "NotOnList" won't be deleted, and that's expected.
		DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
		if ( flags & PALF_TEMPORARY )
			DBWriteContactSettingByte( hContact, "CList", "Hidden", 1 );
	}
	else {
		// already exist
		// Set up a dummy "NotOnList" when adding permanently only
		if ( !( flags & PALF_TEMPORARY ))
			DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
	}

	if (hContact && newJid)
		JabberDBCheckIsTransportedContact( newJid, hContact );
	return hContact;
}

int JabberAddToList( WPARAM wParam, LPARAM lParam )
{
	JABBER_SEARCH_RESULT* jsr = ( JABBER_SEARCH_RESULT * ) lParam;
	if ( jsr->hdr.cbSize != sizeof( JABBER_SEARCH_RESULT ))
		return ( int )NULL;

	return ( int )AddToListByJID( jsr->jid, wParam );	// wParam is flag e.g. PALF_TEMPORARY
}

int JabberAddToListByEvent( WPARAM wParam, LPARAM lParam )
{
	DBEVENTINFO dbei;
	HANDLE hContact;
	char* nick, *firstName, *lastName, *jid;

	JabberLog( "AddToListByEvent" );
	ZeroMemory( &dbei, sizeof( dbei ));
	dbei.cbSize = sizeof( dbei );
	if (( dbei.cbBlob=JCallService( MS_DB_EVENT_GETBLOBSIZE, lParam, 0 )) == ( DWORD )( -1 ))
		return ( int )( HANDLE ) NULL;
	if (( dbei.pBlob=( PBYTE ) alloca( dbei.cbBlob )) == NULL )
		return ( int )( HANDLE ) NULL;
	if ( JCallService( MS_DB_EVENT_GET, lParam, ( LPARAM )&dbei ))
		return ( int )( HANDLE ) NULL;
	if ( strcmp( dbei.szModule, jabberProtoName ))
		return ( int )( HANDLE ) NULL;

/*
	// EVENTTYPE_CONTACTS is when adding from when we receive contact list ( not used in Jabber )
	// EVENTTYPE_ADDED is when adding from when we receive "You are added" ( also not used in Jabber )
	// Jabber will only handle the case of EVENTTYPE_AUTHREQUEST
	// EVENTTYPE_AUTHREQUEST is when adding from the authorization request dialog
*/

	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST )
		return ( int )( HANDLE ) NULL;

	nick = ( char* )( dbei.pBlob + sizeof( DWORD )+ sizeof( HANDLE ));
	firstName = nick + strlen( nick ) + 1;
	lastName = firstName + strlen( firstName ) + 1;
	jid = lastName + strlen( lastName ) + 1;

	TCHAR* newJid = mir_a2t( jid );
	hContact = ( HANDLE ) AddToListByJID( newJid, wParam );
	mir_free( newJid );
	return ( int ) hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthAllow - processes the successful authorization

int JabberAuthAllow( WPARAM wParam, LPARAM lParam )
{
	DBEVENTINFO dbei;
	char* nick, *firstName, *lastName, *jid;

	if ( !jabberOnline )
		return 1;

	memset( &dbei, sizeof( dbei ), 0 );
	dbei.cbSize = sizeof( dbei );
	if (( dbei.cbBlob=JCallService( MS_DB_EVENT_GETBLOBSIZE, wParam, 0 )) == ( DWORD )( -1 ))
		return 1;
	if (( dbei.pBlob=( PBYTE )alloca( dbei.cbBlob )) == NULL )
		return 1;
	if ( JCallService( MS_DB_EVENT_GET, wParam, ( LPARAM )&dbei ))
		return 1;
	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST )
		return 1;
	if ( strcmp( dbei.szModule, jabberProtoName ))
		return 1;

	nick = ( char* )( dbei.pBlob + sizeof( DWORD )+ sizeof( HANDLE ));
	firstName = nick + strlen( nick ) + 1;
	lastName = firstName + strlen( firstName ) + 1;
	jid = lastName + strlen( lastName ) + 1;

	JabberLog( "Send 'authorization allowed' to " TCHAR_STR_PARAM, jid );

	XmlNode presence( "presence" ); presence.addAttr( "to", jid ); presence.addAttr( "type", "subscribed" );
	jabberThreadInfo->send( presence );

	TCHAR* newJid = mir_a2t( jid );

	// Automatically add this user to my roster if option is enabled
	if ( JGetByte( "AutoAdd", TRUE ) == TRUE ) {
		HANDLE hContact;
		JABBER_LIST_ITEM *item;

		if (( item = JabberListGetItemPtr( LIST_ROSTER, newJid )) == NULL || ( item->subscription != SUB_BOTH && item->subscription != SUB_TO )) {
			JabberLog( "Try adding contact automatically jid = " TCHAR_STR_PARAM, jid );
			if (( hContact=AddToListByJID( newJid, 0 )) != NULL ) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and JabberDbSettingChanged().
				DBDeleteContactSetting( hContact, "CList", "NotOnList" );
	}	}	}

	mir_free( newJid );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberAuthDeny - handles the unsuccessful authorization

int JabberAuthDeny( WPARAM wParam, LPARAM lParam )
{
	DBEVENTINFO dbei;
	char* nick, *firstName, *lastName, *jid;

	if ( !jabberOnline )
		return 1;

	JabberLog( "Entering AuthDeny" );
	memset( &dbei, sizeof( dbei ), 0 );
	dbei.cbSize = sizeof( dbei );
	if (( dbei.cbBlob=JCallService( MS_DB_EVENT_GETBLOBSIZE, wParam, 0 )) == ( DWORD )( -1 ))
		return 1;
	if (( dbei.pBlob=( PBYTE ) mir_alloc( dbei.cbBlob )) == NULL )
		return 1;
	if ( JCallService( MS_DB_EVENT_GET, wParam, ( LPARAM )&dbei )) {
		mir_free( dbei.pBlob );
		return 1;
	}
	if ( dbei.eventType != EVENTTYPE_AUTHREQUEST ) {
		mir_free( dbei.pBlob );
		return 1;
	}
	if ( strcmp( dbei.szModule, jabberProtoName )) {
		mir_free( dbei.pBlob );
		return 1;
	}

	nick = ( char* )( dbei.pBlob + sizeof( DWORD )+ sizeof( HANDLE ));
	firstName = nick + strlen( nick ) + 1;
	lastName = firstName + strlen( firstName ) + 1;
	jid = lastName + strlen( lastName ) + 1;

	JabberLog( "Send 'authorization denied' to " TCHAR_STR_PARAM, jid );

	XmlNode presence( "presence" ); presence.addAttr( "to", jid ); presence.addAttr( "type", "unsubscribed" );
	jabberThreadInfo->send( presence );

	mir_free( dbei.pBlob );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberBasicSearch - searches the contact by JID

struct JABBER_SEARCH_BASIC
{	int hSearch;
	char jid[128];
};

static void __cdecl JabberBasicSearchThread( JABBER_SEARCH_BASIC *jsb )
{
	SleepEx( 100, TRUE );

	JABBER_SEARCH_RESULT jsr = { 0 };
	jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
	jsr.hdr.nick = "";
	jsr.hdr.firstName = "";
	jsr.hdr.lastName = "";
	jsr.hdr.email = jsb->jid;

	TCHAR* jid = mir_a2t(jsb->jid);
	_tcsncpy( jsr.jid, jid, SIZEOF( jsr.jid ));
	mir_free( jid );

	jsr.jid[SIZEOF( jsr.jid )-1] = '\0';
	JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, ( HANDLE ) jsb->hSearch, ( LPARAM )&jsr );
	JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) jsb->hSearch, 0 );
	mir_free( jsb );
}

int JabberBasicSearch( WPARAM wParam, LPARAM lParam )
{
	char* szJid = ( char* )lParam;
	JabberLog( "JabberBasicSearch called with lParam = '%s'", szJid );

	JABBER_SEARCH_BASIC *jsb;
	if ( !jabberOnline || ( jsb=( JABBER_SEARCH_BASIC * ) mir_alloc( sizeof( JABBER_SEARCH_BASIC )) )==NULL )
		return 0;

	if ( strchr( szJid, '@' ) == NULL ) {
		const char* p = strstr( szJid, jabberThreadInfo->server );
		if ( !p ) {
			char szServer[ 100 ];
			if ( JGetStaticString( "LoginServer", NULL, szServer, sizeof szServer ))
				strcpy( szServer, "jabber.org" );

			mir_snprintf( jsb->jid, SIZEOF(jsb->jid), "%s@%s", szJid, szServer );
		}
		else strncpy( jsb->jid, szJid, SIZEOF(jsb->jid));
	}
	else strncpy( jsb->jid, szJid, SIZEOF(jsb->jid));

	JabberLog( "Adding '%s' without validation", jsb->jid );
	jsb->hSearch = JabberSerialNext();
	mir_forkthread(( pThreadFunc )JabberBasicSearchThread, jsb );
	return jsb->hSearch;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberContactDeleted - processes a contact deletion

int JabberContactDeleted( WPARAM wParam, LPARAM lParam )
{
	if( !jabberOnline )	// should never happen
		return 0;

	char* szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if ( szProto==NULL || strcmp( szProto, jabberProtoName ))
		return 0;

	DBVARIANT dbv;
	if ( !JGetStringT(( HANDLE ) wParam, JGetByte( (HANDLE ) wParam, "ChatRoom", 0 )?(char*)"ChatRoomID":(char*)"jid", &dbv )) {
		if ( JabberListExist( LIST_ROSTER, dbv.ptszVal )) {
			// Remove from roster, server also handles the presence unsubscription process.
			XmlNodeIq iq( "set" ); iq.addAttrID( JabberSerialNext());
			XmlNode* query = iq.addQuery( "jabber:iq:roster" );
			XmlNode* item = query->addChild( "item" ); item->addAttr( "jid", dbv.ptszVal ); item->addAttr( "subscription", "remove" );
			jabberThreadInfo->send( iq );
		}

		JFreeVariant( &dbv );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberIdleChanged - tracks idle start time for XEP-0012 support

int JabberIdleChanged( WPARAM wParam, LPARAM lParam )
{
	// don't report idle time, if user disabled
	if (lParam & IDF_PRIVACY) {
		jabberIdleStartTime = 0;
		return 0;
	}

	BOOL bIdle = lParam & IDF_ISIDLE;

	// second call, ignore it
	if (bIdle && jabberIdleStartTime)
		return 0;

	jabberIdleStartTime = bIdle ? time( 0 ) : 0;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberDbSettingChanged - process database changes

static TCHAR* sttSettingToTchar( DBCONTACTWRITESETTING* cws )
{
	switch( cws->value.type ) {
	case DBVT_ASCIIZ:
		#if defined( _UNICODE )
			return mir_a2u( cws->value.pszVal );
		#else
			return mir_strdup( cws->value.pszVal );
		#endif

	case DBVT_UTF8:
		#if defined( _UNICODE )
		{	TCHAR* result;
			mir_utf8decode( NEWSTR_ALLOCA(cws->value.pszVal), &result );
			return result;
		}
		#else
			return mir_strdup( mir_utf8decode( NEWSTR_ALLOCA(cws->value.pszVal), NULL ));
		#endif
	}
	return NULL;
}

static void sttRenameGroup( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	DBVARIANT jid, dbv;
	if ( JGetStringT( hContact, "jid", &jid ))
		return;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid.ptszVal );
	JFreeVariant( &jid );
	if ( item == NULL )
		return;

	TCHAR* nick;
	if ( !DBGetContactSettingTString( hContact, "CList", "MyHandle", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else if ( !JGetStringT( hContact, "Nick", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else nick = JabberNickFromJID( item->jid );
	if ( nick == NULL )
		return;

	if ( cws->value.type == DBVT_DELETED ) {
		if ( item->group != NULL ) {
			JabberLog( "Group set to nothing" );
			JabberAddContactToRoster( item->jid, nick, NULL, item->subscription );
		}
	}
	else {
		TCHAR* p = sttSettingToTchar( cws );
		if ( cws->value.pszVal != NULL && lstrcmp( p, item->group )) {
			JabberLog( "Group set to " TCHAR_STR_PARAM, p );
			if ( p )
				JabberAddContactToRoster( item->jid, nick, p, item->subscription );
		}
		mir_free( p );
	}
	mir_free( nick );
}

static void sttRenameContact( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	DBVARIANT jid;
	if ( JGetStringT( hContact, "jid", &jid ))
		return;

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid.ptszVal );
	JFreeVariant( &jid );
	if ( item == NULL )
		return;

	if ( cws->value.type == DBVT_DELETED ) {
		TCHAR* nick = ( TCHAR* )JCallService( MS_CLIST_GETCONTACTDISPLAYNAME, ( WPARAM )hContact, GCDNF_NOMYHANDLE | GCDNF_TCHAR );
		JabberAddContactToRoster( item->jid, nick, item->group, item->subscription );
		mir_free(nick);
		return;
	}

	TCHAR* newNick = sttSettingToTchar( cws );
	if ( newNick ) {
		if ( lstrcmp( item->nick, newNick )) {
			JabberLog( "Renaming contact " TCHAR_STR_PARAM ": " TCHAR_STR_PARAM " -> " TCHAR_STR_PARAM, item->jid, item->nick, newNick );
			JabberAddContactToRoster( item->jid, newNick, item->group, item->subscription );
		}
		mir_free( newNick );
}	}

void sttAddContactForever( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	if ( cws->value.type != DBVT_DELETED && !( cws->value.type==DBVT_BYTE && cws->value.bVal==0 ))
		return;

	DBVARIANT jid, dbv;
	if ( JGetStringT( hContact, "jid", &jid ))
		return;

	TCHAR *nick;
	JabberLog( "Add " TCHAR_STR_PARAM " permanently to list", jid.pszVal );
	if ( !DBGetContactSettingTString( hContact, "CList", "MyHandle", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else if ( !JGetStringT( hContact, "Nick", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else nick = JabberNickFromJID( jid.ptszVal );
	if ( nick == NULL ) {
		JFreeVariant( &jid );
		return;
	}

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid.ptszVal );
	JABBER_SUBSCRIPTION subscription = ( item == NULL ) ? SUB_NONE : item->subscription;

	if ( !DBGetContactSettingTString( hContact, "CList", "Group", &dbv )) {
		JabberAddContactToRoster( jid.ptszVal, nick, dbv.ptszVal, subscription );
		JFreeVariant( &dbv );
	}
	else JabberAddContactToRoster( jid.ptszVal, NULL, NULL, subscription );

	XmlNode presence( "presence" ); presence.addAttr( "to", jid.ptszVal ); presence.addAttr( "type", "subscribe" );
	jabberThreadInfo->send( presence );

	JabberSendGetVcard( jid.ptszVal );

	mir_free( nick );
	DBDeleteContactSetting( hContact, "CList", "Hidden" );
	JFreeVariant( &jid );
}

int JabberDbSettingChanged( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact = ( HANDLE ) wParam;
	if ( hContact == NULL || !jabberConnected )
		return 0;

	DBCONTACTWRITESETTING* cws = ( DBCONTACTWRITESETTING* )lParam;
	if ( strcmp( cws->szModule, "CList" ))
		return 0;

	char* szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
	if ( szProto == NULL || strcmp( szProto, jabberProtoName ))
		return 0;

	if ( !strcmp( cws->szSetting, "Group" ))
		sttRenameGroup( cws, hContact );
	else if ( !strcmp( cws->szSetting, "MyHandle" ))
		sttRenameContact( cws, hContact );
	else if ( !strcmp( cws->szSetting, "NotOnList" ))
		sttAddContactForever( cws, hContact );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileAllow - starts a file transfer

int JabberFileAllow( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	filetransfer* ft = ( filetransfer* ) ccs->wParam;
	ft->std.workingDir = mir_strdup(( char* )ccs->lParam );
	int len = strlen( ft->std.workingDir )-1;
	if ( ft->std.workingDir[len] == '/' || ft->std.workingDir[len] == '\\' )
		ft->std.workingDir[len] = 0;

	switch ( ft->type ) {
	case FT_OOB:
		mir_forkthread(( pThreadFunc )JabberFileReceiveThread, ft );
		break;
	case FT_BYTESTREAM:
		JabberFtAcceptSiRequest( ft );
		break;
	case FT_IBB:
		JabberFtAcceptIbbRequest( ft );
		break;
	}
	return ccs->wParam;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileCancel - cancels a file transfer

int JabberFileCancel( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;
	filetransfer* ft = ( filetransfer* ) ccs->wParam;
	HANDLE hEvent;

	JabberLog( "Invoking FileCancel()" );
	if ( ft->type == FT_OOB ) {
		if ( ft->s ) {
			JabberLog( "FT canceled" );
			JabberLog( "Closing ft->s = %d", ft->s );
			ft->state = FT_ERROR;
			Netlib_CloseHandle( ft->s );
			ft->s = NULL;
			if ( ft->hFileEvent != NULL ) {
				hEvent = ft->hFileEvent;
				ft->hFileEvent = NULL;
				SetEvent( hEvent );
			}
			JabberLog( "ft->s is now NULL, ft->state is now FT_ERROR" );
		}
	}
	else JabberFtCancel( ft );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileDeny - denies a file transfer

int JabberFileDeny( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 1;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	filetransfer* ft = ( filetransfer* )ccs->wParam;

	XmlNodeIq iq( "error", ft->iqId, ft->jid );

	switch ( ft->type ) {
	case FT_OOB:
		{	XmlNode* e = iq.addChild( "error", _T("File transfer refused"));
			e->addAttr( "code", 406 );
			jabberThreadInfo->send( iq );
		}
		break;
	case FT_BYTESTREAM:
	case FT_IBB:
		{	XmlNode* e = iq.addChild( "error", _T("File transfer refused"));
			e->addAttr( "code", 403 ); e->addAttr( "type", "cancel" );
			XmlNode* f = e->addChild( "forbidden" ); f->addAttr( "xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas" );
			XmlNode* t = f->addChild( "text", "File transfer refused" ); t->addAttr( "xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas" );
			jabberThreadInfo->send( iq );
		}
		break;
	}
	delete ft;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberFileResume - processes file renaming etc

int JabberFileResume( WPARAM wParam, LPARAM lParam )
{
	filetransfer* ft = ( filetransfer* )wParam;
	if ( !jabberConnected || ft == NULL )
		return 1;

	PROTOFILERESUME *pfr = (PROTOFILERESUME*)lParam;
	if ( pfr->action == FILERESUME_RENAME ) {
		if ( ft->wszFileName != NULL ) {
			mir_free( ft->wszFileName );
			ft->wszFileName = NULL;
		}

		replaceStr( ft->std.currentFile, pfr->szFilename );
	}

	SetEvent( ft->hWaitEvent );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatar - retrieves the file name of my own avatar

int JabberGetAvatar(WPARAM wParam, LPARAM lParam)
{
	char* buf = ( char* )wParam;
	int  size = ( int )lParam;

	if ( buf == NULL || size <= 0 )
		return -1;

	if ( !JGetByte( "EnableAvatars", TRUE ))
		return -2;

	JabberGetAvatarFileName( NULL, buf, size );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarCaps - returns directives how to process avatars

int JabberGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch( wParam ) {
	case AF_MAXSIZE:
		{
			POINT* size = (POINT*)lParam;
			if ( size )
				size->x = size->y = 96;
		}
      return 0;

	case AF_PROPORTION:
		return PIP_NONE;

	case AF_FORMATSUPPORTED: // Jabber supports avatars of virtually all formats
		return 1;

	case AF_ENABLED:
		return JGetByte( "EnableAvatars", TRUE );
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarInfo - retrieves the avatar info

static int JabberGetAvatarInfo(WPARAM wParam,LPARAM lParam)
{
	if ( !JGetByte( "EnableAvatars", TRUE ))
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )lParam;

	char szHashValue[ MAX_PATH ];
	if ( JGetStaticString( "AvatarHash", AI->hContact, szHashValue, sizeof szHashValue )) {
		JabberLog( "No avatar" );
		return GAIR_NOAVATAR;
	}

	JabberGetAvatarFileName( AI->hContact, AI->filename, sizeof AI->filename );
	AI->format = ( AI->hContact == NULL ) ? PA_FORMAT_PNG : JGetByte( AI->hContact, "AvatarType", 0 );

	if ( ::access( AI->filename, 0 ) == 0 ) {
		char szSavedHash[ 256 ];
		if ( !JGetStaticString( "AvatarSaved", AI->hContact, szSavedHash, sizeof szSavedHash )) {
			if ( !strcmp( szSavedHash, szHashValue )) {
				JabberLog( "Avatar is Ok: %s == %s", szSavedHash, szHashValue );
				return GAIR_SUCCESS;
	}	}	}

	if (( wParam & GAIF_FORCE ) != 0 && AI->hContact != NULL && jabberOnline ) {
		DBVARIANT dbv;
		if ( !JGetStringT( AI->hContact, "jid", &dbv )) {
			JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal );
			if ( item != NULL ) {
				TCHAR szJid[ 512 ];
				BOOL isXVcard = JGetByte( AI->hContact, "AvatarXVcard", 0 );
				if ( item->resourceCount != NULL && !isXVcard ) {
					TCHAR *bestResName = JabberListGetBestClientResourceNamePtr(dbv.ptszVal);
					mir_sntprintf( szJid, SIZEOF( szJid ), bestResName?_T("%s/%s"):_T("%s"), dbv.ptszVal, bestResName );
				}
				else lstrcpyn( szJid, dbv.ptszVal, SIZEOF( szJid ));

				JabberLog( "Rereading %s for " TCHAR_STR_PARAM, isXVcard ? JABBER_FEAT_VCARD_TEMP : JABBER_FEAT_AVATAR, szJid );

				int iqId = JabberSerialNext();
				JabberIqAdd( iqId, IQ_PROC_NONE, JabberIqResultGetAvatar );

				XmlNodeIq iq( "get", iqId, szJid );
				if ( isXVcard )
					iq.addChild( "vCard" )->addAttr( "xmlns", JABBER_FEAT_VCARD_TEMP );
				else
					iq.addQuery( isXVcard ? "" : JABBER_FEAT_AVATAR );
				jabberThreadInfo->send( iq );

				JFreeVariant( &dbv );
				return GAIR_WAITFOR;
	}	}	}

	JabberLog( "No avatar" );
	return GAIR_NOAVATAR;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAwayMsg - returns a contact's away message

static void __cdecl JabberGetAwayMsgThread( HANDLE hContact )
{
	DBVARIANT dbv;
	JABBER_LIST_ITEM *item;
	JABBER_RESOURCE_STATUS *r;
	int i, len, msgCount;

	if ( !JGetStringT( hContact, "jid", &dbv )) {
		if (( item=JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal )) != NULL ) {
			JFreeVariant( &dbv );
			if ( item->resourceCount > 0 ) {
				JabberLog( "resourceCount > 0" );
				r = item->resource;
				len = msgCount = 0;
				for ( i=0; i<item->resourceCount; i++ ) {
					if ( r[i].statusMessage ) {
						msgCount++;
						len += ( _tcslen( r[i].resourceName ) + _tcslen( r[i].statusMessage ) + 8 );
				}	}

				TCHAR* str = ( TCHAR* )alloca( sizeof( TCHAR )*( len+1 ));
				str[0] = str[len] = '\0';
				for ( i=0; i < item->resourceCount; i++ ) {
					if ( r[i].statusMessage ) {
						if ( str[0] != '\0' ) _tcscat( str, _T("\r\n" ));
						if ( msgCount > 1 ) {
							_tcscat( str, _T("( "));
							_tcscat( str, r[i].resourceName );
							_tcscat( str, _T(" ): "));
						}
						_tcscat( str, r[i].statusMessage );
				}	}

				char* msg = mir_t2a(str);
				char* msg2 = JabberUnixToDos(msg);
				JSendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE ) 1, ( LPARAM )msg2 );
				mir_free(msg);
				mir_free(msg2);
				return;
			}

			if ( item->itemResource.statusMessage != NULL ) {
				char* msg = mir_t2a(item->itemResource.statusMessage);
				JSendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE ) 1, ( LPARAM )msg );
				mir_free(msg);
				return;
			}
		}
		else JFreeVariant( &dbv );
	}

	JSendBroadcast( hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, ( HANDLE ) 1, ( LPARAM )"" );
}

int JabberGetAwayMsg( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;
	if ( ccs == NULL )
		return 0;

	JabberLog( "GetAwayMsg called, wParam=%d lParam=%d", wParam, lParam );
	mir_forkthread( JabberGetAwayMsgThread, ccs->hContact );
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetCaps - return protocol capabilities bits

int JabberGetCaps( WPARAM wParam, LPARAM lParam )
{
	switch( wParam ) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_SERVERCLIST | PF1_MODEMSG|PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_FILE;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_NOCUSTOMAUTH | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDUTF;
	case PFLAG_UNIQUEIDTEXT:
		return ( int ) JTranslate( "JID" );
	case PFLAG_UNIQUEIDSETTING:
		return ( int ) "jid";
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetInfo - retrieves a contact info

void JabberProcessIqResultVersion( XmlNode* node, void* userdata, CJabberIqInfo *pInfo );

int JabberGetInfo( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline )
		return 1;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	int result = 1;
	DBVARIANT dbv;
	if ( JGetByte( ccs->hContact, "ChatRoom" , 0) )
	{
		// process chat room request

	} 
	else if ( !JGetStringT( ccs->hContact, "jid", &dbv )) {
		if ( jabberThreadInfo ) {
			TCHAR jid[ 256 ];
			JabberGetClientJID( dbv.ptszVal, jid, SIZEOF( jid ));

			XmlNodeIq iq( g_JabberIqManager.AddHandler( JabberIqResultEntityTime, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_HCONTACT ));
			XmlNode* pReq = iq.addChild( "time" );
			pReq->addAttr( "xmlns", JABBER_FEAT_ENTITY_TIME );
			jabberThreadInfo->send( iq );

			// XEP-0012, last logoff time
			XmlNodeIq iq2( g_JabberIqManager.AddHandler( JabberIqResultLastActivity, JABBER_IQ_TYPE_GET, dbv.ptszVal, JABBER_IQ_PARSE_FROM ));
			iq2.addQuery( JABBER_FEAT_LAST_ACTIVITY );
			jabberThreadInfo->send( iq2 );

			JABBER_LIST_ITEM *item = NULL;

			if (( item = JabberListGetItemPtr( LIST_VCARD_TEMP, dbv.ptszVal )) == NULL)
				item = JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal );

			if ( !item ) {
				TCHAR szBareJid[ 1024 ];
				_tcsncpy( szBareJid, dbv.ptszVal, 1023 );
				TCHAR* pDelimiter = _tcschr( szBareJid, _T('/') );
				if ( pDelimiter ) {
					*pDelimiter = 0;
					pDelimiter++;
					if ( !*pDelimiter )
						pDelimiter = NULL;
				}
				JABBER_LIST_ITEM *tmpItem = NULL;
				if ( pDelimiter && ( tmpItem  = JabberListGetItemPtr( LIST_CHATROOM, szBareJid ))) {
					JABBER_RESOURCE_STATUS *him = NULL;
					for ( int i=0; i < tmpItem->resourceCount; i++ ) {
						JABBER_RESOURCE_STATUS& p = tmpItem->resource[i];
						if ( !lstrcmp( p.resourceName, pDelimiter )) him = &p;
					}
					if ( him ) {
						item = JabberListAdd( LIST_VCARD_TEMP, dbv.ptszVal );
						JabberListAddResource( LIST_VCARD_TEMP, dbv.ptszVal, him->status, him->statusMessage, him->priority );
					}
				}
				else
					item = JabberListAdd( LIST_VCARD_TEMP, dbv.ptszVal );
			}

			if ( item ) {
				if ( item->resource ) {
					for ( int i = 0; i < item->resourceCount; i++ ) {
						TCHAR szp1[ JABBER_MAX_JID_LEN ];
						JabberStripJid( dbv.ptszVal, szp1, SIZEOF( szp1 ));
						mir_sntprintf( jid, 256, _T("%s/%s"), szp1, item->resource[i].resourceName );

						XmlNodeIq iq3( g_JabberIqManager.AddHandler( JabberIqResultLastActivity, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_FROM ));
						iq3.addQuery( JABBER_FEAT_LAST_ACTIVITY );
						jabberThreadInfo->send( iq3 );

						if ( !item->resource[i].dwVersionRequestTime ) {
							XmlNodeIq iq4( g_JabberIqManager.AddHandler( JabberProcessIqResultVersion, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_CHILD_TAG_NODE ));
							XmlNode* query = iq4.addQuery( JABBER_FEAT_VERSION );
							jabberThreadInfo->send( iq4 );
					}	}
				}
				else if ( !item->itemResource.dwVersionRequestTime ) {
					XmlNodeIq iq4( g_JabberIqManager.AddHandler( JabberProcessIqResultVersion, JABBER_IQ_TYPE_GET, item->jid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_CHILD_TAG_NODE ));
					XmlNode* query = iq4.addQuery( JABBER_FEAT_VERSION );
					jabberThreadInfo->send( iq4 );
		}	}	}

		JabberSendGetVcard( dbv.ptszVal );
		JFreeVariant( &dbv );
		result = 0;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetName - returns the protocol name

int JabberGetName( WPARAM wParam, LPARAM lParam )
{
	lstrcpynA(( char* )lParam, jabberModuleName, wParam );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetStatus - returns the protocol status

int JabberGetStatus( WPARAM wParam, LPARAM lParam )
{
	return jabberStatus;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberLoadIcon - loads an icon for the contact list

int JabberLoadIcon( WPARAM wParam, LPARAM lParam )
{
	if (( wParam & 0xffff ) == PLI_PROTOCOL )
		return ( int )CopyIcon( LoadIconEx( "main" ));

	return ( int ) ( HICON ) NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberRecvFile - receives a file

int JabberRecvFile( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA* )lParam;
	DBDeleteContactSetting( ccs->hContact, "CList", "Hidden" );
	return CallService( MS_PROTO_RECVFILE, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberRecvMessage - receives a message

int JabberRecvMessage( WPARAM wParam, LPARAM lParam )
{
	return CallService( MS_PROTO_RECVMSG, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSearchByEmail - searches the contact by its e-mail

int JabberSearchByEmail( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;
	if (( char* )lParam == NULL ) return 0;

	char szServerName[100];
	if ( JGetStaticString( "Jud", NULL, szServerName, sizeof szServerName ))
		strcpy( szServerName, "users.jabber.org" );

	int iqId = JabberSerialNext();
	JabberIqAdd( iqId, IQ_PROC_GETSEARCH, JabberIqResultSetSearch );

	XmlNodeIq iq( "set", iqId, szServerName );
	XmlNode* query = iq.addQuery( "jabber:iq:search" );
	query->addChild( "email", ( char* )lParam );
	jabberThreadInfo->send( iq );
	return iqId;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSearchByName - searches the contact by its first or last name, or by a nickname

int JabberSearchByName( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;

	PROTOSEARCHBYNAME *psbn = ( PROTOSEARCHBYNAME * ) lParam;
	BOOL bIsExtFormat = JGetByte( "ExtendedSearch", TRUE );

	char szServerName[100];
	if ( JGetStaticString( "Jud", NULL, szServerName, sizeof szServerName ))
		strcpy( szServerName, "users.jabber.org" );

	int iqId = JabberSerialNext();
	XmlNodeIq iq( "set", iqId, szServerName );
	XmlNode* query = iq.addChild( "query" ), *field, *x;
	query->addAttr( "xmlns", "jabber:iq:search" );

	if ( bIsExtFormat ) {
		JabberIqAdd( iqId, IQ_PROC_GETSEARCH, JabberIqResultExtSearch );

		TCHAR *szXmlLang = JabberGetXmlLang();
		if ( szXmlLang ) {
			iq.addAttr( "xml:lang", szXmlLang );
			mir_free( szXmlLang );
		}
		x = query->addChild( "x" ); x->addAttr( "xmlns", JABBER_FEAT_DATA_FORMS ); x->addAttr( "type", "submit" );
	}
	else JabberIqAdd( iqId, IQ_PROC_GETSEARCH, JabberIqResultSetSearch );

	if ( psbn->pszNick[0] != '\0' ) {
		if ( bIsExtFormat ) {
			field = x->addChild( "field" ); field->addAttr( "var", "user" );
			field->addChild( "value", psbn->pszNick );
		}
		else query->addChild( "nick", psbn->pszNick );
	}

	if ( psbn->pszFirstName[0] != '\0' ) {
		if ( bIsExtFormat ) {
			field = x->addChild( "field" ); field->addAttr( "var", "fn" );
			field->addChild( "value", psbn->pszFirstName );
		}
		else query->addChild( "first", psbn->pszFirstName );
	}

	if ( psbn->pszLastName[0] != '\0' ) {
		if ( bIsExtFormat ) {
			field = x->addChild( "field" ); field->addAttr( "var", "given" );
			field->addChild( "value", psbn->pszLastName );
		}
		else query->addChild( "last", psbn->pszLastName );
	}

	jabberThreadInfo->send( iq );
	return iqId;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSendFile - sends a file

int JabberSendFile( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;

	CCSDATA *ccs = ( CCSDATA * ) lParam;
	if ( JGetWord( ccs->hContact, "Status", ID_STATUS_OFFLINE ) == ID_STATUS_OFFLINE )
		return 0;

	DBVARIANT dbv;
	if ( JGetStringT( ccs->hContact, "jid", &dbv ))
		return 0;

	char* *files = ( char* * ) ccs->lParam;
	int i, j;
	struct _stat statbuf;
	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal );
	if ( item == NULL ) {
		JFreeVariant( &dbv );
		return 0;
	}

	// Check if another file transfer session request is pending ( waiting for disco result )
	if ( item->ft != NULL ) {
		JFreeVariant( &dbv );
		return 0;
	}

	JabberCapsBits jcb = JabberGetResourceCapabilites( item->jid );

	// fix for very smart clients, like gajim
	if ( !JGetByte( "BsDirect", FALSE ) && !JGetByte( "BsProxyManual", FALSE ) ) {
		// disable bytestreams
		jcb &= ~JABBER_CAPS_BYTESTREAMS;
	}

	// if only JABBER_CAPS_SI_FT feature set (without BS or IBB), disable JABBER_CAPS_SI_FT
	if (( jcb & (JABBER_CAPS_SI_FT | JABBER_CAPS_IBB | JABBER_CAPS_BYTESTREAMS)) == JABBER_CAPS_SI_FT)
		jcb &= ~JABBER_CAPS_SI_FT;

	if (
		// can't get caps
		( jcb & JABBER_RESOURCE_CAPS_ERROR )
		// caps not already received
		|| ( jcb == JABBER_RESOURCE_CAPS_NONE )
		// XEP-0096 and OOB not supported?
		|| !(jcb & ( JABBER_CAPS_SI_FT | JABBER_CAPS_OOB ) )
		) {
		JFreeVariant( &dbv );
		return 0;
	}

	filetransfer* ft = new filetransfer;
	ft->std.hContact = ccs->hContact;
	while( files[ ft->std.totalFiles ] != NULL )
		ft->std.totalFiles++;

	ft->std.files = ( char** ) mir_alloc( sizeof( char* )* ft->std.totalFiles );
	ft->fileSize = ( long* ) mir_alloc( sizeof( long ) * ft->std.totalFiles );
	for( i=j=0; i < ft->std.totalFiles; i++ ) {
		if ( _stat( files[i], &statbuf ))
			JabberLog( "'%s' is an invalid filename", files[i] );
		else {
			ft->std.files[j] = mir_strdup( files[i] );
			ft->fileSize[j] = statbuf.st_size;
			j++;
			ft->std.totalBytes += statbuf.st_size;
	}	}

	ft->std.currentFile = mir_strdup( files[0] );
	ft->szDescription = mir_strdup(( char* )ccs->wParam );
	ft->jid = mir_tstrdup( dbv.ptszVal );
	JFreeVariant( &dbv );

	if ( jcb & JABBER_CAPS_SI_FT )
		JabberFtInitiate( item->jid, ft );
	else if ( jcb & JABBER_CAPS_OOB )
		mir_forkthread(( pThreadFunc )JabberFileServerThread, ft );

	return ( int )( HANDLE ) ft;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSendMessage - sends a message

int JabberGetEventTextChatStates( WPARAM wParam, LPARAM lParam )
{
	DBEVENTGETTEXT *pdbEvent = ( DBEVENTGETTEXT * )lParam;

	int nRetVal = 0;

	if ( pdbEvent->dbei->cbBlob > 0 ) {
		if ( pdbEvent->dbei->pBlob[0] == JABBER_DB_EVENT_CHATSTATES_GONE ) {
			if ( pdbEvent->datatype == DBVT_WCHAR )
				nRetVal = (int)mir_tstrdup(TranslateTS(_T("closed chat session")));
			else if ( pdbEvent->datatype == DBVT_ASCIIZ )
				nRetVal = (int)mir_strdup(Translate("closed chat session"));
		}
	}
	
	return nRetVal;
}

static void __cdecl JabberSendMessageAckThread( HANDLE hContact )
{
	SleepEx( 10, TRUE );
	JabberLog( "Broadcast ACK" );
	JSendBroadcast( hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, ( HANDLE ) 1, 0 );
	JabberLog( "Returning from thread" );
}

static char PGP_PROLOG[] = "-----BEGIN PGP MESSAGE-----\r\n\r\n";
static char PGP_EPILOG[] = "\r\n-----END PGP MESSAGE-----\r\n";

int JabberSendMessage( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;
	int id;

	DBVARIANT dbv;
	if ( !jabberOnline || JGetStringT( ccs->hContact, "jid", &dbv )) {
		JSendBroadcast( ccs->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, ( HANDLE ) 1, 0 );
		return 0;
	}

	char* pszSrc = ( char* )ccs->lParam, *msg;
	int  isEncrypted;

	char* pdest = strstr( pszSrc, PGP_PROLOG );//pdest-string+1 is index of first occurence
	if ( pdest != NULL ) {
		pdest = strstr( pszSrc, PGP_EPILOG );
		int result = ( pdest ) ? strlen( PGP_PROLOG ) : 0;

		char* tempstring = ( char* )alloca( strlen( pszSrc ));
		strncpy( tempstring, pszSrc+strlen(PGP_PROLOG), strlen(pszSrc)-strlen(PGP_EPILOG)-result );
		pszSrc = tempstring;
		isEncrypted = 1;
		ccs->wParam &= ~PREF_UNICODE;
	}
	else isEncrypted = 0;

	if ( ccs->wParam & PREF_UTF )
		msg = JabberUrlEncode( pszSrc );
	else if ( ccs->wParam & PREF_UNICODE )
		msg = JabberTextEncodeW(( wchar_t* )&pszSrc[ strlen( pszSrc )+1 ] );
	else
		msg = JabberTextEncode( pszSrc );

	int nSentMsgId = 0;

	if ( msg != NULL ) {
		char msgType[ 16 ];
		if ( JabberListExist( LIST_CHATROOM, dbv.ptszVal ) && _tcschr( dbv.ptszVal, '/' )==NULL )
			strcpy( msgType, "groupchat" );
		else
			strcpy( msgType, "chat" );

		XmlNode m( "message" ); m.addAttr( "type", msgType );
		if ( !isEncrypted ) {
			XmlNode* body = m.addChild( "body" );
			body->sendText = msg;
		}
		else {
			m.addChild( "body", "[This message is encrypted.]" );
			XmlNode* x = m.addChild( "x" ); x->addAttr( "xmlns", "jabber:x:encrypted" );
			x->sendText = msg;
		}

		TCHAR szClientJid[ 256 ];
		JabberGetClientJID( dbv.ptszVal, szClientJid, SIZEOF( szClientJid ));

		JABBER_RESOURCE_STATUS *r = JabberResourceInfoFromJID( szClientJid );
		if ( r )
			r->bMessageSessionActive = TRUE;

		JabberCapsBits jcb = JabberGetResourceCapabilites( szClientJid );

		if ( jcb & JABBER_RESOURCE_CAPS_ERROR )
			jcb = JABBER_RESOURCE_CAPS_NONE;

		if ( jcb & JABBER_CAPS_CHATSTATES )
			m.addChild( "active" )->addAttr( "xmlns", _T(JABBER_FEAT_CHATSTATES));

		if (
			// if message delivery check disabled by entity caps manager
			( jcb & JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY ) ||
			// if client knows nothing about delivery
			!( jcb & ( JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_RECEIPTS )) ||
			// if message sent to groupchat
			!strcmp( msgType, "groupchat" ) ||
			// if message delivery check disabled in settings
			!JGetByte( "MsgAck", FALSE ) || !JGetByte( ccs->hContact, "MsgAck", TRUE )) {
			if ( !strcmp( msgType, "groupchat" ))
				m.addAttr( "to", dbv.ptszVal );
			else {
				id = JabberSerialNext();
				m.addAttr( "to", szClientJid ); m.addAttrID( id );
			}

			jabberThreadInfo->send( m );
			mir_forkthread( JabberSendMessageAckThread, ccs->hContact );
			nSentMsgId = 1;
		}
		else {
			id = JabberSerialNext();
			m.addAttr( "to", szClientJid ); m.addAttrID( id );

			// message receipts XEP priority
			if ( jcb & JABBER_CAPS_MESSAGE_RECEIPTS ) {
				XmlNode* receiptRequest = m.addChild( "request" );
				receiptRequest->addAttr( "xmlns", JABBER_FEAT_MESSAGE_RECEIPTS );
			}
			else if ( jcb & JABBER_CAPS_MESSAGE_EVENTS ) {
				XmlNode* x = m.addChild( "x" ); x->addAttr( "xmlns", JABBER_FEAT_MESSAGE_EVENTS );
				x->addChild( "delivered" );
				x->addChild( "offline" );
			}
			else
				id = 1;

			jabberThreadInfo->send( m );
			nSentMsgId = id;
	}	}

	JFreeVariant( &dbv );
	return nSentMsgId;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetApparentMode - sets the visibility status

int JabberSetApparentMode( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *ccs = ( CCSDATA * ) lParam;

	if ( ccs->wParam!=0 && ccs->wParam!=ID_STATUS_ONLINE && ccs->wParam!=ID_STATUS_OFFLINE ) return 1;
	int oldMode = JGetWord( ccs->hContact, "ApparentMode", 0 );
	if (( int ) ccs->wParam == oldMode ) return 1;
	JSetWord( ccs->hContact, "ApparentMode", ( WORD )ccs->wParam );

	if ( !jabberOnline ) return 0;

	DBVARIANT dbv;
	if ( !JGetStringT( ccs->hContact, "jid", &dbv )) {
		TCHAR* jid = dbv.ptszVal;
		switch ( ccs->wParam ) {
		case ID_STATUS_ONLINE:
			if ( jabberStatus == ID_STATUS_INVISIBLE || oldMode == ID_STATUS_OFFLINE ) {
				XmlNode p( "presence" ); p.addAttr( "to", jid );
				jabberThreadInfo->send( p );
			}
			break;
		case ID_STATUS_OFFLINE:
			if ( jabberStatus != ID_STATUS_INVISIBLE || oldMode == ID_STATUS_ONLINE )
				JabberSendPresenceTo( ID_STATUS_INVISIBLE, jid, NULL );
			break;
		case 0:
			if ( oldMode == ID_STATUS_ONLINE && jabberStatus == ID_STATUS_INVISIBLE )
				JabberSendPresenceTo( ID_STATUS_INVISIBLE, jid, NULL );
			else if ( oldMode == ID_STATUS_OFFLINE && jabberStatus != ID_STATUS_INVISIBLE )
				JabberSendPresenceTo( jabberStatus, jid, NULL );
			break;
		}
		JFreeVariant( &dbv );
	}

	// TODO: update the zebra list ( jabber:iq:privacy )

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetAvatar - sets an avatar without UI

static int JabberSetAvatar( WPARAM wParam, LPARAM lParam )
{
	char* szFileName = ( char* )lParam;

	if ( jabberConnected )
	{	
		JabberUpdateVCardPhoto( szFileName );
		JabberSendPresence( jabberDesiredStatus, false );
	}
	else 
	{
		// FIXME OLD CODE: If avatar was changed during Jabber was offline. It should be store and send new vcard on online.

		int fileIn = open( szFileName, O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
		if ( fileIn == -1 )
			return 1;

		long  dwPngSize = filelength( fileIn );
		char* pResult = new char[ dwPngSize ];
		if ( pResult == NULL ) {
			close( fileIn );
			return 2;
		}

		read( fileIn, pResult, dwPngSize );
		close( fileIn );

		mir_sha1_byte_t digest[MIR_SHA1_HASH_SIZE];
		mir_sha1_ctx sha1ctx;
		mir_sha1_init( &sha1ctx );
		mir_sha1_append( &sha1ctx, (mir_sha1_byte_t*)pResult, dwPngSize );
		mir_sha1_finish( &sha1ctx, digest );

		char tFileName[ MAX_PATH ];
		JabberGetAvatarFileName( NULL, tFileName, MAX_PATH );
		DeleteFileA( tFileName );

		char buf[MIR_SHA1_HASH_SIZE*2+1];
		for ( int i=0; i<MIR_SHA1_HASH_SIZE; i++ )
			sprintf( buf+( i<<1 ), "%02x", digest[i] );

		JSetByte( "AvatarType", JabberGetPictureType( pResult ));
		JSetString( NULL, "AvatarSaved", buf );

		JabberGetAvatarFileName( NULL, tFileName, MAX_PATH );
		FILE* out = fopen( tFileName, "wb" );
		if ( out != NULL ) {
			fwrite( pResult, dwPngSize, 1, out );
			fclose( out );
		}
		delete pResult;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetAwayMsg - sets the away status message

int JabberSetAwayMsg( WPARAM wParam, LPARAM lParam )
{
	JabberLog( "SetAwayMsg called, wParam=%d lParam=%s", wParam, ( char* )lParam );

	EnterCriticalSection( &modeMsgMutex );

	char **szMsg;
	int desiredStatus = wParam;

	switch ( desiredStatus ) {
	case ID_STATUS_ONLINE:
		szMsg = &modeMsgs.szOnline;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		szMsg = &modeMsgs.szAway;
		desiredStatus = ID_STATUS_AWAY;
		break;
	case ID_STATUS_NA:
		szMsg = &modeMsgs.szNa;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		szMsg = &modeMsgs.szDnd;
		desiredStatus = ID_STATUS_DND;
		break;
	case ID_STATUS_FREECHAT:
		szMsg = &modeMsgs.szFreechat;
		break;
	default:
		LeaveCriticalSection( &modeMsgMutex );
		return 1;
	}

	char* newModeMsg = mir_strdup(( char* )lParam );

	if (( *szMsg==NULL && newModeMsg==NULL ) ||
		( *szMsg!=NULL && newModeMsg!=NULL && !strcmp( *szMsg, newModeMsg )) ) {
		// Message is the same, no update needed
		if ( newModeMsg != NULL ) mir_free( newModeMsg );
	}
	else {
		// Update with the new mode message
		if ( *szMsg != NULL ) mir_free( *szMsg );
		*szMsg = newModeMsg;
		// Send a presence update if needed
		if ( desiredStatus == jabberStatus ) {
			JabberSendPresence( jabberStatus, true );
	}	}

	LeaveCriticalSection( &modeMsgMutex );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetStatus - sets the protocol status

int JabberSetStatus( WPARAM wParam, LPARAM lParam )
{
	JabberLog( "PS_SETSTATUS( %d )", wParam );
	int desiredStatus = wParam;
	jabberDesiredStatus = desiredStatus;

 	if ( desiredStatus == ID_STATUS_OFFLINE ) {
		if ( jabberThreadInfo ) {
			jabberThreadInfo->send( "</stream:stream>" );
			jabberThreadInfo->close();
			jabberThreadInfo = NULL;
			if ( jabberConnected ) {
				jabberConnected = jabberOnline = FALSE;
				JabberUtilsRebuildStatusMenu();
			}
		}

		int oldStatus = jabberStatus;
		jabberStatus = jabberDesiredStatus = ID_STATUS_OFFLINE;
		JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
	}
	else if ( !jabberConnected && !( jabberStatus >= ID_STATUS_CONNECTING && jabberStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES )) {
		if ( jabberConnected )
			return 0;

		ThreadData* thread = new ThreadData( JABBER_SESSION_NORMAL );
		jabberDesiredStatus = desiredStatus;
		int oldStatus = jabberStatus;
		jabberStatus = ID_STATUS_CONNECTING;
		JSendBroadcast( NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, ( HANDLE ) oldStatus, jabberStatus );
		thread->hThread = ( HANDLE ) mir_forkthread(( pThreadFunc )JabberServerThread, thread );
	}
	else JabberSetServerStatus( desiredStatus );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberUserIsTyping - sends a UTN notification

int JabberUserIsTyping( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline ) return 0;

	HANDLE hContact = ( HANDLE ) wParam;
	DBVARIANT dbv;
	if ( JGetStringT( hContact, "jid", &dbv )) return 0;

	JABBER_LIST_ITEM *item;
	if (( item = JabberListGetItemPtr( LIST_ROSTER, dbv.ptszVal )) != NULL ) {
		TCHAR szClientJid[ 256 ];
		JabberGetClientJID( dbv.ptszVal, szClientJid, SIZEOF( szClientJid ));

		JabberCapsBits jcb = JabberGetResourceCapabilites( szClientJid );

		if ( jcb & JABBER_RESOURCE_CAPS_ERROR )
			jcb = JABBER_RESOURCE_CAPS_NONE;

		XmlNode m( "message" ); m.addAttr( "to", szClientJid );

		if ( jcb & JABBER_CAPS_CHATSTATES ) {
			m.addAttr( "type", "chat" );
			m.addAttrID( JabberSerialNext());
			switch ( lParam ){
			case PROTOTYPE_SELFTYPING_OFF:
				m.addChild( "paused" )->addAttr( "xmlns", _T(JABBER_FEAT_CHATSTATES));
				jabberThreadInfo->send( m );
				break;
			case PROTOTYPE_SELFTYPING_ON:
				m.addChild( "composing" )->addAttr( "xmlns", _T(JABBER_FEAT_CHATSTATES));
				jabberThreadInfo->send( m );
				break;
			}
		}
		else if ( jcb & JABBER_CAPS_MESSAGE_EVENTS ) {
			XmlNode* x = m.addChild( "x" ); x->addAttr( "xmlns", JABBER_FEAT_MESSAGE_EVENTS );
			if ( item->messageEventIdStr != NULL )
				x->addChild( "id", item->messageEventIdStr );

			switch ( lParam ){
			case PROTOTYPE_SELFTYPING_OFF:
				jabberThreadInfo->send( m );
				break;
			case PROTOTYPE_SELFTYPING_ON:
				x->addChild( "composing" );
				jabberThreadInfo->send( m );
				break;
	}	}	}

	JFreeVariant( &dbv );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// "/SendXML" - Allows external plugins to send XML to the server

int ServiceSendXML(WPARAM wParam, LPARAM lParam)
{
	return jabberThreadInfo->send( (char*)lParam);
}

int JabberGCGetToolTipText(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
		return 0;

	if (!lParam)
		return 0; //room global tooltip not supported yet

	JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_CHATROOM, (TCHAR*)wParam);
	if ( item == NULL )	return 0;  //no room found

	JABBER_RESOURCE_STATUS * info = NULL;
	for ( int i=0; i < item->resourceCount; i++ ) {
		JABBER_RESOURCE_STATUS& p = item->resource[i];
		if ( !lstrcmp( p.resourceName, (TCHAR*)lParam )) {
			info = &p;
			break;
		}	}
	if ( info==NULL )
		return 0; //no info found

	// ok process info output will be:
	// JID:			real@jid/resource or
	// Nick:		Nickname
	// Status:		StatusText
	// Role:		Moderaror
	// Affiliation:  Affiliation

	TCHAR outBuf[2048];
	outBuf[0]=_T('\0');

	TCHAR * szSeparator= (IsWinVerMEPlus()) ? _T("\r\n") : _T(" | ");

	static const TCHAR * JabberEnum2AffilationStr[]={ _T("None"), _T("Outcast"), _T("Member"), _T("Admin"), _T("Owner") };

	static const TCHAR * JabberEnum2RoleStr[]={ _T("None"), _T("Visitor"), _T("Participant"), _T("Moderator") };

	//FIXME Table conversion fast but is not safe
	static const TCHAR * JabberEnum2StatusStr[]= {	_T("Offline"), _T("Online"), _T("Away"), _T("DND"),
		_T("NA"), _T("Occupied"), _T("Free for chat"),
		_T("Invisible"), _T("On the phone"), _T("Out to lunch"),
		_T("Idle")  };


	//JID:
	if ( _tcschr(info->resourceName,_T('@') != NULL ) ) {
		_tcsncat( outBuf, TranslateT("JID:\t\t"), sizeof(outBuf) );
		_tcsncat( outBuf, info->resourceName, sizeof(outBuf) );
	} else if (lParam) { //or simple nick
		_tcsncat( outBuf, TranslateT("Nick:\t\t"), sizeof(outBuf) );
		_tcsncat( outBuf, (TCHAR*) lParam, sizeof(outBuf) );
	}

	// status
	if ( info->status >= ID_STATUS_OFFLINE && info->status <= ID_STATUS_IDLE  ) {
		_tcsncat( outBuf, szSeparator, sizeof(outBuf) );
		_tcsncat( outBuf, TranslateT("Status:\t\t"), sizeof(outBuf) );
		_tcsncat( outBuf, TranslateTS( JabberEnum2StatusStr [ info->status-ID_STATUS_OFFLINE ]), sizeof(outBuf) );
	}

	// status text
	if ( info->statusMessage ) {
		_tcsncat( outBuf, szSeparator, sizeof(outBuf) );
		_tcsncat( outBuf, TranslateT("Status text:\t"), sizeof(outBuf) );
		_tcsncat( outBuf, info->statusMessage, sizeof(outBuf) );
	}

	// Role
	if ( TRUE || info->role ) {
		_tcsncat( outBuf, szSeparator, sizeof(outBuf) );
		_tcsncat( outBuf, TranslateT("Role:\t\t"), sizeof(outBuf) );
		_tcsncat( outBuf, TranslateTS( JabberEnum2RoleStr[info->role] ), sizeof(outBuf) );
	}

	// Affiliation
	if ( TRUE || info->affiliation ) {
		_tcsncat( outBuf, szSeparator, sizeof(outBuf) );
		_tcsncat( outBuf, TranslateT("Affiliation:\t"), sizeof(outBuf) );
		_tcsncat( outBuf, TranslateTS( JabberEnum2AffilationStr[info->affiliation] ), sizeof(outBuf) );
	}

	// real jid
	if ( info->szRealJid ) {
		_tcsncat( outBuf, szSeparator, sizeof(outBuf) );
		_tcsncat( outBuf, TranslateT("Real JID:\t"), sizeof(outBuf) );
		_tcsncat( outBuf, info->szRealJid, sizeof(outBuf) );
	}

	if ( lstrlen( outBuf ) == 0)
		return 0;

	return (int) mir_tstrdup( outBuf );
}

// File Association Manager plugin support
static int JabberServiceParseXmppURI( WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER( wParam );

	TCHAR *arg = ( TCHAR * )lParam;
	if ( arg == NULL )
		return 1;

	TCHAR szUri[ 1024 ];
	mir_sntprintf( szUri, SIZEOF(szUri), _T("%s"), arg );

	TCHAR *szJid = szUri;

	// skip leading prefix
	szJid = _tcschr( szJid, _T( ':' ));
	if ( szJid == NULL )
		return 1;

	// skip //
	for( ++szJid; *szJid == _T( '/' ); ++szJid );

	// empty jid?
	if ( !*szJid )
		return 1;

	// command code
	TCHAR *szCommand = szJid;
	szCommand = _tcschr( szCommand, _T( '?' ));
	if ( szCommand ) 
		*( szCommand++ ) = 0;

	// parameters
	TCHAR *szSecondParam = szCommand ? _tcschr( szCommand, _T( ';' )) : NULL;
	if ( szSecondParam )
		*( szSecondParam++ ) = 0;

//	TCHAR *szThirdParam = szSecondParam ? _tcschr( szSecondParam, _T( ';' )) : NULL;
//	if ( szThirdParam )
//		*( szThirdParam++ ) = 0;

	// no command or message command
	if ( !szCommand || ( szCommand && !_tcsicmp( szCommand, _T( "message" )))) {
		// message
		if ( ServiceExists( MS_MSG_SENDMESSAGE )) {
			HANDLE hContact = JabberHContactFromJID( szJid, TRUE );
			if ( !hContact )
				hContact = JabberDBCreateContact( szJid, szJid, TRUE, TRUE );
			if ( !hContact )
				return 1;
			CallService( MS_MSG_SENDMESSAGE, (WPARAM)hContact, (LPARAM)NULL );
			return 0;
		}
		return 1;
	}
	else if ( !_tcsicmp( szCommand, _T( "roster" )))
	{
		if ( !JabberHContactFromJID( szJid )) {
			JABBER_SEARCH_RESULT jsr = { 0 };
			jsr.hdr.cbSize = sizeof( JABBER_SEARCH_RESULT );
			jsr.hdr.nick = mir_t2a( szJid );
			_tcsncpy( jsr.jid, szJid, SIZEOF(jsr.jid) - 1 );

			ADDCONTACTSTRUCT acs;
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = jabberProtoName;
			acs.psr = &jsr.hdr;
			CallService( MS_ADDCONTACT_SHOW, (WPARAM)NULL, (LPARAM)&acs );
			mir_free( jsr.hdr.email );
		}
		return 0;
	}
	else if ( !_tcsicmp( szCommand, _T( "join" )))
	{
		// chat join invitation
		JabberGroupchatJoinRoomByJid( NULL, szJid );
		return 0;
	}
	else if ( !_tcsicmp( szCommand, _T( "disco" )))
	{
		// service discovery request
		JabberMenuHandleServiceDiscovery( NULL, (LPARAM)szJid );
		return 0;
	}
	else if ( !_tcsicmp( szCommand, _T( "command" )))
	{
		// ad-hoc commands
		if ( szSecondParam ) {
			if ( !_tcsnicmp( szSecondParam, _T( "node=" ), 5 )) {
				szSecondParam += 5;
				if (!*szSecondParam)
					szSecondParam = NULL;
			}
			else
				szSecondParam = NULL;
		}
		CJabberAdhocStartupParams* pStartupParams = new CJabberAdhocStartupParams( szJid, szSecondParam );
		JabberContactMenuRunCommands( 0, ( LPARAM )pStartupParams );
		return 0;
	}
	else if ( !_tcsicmp( szCommand, _T( "sendfile" )))
	{
		// send file
		if ( ServiceExists( MS_FILE_SENDFILE )) {
			HANDLE hContact = JabberHContactFromJID( szJid, TRUE );
			if ( !hContact )
				hContact = JabberDBCreateContact( szJid, szJid, TRUE, TRUE );
			if ( !hContact )
				return 1;
			CallService( MS_FILE_SENDFILE, ( WPARAM )hContact, ( LPARAM )NULL );
			return 0;
		}
		return 1;
	}

	return 1; /* parse failed */
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service initialization code

int JabberSvcInit( void )
{
	arServices.insert( JCreateServiceFunction( PS_GETCAPS, JabberGetCaps ));
	arServices.insert( JCreateServiceFunction( PS_GETNAME, JabberGetName ));
	arServices.insert( JCreateServiceFunction( PS_LOADICON, JabberLoadIcon ));
	arServices.insert( JCreateServiceFunction( PS_BASICSEARCH, JabberBasicSearch ));
	arServices.insert( JCreateServiceFunction( PS_SEARCHBYEMAIL, JabberSearchByEmail ));
	arServices.insert( JCreateServiceFunction( PS_SEARCHBYNAME, JabberSearchByName ));
	arServices.insert( JCreateServiceFunction( PS_ADDTOLIST, JabberAddToList ));
	arServices.insert( JCreateServiceFunction( PS_ADDTOLISTBYEVENT, JabberAddToListByEvent ));
	arServices.insert( JCreateServiceFunction( PS_AUTHALLOW, JabberAuthAllow ));
	arServices.insert( JCreateServiceFunction( PS_AUTHDENY, JabberAuthDeny ));
	arServices.insert( JCreateServiceFunction( PS_SETSTATUS, JabberSetStatus ));
	arServices.insert( JCreateServiceFunction( PS_GETAVATARINFO, JabberGetAvatarInfo ));
	arServices.insert( JCreateServiceFunction( PS_GETSTATUS, JabberGetStatus ));
	arServices.insert( JCreateServiceFunction( PS_SETAWAYMSG, JabberSetAwayMsg ));
	arServices.insert( JCreateServiceFunction( PS_FILERESUME, JabberFileResume ));

	arServices.insert( JCreateServiceFunction( PS_SET_LISTENINGTO, JabberSetListeningTo ));

	arServices.insert( JCreateServiceFunction( PSS_GETINFO, JabberGetInfo ));
	arServices.insert( JCreateServiceFunction( PSS_SETAPPARENTMODE, JabberSetApparentMode ));
	arServices.insert( JCreateServiceFunction( PSS_MESSAGE, JabberSendMessage ));
	arServices.insert( JCreateServiceFunction( PSS_GETAWAYMSG, JabberGetAwayMsg ));
	arServices.insert( JCreateServiceFunction( PSS_FILEALLOW, JabberFileAllow ));
	arServices.insert( JCreateServiceFunction( PSS_FILECANCEL, JabberFileCancel ));
	arServices.insert( JCreateServiceFunction( PSS_FILEDENY, JabberFileDeny ));
	arServices.insert( JCreateServiceFunction( PSS_FILE, JabberSendFile ));
	arServices.insert( JCreateServiceFunction( PSR_MESSAGE, JabberRecvMessage ));
	arServices.insert( JCreateServiceFunction( PSR_FILE, JabberRecvFile ));
	arServices.insert( JCreateServiceFunction( PSS_USERISTYPING, JabberUserIsTyping ));

	//JEP-055 aware CUSTOM SEARCHING (jabber_search.h)
	arServices.insert( JCreateServiceFunction( PS_CREATEADVSEARCHUI, JabberSearchCreateAdvUI ));
	arServices.insert( JCreateServiceFunction( PS_SEARCHBYADVANCED, JabberSearchByAdvanced ));

	// Protocol services and events...
	heventRawXMLIn = JCreateHookableEvent( JE_RAWXMLIN );
	heventRawXMLOut = JCreateHookableEvent( JE_RAWXMLOUT );
	heventXStatusIconChanged = JCreateHookableEvent( JE_CUSTOMSTATUS_EXTRAICON_CHANGED );
	heventXStatusChanged = JCreateHookableEvent( JE_CUSTOMSTATUS_CHANGED );
	arServices.insert( JCreateServiceFunction( JS_GETCUSTOMSTATUSICON, JabberGetXStatusIcon ));
	arServices.insert( JCreateServiceFunction( JS_GETXSTATUS, JabberGetXStatus ));
	arServices.insert( JCreateServiceFunction( JS_SETXSTATUS, JabberSetXStatus ));

	arServices.insert( JCreateServiceFunction( JS_SENDXML, ServiceSendXML ));
	arServices.insert( JCreateServiceFunction( PS_GETMYAVATAR, JabberGetAvatar ));
	arServices.insert( JCreateServiceFunction( PS_GETAVATARCAPS, JabberGetAvatarCaps ));
	arServices.insert( JCreateServiceFunction( PS_SETMYAVATAR, JabberSetAvatar ));

	// service to get from protocol chat buddy info
	arServices.insert( JCreateServiceFunction( MS_GC_PROTO_GETTOOLTIPTEXT, JabberGCGetToolTipText ));

	// XMPP URI parser service for "File Association Manager" plugin
	arServices.insert( JCreateServiceFunction( JS_PARSE_XMPP_URI, JabberServiceParseXmppURI ));

	return 0;
}

int JabberSvcUninit()
{
	for ( int i=0; i < arServices.getCount(); i++ )
		DestroyServiceFunction( arServices[i] );
	arServices.destroy();

	DestroyHookableEvent( heventRawXMLIn );
	DestroyHookableEvent( heventRawXMLOut );
	DestroyHookableEvent( heventXStatusIconChanged );
	DestroyHookableEvent( heventXStatusChanged );

	return 0;
}

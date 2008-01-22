/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan

Idea & portions of code by Artem Shpynov

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
Last change on : $Date: 2006-07-13 16:11:29 +0400
Last change by : $Author$

*/

#include "jabber.h"
#include "jabber_list.h"

#include <commctrl.h>
#include "m_icolib.h"

#include "resource.h"

#define IDI_ONLINE                      104
#define IDI_OFFLINE                     105
#define IDI_AWAY                        128
#define IDI_FREE4CHAT                   129
#define IDI_INVISIBLE                   130
#define IDI_NA                          131
#define IDI_DND                         158
#define IDI_OCCUPIED                    159
#define IDI_ONTHEPHONE                  1002
#define IDI_OUTTOLUNCH                  1003

HIMAGELIST hAdvancedStatusIcon = NULL;
struct
{
	TCHAR* mask;
	char*  proto;
	int    startIndex;
}
static TransportProtoTable[] =
{
	{ _T("|icq*|jit*"),      "ICQ",           -1},
	{ _T("msn*"),            "MSN",           -1},
	{ _T("yahoo*"),          "YAHOO",         -1},
	{ _T("mrim*"),           "MRA",           -1},
	{ _T("aim*"),            "AIM",           -1},
	//request #3094
	{ _T("|gg*|gadu*"),      "GaduGadu",      -1},
	{ _T("tv*"),             "TV",            -1},
	{ _T("dict*"),           "Dictionary",    -1},
	{ _T("weather*"),        "Weather",       -1},
	{ _T("sms*"),            "SMS",           -1},
	{ _T("smtp*"),           "SMTP",          -1},
	//j2j
	{ _T("gtalk.*.*"),       "GTalk",         -1},
	{ _T("xmpp.*.*"),        "Jabber2Jabber", -1},
	//jabbim.cz - services
	{ _T("disk*"),           "Jabber Disk",   -1},
	{ _T("irc*"),            "IRC",           -1},
	{ _T("rss*"),            "RSS",           -1},
	{ _T("tlen*"),           "Tlen",          -1}
};

static int skinIconStatusToResourceId[] = {IDI_OFFLINE,IDI_ONLINE,IDI_AWAY,IDI_DND,IDI_NA,IDI_NA,/*IDI_OCCUPIED,*/IDI_FREE4CHAT,IDI_INVISIBLE,IDI_ONTHEPHONE,IDI_OUTTOLUNCH};
static int skinStatusToJabberStatus[] = {0,1,2,3,4,4,6,7,2,2};

/////////////////////////////////////////////////////////////////////////////////////////
// Icons init

struct
{
	char*  szDescr;
	char*  szName;
	int    defIconID;
	char*  szSection;
	HANDLE hIconLibItem;
}
static iconList[] =
{
	{   LPGEN("Protocol icon"),         "main",             IDI_JABBER            },
	{   LPGEN("Agents list"),           "Agents",           IDI_AGENTS            },
	{   LPGEN("Transports"),            "transport",        IDI_TRANSPORT         },
	{   LPGEN("Registered transports"), "transport_loc",    IDI_TRANSPORTL        },
	{   LPGEN("Change password"),       "key",              IDI_KEYS              },
	{   LPGEN("Multi-User Conference"), "group",            IDI_GROUP             },
	{   LPGEN("Personal vCard"),        "vcard",            IDI_VCARD             },
	{   LPGEN("Request authorization"), "Request",          IDI_REQUEST           },
	{   LPGEN("Grant authorization"),   "Grant",            IDI_GRANT             },
	{   LPGEN("Revoke authorization"),  "Revoke",           IDI_AUTHREVOKE        },
	{   LPGEN("Convert to room"),       "convert",          IDI_USER2ROOM         },
	{   LPGEN("Add to roster"),         "addroster",        IDI_ADDROSTER         },
	{   LPGEN("Login/logout"),          "trlogonoff",       IDI_LOGIN             },
	{   LPGEN("Resolve nicks"),         "trresolve",        IDI_REFRESH           },
	{   LPGEN("Bookmarks"),             "bookmarks",        IDI_BOOKMARKS         }, 
	{   LPGEN("Privacy Lists"),         "privacylists",     IDI_PRIVACY_LISTS     },
	{   LPGEN("Service Discovery"),     "servicediscovery", IDI_SERVICE_DISCOVERY },
	{   LPGEN("AdHoc Command"),         "adhoc",            IDI_COMMAND           },
	{   LPGEN("XML Console"),           "xmlconsole",       IDI_CONSOLE           },

	{   LPGEN("Discovery succeeded"),   "disco_ok",         IDI_DISCO_OK,           LPGEN("Dialogs") },
	{   LPGEN("Discovery failed"),      "disco_fail",       IDI_DISCO_FAIL,         LPGEN("Dialogs") },
	{   LPGEN("Discovery in progress"), "disco_progress",   IDI_DISCO_PROGRESS,     LPGEN("Dialogs") },
	{   LPGEN("View as tree"),          "sd_view_tree",     IDI_VIEW_TREE,          LPGEN("Dialogs") },
	{   LPGEN("View as list"),          "sd_view_list",     IDI_VIEW_LIST,          LPGEN("Dialogs") },
	{   LPGEN("Apply filter"),          "sd_filter_apply",  IDI_FILTER_APPLY,       LPGEN("Dialogs") },
	{   LPGEN("Reset filter"),          "sd_filter_reset",  IDI_FILTER_RESET,       LPGEN("Dialogs") },

	{   LPGEN("Navigate home"),         "sd_nav_home",      IDI_NAV_HOME,           LPGEN("Dialogs/Discovery") },
	{   LPGEN("Refresh node"),          "sd_nav_refresh",   IDI_NAV_REFRESH,        LPGEN("Dialogs/Discovery") },
	{   LPGEN("Browse node"),           "sd_browse",        IDI_BROWSE,             LPGEN("Dialogs/Discovery") },
	{   LPGEN("RSS service"),           "node_rss",         IDI_NODE_RSS,           LPGEN("Dialogs/Discovery") },
	{   LPGEN("Server"),                "node_server",      IDI_NODE_SERVER,        LPGEN("Dialogs/Discovery") },
	{   LPGEN("Storage service"),       "node_store",       IDI_NODE_STORE,         LPGEN("Dialogs/Discovery") },
	{   LPGEN("Weather service"),       "node_weather",     IDI_NODE_WEATHER,       LPGEN("Dialogs/Discovery") },

	{   LPGEN("Generic privacy list"),  "pl_list_any",      IDI_PL_LIST_ANY,        LPGEN("Dialogs/Privacy") },
	{   LPGEN("Active privacy list"),   "pl_list_active",   IDI_PL_LIST_ACTIVE,     LPGEN("Dialogs/Privacy") },
	{   LPGEN("Default privacy list"),  "pl_list_default",  IDI_PL_LIST_DEFAULT,    LPGEN("Dialogs/Privacy") },
	{   LPGEN("Move up"),				"arrow_up",			IDI_ARROW_UP,			LPGEN("Dialogs/Privacy") },
	{   LPGEN("Move down"),				"arrow_down",		IDI_ARROW_DOWN,			LPGEN("Dialogs/Privacy") },
	{   LPGEN("Allow Messages"),        "pl_msg_allow",     IDI_PL_MSG_ALLOW,       LPGEN("Dialogs/Privacy") },
	{   LPGEN("Allow Presences (in)"),  "pl_prin_allow",    IDI_PL_PRIN_ALLOW,      LPGEN("Dialogs/Privacy") },
	{   LPGEN("Allow Presences (out)"), "pl_prout_allow",   IDI_PL_PROUT_ALLOW,     LPGEN("Dialogs/Privacy") },
	{   LPGEN("Allow Queries"),         "pl_iq_allow",      IDI_PL_QUERY_ALLOW,     LPGEN("Dialogs/Privacy") },
	{   LPGEN("Deny Messages"),         "pl_msg_deny",      IDI_PL_MSG_DENY,        LPGEN("Dialogs/Privacy") },
	{   LPGEN("Deny Presences (in)"),   "pl_prin_deny",     IDI_PL_PRIN_DENY,       LPGEN("Dialogs/Privacy") },
	{   LPGEN("Deny Presences (out)"),  "pl_prout_deny",    IDI_PL_PROUT_DENY,      LPGEN("Dialogs/Privacy") },
	{   LPGEN("Deny Queries"),          "pl_iq_deny",       IDI_PL_QUERY_DENY,      LPGEN("Dialogs/Privacy") },
};

void CJabberProto::JabberIconsInit( void )
{
	SKINICONDESC sid = {0};
	char szFile[MAX_PATH];
	GetModuleFileNameA(hInst, szFile, MAX_PATH);

	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszDefaultFile = szFile;
	sid.cx = sid.cy = 16;

	char *szRootSection = Translate( szProtoName );

	for ( int i = 0; i < SIZEOF(iconList); i++ ) {
		char szSettingName[100];
		char szSectionName[100];
		mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", szProtoName, iconList[i].szName );
		if ( iconList[i].szSection ) {
			mir_snprintf( szSectionName, sizeof( szSectionName ), "%s/%s", szRootSection, iconList[i].szSection );
			sid.pszSection = szSectionName;
		}
		else sid.pszSection = szRootSection;

		sid.pszName = szSettingName;
		sid.pszDescription = Translate( iconList[i].szDescr );
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = ( HANDLE )CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
}	}

HANDLE CJabberProto::GetIconHandle( int iconId )
{
	for ( int i=0; i < SIZEOF(iconList); i++ )
		if ( iconList[i].defIconID == iconId )
			return iconList[i].hIconLibItem;

	return NULL;
}

HICON CJabberProto::LoadIconEx( const char* name )
{
	char szSettingName[100];
	mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", szProtoName, name );
	return ( HICON )JCallService( MS_SKIN2_GETICON, 0, (LPARAM)szSettingName );
}

/////////////////////////////////////////////////////////////////////////////////////////
// internal functions

static inline TCHAR qtoupper( TCHAR c )
{
	return ( c >= 'a' && c <= 'z' ) ? c - 'a' + 'A' : c;
}

static BOOL WildComparei( const TCHAR* name, const TCHAR* mask )
{
	const TCHAR* last='\0';
	for ( ;; mask++, name++) {
		if ( *mask != '?' && qtoupper( *mask ) != qtoupper( *name ))
			break;
		if ( *name == '\0' )
			return ((BOOL)!*mask);
	}

	if ( *mask != '*' )
		return FALSE;

	for (;; mask++, name++ ) {
		while( *mask == '*' ) {
			last = mask++;
			if ( *mask == '\0' )
				return ((BOOL)!*mask);   /* true */
		}

		if ( *name == '\0' )
			return ((BOOL)!*mask);      /* *mask == EOS */
		if ( *mask != '?' && qtoupper( *mask ) != qtoupper( *name ))
			name -= (size_t)(mask - last) - 1, mask = last;
}	}

static BOOL MatchMask( const TCHAR* name, const TCHAR* mask)
{
	if ( !mask || !name )
		return mask == name;

	if ( *mask != '|' )
		return WildComparei( name, mask );

	TCHAR* temp = NEWTSTR_ALLOCA(mask);
	for ( int e=1; mask[e] != '\0'; e++ ) {
		int s = e;
		while ( mask[e] != '\0' && mask[e] != '|')
			e++;

		temp[e]= _T('\0');
		if ( WildComparei( name, temp+s ))
			return TRUE;

		if ( mask[e] == 0 )
			return FALSE;
	}

	return FALSE;
}

static HICON ExtractIconFromPath(const char *path, BOOL * needFree)
{
	char *comma;
	char file[MAX_PATH],fileFull[MAX_PATH];
	int n;
	HICON hIcon;
	lstrcpynA(file,path,sizeof(file));
	comma=strrchr(file,',');
	if(comma==NULL) n=0;
	else {n=atoi(comma+1); *comma=0;}
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)file, (LPARAM)fileFull);
	hIcon=NULL;
	ExtractIconExA(fileFull,n,NULL,&hIcon,1);
	if (needFree)
		*needFree=(hIcon!=NULL);

	return hIcon;
}

static HICON LoadTransportIcon(char *filename,int i,char *IconName,char *SectName,char *Description,int internalidx, BOOL * needFree)
{
	char szPath[MAX_PATH],szMyPath[MAX_PATH], szFullPath[MAX_PATH],*str;
	HICON hIcon=NULL;
	BOOL has_proto_icon=FALSE;
	SKINICONDESC sid={0};
	if (needFree) *needFree=FALSE;
	GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
	str=strrchr(szPath,'\\');
	if(str!=NULL) *str=0;
	_snprintf(szMyPath, sizeof(szMyPath), "%s\\Icons\\%s", szPath, filename);
	_snprintf(szFullPath, sizeof(szFullPath), "%s\\Icons\\%s,%d", szPath, filename, i);
	BOOL nf;
	HICON hi=ExtractIconFromPath(szFullPath,&nf);
	if (hi) has_proto_icon=TRUE;
	if (hi && nf) DestroyIcon(hi);
	if (!ServiceExists(MS_SKIN2_ADDICON)) {
		hIcon=ExtractIconFromPath(szFullPath,needFree);
		if (hIcon) return hIcon;
		_snprintf(szFullPath, sizeof(szFullPath), "%s,%d", szMyPath, internalidx);
		hIcon=ExtractIconFromPath(szFullPath,needFree);
		if (hIcon) return hIcon;
	}
	else {
		if ( IconName != NULL && SectName != NULL)  {
			sid.cbSize = sizeof(sid);
			sid.cx=16;
			sid.cy=16;
			sid.hDefaultIcon = (has_proto_icon)?NULL:(HICON)CallService(MS_SKIN_LOADPROTOICON,(WPARAM)NULL,(LPARAM)(-internalidx));
			sid.pszSection = Translate(SectName);
			sid.pszName=IconName;
			sid.pszDescription=Description;
			sid.pszDefaultFile=szMyPath;
			sid.iDefaultIndex=i;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}
		return ((HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IconName));
	}
	return NULL;
}

static HICON LoadSmallIcon(HINSTANCE hInstance, LPCTSTR lpIconName)
{
	HICON hIcon=NULL;                 // icon handle
	int index=-(int)lpIconName;
	TCHAR filename[MAX_PATH]={0};
	GetModuleFileName(hInstance,filename,MAX_PATH);
	ExtractIconEx(filename,index,NULL,&hIcon,1);
	return hIcon;
}

int CJabberProto::LoadAdvancedIcons(int iID)
{
	int i;
	char * proto=TransportProtoTable[iID].proto;
	char * defFile[MAX_PATH]={0};
	char * Group[255];
	char * Uname[255];
	int first=-1;
	HICON empty=LoadSmallIcon(NULL,MAKEINTRESOURCE(102));

	_snprintf((char *)Group, sizeof(Group),"%s/%s/%s %s",Translate("Status Icons"), szModuleName, proto, Translate("transport"));
	_snprintf((char *)defFile, sizeof(defFile),"proto_%s.dll",proto);
	if (!hAdvancedStatusIcon)
		hAdvancedStatusIcon=(HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST,0,0);

	EnterCriticalSection( &modeMsgMutex );
	for (i=0; i<ID_STATUS_ONTHEPHONE-ID_STATUS_OFFLINE; i++) {
		HICON hicon;
		BOOL needFree;
		int n=skinStatusToJabberStatus[i];
		char * descr=(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,n+ID_STATUS_OFFLINE,0);
		_snprintf((char *)Uname, sizeof(Uname),"%s_Transport_%s_%d",szProtoName,proto,n);
		hicon=(HICON)LoadTransportIcon((char*)defFile,-skinIconStatusToResourceId[i],(char*)Uname,(char*)Group,(char*)descr,-(n+ID_STATUS_OFFLINE),&needFree);
		int index=(TransportProtoTable[iID].startIndex == -1)?-1:TransportProtoTable[iID].startIndex+n;
		int added=ImageList_ReplaceIcon(hAdvancedStatusIcon,index,hicon?hicon:empty);
		if (first == -1) first=added;
		if (hicon && needFree) DestroyIcon(hicon);
	}

	if ( TransportProtoTable[ iID ].startIndex == -1 )
		TransportProtoTable[ iID ].startIndex = first;
	LeaveCriticalSection( &modeMsgMutex );
	return 0;
}

static int GetTransportProtoID( TCHAR* TransportDomain )
{
	for ( int i=0; i<SIZEOF(TransportProtoTable); i++ )
		if ( MatchMask( TransportDomain, TransportProtoTable[i].mask ))
			return i;

	return -1;
}

int CJabberProto::GetTransportStatusIconIndex(int iID, int Status)
{
	if ( iID < 0 || iID >= SIZEOF( TransportProtoTable ))
		return -1;

	//icons not loaded - loading icons
	if ( TransportProtoTable[iID].startIndex == -1 )
		LoadAdvancedIcons( iID );

	//some fault on loading icons
	if ( TransportProtoTable[ iID ].startIndex == -1 )
		return -1;

	if ( Status < ID_STATUS_OFFLINE )
		Status = ID_STATUS_OFFLINE;

	return TransportProtoTable[iID].startIndex + skinStatusToJabberStatus[ Status - ID_STATUS_OFFLINE ];
}

/////////////////////////////////////////////////////////////////////////////////////////
// a hook for the IcoLib plugin

int CJabberProto::OnReloadIcons(WPARAM wParam, LPARAM lParam)
{
	for ( int i=0; i < SIZEOF(TransportProtoTable); i++ )
		if ( TransportProtoTable[i].startIndex != -1 )
			LoadAdvancedIcons(i);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Prototype for Jabber and other protocols to return index of Advanced status
// wParam - HCONTACT of called protocol
// lParam - should be 0 (reserverd for futher usage)
// return value: -1 - no Advanced status
// : other - index of icons in clcimagelist.
// if imagelist require advanced painting status overlay(like xStatus)
// index should be shifted to HIWORD, LOWORD should be 0

int __cdecl CJabberProto::JGetAdvancedStatusIcon(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact=(HANDLE) wParam;
	if ( !hContact )
		return -1;

	if ( !JGetByte( hContact, "IsTransported", 0 ))
		return -1;

	DBVARIANT dbv;
	if ( JGetStringT( hContact, "Transport", &dbv ))
		return -1;

	int iID = GetTransportProtoID( dbv.ptszVal );
	DBFreeVariant(&dbv);
	if ( iID >= 0 ) {
		WORD Status = ID_STATUS_OFFLINE;
		Status = JGetWord( hContact, "Status", ID_STATUS_OFFLINE );
		if ( Status < ID_STATUS_OFFLINE )
			Status = ID_STATUS_OFFLINE;
		else if (Status > ID_STATUS_INVISIBLE )
			Status = ID_STATUS_ONLINE;
		return GetTransportStatusIconIndex( iID, Status );
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   Transport check functions

BOOL CJabberProto::JabberDBCheckIsTransportedContact(const TCHAR* jid, HANDLE hContact)
{
	// check if transport is already set
	if ( !jid || !hContact )
		return FALSE;

	// strip domain part from jid
	TCHAR* domain  = _tcschr(( TCHAR* )jid, '@' );
	BOOL   isAgent = (domain == NULL) ? TRUE : FALSE;
	BOOL   isTransported = FALSE;
	if ( domain!=NULL )
		domain = NEWTSTR_ALLOCA(domain+1);
	else
		domain = NEWTSTR_ALLOCA(jid);

	TCHAR* resourcepos = _tcschr( domain, '/' );
	if ( resourcepos != NULL )
		*resourcepos = '\0';

	for ( int i=0; i < SIZEOF(TransportProtoTable); i++ ) {
		if ( MatchMask( domain, TransportProtoTable[i].mask )) {
			GetTransportStatusIconIndex( GetTransportProtoID( domain ), ID_STATUS_OFFLINE );
			isTransported = TRUE;
			break;
	}	}

	if ( jabberTransports.getIndex( domain ) == -1 ) {
		if ( isAgent ) {
			jabberTransports.insert( _tcsdup(domain) ); 
			JSetByte( hContact, "IsTransport", 1 );
	}	}

	if ( isTransported ) {
		JSetStringT( hContact, "Transport", domain );
		JSetByte( hContact, "IsTransported", 1 );
	}
	return isTransported;
}

void CJabberProto::JabberCheckAllContactsAreTransported()
{
	HANDLE hContact = ( HANDLE ) JCallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		char* szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
		if ( !lstrcmpA( szProtoName, szProto )) {
			DBVARIANT dbv;
			if ( !JGetStringT( hContact, "jid", &dbv )) {
				JabberDBCheckIsTransportedContact( dbv.ptszVal, hContact );
				JFreeVariant( &dbv );
		}	}

		hContact = ( HANDLE )JCallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM )hContact, 0 );
}	}

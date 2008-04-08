#include "aim.h"

#include "m_genmenu.h"

#pragma warning(disable:4355)

CAimProto::CAimProto( const char* aProtoName, const TCHAR* aUserName )
{
	m_tszUserName = mir_tstrdup( aUserName );
	m_szModuleName = mir_strdup( aProtoName );
	m_szProtoName = mir_strdup( aProtoName );
	_strlwr( m_szProtoName );
	m_szProtoName[0] = toupper( m_szProtoName[0] );
	LOG( "Setting protocol/module name to '%s/%s'", m_szProtoName, m_szModuleName );

	//create some events
	hAvatarEvent = CreateEvent(NULL,false,false,NULL);
	hAwayMsgEvent = CreateEvent(NULL,false,false,NULL);

	char* p = NEWSTR_ALLOCA( m_szModuleName );
	CharUpperA( p );
	GROUP_ID_KEY = strlcat(p,AIM_MOD_GI);
	ID_GROUP_KEY = strlcat(p,AIM_MOD_IG);
	FILE_TRANSFER_KEY = strlcat(p,AIM_KEY_FT);

	InitializeCriticalSection( &modeMsgsMutex );
	InitializeCriticalSection( &statusMutex );
	InitializeCriticalSection( &connectionMutex );
	InitializeCriticalSection( &SendingMutex );
	InitializeCriticalSection( &avatarMutex );

	CreateProtoService(PS_GETNAME, &CAimProto::GetName );
	CreateProtoService(PS_GETSTATUS, &CAimProto::GetStatus );
	CreateProtoService(PS_GETAVATARINFO, &CAimProto::GetAvatarInfo );

	InitIcons();
	InitMenus();

	HookProtoEvent(ME_SYSTEM_MODULESLOADED, &CAimProto::OnModulesLoaded);
	HookProtoEvent(ME_SYSTEM_PRESHUTDOWN, &CAimProto::OnPreShutdown);
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CAimProto::OnPreBuildContactMenu);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CAimProto::OnSettingChanged);
	HookProtoEvent(ME_DB_CONTACT_DELETED, &CAimProto::OnContactDeleted);

	if ( getByte( AIM_KEY_KA, 0 ))
		mir_forkthread(( pThreadFunc )aim_keepalive_thread, this );
}

CAimProto::~CAimProto()
{
	aim_links_destroy();

	CallService( MS_CLIST_REMOVEMAINMENUITEM, ( WPARAM )hMenuRoot, 0 );

	CallService( MS_CLIST_REMOVECONTACTMENUITEM, ( WPARAM )hHTMLAwayContextMenuItem, 0 );
	CallService( MS_CLIST_REMOVECONTACTMENUITEM, ( WPARAM )hAddToServerListContextMenuItem, 0 );
	CallService( MS_CLIST_REMOVECONTACTMENUITEM, ( WPARAM )hReadProfileMenuItem, 0 );

	if(hDirectBoundPort)
		Netlib_CloseHandle(hDirectBoundPort);
	if(hServerConn)
		Netlib_CloseHandle(hServerConn);
	Netlib_CloseHandle(hNetlib);
	Netlib_CloseHandle(hNetlibPeer);

	DeleteCriticalSection(&modeMsgsMutex);
	DeleteCriticalSection(&statusMutex);
	DeleteCriticalSection(&connectionMutex);
	DeleteCriticalSection(&SendingMutex);
	DeleteCriticalSection(&avatarMutex);

	delete[] CWD;
	delete[] szModeMsg;
	delete[] COOKIE;
	delete[] MAIL_COOKIE;
	delete[] AVATAR_COOKIE;
	delete[] GROUP_ID_KEY;
	delete[] ID_GROUP_KEY;
	delete[] FILE_TRANSFER_KEY;
	
	mir_free( m_szModuleName );
	mir_free( m_tszUserName );
	mir_free( m_szProtoName );
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

int CAimProto::OnModulesLoaded( WPARAM wParam, LPARAM lParam )
{
	char store[MAX_PATH];
	CallService(MS_DB_GETPROFILEPATH, MAX_PATH-1,(LPARAM)&store);
	if(store[lstrlenA(store)-1]=='\\')
		store[lstrlenA(store)-1]='\0';
	CWD=(char*)new char[lstrlenA(store)+1];
	memcpy(CWD,store,lstrlenA(store));
	memcpy(&CWD[lstrlenA(store)],"\0",1);

	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName = "AOL Instant Messenger server connection";
	hNetlib = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);

	char szP2P[128];
	mir_snprintf(szP2P, sizeof(szP2P), "%sP2P", m_szModuleName);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING;
	nlu.szDescriptiveName = "AOL Instant Messenger Client-to-client connection";
	nlu.szSettingsModule = szP2P;
	nlu.minIncomingPorts = 1;
	hNetlibPeer = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);

	DBVARIANT dbv;
	if (getString(AIM_KEY_HN, &dbv))
		setString(AIM_KEY_HN, AIM_DEFAULT_SERVER);
	else
		DBFreeVariant(&dbv);
	
	if(getWord( AIM_KEY_GP, -1)==-1)
		setWord( AIM_KEY_GP, DEFAULT_GRACE_PERIOD);

	if(getString(AIM_KEY_PW, &dbv))
	{
		if (!getString(OLD_KEY_PW, &dbv))
		{
			setString(AIM_KEY_PW, dbv.pszVal);
			DBDeleteContactSetting(NULL, m_szModuleName, OLD_KEY_PW);
			DBFreeVariant(&dbv);
		}
	}
	else DBFreeVariant(&dbv);

	if(getByte( AIM_KEY_DM,-1)==-1)
	{
		int i=getByte( OLD_KEY_DM,-1);
		if(i!=-1)
		{
			if(i==1)
				setByte( AIM_KEY_DM,0);
			else
				setByte( AIM_KEY_DM,1);
			DBDeleteContactSetting(NULL, m_szModuleName, OLD_KEY_DM);
	}	}

	HookProtoEvent(ME_OPT_INITIALISE, &CAimProto::OnOptionsInit);
	HookProtoEvent(ME_USERINFO_INITIALISE, &CAimProto::OnUserInfoInit);
	HookProtoEvent(ME_IDLE_CHANGED, &CAimProto::OnIdleChanged);
	HookProtoEvent(ME_CLIST_EXTRA_LIST_REBUILD, &CAimProto::OnExtraIconsRebuild);
	HookProtoEvent(ME_CLIST_EXTRA_IMAGE_APPLY, &CAimProto::OnExtraIconsApply);

	offline_contacts();
	aim_links_init();
	return 0;
}

int CAimProto::OnPreShutdown( WPARAM wParam, LPARAM lParam )
{
	shutting_down = 1;
	if ( hDirectBoundPort ) {
		freeing_DirectBoundPort = 1;
		SOCKET s = CallService(MS_NETLIB_GETSOCKET, LPARAM(hDirectBoundPort), 0);
		if (s != INVALID_SOCKET)
			shutdown(s, 2);
	}
	if ( hServerPacketRecver ) {
		SOCKET s = CallService(MS_NETLIB_GETSOCKET, LPARAM(hServerPacketRecver), 0);
		if (s != INVALID_SOCKET)
			shutdown(s, 2);
	}
	if ( hServerConn ) {
		SOCKET s = CallService(MS_NETLIB_GETSOCKET, LPARAM(hServerConn), 0);
		if (s != INVALID_SOCKET)
			shutdown(s, 2);
	}
	if ( hMailConn ) {
		SOCKET s = CallService(MS_NETLIB_GETSOCKET, LPARAM(hMailConn), 0);
		if (s != INVALID_SOCKET)
			shutdown(s, 2);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AddToList - adds a contact to the contact list

HANDLE CAimProto::AddToList( int flags, PROTOSEARCHRESULT* psr )
{
	if ( state != 1 )
		return 0;

	HANDLE hContact = find_contact( psr->nick );
	if (!hContact )
		hContact = add_contact( psr->nick );

	return hContact; //See authrequest for serverside addition
}

HANDLE __cdecl CAimProto::AddToListByEvent( int flags, int iContact, HANDLE hDbEvent )
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthAllow - processes the successful authorization

int CAimProto::Authorize( HANDLE hContact )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthDeny - handles the unsuccessful authorization

int CAimProto::AuthDeny( HANDLE hContact, const char* szReason )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_ADDED

int __cdecl CAimProto::AuthRecv( HANDLE hContact, PROTORECVEVENT* evt )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int __cdecl CAimProto::AuthRequest( HANDLE hContact, const char* szMessage )
{	
	//Not a real authrequest- only used b/c we don't know the group until now.
	if ( state != 1 )
		return 1;

	DBVARIANT dbv;
	if ( !DBGetContactSettingString( hContact, MOD_KEY_CL, OTH_KEY_GP, &dbv )) {
		add_contact_to_group( hContact, dbv.pszVal );
		DBFreeVariant( &dbv );
	}
	else add_contact_to_group( hContact, AIM_DEFAULT_GROUP );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// ChangeInfo 

HANDLE __cdecl CAimProto::ChangeInfo( int iInfoType, void* pInfoData )
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileAllow - starts a file transfer

int __cdecl CAimProto::FileAllow( HANDLE hContact, HANDLE hTransfer, const char* szPath )
{
	int ft = getByte( hContact, AIM_KEY_FT, -1 );
	if ( ft != -1 ) {
		char *szDesc, *szFile, *local_ip, *verified_ip, *proxy_ip;
		szFile = (char*)szPath + sizeof(DWORD);
		szDesc = szFile + lstrlenA(szFile) + 1;
		local_ip = szDesc + lstrlenA(szDesc) + 1;
		verified_ip = local_ip + lstrlenA(local_ip) + 1;
		proxy_ip = verified_ip + lstrlenA(verified_ip) + 1;
		int size = lstrlenA(szFile)+lstrlenA(szDesc)+lstrlenA(local_ip)+lstrlenA(verified_ip)+lstrlenA(proxy_ip)+5+sizeof(HANDLE);
		char *data=new char[size];
		memcpy(data,(char*)&hContact,sizeof(HANDLE));
		memcpy(&data[sizeof(HANDLE)],szFile,size-sizeof(HANDLE));
		setString( hContact, AIM_KEY_FN, szPath );
		mir_forkthread((pThreadFunc)accept_file_thread, new file_thread_param(this, data));
		return (int)hContact;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileCancel - cancels a file transfer

int __cdecl CAimProto::FileCancel( HANDLE hContact, HANDLE hTransfer )
{
	DBVARIANT dbv;
	if ( !getString( hContact, AIM_KEY_SN, &dbv )) {
		LOG("We are cancelling a file transfer.");

		char cookie[8];
		read_cookie( hContact, cookie );
		aim_deny_file( hServerConn, seqno, dbv.pszVal, cookie );
		DBFreeVariant( &dbv );
	}

	if ( getByte( hContact, AIM_KEY_FT, -1 ) != -1 ) {
		HANDLE Connection = (HANDLE)getDword( hContact, AIM_KEY_DH, 0 );
		if ( Connection )
			Netlib_CloseHandle( Connection );
	}
	DBDeleteContactSetting( hContact, m_szModuleName, AIM_KEY_FT );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileDeny - denies a file transfer

int __cdecl CAimProto::FileDeny( HANDLE hContact, HANDLE hTransfer, const char* szReason )
{
	DBVARIANT dbv;
	if ( !getString( hContact, AIM_KEY_SN, &dbv )) {
		LOG("We are denying a file transfer.");

		char cookie[8];
		read_cookie( hContact,cookie );
		aim_deny_file( hServerConn, seqno, dbv.pszVal, cookie );
		DBFreeVariant( &dbv );
	}
	DBDeleteContactSetting( hContact, m_szModuleName, AIM_KEY_FT );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileResume - processes file renaming etc

int __cdecl CAimProto::FileResume( HANDLE hTransfer, int* action, const char** szFilename )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

DWORD __cdecl CAimProto::GetCaps( int type, HANDLE hContact )
{
	switch ( type ) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSG | PF1_BASICSEARCH | PF1_FILE;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_ONTHEPHONE;

	case PFLAGNUM_3:
		return PF2_SHORTAWAY;

	case PFLAGNUM_4:
		return PF4_SUPPORTTYPING | PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTIDLE | PF4_AVATARS;

	case PFLAGNUM_5:
		return PF2_ONTHEPHONE;

	case PFLAG_MAXLENOFMESSAGE:
		return 1024;

	case PFLAG_UNIQUEIDTEXT:
		return (int) "Screen Name";

	case PFLAG_UNIQUEIDSETTING:
		return (int) AIM_KEY_SN;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetIcon - loads an icon for the contact list

HICON __cdecl CAimProto::GetIcon( int iconIndex )
{
	return ( LOWORD(iconIndex) == PLI_PROTOCOL) ? CopyIcon(LoadIconEx("aim")) : 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int __cdecl CAimProto::GetInfo( HANDLE hContact, int infoType )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by JID

struct basic_search_ack_param
{
	basic_search_ack_param( CAimProto* _ppro, const char* szId ) :
		ppro( _ppro ),
		snsearch( strldup( szId, lstrlenA( szId )))
	{}

	~basic_search_ack_param()
	{	delete[] snsearch;
	}

	CAimProto* ppro;
	char* snsearch;
};

static void __cdecl basic_search_ack_success(basic_search_ack_param* p)
{
	if ( char *sn = normalize_name( p->snsearch )) { // normalize it
		PROTOSEARCHRESULT psr;
		if (lstrlenA(sn) > 32) {
			ProtoBroadcastAck( p->ppro->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
			delete[] sn;
			delete p;
			return;
		}
		ZeroMemory(&psr, sizeof(psr));
		psr.cbSize = sizeof(psr);
		psr.nick = sn;
		ProtoBroadcastAck( p->ppro->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM) & psr);
		ProtoBroadcastAck( p->ppro->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
		delete[] sn;
	}
	delete p;
}

HANDLE __cdecl CAimProto::SearchBasic( const char* szId )
{
	if ( state != 1 )
		return 0;

	//duplicating the parameter so that it isn't deleted before it's needed- e.g. this function ends before it's used
	mir_forkthread(( pThreadFunc )basic_search_ack_success, new basic_search_ack_param( this, szId ));
	return ( HANDLE )1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE __cdecl CAimProto::SearchByEmail( const char* email )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByName - searches the contact by its first or last name, or by a nickname

HANDLE __cdecl CAimProto::SearchByName( const char* nick, const char* firstName, const char* lastName )
{
	return NULL;
}

HWND __cdecl CAimProto::SearchAdvanced( HWND owner )
{
	return NULL;
}

HWND __cdecl CAimProto::CreateExtendedSearchUI( HWND owner )
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvContacts

int __cdecl CAimProto::RecvContacts( HANDLE hContact, PROTORECVEVENT* )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvFile

int __cdecl CAimProto::RecvFile( HANDLE hContact, PROTORECVFILE* evt )
{
	CCSDATA ccs = { hContact, PSR_FILE, 0, ( LPARAM )evt };
	return CallService( MS_PROTO_RECVFILE, 0, ( LPARAM )&ccs );
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvMsg

int __cdecl CAimProto::RecvMsg( HANDLE hContact, PROTORECVEVENT* pre )
{
	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof( dbei );
	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags & PREF_CREATEREAD ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_MESSAGE;
	char* buf = 0;
	if ( pre->flags & PREF_UNICODE ) {
		LOG("Recieved a unicode message.");
		wchar_t* wbuf=(wchar_t*)&pre->szMessage[lstrlenA(pre->szMessage)+1];
		wchar_t* st_wbuf;
		if ( getByte( AIM_KEY_FI, 0)) {
			LOG("Converting from html to bbcodes then stripping leftover html.(U)");
			wchar_t* bbuf = html_to_bbcodes(wbuf);
			st_wbuf = strip_html(bbuf);
			delete[] bbuf;
		}
		else {
			LOG("Stripping html.(U)");
			st_wbuf = strip_html(wbuf);
		}

		buf = (char *)malloc(wcslen(st_wbuf)*3+3);
		WideCharToMultiByte( CP_ACP, 0,st_wbuf, -1,buf,wcslen(st_wbuf)+1, NULL, NULL);
		memcpy(&buf[strlen(buf)+1],st_wbuf,lstrlenA(buf)*2+2);
		delete[] st_wbuf;
		dbei.pBlob = (PBYTE)buf;
		dbei.cbBlob = lstrlenA(buf)+1;
		dbei.cbBlob *= (sizeof(wchar_t)+1);
	}
	else
	{
		LOG("Received a non-unicode message.");
		if(getByte( AIM_KEY_FI, 0)) 		{
			LOG("Converting from html to bbcodes then stripping leftover html.");
			char* bbuf = html_to_bbcodes(pre->szMessage);
			buf = strip_html(bbuf);
			delete[] bbuf;
		}
		else {
			LOG("Stripping html.");
			buf = strip_html(pre->szMessage);
		}
		dbei.pBlob = (PBYTE)buf;
		dbei.cbBlob = lstrlenA(buf)+1;
	}
	CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
	if(buf) free(buf);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvUrl

int __cdecl CAimProto::RecvUrl( HANDLE hContact, PROTORECVEVENT* )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int __cdecl CAimProto::SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

int __cdecl CAimProto::SendFile( HANDLE hContact, const char* szDescription, char** ppszFiles )
{
	if ( state != 1 )
		return 0;

	if ( hContact && szDescription && ppszFiles ) {
		if ( getByte( hContact, AIM_KEY_FT, -1 ) != -1 ) {
			ShowPopup("Aim Protocol","Cannot start a file transfer with this contact while another file transfer with the same contact is pending.", 0);
			return 0;
		}

		char** files = ppszFiles;
		char* pszDesc = ( char* )szDescription;
		char* pszFile = strrchr(files[0], '\\');
		pszFile++;
		struct _stat statbuf;
		_stat(files[0],&statbuf);
		unsigned long pszSize = statbuf.st_size;
		DBVARIANT dbv;
		if ( !getString(hContact, AIM_KEY_SN, &dbv )) {
			for ( int file_amt = 0; files[file_amt]; file_amt++ )
				if ( file_amt == 1 ) {
					ShowPopup("Aim Protocol","Aim allows only one file to be sent at a time.", 0);
					DBFreeVariant(&dbv);
					return 0;
				}

			char cookie[8];
			create_cookie( hContact );
			read_cookie( hContact, cookie );
			setString( hContact, AIM_KEY_FN, files[0] );
			setDword( hContact, AIM_KEY_FS, pszSize );
			setByte( hContact, AIM_KEY_FT, 1 );
			setString( hContact, AIM_KEY_FD, pszDesc );
			int force_proxy = getByte( AIM_KEY_FP, 0 );
			if ( force_proxy ) {
				LOG("We are forcing a proxy file transfer.");
				HANDLE hProxy = aim_peer_connect("ars.oscar.aol.com",5190);
				if ( hProxy ) {
					setByte( hContact, AIM_KEY_PS, 1 );
					setDword( hContact, AIM_KEY_DH, (DWORD)hProxy );//not really a direct connection
					mir_forkthread(( pThreadFunc )aim_proxy_helper, new aim_proxy_helper_param(this, hContact));
				}
			}
			else aim_send_file( hServerConn, seqno, dbv.pszVal, cookie, InternalIP, LocalPort, 0, 1, pszFile, pszSize, pszDesc );

			DBFreeVariant( &dbv );
			return (int)hContact;
		}
		DBDeleteContactSetting( hContact, m_szModuleName, AIM_KEY_FT );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendMessage - sends a message

int __cdecl CAimProto::SendMsg( HANDLE hContact, int flags, const char* pszSrc )
{
	DBVARIANT dbv;
	if ( !getString( hContact, AIM_KEY_SN, &dbv )) {
		if ( 0 == getByte( AIM_KEY_DC, 1))
			mir_forkthread(( pThreadFunc )msg_ack_success, new msg_ack_success_param( this, hContact ));

		char* msg = strldup( pszSrc, lstrlenA( pszSrc ));
		char* smsg = strip_carrots( msg );
		delete[] msg;

		if ( getByte( AIM_KEY_FO, 0 )) {
			char* html_msg = bbcodes_to_html(smsg);
			delete[] smsg;
			if ( aim_send_plaintext_message( hServerConn, seqno, dbv.pszVal,html_msg, 0 )) {
				delete[] html_msg;
				DBFreeVariant( &dbv );
				return 1;
			}
			delete[] html_msg;
		}
		else {
			if ( aim_send_plaintext_message( hServerConn, seqno, dbv.pszVal, smsg, 0 )) {
				delete[] smsg;
				DBFreeVariant(&dbv);
				return 1;
			}
			delete[] smsg;
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int __cdecl CAimProto::SendUrl( HANDLE hContact, int flags, const char* url )
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetApparentMode - sets the visibility m_iStatus

int __cdecl CAimProto::SetApparentMode( HANDLE hContact, int mode )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetStatus - sets the protocol m_iStatus

int __cdecl CAimProto::SetStatus( int iNewStatus )
{
	if ( iNewStatus == m_iStatus )
		return 0;

	if ( shutting_down )
		return 0;

	EnterCriticalSection( &statusMutex );
	start_connection( iNewStatus );
	if ( state == 1 ) {
		switch(m_iStatus) {
		case ID_STATUS_OFFLINE:
			broadcast_status(ID_STATUS_OFFLINE);
			break;

		case ID_STATUS_ONLINE:
		case ID_STATUS_FREECHAT:
			broadcast_status(ID_STATUS_ONLINE);
			aim_set_away(hServerConn,seqno,NULL);//unset away message
			aim_set_invis(hServerConn,seqno,AIM_STATUS_ONLINE,AIM_STATUS_NULL);//online not invis
			break;

		case ID_STATUS_INVISIBLE:
			broadcast_status(m_iStatus);
			aim_set_invis(hServerConn,seqno,AIM_STATUS_INVISIBLE,AIM_STATUS_NULL);
			break;

		case ID_STATUS_AWAY:
		case ID_STATUS_OUTTOLUNCH:
		case ID_STATUS_NA:
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
		case ID_STATUS_ONTHEPHONE:
			broadcast_status(ID_STATUS_AWAY);
			if(m_iStatus != ID_STATUS_AWAY) {
				assign_modmsg(DEFAULT_AWAY_MSG);
				aim_set_away(hServerConn,seqno,szModeMsg);//set actual away message
				aim_set_invis(hServerConn,seqno,AIM_STATUS_AWAY,AIM_STATUS_NULL);//away not invis
			}
			//see SetAwayMsg for m_iStatus away
			break;
	}	}
	LeaveCriticalSection(&statusMutex);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetAwayMsg - returns a contact's away message

int __cdecl CAimProto::GetAwayMsg( HANDLE hContact )
{
	if ( state != 1 )
		return 0;

	int m_iStatus = getWord( hContact, AIM_KEY_ST, ID_STATUS_OFFLINE );
	if ( ID_STATUS_AWAY != m_iStatus )
		return 0;

	DBVARIANT dbv;
	if ( !getString( hContact, AIM_KEY_SN, &dbv )) {
		awaymsg_request_handler( dbv.pszVal );
		DBFreeVariant( &dbv );
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG

int __cdecl CAimProto::RecvAwayMsg( HANDLE hContact, int statusMode, PROTORECVEVENT* pre )
{	
	ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)pre->szMessage);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AWAYMSG

int __cdecl CAimProto::SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg )
{	
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetAwayMsg - sets the away m_iStatus message

int __cdecl CAimProto::SetAwayMsg( int status, const char* msg )
{
	EnterCriticalSection(&modeMsgsMutex);
	if ( state == 1 )
		switch( status ) {
		case ID_STATUS_AWAY:
		case ID_STATUS_OUTTOLUNCH:
		case ID_STATUS_NA:
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
		case ID_STATUS_ONTHEPHONE:
			if ( !msg )
				msg = ( const char* )&DEFAULT_AWAY_MSG;

			assign_modmsg(( char* )msg );
			aim_set_away( hServerConn, seqno, szModeMsg ); //set actual away message
		}
	LeaveCriticalSection(&modeMsgsMutex);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UserIsTyping - sends a UTN notification

int __cdecl CAimProto::UserIsTyping( HANDLE hContact, int type )
{
	DBVARIANT dbv;
	if ( !getString( hContact, AIM_KEY_SN, &dbv )) {
		if ( type == PROTOTYPE_SELFTYPING_ON )
			aim_typing_notification( hServerConn, seqno, dbv.pszVal, 0x0002 );
		else if ( type == PROTOTYPE_SELFTYPING_OFF )
			aim_typing_notification( hServerConn, seqno, dbv.pszVal, 0x0000 );
		DBFreeVariant( &dbv );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnEvent - maintain protocol events

int __cdecl CAimProto::OnEvent( PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam )
{
	switch( eventType ) {
		case EV_PROTO_ONLOAD:    return OnModulesLoaded( 0, 0 );
		case EV_PROTO_ONEXIT:    return OnPreShutdown( 0, 0 );
		case EV_PROTO_ONOPTIONS: return OnOptionsInit( wParam, lParam );
	}
	return 1;
}

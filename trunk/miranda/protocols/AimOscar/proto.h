#ifndef _JABBER_PROTO_H_
#define _JABBER_PROTO_H_

#include "m_protoint.h"

struct CAimProto;
typedef int ( __cdecl CAimProto::*AimEventFunc )( WPARAM, LPARAM );
typedef int ( __cdecl CAimProto::*AimServiceFunc )( WPARAM, LPARAM );
typedef int ( __cdecl CAimProto::*AimServiceFuncParam )( WPARAM, LPARAM, LPARAM );

struct CAimProto : public PROTO_INTERFACE
{
				CAimProto( const char*, const TCHAR* );
				~CAimProto();

				__inline void* operator new( size_t size )
				{	return calloc( 1, size );
				}
				__inline void operator delete( void* p )
				{	free( p );
				}

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	HANDLE __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr );
	virtual	HANDLE __cdecl AddToListByEvent( int flags, int iContact, HANDLE hDbEvent );

	virtual	int    __cdecl Authorize( HANDLE hContact );
	virtual	int    __cdecl AuthDeny( HANDLE hContact, const char* szReason );
	virtual	int    __cdecl AuthRecv( HANDLE hContact, PROTORECVEVENT* );
	virtual	int    __cdecl AuthRequest( HANDLE hContact, const char* szMessage );

	virtual	HANDLE __cdecl ChangeInfo( int iInfoType, void* pInfoData );

	virtual	int    __cdecl FileAllow( HANDLE hContact, HANDLE hTransfer, const char* szPath );
	virtual	int    __cdecl FileCancel( HANDLE hContact, HANDLE hTransfer );
	virtual	int    __cdecl FileDeny( HANDLE hContact, HANDLE hTransfer, const char* szReason );
	virtual	int    __cdecl FileResume( HANDLE hTransfer, int* action, const char** szFilename );

	virtual	DWORD  __cdecl GetCaps( int type, HANDLE hContact = NULL );
	virtual	HICON  __cdecl GetIcon( int iconIndex );
	virtual	int    __cdecl GetInfo( HANDLE hContact, int infoType );

	virtual	HANDLE __cdecl SearchBasic( const char* id );
	virtual	HANDLE __cdecl SearchByEmail( const char* email );
	virtual	HANDLE __cdecl SearchByName( const char* nick, const char* firstName, const char* lastName );
	virtual	HWND   __cdecl SearchAdvanced( HWND owner );
	virtual	HWND   __cdecl CreateExtendedSearchUI( HWND owner );

	virtual	int    __cdecl RecvContacts( HANDLE hContact, PROTORECVEVENT* );
	virtual	int    __cdecl RecvFile( HANDLE hContact, PROTORECVFILE* );
	virtual	int    __cdecl RecvMsg( HANDLE hContact, PROTORECVEVENT* );
	virtual	int    __cdecl RecvUrl( HANDLE hContact, PROTORECVEVENT* );

	virtual	int    __cdecl SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList );
	virtual	int    __cdecl SendFile( HANDLE hContact, const char* szDescription, char** ppszFiles );
	virtual	int    __cdecl SendMsg( HANDLE hContact, int flags, const char* msg );
	virtual	int    __cdecl SendUrl( HANDLE hContact, int flags, const char* url );

	virtual	int    __cdecl SetApparentMode( HANDLE hContact, int mode );
	virtual	int    __cdecl SetStatus( int iNewStatus );
	void                   SetStatusWorker( int iNewStatus );

	virtual	int    __cdecl GetAwayMsg( HANDLE hContact );
	virtual	int    __cdecl RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt );
	virtual	int    __cdecl SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg );
	virtual	int    __cdecl SetAwayMsg( int m_iStatus, const char* msg );

	virtual	int    __cdecl UserIsTyping( HANDLE hContact, int type );

	virtual	int    __cdecl OnEvent( PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam );

	//====| Services |====================================================================
	int  __cdecl SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam);

	int  __cdecl GetName(WPARAM wParam, LPARAM lParam);
	int  __cdecl GetStatus(WPARAM wParam, LPARAM lParam);
	int  __cdecl GetAvatarInfo(WPARAM wParam, LPARAM lParam);
	int  __cdecl SendMsgW(WPARAM /*wParam*/, LPARAM lParam);

	int  __cdecl ManageAccount(WPARAM wParam, LPARAM lParam);
	int  __cdecl CheckMail(WPARAM wParam, LPARAM lParam);
	int  __cdecl InstantIdle(WPARAM wParam, LPARAM lParam);
	int  __cdecl GetHTMLAwayMsg(WPARAM wParam, LPARAM lParam);
	int  __cdecl GetProfile(WPARAM wParam, LPARAM lParam);
	int  __cdecl EditProfile(WPARAM wParam, LPARAM lParam);
	int  __cdecl AddToServerList(WPARAM wParam, LPARAM lParam);

	//====| Events |======================================================================
	int  __cdecl OnContactDeleted(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnExtraIconsRebuild( WPARAM wParam, LPARAM lParam );
	int  __cdecl OnExtraIconsApply( WPARAM wParam, LPARAM lParam );
	int  __cdecl OnIdleChanged( WPARAM wParam, LPARAM lParam );
	int  __cdecl OnModulesLoaded( WPARAM wParam, LPARAM lParam );
	int  __cdecl OnOptionsInit(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnPreBuildContactMenu(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnPreShutdown(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnSettingChanged(WPARAM wParam,LPARAM lParam);
	int  __cdecl OnUserInfoInit(WPARAM wParam,LPARAM lParam);

	//====| Data |========================================================================
	char* CWD;//current working directory
	char* GROUP_ID_KEY;
	char* ID_GROUP_KEY;
	char* FILE_TRANSFER_KEY;
	
	CRITICAL_SECTION modeMsgsMutex;
	CRITICAL_SECTION statusMutex;
	CRITICAL_SECTION connectionMutex;
	CRITICAL_SECTION SendingMutex;
	CRITICAL_SECTION avatarMutex;

	char* COOKIE;
	int COOKIE_LENGTH;
	char* MAIL_COOKIE;
	int MAIL_COOKIE_LENGTH;
	char* AVATAR_COOKIE;
	int AVATAR_COOKIE_LENGTH;

	char *username;
	char *password;
	unsigned short seqno;//main connection sequence number
	int state;//m_iStatus of the connection; e.g. whether connected or not
	int packet_offset;//current offset of main connection client to server packet
	int initial_status;//start up m_iStatus
	char* szModeMsg;//away message
	unsigned short port;

	//Some bools to keep track of different things
	bool request_HTML_profile;
	bool extra_icons_loaded;
	bool freeing_DirectBoundPort;
	bool shutting_down;
	bool idle;
	bool instantidle;
	bool checking_mail;
	bool list_received;
	HANDLE hKeepAliveEvent;

	//Some main connection stuff
	HANDLE hServerConn;//handle to the main connection
	HANDLE hServerPacketRecver;//handle to the listening device
	HANDLE hNetlib;//handle to netlib
	unsigned long InternalIP;// our ip
	unsigned short LocalPort;// our port
	
	//Peer connection stuff
	HANDLE hNetlibPeer;//handle to the peer netlib
	HANDLE hDirectBoundPort;//direct connection listening port
	HANDLE current_rendezvous_accept_user;//hack

	//Handles for the context menu items
	HANDLE hMenuRoot;
	HANDLE hHTMLAwayContextMenuItem;
	HANDLE hAddToServerListContextMenuItem;
	HANDLE hReadProfileMenuItem;

	//Some mail connection stuff
	HANDLE hMailConn;
	unsigned short mail_seqno;
	int mail_packet_offset;
	
	//avatar connection stuff
	HANDLE hAvatarConn;
	unsigned short avatar_seqno;
	HANDLE hAvatarEvent;
	bool AvatarLimitThread;

	//away message retrieval stuff
	HANDLE hAwayMsgEvent;

	//Some Icon handles
	HANDLE bot_icon;
	HANDLE icq_icon;
	HANDLE aol_icon;
	HANDLE hiptop_icon;
	HANDLE admin_icon;
	HANDLE confirmed_icon;
	HANDLE unconfirmed_icon;

	//////////////////////////////////////////////////////////////////////////////////////
	// avatars.cpp

	void   avatar_request_handler(TLV &tlv, HANDLE &hContact, char* sn,int &offset);
	void   avatar_retrieval_handler(SNAC &snac);
	void   avatar_apply(HANDLE &hContact,char* sn,char* filename);

	//////////////////////////////////////////////////////////////////////////////////////
	// away.cpp

	void   awaymsg_request_handler(char* sn);
	void   awaymsg_retrieval_handler(char* sn,char* msg);

	//////////////////////////////////////////////////////////////////////////////////////
	// client.cpp

	int    aim_send_connection_packet(HANDLE hServerConn,unsigned short &seqno,char *buf);
	int    aim_authkey_request(HANDLE hServerConn,unsigned short &seqno);
	int    aim_auth_request(HANDLE hServerConn,unsigned short &seqno,char* key,char* language,char* country);
	int    aim_send_cookie(HANDLE hServerConn,unsigned short &seqno,int cookie_size,char * cookie);
	int    aim_send_service_request(HANDLE hServerConn,unsigned short &seqno);
	int    aim_new_service_request(HANDLE hServerConn,unsigned short &seqno,unsigned short service);
	int    aim_request_rates(HANDLE hServerConn,unsigned short &seqno);
	int    aim_request_icbm(HANDLE hServerConn,unsigned short &seqno);
	int    aim_set_icbm(HANDLE hServerConn,unsigned short &seqno);
	int    aim_set_profile(HANDLE hServerConn,unsigned short &seqno,char *msg);//user info
	int    aim_set_away(HANDLE hServerConn,unsigned short &seqno,char *msg);//user info
	int    aim_set_invis(HANDLE hServerConn,unsigned short &seqno,char* m_iStatus,char* status_flag);
	int    aim_request_list(HANDLE hServerConn,unsigned short &seqno);
	int    aim_activate_list(HANDLE hServerConn,unsigned short &seqno);
	int    aim_accept_rates(HANDLE hServerConn,unsigned short &seqno);
	int    aim_set_caps(HANDLE hServerConn,unsigned short &seqno);
	int    aim_client_ready(HANDLE hServerConn,unsigned short &seqno);
	int    aim_mail_ready(HANDLE hServerConn,unsigned short &seqno);
	int    aim_avatar_ready(HANDLE hServerConn,unsigned short &seqno);
	int    aim_send_plaintext_message(HANDLE hServerConn,unsigned short &seqno,char* sn,char* msg,bool auto_response);
	int    aim_send_unicode_message(HANDLE hServerConn,unsigned short &seqno,char* sn,wchar_t* msg);
	int    aim_query_away_message(HANDLE hServerConn,unsigned short &seqno,char* sn);
	int    aim_query_profile(HANDLE hServerConn,unsigned short &seqno,char* sn);
	int    aim_delete_contact(HANDLE hServerConn,unsigned short &seqno,char* sn,unsigned short item_id,unsigned short group_id);
	int    aim_add_contact(HANDLE hServerConn,unsigned short &seqno,char* sn,unsigned short item_id,unsigned short group_id);
	int    aim_add_group(HANDLE hServerConn,unsigned short &seqno,char* name,unsigned short group_id);
	int    aim_mod_group(HANDLE hServerConn,unsigned short &seqno,char* name,unsigned short group_id,char* members,unsigned short members_length);
	int    aim_keepalive(HANDLE hServerConn,unsigned short &seqno);
	int    aim_send_file(HANDLE hServerConn,unsigned short &seqno,char* sn,char* icbm_cookie,unsigned long ip, unsigned short port, bool force_proxy, unsigned short request_num ,char* file_name,unsigned long total_bytes,char* descr);//used when requesting a regular file transfer
	int    aim_send_file_proxy(HANDLE hServerConn,unsigned short &seqno,char* sn,char* icbm_cookie, char* file_name,unsigned long total_bytes,char* descr,unsigned long proxy_ip, unsigned short port);
	int    aim_file_redirected_request(HANDLE hServerConn,unsigned short &seqno,char* sn,char* icbm_cookie);
	int    aim_file_proxy_request(HANDLE hServerConn,unsigned short &seqno,char* sn,char* icbm_cookie,char request_num,unsigned long proxy_ip, unsigned short port);
	int    aim_accept_file(HANDLE hServerConn,unsigned short &seqno,char* sn,char* icbm_cookie);
	int    aim_deny_file(HANDLE hServerConn,unsigned short &seqno,char* sn,char* icbm_cookie);
	int    aim_typing_notification(HANDLE hServerConn,unsigned short &seqno,char* sn,unsigned short type);
	int    aim_set_idle(HANDLE hServerConn,unsigned short &seqno,unsigned long seconds);
	int    aim_request_mail(HANDLE hServerConn,unsigned short &seqno);
	int    aim_request_avatar(HANDLE hServerConn,unsigned short &seqno,char* sn, char* hash, unsigned short hash_size);//family 0x0010

	//////////////////////////////////////////////////////////////////////////////////////
	// connection.cpp

	int    LOG(const char *fmt, ...);
	HANDLE aim_connect(char* server);
	HANDLE aim_peer_connect(char* ip,unsigned short port);

	//////////////////////////////////////////////////////////////////////////////////////
	// error.cpp

	void login_error(unsigned short* error);
	void get_error(unsigned short* error);

	//////////////////////////////////////////////////////////////////////////////////////
	// file.cpp

	void   sending_file(HANDLE hContact, HANDLE hNewConnection);
	void   receiving_file(HANDLE hContact, HANDLE hNewConnection);

	//////////////////////////////////////////////////////////////////////////////////////
	// links.cpp

	ATOM   aWatcherClass;
	HWND   hWatcher;

	void   aim_links_unregister();
	void   aim_links_init();
	void   aim_links_destroy();
	void   aim_links_regwatcher();
	void   aim_links_unregwatcher();

	//////////////////////////////////////////////////////////////////////////////////////
	// packets.cpp

	int    aim_writesnac(unsigned short service, unsigned short subgroup,unsigned short request_id,unsigned short &offset,char* out);
	int    aim_writetlv(unsigned short type,unsigned short size ,char* value,unsigned short &offset,char* out);
	int    aim_sendflap(HANDLE conn, char type,unsigned short length,char *buf, unsigned short &seqno);
	int    aim_writefamily(char *buf,unsigned short &offset,char* out);
	int    aim_writegeneric(unsigned short size,char *buf,unsigned short &offset,char* out);

	//////////////////////////////////////////////////////////////////////////////////////
	// server.cpp

	void   snac_md5_authkey(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);
	int    snac_authorization_reply(SNAC &snac);
	void   snac_supported_families(SNAC &snac, HANDLE hServerConn,unsigned short &seqno);
	void   snac_supported_family_versions(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);//family 0x0001
	void   snac_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);//family 0x0001
	void   snac_mail_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);// family 0x0001
	void   snac_avatar_rate_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);// family 0x0001
	void   snac_service_redirect(SNAC &snac);// family 0x0001
	void   snac_icbm_limitations(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);//family 0x0004
	void   snac_user_online(SNAC &snac);
	void   snac_user_offline(SNAC &snac);
	void   snac_error(SNAC &snac);//family 0x0003 or x0004
	void   snac_contact_list(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);
	void   snac_message_accepted(SNAC &snac);//family 0x004
	void   snac_received_message(SNAC &snac,HANDLE hServerConn,unsigned short &seqno);//family 0x0004
	void   snac_busted_payload(SNAC &snac);//family 0x0004
	void   snac_received_info(SNAC &snac);//family 0x0002
	void   snac_typing_notification(SNAC &snac);//family 0x004
	void   snac_list_modification_ack(SNAC &snac);//family 0x0013
	void   snac_mail_response(SNAC &snac);//family 0x0018
	void   snac_retrieve_avatar(SNAC &snac);//family 0x0010

	//////////////////////////////////////////////////////////////////////////////////////
	// themes.cpp

	void   InitIcons(void);
	void   InitMenus(void);
	HICON  LoadIconEx(const char* name);
	HANDLE GetIconHandle(const char* name);
	void   ReleaseIconEx(const char* name);

	//////////////////////////////////////////////////////////////////////////////////////
	// utilities.cpp

	void   broadcast_status(int status);
	void   start_connection(int initial_status);
	HANDLE find_contact(char * sn);
	HANDLE add_contact(char* buddy);
	void   add_contacts_to_groups();
	void   add_contact_to_group(HANDLE hContact,char* group);
	void   offline_contacts();
	void   offline_contact(HANDLE hContact, bool remove_settings);
	void   add_AT_icons();
	void   remove_AT_icons();
	void   add_ES_icons();
	void   remove_ES_icons();
	void   execute_cmd(char* type,char* arg);
	char*  strip_special_chars(char *src,HANDLE hContact);
	void   assign_modmsg(char* msg);

	FILE*  open_contact_file(char* sn, char* file, char* mode, char* &path, bool contact_dir);
	void   write_away_message(HANDLE hContact,char* sn,char* msg);
	void   write_profile(char* sn,char* msg);

	WORD   search_for_free_group_id(char *name);
	WORD   search_for_free_item_id(HANDLE hbuddy);
	char*  get_members_of_group( WORD group_id, WORD& size);

	void   create_cookie(HANDLE hContact);
	void   read_cookie(HANDLE hContact,char* cookie);
	void   write_cookie(HANDLE hContact,char* cookie);

	void   load_extra_icons();

	void   ShowPopup( const char* title, const char* msg, int flags, char* url = 0 );

	//////////////////////////////////////////////////////////////////////////////////////
	HANDLE CreateProtoEvent(const char* szEvent);
	void   CreateProtoService(const char* szService, AimServiceFunc serviceProc);
	void   CreateProtoServiceParam(const char* szService, AimServiceFuncParam serviceProc, LPARAM lParam);
	void   HookProtoEvent(const char* szEvent, AimEventFunc pFunc);

	int    getByte( const char* name, BYTE defaultValue );
	int    getByte( HANDLE hContact, const char* name, BYTE defaultValue );
	int    getDword( const char* name, DWORD defaultValue );
	int    getDword( HANDLE hContact, const char* name, DWORD defaultValue );
	int    getString( const char* name, DBVARIANT* );
	int    getString( HANDLE hContact, const char* name, DBVARIANT* );
	int    getTString( const char* name, DBVARIANT* );
	int    getTString( HANDLE hContact, const char* name, DBVARIANT* );
	int    getWord( const char* name, WORD defaultValue );
	int    getWord( HANDLE hContact, const char* name, WORD defaultValue );

	void   setByte( const char* name, BYTE value );
	void   setByte( HANDLE hContact, const char* name, BYTE value );
	void   setDword( const char* name, DWORD value );
	void   setDword( HANDLE hContact, const char* name, DWORD value );
	void   setString( const char* name, const char* value );
	void   setString( HANDLE hContact, const char* name, const char* value );
	void   setTString( const char* name, const TCHAR* value );
	void   setTString( HANDLE hContact, const char* name, const TCHAR* value );
	void   setWord( const char* name, WORD value );
	void   setWord( HANDLE hContact, const char* name, WORD value );
};

#endif

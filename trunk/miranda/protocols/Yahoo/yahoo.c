/*
 * $Id$
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */


#include <time.h>
#include <stdio.h>
#include <sys/stat.h>

/*
 * Miranda headers
 */
#include "yahoo.h"
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_message.h>

typedef struct {
	int id;
	char *label;
} yahoo_idlabel;

typedef struct {
	int id;
	char *who;
} yahoo_authorize_data;

yahoo_idlabel yahoo_status_codes[] = {
	{YAHOO_STATUS_AVAILABLE, "Available"},
	{YAHOO_STATUS_BRB, "BRB"},
	{YAHOO_STATUS_BUSY, "Busy"},
	{YAHOO_STATUS_NOTATHOME, "Not At Home"},
	{YAHOO_STATUS_NOTATDESK, "Not At my Desk"},
	{YAHOO_STATUS_NOTINOFFICE, "Not In The Office"},
	{YAHOO_STATUS_ONPHONE, "On The Phone"},
	{YAHOO_STATUS_ONVACATION, "On Vacation"},
	{YAHOO_STATUS_OUTTOLUNCH, "Out To Lunch"},
	{YAHOO_STATUS_STEPPEDOUT, "Stepped Out"},
	{YAHOO_STATUS_INVISIBLE, "Invisible"},
	{YAHOO_STATUS_IDLE, "Idle"},
	{YAHOO_STATUS_OFFLINE, "Offline"},
	{YAHOO_STATUS_CUSTOM, "[Custom]"},
	{YAHOO_STATUS_NOTIFY, "Notify"},
	{0, NULL}
};

#define MAX_PREF_LEN 255

yahoo_local_account * ylad = NULL;

int do_yahoo_debug = 0;
int conn_type = 1;
static int ping_timer = 0;

extern int poll_loop;

void ext_yahoo_got_im(int id, char *who, char *msg, long tm, int stat, int utf8);

char * yahoo_status_code(enum yahoo_status s)
{
	int i;
	for(i=0; yahoo_status_codes[i].label; i++)
		if(yahoo_status_codes[i].id == s)
			return yahoo_status_codes[i].label;
	return "Unknown";
}

int miranda_to_yahoo(int myyahooStatus)
{
    int ret = YAHOO_STATUS_OFFLINE;
    switch (myyahooStatus) {
    case ID_STATUS_ONLINE: 
                        ret = YAHOO_STATUS_AVAILABLE;
                        break;
    case ID_STATUS_NA:
                        ret = YAHOO_STATUS_BRB;
                        break;
    case ID_STATUS_OCCUPIED:
                        ret = YAHOO_STATUS_BUSY;
                        break;
    case ID_STATUS_AWAY:
                        ret = YAHOO_STATUS_NOTATDESK;
                        break;
    case ID_STATUS_ONTHEPHONE:
                        ret = YAHOO_STATUS_ONPHONE;
                        break;
    case ID_STATUS_OUTTOLUNCH:
                        ret = YAHOO_STATUS_OUTTOLUNCH;                            
                        break;
    case ID_STATUS_INVISIBLE:
                        ret = YAHOO_STATUS_INVISIBLE;
                        break;
    case ID_STATUS_DND:
                        ret = YAHOO_STATUS_STEPPEDOUT;
                        break;
    }                                                
    
    return ret;
}

void yahoo_set_status(int myyahooStatus, char *msg, int away)
{
	LOG(("yahoo_set_status myyahooStatus: %d, msg: %s, away: %d", myyahooStatus, msg, away));
	
    if (YAHOO_CUSTOM_STATUS != myyahooStatus)
        yahoo_set_away(ylad->id, miranda_to_yahoo(myyahooStatus), msg, away);
    else
	    yahoo_set_away(ylad->id, YAHOO_CUSTOM_STATUS, msg, away);
}

int yahoo_to_miranda_status(int yahooStatus, int away)
{
    int ret = ID_STATUS_OFFLINE;
    switch (yahooStatus) {
    case YAHOO_STATUS_AVAILABLE: 
                        ret = ID_STATUS_ONLINE;
                        break;
    case YAHOO_STATUS_BRB:
                        ret = ID_STATUS_NA;
                        break;
    case YAHOO_STATUS_BUSY:
                        ret = ID_STATUS_OCCUPIED;
                        break;
    case YAHOO_STATUS_ONPHONE:
                        ret = ID_STATUS_ONTHEPHONE;
                        break;
    case YAHOO_STATUS_OUTTOLUNCH:
                        ret = ID_STATUS_OUTTOLUNCH;                            
                        break;
    case YAHOO_STATUS_INVISIBLE:
                        ret = ID_STATUS_INVISIBLE;
                        break;
    case YAHOO_STATUS_NOTATHOME:
                        ret = ID_STATUS_AWAY;
                        break;
    case YAHOO_STATUS_NOTATDESK:
                        ret = ID_STATUS_AWAY;
                        break;
    case YAHOO_STATUS_NOTINOFFICE:
                        ret = ID_STATUS_AWAY;
                        break;
    case YAHOO_STATUS_ONVACATION:
                        ret = ID_STATUS_AWAY;
                        break;
    case YAHOO_STATUS_STEPPEDOUT:
                        ret = ID_STATUS_DND;
                        break;
    case YAHOO_STATUS_IDLE:
                        ret = ID_STATUS_AWAY;
                        break;
    case YAHOO_STATUS_CUSTOM:
                        ret = (away ? ID_STATUS_OCCUPIED:ID_STATUS_ONLINE);
                        break;
    }
    return ret;
}

int ext_yahoo_log(char *fmt,...)
{
	char		str[ 4096 ];
	va_list ap;
	int tBytes;

	va_start(ap, fmt);
	
	tBytes = _vsnprintf( str, sizeof( str ), fmt, ap );
	if ( tBytes > 0 )
		str[ tBytes ] = 0;

	YAHOO_CallService( MS_NETLIB_LOG, ( WPARAM )hNetlibUser, ( LPARAM )str );

	va_end(ap);
	return 0;
}

void get_fd(int id, int fd, int error, void *data) 
{
    y_filetransfer *sf = (y_filetransfer*) data;
    char buf[1024];
    int size = 0;
	DWORD dw;
	struct _stat statbuf;
	
	if (_stat( sf->filename, &statbuf ) != 0 )
		error = 1;
	
    if(!error) {
		 HANDLE myhFile    = CreateFile(sf->filename,
                                   GENERIC_READ,
                                   FILE_SHARE_READ|FILE_SHARE_WRITE,
			           NULL,
			           OPEN_EXISTING,
			           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			           0);


		if(myhFile !=INVALID_HANDLE_VALUE) {
			PROTOFILETRANSFERSTATUS pfts;

			DWORD lNotify = GetTickCount();
			LOG(("proto: %s, hContact: %d", yahooProtocolName, sf->hContact));
			
			//ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, sf, 0);
			ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, sf, 0);
			//ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, ACKRESULT_SENTREQUEST, sf, 0);
			//ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, sf, 0);
			
			do {
				ReadFile(myhFile, buf, 1024, &dw, NULL);
			
				if (dw) {
					//dw = send(fd, buf, dw, 0);
					dw = Netlib_Send((SOCKET)fd, buf, dw, MSG_NODUMP);
					size += dw;
					
					if(GetTickCount() >= lNotify + 500 || dw < 1024 || size == statbuf.st_size) {
						
					LOG(("DOING UI Notify. Got %d/%d", size, statbuf.st_size));
					memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
					pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
					pfts.hContact = sf->hContact;
					pfts.sending = 1;
					pfts.files = &sf->filename;
					pfts.totalFiles = 1;//ntohs(1);
					pfts.currentFileNumber = 0;
					pfts.totalBytes = statbuf.st_size;//sf->size;
					pfts.totalProgress = size;
					pfts.workingDir = "C:\\";//sf->filename;//ft->savepath;
					pfts.currentFile = sf->filename;
					pfts.currentFileSize = statbuf.st_size; //ntohl(ft->hdr.size);
					pfts.currentFileTime = statbuf.st_mtime;//ntohl(ft->hdr.modtime);
					pfts.currentFileProgress = size;
					
					ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, ACKRESULT_DATA, sf, (LPARAM) & pfts);
					lNotify = GetTickCount();
					}
				}
				
				if (sf->cancel) {
					LOG(("Upload Cancelled! "));
					error = 1;
					break;
				}
			} while ( dw == 1024);
	    CloseHandle(myhFile);
		}
    }

    ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, !error ? ACKRESULT_SUCCESS:ACKRESULT_FAILED, sf, 0);
}

void YAHOO_SendFile(y_filetransfer *sf)
{
	long tFileSize = 0;
	{	struct _stat statbuf;
		if ( _stat( sf->filename, &statbuf ) == 0 )
			tFileSize += statbuf.st_size;
	}

	yahoo_send_file(ylad->id, sf->who, sf->msg, sf->filename, tFileSize, &get_fd, sf);
}

void get_url(int id, int fd, int error,	const char *filename, unsigned long size, void *data) 
{
    y_filetransfer *sf = (y_filetransfer*) data;
    char buf[1024];
    int rsize = 0;
	DWORD dw, c;
	
    if(!error) {
		 HANDLE myhFile;
		 
		 wsprintf(buf, "%s/%s", sf->savepath, sf->filename);
		 myhFile    = CreateFile(buf,
                                   GENERIC_WRITE,
                                   FILE_SHARE_WRITE,
								   NULL, OPEN_ALWAYS,  FILE_ATTRIBUTE_NORMAL,  0);

		if(myhFile !=INVALID_HANDLE_VALUE) {
			PROTOFILETRANSFERSTATUS pfts;

			DWORD lNotify = GetTickCount();
			LOG(("proto: %s, hContact: %d", yahooProtocolName, sf->hContact));
			
			ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, sf, 0);
			
			do {
				dw = Netlib_Recv((SOCKET)fd, buf, 1024, MSG_NODUMP);
			
				if (dw) {
					WriteFile(myhFile, buf, dw, &c, NULL);
					rsize += dw;
					
					/*LOG(("Got %d/%d", rsize, size));*/
					if(GetTickCount() >= lNotify + 500 || dw <= 0 || rsize == size) {
						
					LOG(("DOING UI Notify. Got %d/%d", rsize, size));
					memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
					pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
					pfts.hContact = sf->hContact;
					pfts.sending = 0;
					pfts.files = &sf->filename;
					pfts.totalFiles = 1;//ntohs(1);
					pfts.currentFileNumber = 0;
					pfts.totalBytes = size;//sf->size;
					pfts.totalProgress = rsize;
					pfts.workingDir = sf->savepath;//ft->savepath;
					pfts.currentFile = sf->filename;
					pfts.currentFileSize = size; //ntohl(ft->hdr.size);
					pfts.currentFileTime = time(NULL);//ntohl(ft->hdr.modtime);
					pfts.currentFileProgress = rsize;
					
					ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, ACKRESULT_DATA, sf, (LPARAM) & pfts);
					lNotify = GetTickCount();
					}
				}
				
				if (sf->cancel) {
					LOG(("Recv Cancelled! "));
					error = 1;
					break;
				}
			} while ( dw > 0 );
	    CloseHandle(myhFile);
		}
    }

    ProtoBroadcastAck(yahooProtocolName, sf->hContact, ACKTYPE_FILE, !error ? ACKRESULT_SUCCESS:ACKRESULT_FAILED, sf, 0);
}

void YAHOO_RecvFile(y_filetransfer *ft)
{
	yahoo_get_url_handle(ylad->id, ft->url, &get_url, ft);
}

void YAHOO_basicsearch(const char *nick)
{
	yahoo_search(ylad->id, YAHOO_SEARCH_YID, nick, YAHOO_GENDER_NONE, YAHOO_AGERANGE_NONE, 0, 1);
	
}

void YAHOO_remove_buddy(const char *who)
{
    LOG(("YAHOO_remove_buddy '%s'", who));
	HANDLE hContact;
	DBVARIANT dbv;
	
	hContact = getbuddyH(who);

	//if ( DBGetContactSetting( hContact, "Clist", "Group", &dbv ))
	//	return;
	if ( DBGetContactSetting( hContact, yahooProtocolName, "YGroup", &dbv ))
		return;
	
	yahoo_remove_buddy(ylad->id, who, dbv.pszVal);
	DBFreeVariant( &dbv );
}

void YAHOO_sendtyping(const char *who, int stat)
{
	LOG(("[Yahoo_sendtyping] Sending Typing Notification to '%s', state: %d", who, stat));
	yahoo_send_typing(ylad->id, NULL, who, stat);
}

void YAHOO_reject(const char *who, const char *msg)
{
    yahoo_reject_buddy(ylad->id, who, msg);
    //YAHOO_remove_buddy(who);
    //yahoo_refresh(ylad->id);
}

void yahoo_logout()
{
	poll_loop = 0;
	
	if (ylad == NULL)
        return;
        
	if (ylad->id <= 0) {
		return;
	}

	//stop_timer();
	
	if (yahooLoggedIn) {
		yahooLoggedIn = FALSE;
		yahoo_logoff(ylad->id);
	}
	
	//pthread_mutex_lock(&connectionHandleMutex);
    
    //yahoo_util_broadcaststatus(ID_STATUS_OFFLINE);
	//yahoo_logoff_buddies();	
	if (ylad)
		yahoo_close(ylad->id);
	
	FREE(ylad);
	ylad = NULL;
	
	//pthread_mutex_unlock(&connectionHandleMutex);
}

void yahoo_send_msg(const char *id, const char *msg, int utf8)
{
	LOG(("yahoo_send_msg: %s: %s, utf: %d", id, msg, utf8));
    if (YAHOO_GetByte( "DisableUTF8", 0 ) ) {
		/* need to convert it to ascii argh */
		char 	*umsg = msg;
		
		if (utf8) {
			wchar_t* tRealBody = NULL;
			
			umsg = (char *) alloca(lstrlen(msg) + 1);
			lstrcpy(umsg, msg);
			Utf8Decode( umsg, 0, &tRealBody );
			free( tRealBody );
		} 
			
		yahoo_send_im(ylad->id, NULL, id, umsg, 0);
    } else {
	    char *tmp;

		if (!utf8) {
			tmp = y_str_to_utf8(msg);
			yahoo_send_im(ylad->id, NULL, id, tmp, 1);
			free(tmp);
		} else 
			yahoo_send_im(ylad->id, NULL, id, msg, 1);
    } 
   
}

HANDLE getbuddyH(const char *yahoo_id)
{
	char  *szProto;
	HANDLE hContact;
		
	for ( hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		   hContact != NULL;
			hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM )hContact, 0 ))
	{
		szProto = ( char* )YAHOO_CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact, 0 );
		if ( szProto != NULL && !lstrcmp( szProto, yahooProtocolName ))
		{
			DBVARIANT dbv;
			if ( DBGetContactSetting( hContact, yahooProtocolName, YAHOO_LOGINID, &dbv ))
				continue;

			{	
                int tCompareResult = lstrcmpi( dbv.pszVal, yahoo_id );
				DBFreeVariant( &dbv );
				if ( tCompareResult )
					continue;
			}

			return hContact;
			}	
    }

	return NULL;
}

HANDLE add_buddy( const char *yahoo_id, const char *yahoo_name, DWORD flags )
{
	//char *szProto;
	HANDLE hContact;
	//DBVARIANT dbv;
	
	hContact = getbuddyH(yahoo_id);
	if (hContact != NULL) {
		if ( !( flags & PALF_TEMPORARY ) && DBGetContactSettingByte( hContact, "CList", "NotOnList", 1 )) 
		{
			DBDeleteContactSetting( hContact, "CList", "NotOnList" );
			DBDeleteContactSetting( hContact, "CList", "Hidden" );

        }
		return hContact;
    }

	//not already there: add
	hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_ADD, 0, 0 );
	YAHOO_CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact,( LPARAM )yahooProtocolName );
	YAHOO_SetString( hContact, YAHOO_LOGINID, yahoo_id );
	if (lstrlen(yahoo_name) > 0)
		YAHOO_SetString( hContact, "Nick", yahoo_name );
	else
	    YAHOO_SetString( hContact, "Nick", yahoo_id );
	    
	//DBWriteContactSettingWord(hContact, yahooProtocolName, "Status", ID_STATUS_OFFLINE);

   	if (flags & PALF_TEMPORARY ) {
		DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
		DBWriteContactSettingByte( hContact, "CList", "Hidden", 1 );
    }	
	return hContact;
}

const char *find_buddy( const char *yahoo_id)
{
	//char  *szProto;
	static char nick[128];
	HANDLE hContact;
	DBVARIANT dbv;
	
	hContact = getbuddyH(yahoo_id);
	if (hContact != NULL) {
		if ( DBGetContactSetting( hContact, yahooProtocolName, "Nick", &dbv ))
			return NULL;
	
		strncpy(nick, dbv.pszVal, 128);
		DBFreeVariant( &dbv );
		return nick;
		
    }

	return NULL;
}

/* Required handlers bellow */

/* Conference handlers */
void ext_yahoo_got_conf_invite(int id, char *who, char *room, char *msg, YList *members)
{
	char z[1024];
	
	_snprintf(z, sizeof(z), "[miranda] Got conference invite to room: %s with msg: %s", room ?room:"", msg ?msg:"");
	ext_yahoo_got_im(id, who, z, 0, 0, 0);
}

void ext_yahoo_conf_userdecline(int id, char *who, char *room, char *msg)
{
}

void ext_yahoo_conf_userjoin(int id, char *who, char *room)
{
}

void ext_yahoo_conf_userleave(int id, char *who, char *room)
{
}

void ext_yahoo_conf_message(int id, char *who, char *room, char *msg, int utf8)
{
}

/* chat handlers */
void ext_yahoo_chat_cat_xml(int id, char *xml) 
{
}

void ext_yahoo_chat_join(int id, char *room, char * topic, YList *members, int fd)
{
}

void ext_yahoo_chat_userjoin(int id, char *room, struct yahoo_chat_member *who)
{
}

void ext_yahoo_chat_userleave(int id, char *room, char *who)
{
}
void ext_yahoo_chat_message(int id, char *who, char *room, char *msg, int msgtype, int utf8)
{
}

/* Other handlers */
void ext_yahoo_status_changed(int id, const char *who, int stat, const char *msg, int away, int idle, int mobile)
{
	HANDLE 	hContact = 0;
	time_t  idlets = 0;
	
	LOG(("ext_yahoo_status_changed for %s with msg %s (stat: %d, away: %d, idle: %d seconds)", who, msg, stat, away, idle));
	
	hContact = getbuddyH(who);
	if (hContact == NULL) {
		LOG(("Buddy Not Found. Adding..."));
		hContact = add_buddy(who, who, 0);
	}
	
	if (!mobile)
		YAHOO_SetWord(hContact, "Status", yahoo_to_miranda_status(stat,away));
	else
		YAHOO_SetWord(hContact, "Status", ID_STATUS_ONTHEPHONE);
	
	YAHOO_SetWord(hContact, "YStatus", stat);
	YAHOO_SetWord(hContact, "YAway", away);
	YAHOO_SetWord(hContact, "Mobile", mobile);
	YAHOO_SetString(hContact, "YMsg", (msg != NULL) ? msg : "");
	
	
	if ( (away == 2) || (stat == YAHOO_STATUS_IDLE) || (idle > 0)) {
		LOG(("%s idle for %d:%02d:%02d", who, idle/3600, (idle/60)%60, idle%60));
		
		time(&idlets);
		//if (idle > 0) || (stat == YAHOO_STATUS_IDLE) ) {
		idlets -= idle;
		//} 
	} 
		
	DBWriteContactSettingDword(hContact, yahooProtocolName, "IdleTS", idlets);
		
	if(msg) {
		LOG(("%s custom message '%s'", who, msg));
	}
}

void ext_yahoo_got_buddies(int id, YList * buds)
{
    LOG(("ext_yahoo_got_buddies"));
    
	if (buds == NULL) {
		LOG(("No Buddies to work on!"));
		return;
	}
	
	LOG(("Walking buddy list..."));
	for(; buds; buds = buds->next) {
	    HANDLE hContact;
	    
		struct yahoo_buddy *bud = buds->data;
		if (bud == NULL) {
			LOG(("EMPTY BUDDY LIST??"));
			continue;
		}
		
		if (bud->real_name)
			LOG(("id = %s name = %s", bud->id, bud->real_name));
		
		hContact = getbuddyH(bud->id);
		if (hContact == NULL)
			hContact = add_buddy(bud->id, (bud->real_name != NULL) ? bud->real_name : bud->id, 0);
		
		if (bud->group)
			YAHOO_SetString( hContact, "YGroup", bud->group);
		
		if (bud->yab_entry) {
		  LOG(("YAB_ENTRY"));
		  
		  if (bud->yab_entry->fname) 
		    YAHOO_SetString( hContact, "FirstName", bud->yab_entry->fname);
		  
		  
		  if (bud->yab_entry->lname) 
		      YAHOO_SetString( hContact, "LastName", bud->yab_entry->lname);
		  
		  
		  if (bud->yab_entry->nname) 
		      YAHOO_SetString( hContact, "Nick", bud->yab_entry->nname);
		  
		  
		  if (bud->yab_entry->email) 
			YAHOO_SetString( hContact, "e-mail", bud->yab_entry->email);
		  
		  
		}
	}

}

void ext_yahoo_got_ignore(int id, YList * igns)
{
    LOG(("ext_yahoo_got_ignore"));
}

void ext_yahoo_got_im(int id, char *who, char *msg, long tm, int stat, int utf8)
{
    char *umsg, *c = msg;
	int oidx = 0;
	wchar_t* tRealBody = NULL;
	int      tRealBodyLen = 0;

    LOG(("YAHOO_GOT_IM %s: %s tm:%d stat:%d utf8:%d", who, msg, tm, stat, utf8));
   	
	if(stat == 2) {
		LOG(("Error sending message to %s", who));
		return;
	}

	if(!msg) {
		LOG(("Empty Incoming Message, exiting."));
		return;
	}

	umsg = (char *) alloca(lstrlen(msg) + 1);
	while ( *c != '\0') {
		        // Strip the font and font size tag
        if (!strnicmp(c,"<font face=",11) || !strnicmp(c,"<font size=",11) || 
			!strnicmp(c, "<font color=",12) || !strnicmp(c,"</font>",6) ||
			// strip the fade tag
			!strnicmp(c, "<FADE ",6) || !strnicmp(c,"</FADE>",7) ||
			// strip the alternate colors tag
			!strnicmp(umsg ,"<ALT ",5) || !strnicmp(umsg ,"</ALT>",6)){ 
                while ((*c++ != '>') && (*c != '\0')); 
		} else
        // strip ANSI color combination
        if ((*c == 0x1b) && (*(c+1) == '[')){ 
               while ((*c++ != 'm') && (*c != '\0')); 
		} else
		
		if (*c != '\0'){
			umsg[oidx++] = *c;
			c++;
		}
	}

	umsg[oidx++]= '\0';
	
	
	/* Need to strip off formatting stuff first. Then do all decoding/converting */
	if (utf8){	
		Utf8Decode( umsg, 0, &tRealBody );
		tRealBodyLen = wcslen( tRealBody );
	}

	LOG(("%s: %s", who, umsg));
	
	//if(!strcmp(umsg, "<ding>")) 
	//	:P("\a");
	int msgLen = (lstrlen(umsg) + 1) * (sizeof(wchar_t) + 1);
	char* tMsgBuf = ( char* )alloca( msgLen );
	char* p = tMsgBuf;

	strcpy( p, umsg );
	
	p += lstrlen(umsg) + 1;

	if ( tRealBodyLen != 0 ) {
		memcpy( p, tRealBody, sizeof( wchar_t )*( tRealBodyLen+1 ));
		free( tRealBody );
	}

	CCSDATA ccs;
	PROTORECVEVENT pre;
	HANDLE hContact;

	ccs.szProtoService = PSR_MESSAGE;
	ccs.hContact = hContact = add_buddy(who, who, PALF_TEMPORARY);
	ccs.wParam = 0;
	ccs.lParam = (LPARAM) & pre;
	pre.flags = PREF_UNICODE;
	
	if (tm)
			pre.timestamp = tm;
	else
			pre.timestamp = time(NULL);
			
	pre.szMessage = tMsgBuf;
	pre.lParam = 0;
	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM) & ccs);

}

void ext_yahoo_rejected(int id, char *who, char *msg)
{
   	char buff[1024]={0};
   	HANDLE hContact;
    LOG(("ext_yahoo_rejected"));
	snprintf(buff, sizeof(buff), "%s has rejected your request and sent the following message:", who);

	hContact = add_buddy(who, who, 0);
	
    YAHOO_CallService( MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);	
    YAHOO_remove_buddy(who);
        
    MessageBox( NULL, msg, buff, MB_OK | MB_ICONINFORMATION );

}

void YAHOO_add_buddy(const char *who, const char *group, const char *msg)
{
	/* We adding a buddy to our list.
	  2 Stages.
	1. We send add buddy packet.
	2. We get a packet back from the server confirming add.
	
	No refresh needed. */
    yahoo_add_buddy(ylad->id, who, group, msg);
}

void ext_yahoo_contact_added(int id, char *myid, char *who, char *msg)
{
    LOG(("ext_yahoo_contact_added %s added you w/ msg '%s'", who, msg));

   	//char buff[1024];
	char *szBlob,*pCurBlob;
	char m[1024], m1[5];
   	DWORD dwUin;
	HANDLE hContact=NULL;
	//int found = 0;

	CCSDATA ccs;
	PROTORECVEVENT pre;

	hContact = add_buddy(who, who, PALF_TEMPORARY);
	
	ccs.szProtoService= PSR_AUTH;
	ccs.hContact=hContact;
	ccs.wParam=0;
	ccs.lParam=(LPARAM)&pre;
	pre.flags=0;
	pre.timestamp=time(NULL);
	
	m1[0]='\0';
	if (msg == NULL)
	  m[0]='\0';
    else
        strcpy(m, msg);
	 
	pre.lParam=sizeof(DWORD)+sizeof(HANDLE)+lstrlen(who)+lstrlen(m1)+lstrlen(m1)+lstrlen(who)+lstrlen(m)+5;
	
	pCurBlob=szBlob=(char *)malloc(pre.lParam);
    /*
       Added blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), 
                  last(ASCIIZ), email(ASCIIZ)
                  
       Auth blob is: uin(DWORD),hcontact(HANDLE),nick(ASCIIZ),first(ASCIIZ),
                  last(ASCIIZ),email(ASCIIZ),reason(ASCIIZ)

	//blob is: 0(DWORD), nick(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), email(ASCIIZ), ""(ASCIIZ)

    */

	// UIN
	dwUin = 0;
    memcpy(pCurBlob,&dwUin,sizeof(DWORD)); 
    pCurBlob+=sizeof(DWORD);

    // hContact
	memcpy(pCurBlob,&hContact,sizeof(HANDLE)); 
    pCurBlob+=sizeof(HANDLE);
    
    // NICK
    strcpy((char *)pCurBlob,who); 
    pCurBlob+=lstrlen((char *)pCurBlob)+1;
    
    // FIRST
    strcpy((char *)pCurBlob,m1); 
    pCurBlob+=lstrlen((char *)pCurBlob)+1;
    
    // LAST
    strcpy((char *)pCurBlob,m1); 
    pCurBlob+=lstrlen((char *)pCurBlob)+1;
    
    // E-mail    
	strcpy((char *)pCurBlob,who); 
	pCurBlob+=lstrlen((char *)pCurBlob)+1;
	
	// Reason
	strcpy((char *)pCurBlob, m); 
	
	pre.szMessage=(char *)szBlob;
	CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
}

void ext_yahoo_typing_notify(int id, char *who, int stat)
{
    const char *c;
    HANDLE hContact;
    
    LOG(("[ext_yahoo_typing_notify] who: '%s' stat: %d ", who, stat));

	hContact = getbuddyH(who);
	if (!hContact) return;
	
	c = YAHOO_GetContactName(hContact);
	
	CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)stat?10:0);

    if (!YAHOO_GetByte("DisplayTyping", 1)) return;

	if(ServiceExists( MS_POPUP_ADDPOPUPEX ))
		YAHOO_ShowPopup( c, Translate( "typing..." ), YAHOO_ALLOW_ENTER + YAHOO_ALLOW_MSGBOX + YAHOO_NOTIFY_POPUP );
}

void ext_yahoo_game_notify(int id, char *who, int stat)
{
    LOG(("[ext_yahoo_game_notify] id: %d, who: %s, stat: %d", id, who, stat));
    /* FIXME - Not Implemented - this informs you someone else is playing on Yahoo! Games */
    /* Also Stubbed in Sample Client */
	
}

void ext_yahoo_mail_notify(int id, char *from, char *subj, int cnt)
{
    SkinPlaySound( mailsoundname );

    if (!YAHOO_GetByte( "DisableYahoomail", 0)) {    
        LOG(("ext_yahoo_mail_notify"));
        
        if(ServiceExists( MS_POPUP_ADDPOPUPEX )) {
	        char z[256], title[31];
    
			if (from == NULL) {
                strcpy(title, "New Mail");
                wsprintf(z, "You Have %i unread msgs", cnt);
			} else {
                wsprintf(title, "New Mail (%i msgs)", cnt);
                _snprintf(z, 256, "From: %s\nSubject: %s", from, subj);
			}

            YAHOO_ShowPopup( title, z, YAHOO_ALLOW_ENTER + YAHOO_ALLOW_MSGBOX + YAHOO_MAIL_POPUP );
		}
    }
}    
    
/* WEBCAM callbacks */
void ext_yahoo_got_webcam_image(int id, const char *who,
		const unsigned char *image, unsigned int image_size, unsigned int real_size,
		unsigned int timestamp)
{
    LOG(("ext_yahoo_got_webcam_image"));
}

void ext_yahoo_webcam_viewer(int id, char *who, int connect)
{
    LOG(("ext_yahoo_webcam_viewer"));
}

void ext_yahoo_webcam_closed(int id, char *who, int reason)
{
    LOG(("ext_yahoo_webcam_closed"));
}

void ext_yahoo_webcam_data_request(int id, int send)
{
    LOG(("ext_yahoo_webcam_data_request"));
}

void ext_yahoo_webcam_invite(int id, char *from)
{
    LOG(("ext_yahoo_webcam_invite"));
	
	ext_yahoo_got_im(id, from, "[miranda] Got webcam invite. (not currently supported)", 0, 0, 0);
}

void ext_yahoo_webcam_invite_reply(int id, char *from, int accept)
{
    LOG(("ext_yahoo_webcam_invite_reply"));
}

void ext_yahoo_system_message(int id, char *msg)
{
	LOG(("Yahoo System Message: %s", msg));
}

void ext_yahoo_got_file(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize)
{
    CCSDATA ccs;
    PROTORECVEVENT pre;
	HANDLE hContact;
	char *szBlob;
	y_filetransfer *ft;
	
    LOG(("[ext_yahoo_got_file] id: %d, who: %s, url: %s, expires: %d, msg: %s, fname: %s, fsize: %d", id, who, url, expires, msg, fname, fesize));
	
	hContact = getbuddyH(who);
	if (hContact == NULL) 
		hContact = add_buddy(who, who, PALF_TEMPORARY);
	
	ft= (y_filetransfer*) malloc(sizeof(y_filetransfer));
	ft->who = strdup(who);
	if (msg != NULL)
		ft->msg = strdup(msg);
	else
		ft->msg = strdup("[no description given]");
	
	ft->hContact = hContact;
	if (fname != NULL)
		ft->filename = strdup(fname);
	else {
		char *start, *end;
		
		/* based on how gaim does this */
		start = strrchr(url, '/');
		if (start)
			start++;
		
		end = strrchr(url, '?');
		
		if (start && *start && end) {
			/* argh WINDOWS SUCKS!!! */
			//ft->filename = strndup(start, end-start);
			ft->filename = (char *)malloc(end - start + 1);
			strncpy(ft->filename, start, end-start);
			ft->filename[end-start] = '\0';
		} else 
			ft->filename = strdup("filename.ext");
	}
	
	ft->url = strdup(url);
	ft->fsize = fesize;
	ft->cancel = 0;
	
    // blob is DWORD(*ft), ASCIIZ(filenames), ASCIIZ(description)
    szBlob = (char *) malloc(sizeof(DWORD) + lstrlen(ft->filename) + lstrlen(ft->msg) + 2);
    *((PDWORD) szBlob) = (DWORD) ft;
    strcpy(szBlob + sizeof(DWORD), ft->filename);
    strcpy(szBlob + sizeof(DWORD) + lstrlen(ft->filename) + 1, ft->msg);

	pre.flags = 0;
	pre.timestamp = time(NULL);
    pre.szMessage = szBlob;
    pre.lParam = 0;
    ccs.szProtoService = PSR_FILE;
    ccs.hContact = ft->hContact;
    ccs.wParam = 0;
    ccs.lParam = (LPARAM) & pre;
    CallService(MS_PROTO_CHAINRECV, 0, (LPARAM) & ccs);
	free(szBlob);
}

void ext_yahoo_got_identities(int id, YList * ids)
{
    LOG(("ext_yahoo_got_identities"));
    /* FIXME - Not implemented - Got list of Yahoo! identities */
    /* We currently only use the default identity */
    /* Also Stubbed in Sample Client */
	
}

extern char *Bcookie;
void ext_yahoo_got_cookies(int id)
{
//    char z[1024];

    LOG(("ext_yahoo_got_cookies "));
/*    LOG(("Y Cookie: '%s'", yahoo_get_cookie(id, "y")));
    LOG(("T Cookie: '%s'", yahoo_get_cookie(id, "t")));
    LOG(("C Cookie: '%s'", yahoo_get_cookie(id, "c")));
    LOG(("Login Cookie: '%s'", yahoo_get_cookie(id, "login")));
    
    //wsprintf(z, "Cookie: %s; C=%s; Y=%s; T=%s", Bcookie, yahoo_get_cookie(id, "c"), yahoo_get_cookie(id, "y"), yahoo_get_cookie(id, "t"));
    //wsprintf(z, "Cookie: %s; Y=%s", Bcookie, yahoo_get_cookie(id, "y"), yahoo_get_cookie(id, "t"));    
    wsprintf(z, "Cookie: Y=%s; T=%s", yahoo_get_cookie(id, "y"), yahoo_get_cookie(id, "t"));    
    LOG(("Our Cookie: '%s'", z));
    YAHOO_CallService(MS_NETLIB_SETSTICKYHEADERS, (WPARAM)hNetlibUser, (LPARAM)z);*/
	
	if (YAHOO_GetByte( "UseYAB", 1 )) {
		LOG(("GET YAB [Before final check] "));
		yahoo_get_yab(id);
	}
}

void ext_yahoo_got_ping(int id)
{
    int status;
    
    LOG(("ext_yahoo_got_ping"));
    
	status = DBGetContactSettingWord(NULL, yahooProtocolName, "StartupStatus", ID_STATUS_ONLINE);
	if (status != yahooStatus) {
		LOG(("[COOKIES] Updating Status to %d ", status));    
		yahoo_util_broadcaststatus(status);
		
		if (status != ID_STATUS_INVISIBLE) // don't generate a bogus packet for Invisible state
			yahoo_set_status(status, NULL, (status != ID_STATUS_ONLINE) ? 1 : 0);
	}
	
}

void ext_yahoo_login_response(int id, int succ, char *url)
{
	char buff[1024];

	LOG(("ext_yahoo_login_response"));
	
	if(succ == YAHOO_LOGIN_OK) {
		ylad->status = yahoo_current_status(id);
		LOG(("logged in status-> %d", ylad->status));
        yahooLoggedIn=TRUE;
			
		return;
	} else if(succ == YAHOO_LOGIN_UNAME) {

		snprintf(buff, sizeof(buff), "Could not log into Yahoo service - username not recognised.  Please verify that your username is correctly typed.");
	} else if(succ == YAHOO_LOGIN_PASSWD) {

		snprintf(buff, sizeof(buff), "Could not log into Yahoo service - password incorrect.  Please verify that your username and password are correctly typed.");

	} else if(succ == YAHOO_LOGIN_LOCK) {
		
		snprintf(buff, sizeof(buff), "Could not log into Yahoo service.  Your account has been locked.\nVisit %s to reactivate it.", url);

	} else if(succ == YAHOO_LOGIN_DUPL) {
		snprintf(buff, sizeof(buff), "You have been logged out of the yahoo service, possibly due to a duplicate login.");
	}else if(succ == -1) {
		/// Can't Connect or got disconnected.
		if (yahooStatus == ID_STATUS_CONNECTING)
			snprintf(buff, sizeof(buff), "Could not connect to the Yahoo service. Check your server/port and proxy settings.");	
		else
			return;
	} else {
		snprintf(buff, sizeof(buff), "Could not log in, unknown reason: %d.", succ);
	}

	ext_yahoo_log(buff);
	
	poll_loop = 0;
	/*
       yahoo_logout(); -- The following Line MAKES us LOOP and CPU 100%
     */
	//
	// Show Error Message
	//
	if (YAHOO_GetByte( "ShowErrors", 1 )) {
		if (YAHOO_hasnotification())
			YAHOO_shownotification("Yahoo Login Error", buff, NIIF_ERROR);
		else
			MessageBox(NULL, buff, "Yahoo Login Error", MB_OK | MB_ICONINFORMATION);
	}
}

void ext_yahoo_error(int id, char *err, int fatal)
{
    LOG(("Yahoo Error: %s", err));
    
	if(fatal)
		yahoo_logout();
}

int ext_yahoo_connect(char *h, int p)
{
	NETLIBOPENCONNECTION ncon;
    HANDLE con;
    
	LOG(("ext_yahoo_connect %s:%d", h, p));
	
    ncon.cbSize = sizeof(ncon);
    ncon.szHost = h;
    ncon.wPort = p;
    ncon.flags = 0;

	poll_loop = 1;
/*    if (CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM) hNetlib, (LPARAM) & nlus) && !(nlus.useProxy && nlus.dnsThroughProxy)) {
        struct in_addr addr;
        DWORD ip = aim_toc_dnslookup(host);

        if (ip) {
            addr.S_un.S_addr = ip;
            _snprintf(host, sizeof(host), "%s", inet_ntoa(addr));
            ncon.szHost = host;
        }
    }
    LOG(("connecting to %s:%d", host, ncon.wPort));*/
	
    con = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) hNetlibUser, (LPARAM) & ncon);
    if (!con) {
        char z[1024];
        
        yahoo_util_broadcaststatus(ID_STATUS_OFFLINE);

        wsprintf(z, "Connection to %s:%d failed", ncon.szHost, ncon.wPort);
        
       	if (YAHOO_hasnotification())
       		 YAHOO_shownotification("Yahoo Login Error", z, NIIF_ERROR);
	    else
	         MessageBox(NULL, z, "Yahoo Login Error", MB_OK | MB_ICONINFORMATION);

        return -1;
    }

    return (int)con;
}

/*************************************
 * Callback handling code starts here
 */
YList *connections = NULL;
static int connection_tags=0;

int ext_yahoo_add_handler(int id, int fd, yahoo_input_condition cond, void *data)
{
	struct _conn *c = y_new0(struct _conn, 1);
	c->tag = ++connection_tags;
	c->id = id;
	c->fd = fd;
	c->cond = cond;
	c->data = data;

	LOG(("Add %d for %d, tag %d", fd, id, c->tag));

	connections = y_list_prepend(connections, c);

	return c->tag;
}

void ext_yahoo_remove_handler(int id, int tag)
{
	YList *l;
	LOG(("ext_yahoo_remove_handler id:%d tag:%d ", id, tag));
	
	for(l = connections; l; l = y_list_next(l)) {
		struct _conn *c = l->data;
		if(c->tag == tag) {
			/* don't actually remove it, just mark it for removal */
			/* we'll remove when we start the next poll cycle */
			LOG(("Marking id:%d fd:%d tag:%d for removal", c->id, c->fd, c->tag));
			c->remove = 1;
			return;
		}
	}
}

struct connect_callback_data {
	yahoo_connect_callback callback;
	void * callback_data;
	int id;
	int tag;
};

static void connect_complete(void *data, int source, yahoo_input_condition condition)
{
	struct connect_callback_data *ccd = data;
	int error = 0;//, err_size = sizeof(error);

	ext_yahoo_remove_handler(0, ccd->tag);
	
	// We Need to read the Socket error
	//getsockopt(source, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&err_size);
	NETLIBSELECT tSelect = {0};
	tSelect.cbSize = sizeof( tSelect );
	//tSelect.dwTimeout = T->mGatewayTimeout * 1000;
	tSelect.dwTimeout = 1;
	tSelect.hReadConns[ 0 ] = ( HANDLE )source;
	error = YAHOO_CallService( MS_NETLIB_SELECT, 0, ( LPARAM )&tSelect );

	if(error) {
		//close(source);
		Netlib_CloseHandle((HANDLE)source);
		source = -1;
	}

	LOG(("Connected fd: %d, error: %d", source, error));

	ccd->callback(source, error, ccd->callback_data);
	FREE(ccd);
}

void yahoo_callback(struct _conn *c, yahoo_input_condition cond)
{
	int ret=1;
	char buff[1024]={0};

	LOG(("yahoo_callback"));
	if(c->id < 0) {
		connect_complete(c->data, c->fd, cond);
	} else if (c->fd > 0) {
	
		if(cond & YAHOO_INPUT_READ)
			ret = yahoo_read_ready(c->id, c->fd, c->data);
		if(ret>0 && cond & YAHOO_INPUT_WRITE)
			ret = yahoo_write_ready(c->id, c->fd, c->data);

		if(ret == -1)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error (%d): %s", errno, strerror(errno));
		else if(ret == 0)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error: Server closed socket");

//		if(buff[0])
//			print_message((buff));
	}
}

int ext_yahoo_connect_async(int id, char *host, int port, 
		yahoo_connect_callback callback, void *data)
{
    int error;
    
    LOG(("ext_yahoo_connect_async %s:%d", host, port));
    
    error = ext_yahoo_connect(host, port);

	if(error != -1) {
		//LOG(("Connected fd: %d, error: %d", hServerConn, error));
		
		callback(error, 0, data);
		return 0;
	} else {
		//close(servfd);
		return -1;
	}

    
    
}
/*
 * Callback handling code ends here
 ***********************************/
void ext_yahoo_chat_yahoologout(int id)
{ 
 	LOG(("got chat logout"));
}
void ext_yahoo_chat_yahooerror(int id)
{ 
 	LOG(("got chat error"));
}

void ext_yahoo_got_search_result(int id, int found, int start, int total, YList *contacts)
{
	LOG(("got search result: "));
	
	LOG(("ID: %d", id));
	LOG(("Found: %d", found));
	LOG(("Start: %d", start));
	LOG(("Total: %d", total));
	
	
    PROTOSEARCHRESULT psr;
	struct yahoo_found_contact *yct=NULL;
	char *c;
	//int i;
	YList *en=contacts;
/*    if (aim_util_isme(sn)) {
        ProtoBroadcastAck(AIM_PROTO, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
        return;
    }
    */
	
    ZeroMemory(&psr, sizeof(psr));
    psr.cbSize = sizeof(psr);
	
	while (en) {
		yct = en->data;
    //psr.nick = (char *)snsearch;
		psr.nick = (char *)yct->id;
		c = (char *)malloc(10);
		itoa(yct->age, c,10);
		psr.firstName = yct->gender;
		psr.lastName = (char *)c;
		psr.email = (char *)yct->location;
    //psr.email = (char *)snsearch;
    
	//void yahoo_search(int id, enum yahoo_search_type t, const char *text, enum yahoo_search_gender g, enum yahoo_search_agerange ar, 
	//	int photo, int yahoo_only)

		YAHOO_SendBroadcast(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM) & psr);
		en = y_list_next(en);
	}
    YAHOO_SendBroadcast(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}


void register_callbacks()
{
	static struct yahoo_callbacks yc;

	yc.ext_yahoo_login_response = ext_yahoo_login_response;
	yc.ext_yahoo_got_buddies = ext_yahoo_got_buddies;
	yc.ext_yahoo_got_ignore = ext_yahoo_got_ignore;
	yc.ext_yahoo_got_identities = ext_yahoo_got_identities;
	yc.ext_yahoo_got_cookies = ext_yahoo_got_cookies;
	yc.ext_yahoo_status_changed = ext_yahoo_status_changed;
	yc.ext_yahoo_got_im = ext_yahoo_got_im;
	yc.ext_yahoo_got_conf_invite = ext_yahoo_got_conf_invite;
	yc.ext_yahoo_conf_userdecline = ext_yahoo_conf_userdecline;
	yc.ext_yahoo_conf_userjoin = ext_yahoo_conf_userjoin;
	yc.ext_yahoo_conf_userleave = ext_yahoo_conf_userleave;
	yc.ext_yahoo_conf_message = ext_yahoo_conf_message;
	yc.ext_yahoo_chat_cat_xml = ext_yahoo_chat_cat_xml;
	yc.ext_yahoo_chat_join = ext_yahoo_chat_join;
	yc.ext_yahoo_chat_userjoin = ext_yahoo_chat_userjoin;
	yc.ext_yahoo_chat_userleave = ext_yahoo_chat_userleave;
	yc.ext_yahoo_chat_message = ext_yahoo_chat_message;
	yc.ext_yahoo_chat_yahoologout = ext_yahoo_chat_yahoologout;
	yc.ext_yahoo_chat_yahooerror = ext_yahoo_chat_yahooerror;
	yc.ext_yahoo_got_webcam_image = ext_yahoo_got_webcam_image;
	yc.ext_yahoo_webcam_invite = ext_yahoo_webcam_invite;
	yc.ext_yahoo_webcam_invite_reply = ext_yahoo_webcam_invite_reply;
	yc.ext_yahoo_webcam_closed = ext_yahoo_webcam_closed;
	yc.ext_yahoo_webcam_viewer = ext_yahoo_webcam_viewer;
	yc.ext_yahoo_webcam_data_request = ext_yahoo_webcam_data_request;
	yc.ext_yahoo_got_file = ext_yahoo_got_file;
	yc.ext_yahoo_contact_added = ext_yahoo_contact_added;
	yc.ext_yahoo_rejected = ext_yahoo_rejected;
	yc.ext_yahoo_typing_notify = ext_yahoo_typing_notify;
	yc.ext_yahoo_game_notify = ext_yahoo_game_notify;
	yc.ext_yahoo_mail_notify = ext_yahoo_mail_notify;
	yc.ext_yahoo_got_search_result = ext_yahoo_got_search_result;
	yc.ext_yahoo_system_message = ext_yahoo_system_message;
	yc.ext_yahoo_error = ext_yahoo_error;
	yc.ext_yahoo_log = ext_yahoo_log;
	yc.ext_yahoo_add_handler = ext_yahoo_add_handler;
	yc.ext_yahoo_remove_handler = ext_yahoo_remove_handler;
	yc.ext_yahoo_connect = ext_yahoo_connect;
	yc.ext_yahoo_connect_async = ext_yahoo_connect_async;

	yc.ext_yahoo_got_ping  = ext_yahoo_got_ping;
	yahoo_register_callbacks(&yc);
	
}

static VOID CALLBACK yahoo_ping_timeout(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	LOG(("[TIMER] yahoo_ping_timeout"));
	
	if (yahooLoggedIn && (ylad != NULL) && (ylad->id > 0) ) {
		LOG(("[TIMER] Sending a keep alive message"));
		yahoo_keepalive(ylad->id);
	} 
}

void stop_timer()
{
	if (ping_timer != 0) {
	 LOG(("[TIMER] Killing Timer!"));
	 KillTimer(NULL, ping_timer);
	 ping_timer = 0;
	}
}

void start_timer()
{
    stop_timer();
    
	LOG(("[TIMER] Starting Timer!"));
	// set Timer for 10 minutes 
	ping_timer = SetTimer(NULL, 0, 1 * 60 * 1000, yahoo_ping_timeout);
	if (ping_timer == 0) {
	 	LOG(("[TIMER] Can not create Timer!"));
	}else{
        LOG(("[TIMER] Timer Set: %d!", ping_timer));
	}

}

void ext_yahoo_login(int login_mode)
{
	char host[128];
	int port=0;
    DBVARIANT dbv;

	LOG(("ext_yahoo_login"));

	if (!DBGetContactSetting(NULL, yahooProtocolName, YAHOO_LOGINSERVER, &dbv)) {
        _snprintf(host, sizeof(host), "%s", dbv.pszVal);
        DBFreeVariant(&dbv);
    }
    else {
        //_snprintf(host, sizeof(host), "%s", pager_host);
       	if (YAHOO_hasnotification())
       		 YAHOO_shownotification("Yahoo Login Error", "Please enter Yahoo server to Connect to in Options.", NIIF_ERROR);
	    else
	         MessageBox(NULL, "Please enter Yahoo server to Connect to in Options.", "Yahoo Login Error", MB_OK | MB_ICONINFORMATION);

        return;
    }

	port = DBGetContactSettingWord(NULL, yahooProtocolName, YAHOO_LOGINPORT, 5050);
	
	//ylad->id = yahoo_init(ylad->yahoo_id, ylad->password);
	ylad->id = yahoo_init_with_attributes(ylad->yahoo_id, ylad->password, 
			"pager_host", host,
			"pager_port", port,
			NULL);

	ylad->status = YAHOO_STATUS_OFFLINE;
	yahoo_login(ylad->id, login_mode);

	if (ylad->id <= 0) {
		LOG(("Could not connect to Yahoo server.  Please verify that you are connected to the net and the pager host and port are correctly entered."));
		return;
	}

	//rearm(&pingTimer, 600);
}

void YAHOO_refresh() 
{
	yahoo_refresh(ylad->id);
}




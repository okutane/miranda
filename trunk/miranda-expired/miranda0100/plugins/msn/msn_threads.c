/*
Miranda ICQ: the free icq client for MS Windows 
Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede

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
#include "msn_global.h"
#include "../../miranda32/protocols/protocols/m_protomod.h"
#include "../../miranda32/protocols/protocols/m_protosvc.h"
#include "../../miranda32/ui/contactlist/m_clist.h"

int MSN_HandleCommands(struct ThreadData *info,char *cmdString);
int MSN_HandleErrors(struct ThreadData *info,char *cmdString);

extern SOCKET msnNSSocket;
extern volatile LONG msnLoggedIn;
extern int msnStatusMode,msnDesiredStatus;

void __cdecl MSNServerThread(struct ThreadData *info)
{
	MSN_DebugLog(MSN_LOG_DEBUG,"Thread started: server='%s', type=%d",info->server,info->type);

	{	SOCKADDR_IN sockaddr;
		WORD port;

		info->s=socket(AF_INET,SOCK_STREAM,0);
		if(info->type==SERVER_SWITCHBOARD) Switchboards_New(info->s);
		if(info->type==SERVER_SWITCHBOARD) Switchboards_ChangeStatus(SBSTATUS_DNSLOOKUP);
		if((sockaddr.sin_addr.S_un.S_addr=MSN_WS_ResolveName(info->server,&port,MSN_DEFAULT_PORT))==SOCKET_ERROR)
		{
			MSN_DebugLog(MSN_LOG_FATAL,"Failed to resolve '%s'",info->server);
			if(info->type==SERVER_NOTIFICATION || info->type==SERVER_DISPATCH) {
				CmdQueue_AddProtoAck(NULL,ACKTYPE_LOGIN,ACKRESULT_FAILED,NULL,LOGINERR_NONETWORK);
				CmdQueue_AddProtoAck(NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)msnStatusMode,ID_STATUS_OFFLINE);
				msnStatusMode=ID_STATUS_OFFLINE;
			}
			return;
		}

		MSN_DebugLog(MSN_LOG_DEBUG,"Name resolved");

		sockaddr.sin_port=htons(port);
		sockaddr.sin_family=AF_INET;

		if(info->type==SERVER_SWITCHBOARD) Switchboards_ChangeStatus(SBSTATUS_CONNECTING);
		if(connect(info->s,(SOCKADDR*)&sockaddr,sizeof(sockaddr))==SOCKET_ERROR) {
			MSN_DebugLog(MSN_LOG_FATAL,"Connection Failed (%d)",WSAGetLastError());
			if(info->type==SERVER_NOTIFICATION || info->type==SERVER_DISPATCH) {
				CmdQueue_AddProtoAck(NULL,ACKTYPE_LOGIN,ACKRESULT_FAILED,NULL,LOGINERR_NOSERVER);
				CmdQueue_AddProtoAck(NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)msnStatusMode,ID_STATUS_OFFLINE);
				msnStatusMode=ID_STATUS_OFFLINE;
			}
			return;
		}
		MSN_DebugLog(MSN_LOG_DEBUG,"Connected");
	}

	if(info->type==SERVER_DISPATCH || info->type==SERVER_NOTIFICATION)
		MSN_SendPacket(info->s,"VER","MSNP2");		 //section 7.1
	else if(info->type==SERVER_SWITCHBOARD) {
		DBVARIANT dbv;
		Switchboards_ChangeStatus(SBSTATUS_AUTHENTICATING);
		DBGetContactSetting(NULL,MSNPROTONAME,"e-mail",&dbv);
		MSN_SendPacket(info->s,info->caller?"USR":"ANS","%s %s",dbv.pszVal,info->cookie);
		//for 'ANS'/callee the cookie contains two parameters. See section 8.4
		DBFreeVariant(&dbv);
	}
	if(info->type==SERVER_NOTIFICATION) {
		msnNSSocket=info->s;
		InterlockedIncrement((PLONG)&msnLoggedIn);
	}

	MSN_DebugLog(MSN_LOG_DEBUG,"Entering main recv loop");
	info->bytesInData=0;
	for(;;) {
		int recvResult;
		recvResult=MSN_WS_Recv(info->s,info->data+info->bytesInData,sizeof(info->data)-info->bytesInData);
		if(!recvResult) break;
		info->bytesInData+=recvResult;
		//pull off each line for parsing
		for(;;) {
			char *peol;
			char msg[sizeof(info->data)];
			int handlerResult;

			peol=strchr(info->data,'\r');
			if(peol==NULL) break;
			if(info->bytesInData<peol-info->data+2) break;  //wait for full line end
			memcpy(msg,info->data,peol-info->data); msg[peol-info->data]=0;
			if(peol[1]!='\n')
				MSN_DebugLog(MSN_LOG_WARNING,"Dodgy line ending to command: ignoring");
			info->bytesInData-=(peol-info->data)+1+(peol[1]=='\n');
			memmove(info->data,peol+1+(peol[1]=='\n'),info->bytesInData);

			MSN_DebugLog(MSN_LOG_PACKETS,"RECV:%s",msg);
			if(!isalnum(msg[0]) || !isalnum(msg[1]) || !isalnum(msg[2]) || (msg[3] && msg[3]!=' ')) {
				MSN_DebugLog(MSN_LOG_ERROR,"Invalid command name");
				continue;
			}
			if(isdigit(msg[0]) && isdigit(msg[1]) && isdigit(msg[2]))   //all error messages
				handlerResult=MSN_HandleErrors(info,msg);
			else
				handlerResult=MSN_HandleCommands(info,msg);
			if(handlerResult) break;
		}
		if(info->bytesInData==sizeof(info->data)) {
			MSN_DebugLog(MSN_LOG_FATAL,"sizeof(data) is too small: longest line won't fit");
			break;
		}
	}
	if(info->type==SERVER_NOTIFICATION) {
		if(InterlockedDecrement((PLONG)&msnLoggedIn)==0) {
			CmdQueue_AddProtoAck(NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)msnStatusMode,ID_STATUS_OFFLINE);
			msnStatusMode=ID_STATUS_OFFLINE;
		}
	}
	else if(info->type==SERVER_SWITCHBOARD) Switchboards_Delete();
	closesocket(info->s);
	free(info);
	MSN_DebugLog(MSN_LOG_DEBUG,"Thread ending now");
}
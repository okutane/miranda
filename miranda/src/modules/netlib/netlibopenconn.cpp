/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
#include "netlib.h"

extern CRITICAL_SECTION csNetlibUser;
extern HANDLE hConnectionOpenMutex;
extern DWORD g_LastConnectionTick;
extern int connectionTimeout;
static int iUPnPCleanup = 0;

#define RECV_DEFAULT_TIMEOUT	60000

//returns in network byte order
DWORD DnsLookup(struct NetlibUser *nlu,const char *szHost)
{
	HOSTENT* host;
	DWORD ip = inet_addr( szHost);
	if ( ip != INADDR_NONE )
		return ip;

	host = gethostbyname( szHost );
	if ( host )
		return *( u_long* )host->h_addr_list[0];

	Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"gethostbyname",WSAGetLastError());
	return 0;
}

int WaitUntilReadable(SOCKET s, DWORD dwTimeout, bool check)
{
	fd_set readfd;
	TIMEVAL tv;

	if (s == INVALID_SOCKET) return SOCKET_ERROR;

	tv.tv_sec  = dwTimeout / 1000;
	tv.tv_usec = (dwTimeout % 1000) * 1000;

	FD_ZERO(&readfd);
	FD_SET(s, &readfd);

	int result = select(0, &readfd, 0, 0, &tv);
	if (result == 0 && !check) SetLastError(ERROR_TIMEOUT);
	return result;
}

int WaitUntilWritable(SOCKET s,DWORD dwTimeout)
{
	fd_set writefd;
	TIMEVAL tv;

	tv.tv_sec = dwTimeout / 1000;
	tv.tv_usec = (dwTimeout % 1000) * 1000;

	FD_ZERO(&writefd);
	FD_SET(s, &writefd);

	switch(select(0, 0, &writefd, 0, &tv)) {
		case 0:
			SetLastError(ERROR_TIMEOUT);
		case SOCKET_ERROR:
			return 0;
	}
	return 1;
}

bool RecvUntilTimeout(struct NetlibConnection *nlc, char *buf, int len, int flags, DWORD dwTimeout)
{
	int nReceived = 0;
	DWORD dwTimeNow, dwCompleteTime = GetTickCount() + dwTimeout;

	while ((dwTimeNow = GetTickCount()) < dwCompleteTime) 
	{
		if (WaitUntilReadable(nlc->s, dwCompleteTime - dwTimeNow) <= 0) return false; 
		nReceived = NLRecv(nlc, buf, len, flags);
		if (nReceived <= 0) return false;

		buf += nReceived;
		len -= nReceived;
		if (len <= 0) return true;
	}
	SetLastError( ERROR_TIMEOUT );
	return false;
}

static int NetlibInitSocks4Connection(struct NetlibConnection *nlc,struct NetlibUser *nlu,NETLIBOPENCONNECTION *nloc)
{	//http://www.socks.nec.com/protocol/socks4.protocol and http://www.socks.nec.com/protocol/socks4a.protocol
	PBYTE pInit;
	int nUserLen,nHostLen,len;
	BYTE reply[8];

	nUserLen=lstrlenA(nlu->settings.szProxyAuthUser);
	nHostLen=lstrlenA(nloc->szHost);
	pInit=(PBYTE)mir_alloc(10+nUserLen+nHostLen);
	pInit[0]=4;   //SOCKS4
	pInit[1]=1;   //connect
	*(PWORD)(pInit+2)=htons(nloc->wPort);
	if(nlu->settings.szProxyAuthUser==NULL) pInit[8]=0;
	else lstrcpyA((char*)pInit+8,nlu->settings.szProxyAuthUser);
	if(nlu->settings.dnsThroughProxy) {
		if((*(PDWORD)(pInit+4)=DnsLookup(nlu,nloc->szHost))==0) {
			// FIXME: very suspect code :)
			*(PDWORD)(pInit+4)=0x01000000;
			lstrcpyA((char*)pInit+9+nUserLen,nloc->szHost);
			len=10+nUserLen+nHostLen;
		}
		else len=9+nUserLen;
	}
	else {
		*(PDWORD)(pInit+4)=DnsLookup(nlu,nloc->szHost);
		if(*(PDWORD)(pInit+4)==0) {
			mir_free(pInit);
			return 0;
		}
		len=9+nUserLen;
	}
	if(NLSend(nlc,(char*)pInit,len,MSG_DUMPPROXY)==SOCKET_ERROR) {
		Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"NLSend",GetLastError());
		mir_free(pInit);
		return 0;
	}
	mir_free(pInit);

	if (!RecvUntilTimeout(nlc,(char*)reply,SIZEOF(reply),MSG_DUMPPROXY,RECV_DEFAULT_TIMEOUT)) {
		Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"RecvUntilTimeout",GetLastError());
		return 0;
	}
	if ( reply[1] != 90 ) {
		switch( reply[1] ) {
			case 91: SetLastError(ERROR_ACCESS_DENIED); break;
			case 92: SetLastError(ERROR_CONNECTION_UNAVAIL); break;
			case 93: SetLastError(ERROR_INVALID_ACCESS); break;
			default: SetLastError(ERROR_INVALID_DATA); break;
		}
		Netlib_Logf(nlu,"%s %d: Proxy connection failed (%u)",__FILE__,__LINE__, GetLastError());
		return 0;
	}
	//connected
	return 1;
}

static int NetlibInitSocks5Connection(struct NetlibConnection *nlc,struct NetlibUser *nlu,NETLIBOPENCONNECTION *nloc)
{	//rfc1928
	BYTE buf[258];

	buf[0]=5;  //yep, socks5
	buf[1]=1;  //one auth method
	buf[2]=nlu->settings.useProxyAuth?2:0;
	if(NLSend(nlc,(char*)buf,3,MSG_DUMPPROXY)==SOCKET_ERROR) {
		Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"NLSend",GetLastError());
		return 0;
	}

	//confirmation of auth method
	if (!RecvUntilTimeout(nlc,(char*)buf,2,MSG_DUMPPROXY,RECV_DEFAULT_TIMEOUT)) {
		Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"RecvUntilTimeout",GetLastError());
		return 0;
	}
	if((buf[1]!=0 && buf[1]!=2)) {
		SetLastError(ERROR_INVALID_ID_AUTHORITY);
		Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"NLRecv",GetLastError());
		return 0;
	}

	if(buf[1]==2) {		//rfc1929
		int nUserLen,nPassLen;
		PBYTE pAuthBuf;

		nUserLen=lstrlenA(nlu->settings.szProxyAuthUser);
		nPassLen=lstrlenA(nlu->settings.szProxyAuthPassword);
		pAuthBuf=(PBYTE)mir_alloc(3+nUserLen+nPassLen);
		pAuthBuf[0]=1;		//auth version
		pAuthBuf[1]=nUserLen;
		memcpy(pAuthBuf+2,nlu->settings.szProxyAuthUser,nUserLen);
		pAuthBuf[2+nUserLen]=nPassLen;
		memcpy(pAuthBuf+3+nUserLen,nlu->settings.szProxyAuthPassword,nPassLen);
		if(NLSend(nlc,(char*)pAuthBuf,3+nUserLen+nPassLen,MSG_DUMPPROXY)==SOCKET_ERROR) {
			Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"NLSend",GetLastError());
			mir_free(pAuthBuf);
			return 0;
		}
		mir_free(pAuthBuf);

		if (!RecvUntilTimeout(nlc,(char*)buf,2,MSG_DUMPPROXY,RECV_DEFAULT_TIMEOUT)) {
			Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"RecvUntilTimeout",GetLastError());
			return 0;
		}
		if(buf[1]) {
			SetLastError(ERROR_ACCESS_DENIED);
			Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"RecvUntilTimeout",GetLastError());
			return 0;
		}
	}

	{	PBYTE pInit;
		int nHostLen;
		DWORD hostIP;

		if(nlu->settings.dnsThroughProxy) {
			if((hostIP=inet_addr(nloc->szHost))==INADDR_NONE)
				nHostLen=lstrlenA(nloc->szHost)+1;
			else nHostLen=4;
		}
		else {
			if((hostIP=DnsLookup(nlu,nloc->szHost))==0)
				return 0;
			nHostLen=4;
		}
		pInit=(PBYTE)mir_alloc(6+nHostLen);
		pInit[0]=5;   //SOCKS5
		pInit[1]= nloc->flags & NLOCF_UDP ? 3 : 1; //connect or UDP
		pInit[2]=0;   //reserved
		if(hostIP==INADDR_NONE) {		 //DNS lookup through proxy
			pInit[3]=3;
			pInit[4]=nHostLen-1;
			memcpy(pInit+5,nloc->szHost,nHostLen-1);
		}
		else {
			pInit[3]=1;
			*(PDWORD)(pInit+4)=hostIP;
		}
		*(PWORD)(pInit+4+nHostLen)=htons(nloc->wPort);
		if(NLSend(nlc,(char*)pInit,6+nHostLen,MSG_DUMPPROXY)==SOCKET_ERROR) {
			Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"NLSend",GetLastError());
			mir_free(pInit);
			return 0;
		}
		mir_free(pInit);
	}

	if (!RecvUntilTimeout(nlc,(char*)buf,5,MSG_DUMPPROXY,RECV_DEFAULT_TIMEOUT)) {
		Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"RecvUntilTimeout",GetLastError());
		return 0;
	}

	if ( buf[0]!=5 || buf[1] ) {
		const char* err = "Unknown response"; 
		if ( buf[0] != 5 )
			SetLastError(ERROR_BAD_FORMAT);
		else 
			switch(buf[1]) {
				case 1: SetLastError(ERROR_GEN_FAILURE); err = "General failure"; break;
				case 2: SetLastError(ERROR_ACCESS_DENIED); err = "Connection not allowed by ruleset";  break;
				case 3: SetLastError(WSAENETUNREACH); err = "Network unreachable"; break;
				case 4: SetLastError(WSAEHOSTUNREACH); err = "Host unreachable"; break;
				case 5: SetLastError(WSAECONNREFUSED); err = "Connection refused by destination host"; break;
				case 6: SetLastError(WSAETIMEDOUT); err = "TTL expired"; break;
				case 7: SetLastError(ERROR_CALL_NOT_IMPLEMENTED); err = "Command not supported / protocol error"; break;
				case 8: SetLastError(ERROR_INVALID_ADDRESS); err = "Address type not supported"; break;
				default: SetLastError(ERROR_INVALID_DATA); break;
			}
		Netlib_Logf(nlu,"%s %d: Proxy conection failed. %s.",__FILE__,__LINE__, err);
		return 0;
	}
	{
		int nRecvSize = 0;
		switch( buf[3] ) {
		case 1:// ipv4 addr
			nRecvSize = 5;
			break;
		case 3:// dns name addr
			nRecvSize = buf[4] + 2;
			break;
		case 4:// ipv6 addr
			nRecvSize = 17;
			break;
		default:
			Netlib_Logf(nlu,"%s %d: %s() unknown address type (%u)",__FILE__,__LINE__,"NetlibInitSocks5Connection",(int)buf[3]);
			return 0;
		}
		if (!RecvUntilTimeout(nlc,(char*)buf,nRecvSize,MSG_DUMPPROXY,RECV_DEFAULT_TIMEOUT)) {
			Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"RecvUntilTimeout",GetLastError());
			return 0;
	}	}

	//connected
	return 1;
}

static int NetlibInitHttpsConnection(struct NetlibConnection *nlc,struct NetlibUser *nlu,NETLIBOPENCONNECTION *nloc)
{	//rfc2817
	NETLIBHTTPREQUEST nlhrSend = {0}, *nlhrReply;
	char szUrl[512];

	nlhrSend.cbSize = sizeof(nlhrSend);
	nlhrSend.requestType = REQUEST_CONNECT;
	nlhrSend.flags = NLHRF_GENERATEHOST | NLHRF_DUMPPROXY | NLHRF_SMARTAUTHHEADER | NLHRF_HTTP11 | NLHRF_NOPROXY;
	if (nlu->settings.dnsThroughProxy) 
	{
		mir_snprintf(szUrl, SIZEOF(szUrl), "%s:%u", nloc->szHost, nloc->wPort);
	}
	else 
	{
		IN_ADDR addr;
		DWORD ip = DnsLookup(nlu, nloc->szHost);
		if (ip == 0) return 0;
		addr.S_un.S_addr = ip;
		mir_snprintf(szUrl, SIZEOF(szUrl), "%s:%u", inet_ntoa(addr), nloc->wPort);
	}
	nlhrSend.szUrl = szUrl;

	nlc->usingHttpGateway = true;

	if (NetlibHttpSendRequest((WPARAM)nlc,(LPARAM)&nlhrSend) == SOCKET_ERROR)
	{
		nlc->usingHttpGateway = false;
		return 0;
	}
	nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpRecvHeaders((WPARAM)nlc, MSG_DUMPPROXY | MSG_RAW);
	nlc->usingHttpGateway = false;
	if (nlhrReply == NULL) return 0;
	if (nlhrReply->resultCode < 200 || nlhrReply->resultCode >= 300)
	{
		NetlibHttpSetLastErrorUsingHttpResult(nlhrReply->resultCode);
		Netlib_Logf(nlu,"%s %d: %s request failed (%u %s)",__FILE__,__LINE__,nlu->settings.proxyType==PROXYTYPE_HTTP?"HTTP":"HTTPS",nlhrReply->resultCode,nlhrReply->szResultDescr);
		NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
		return 0;
	}
	NetlibHttpFreeRequestStruct(0, (LPARAM)nlhrReply);
	//connected
	return 1;
}

static void FreePartiallyInitedConnection(struct NetlibConnection *nlc)
{
	DWORD dwOriginalLastError=GetLastError();

	if (nlc->s!=INVALID_SOCKET) closesocket(nlc->s);
	mir_free(nlc->nlhpi.szHttpPostUrl);
	mir_free(nlc->nlhpi.szHttpGetUrl);
	mir_free((char*)nlc->nloc.szHost);
	NetlibDeleteNestedCS(&nlc->ncsSend);
	NetlibDeleteNestedCS(&nlc->ncsRecv);
	CloseHandle(nlc->hOkToCloseEvent);
	DeleteCriticalSection(&nlc->csHttpSequenceNums);
	mir_free(nlc);
	SetLastError(dwOriginalLastError);
}


static int my_connect(NetlibConnection *nlc, NETLIBOPENCONNECTION * nloc)
{
	int rc=0, retrycnt=0;
	unsigned int dwTimeout=( nloc->cbSize==sizeof(NETLIBOPENCONNECTION) && nloc->flags&NLOCF_V2 ) ? nloc->timeout : 0;
	u_long notblocking=1;	
	TIMEVAL tv;
	DWORD lasterr = 0;	
	int waitdiff;
	// if dwTimeout is zero then its an old style connection or new with a 0 timeout, select() will error quicker anyway
	if ( dwTimeout == 0 )
		dwTimeout += 60;

	// this is for XP SP2 where there is a default connection attempt limit of 10/second
	if (connectionTimeout)
	{
		WaitForSingleObject(hConnectionOpenMutex, 10000);
		waitdiff = GetTickCount() - g_LastConnectionTick;
		if (waitdiff < connectionTimeout) SleepEx(connectionTimeout, TRUE);
		g_LastConnectionTick = GetTickCount();
		ReleaseMutex(hConnectionOpenMutex);
	}

	// might of died in between the wait
	if ( Miranda_Terminated() )  {
		rc=SOCKET_ERROR;
		lasterr=ERROR_TIMEOUT;
		goto unblock;
	}

retry:
	nlc->s=socket(AF_INET,nloc->flags & NLOCF_UDP ? SOCK_DGRAM : SOCK_STREAM, 0);
	if (nlc->s == INVALID_SOCKET) return SOCKET_ERROR;

	// return the socket to non blocking
	if ( ioctlsocket(nlc->s, FIONBIO, &notblocking) != 0 ) return SOCKET_ERROR;

	if (nlc->nlu->settings.specifyOutgoingPorts && nlc->nlu->settings.szOutgoingPorts) 
	{
		if (!BindSocketToPort(nlc->nlu->settings.szOutgoingPorts, nlc->s, &nlc->nlu->inportnum))
			Netlib_Logf(nlc->nlu,"Netlib connect: Not enough ports for outgoing connections specified");
	} 

	// try a connect
	if ( connect(nlc->s, (SOCKADDR*)&nlc->sinProxy, sizeof(nlc->sinProxy)) == 0 ) {
		goto unblock;
	}

	// didn't work, was it cos of nonblocking?
	if ( WSAGetLastError() != WSAEWOULDBLOCK ) {
		rc=SOCKET_ERROR;
		goto unblock;
	}
	// setup select()
	tv.tv_sec=1;
	tv.tv_usec=0;
	for (;;) {		
		fd_set r, w, e;
		FD_ZERO(&r); FD_ZERO(&w); FD_ZERO(&e);
		FD_SET(nlc->s, &r);
		FD_SET(nlc->s, &w);
		FD_SET(nlc->s, &e);		
		if ( (rc=select(0, &r, &w, &e, &tv)) == SOCKET_ERROR ) {
			break;
		}			
		if ( rc > 0 ) {			
			if ( FD_ISSET(nlc->s, &r) ) {
				// connection was closed
				rc=SOCKET_ERROR;
				lasterr=WSAECONNRESET;
			}
			if ( FD_ISSET(nlc->s, &w) ) {
				// connection was successful
				rc=0;
			}
			if ( FD_ISSET(nlc->s, &e) ) {
				// connection failed.
				int len=sizeof(lasterr);
				rc=SOCKET_ERROR;
				getsockopt(nlc->s,SOL_SOCKET,SO_ERROR,(char*)&lasterr,&len);
				if (lasterr == WSAEADDRINUSE && ++retrycnt <= 2) 
				{
					closesocket(nlc->s);
					goto retry;
				}
			}
			goto unblock;
		} else if ( Miranda_Terminated() ) {
			rc=SOCKET_ERROR;
			lasterr=ERROR_TIMEOUT;
			goto unblock;
		} else if ( nloc->cbSize==sizeof(NETLIBOPENCONNECTION) && nloc->flags&NLOCF_V2 && nloc->waitcallback != NULL 
			&& nloc->waitcallback(&dwTimeout) == 0) {
			rc=SOCKET_ERROR;
			lasterr=ERROR_TIMEOUT;
			goto unblock;
		}
		if ( --dwTimeout == 0 ) {
			rc=SOCKET_ERROR;
			lasterr=ERROR_TIMEOUT;
			goto unblock;
		}
	}
unblock:	
	notblocking=0;
	ioctlsocket(nlc->s, FIONBIO, &notblocking);
	if (lasterr) SetLastError(lasterr);
	return rc;
}

bool NetlibDoConnect(NetlibConnection *nlc)
{
	NETLIBOPENCONNECTION *nloc = &nlc->nloc;
	NetlibUser *nlu = nlc->nlu;
	bool usingProxy = nlu->settings.useProxy && nlu->settings.szProxyServer && nlu->settings.szProxyServer[0];

	nlc->proxyAuthNeeded = true;
	nlc->sinProxy.sin_family = AF_INET;
	if (usingProxy) 
	{
		nlc->sinProxy.sin_port = htons(nlu->settings.wProxyPort);
		nlc->sinProxy.sin_addr.S_un.S_addr = DnsLookup(nlu, nlu->settings.szProxyServer);
	}
	else 
	{
		nlc->sinProxy.sin_port = htons(nloc->wPort);
		nlc->sinProxy.sin_addr.S_un.S_addr = DnsLookup(nlu, nloc->szHost);
	}

	if (nlc->sinProxy.sin_addr.S_un.S_addr == 0) return false;

	if (my_connect(nlc, nloc)==SOCKET_ERROR) 
	{
		Netlib_Logf(nlu,"%s %d: %s() failed (%u)",__FILE__,__LINE__,"connect",WSAGetLastError());
		return false;
	}

	if (usingProxy && !((nloc->flags & (NLOCF_HTTP | NLOCF_SSL)) == NLOCF_HTTP && 
		(nlu->settings.proxyType == PROXYTYPE_HTTP || nlu->settings.proxyType == PROXYTYPE_HTTPS)))
	{
		if (!WaitUntilWritable(nlc->s,30000)) return false;

		switch(nlu->settings.proxyType) 
		{
			case PROXYTYPE_SOCKS4:
				if (!NetlibInitSocks4Connection(nlc, nlu, nloc)) return false;
				break;

			case PROXYTYPE_SOCKS5:
				if (!NetlibInitSocks5Connection(nlc, nlu, nloc)) return false;
				break;

			case PROXYTYPE_HTTPS:
				if (!NetlibInitHttpsConnection(nlc, nlu, nloc)) return false;
				break;

			case PROXYTYPE_HTTP:
				if (!(nlu->user.flags & NUF_HTTPGATEWAY || nloc->flags & NLOCF_HTTPGATEWAY) || nloc->flags & NLOCF_SSL)
				{
					//NLOCF_HTTP not specified and no HTTP gateway available: try HTTPS
					if (!NetlibInitHttpsConnection(nlc, nlu, nloc))
					{
						//can't do HTTPS: try direct
						closesocket(nlc->s);

						nlc->sinProxy.sin_family = AF_INET;
						nlc->sinProxy.sin_port = htons(nloc->wPort);
						nlc->sinProxy.sin_addr.S_un.S_addr = DnsLookup(nlu, nloc->szHost);
						if(nlc->sinProxy.sin_addr.S_un.S_addr == 0 || my_connect(nlc, nloc) == SOCKET_ERROR) 
						{
							if (nlc->sinProxy.sin_addr.S_un.S_addr)
								Netlib_Logf(nlu, "%s %d: %s() failed (%u)", __FILE__, __LINE__, "connect", WSAGetLastError());
							return false;
						}
					}
				}
				else if (!NetlibInitHttpConnection(nlc, nlu, nloc)) return false;
				break;

			default:
				SetLastError(ERROR_INVALID_PARAMETER);
				FreePartiallyInitedConnection(nlc);
				return false;
		}
	}
	else if (nloc->flags & NLOCF_HTTPGATEWAY)
	{
		if (!NetlibInitHttpConnection(nlc, nlu, nloc)) return false;
		nlc->usingDirectHttpGateway = true;
	}

	if (NLOCF_SSL & nloc->flags)
	{
		Netlib_Logf(nlc->nlu, "(%d) Connected to %s:%d, Starting SSL negotiation", nlc->s, nloc->szHost, nloc->wPort);

		nlc->hSsl = si.connect(nlc->s, nloc->szHost, nlc->nlu->settings.validateSSL);
		if (nlc->hSsl == NULL)
		{
			Netlib_Logf(nlc->nlu, "(%d %s) Failure to negotiate SSL connection", nlc->s, nloc->szHost);
			return false;
		}
	}

	Netlib_Logf(nlu,"(%d) Connected to %s:%d",nlc->s,nloc->szHost,nloc->wPort);
	return true;
}

bool NetlibReconnect(NetlibConnection *nlc)
{
	char buf[4];
	bool opened;  

	switch (WaitUntilReadable(nlc->s, 0, true))
	{
	case SOCKET_ERROR:
		opened = false;
		break;

	case 0:
		opened = true;
		break;

	case 1:
		opened = recv(nlc->s, buf, 1, MSG_PEEK) > 0;
		break;
	}

	if (!opened)
	{
		if (nlc->hSsl)
		{
			si.sfree(nlc->hSsl);
			nlc->hSsl = NULL;
		}
		closesocket(nlc->s);
		nlc->s = INVALID_SOCKET;

		if (nlc->usingHttpGateway)
			return my_connect(nlc, &nlc->nloc) == 0;
		else
			return NetlibDoConnect(nlc);
	}
	return true;
}

INT_PTR NetlibOpenConnection(WPARAM wParam,LPARAM lParam)
{
	NETLIBOPENCONNECTION *nloc=(NETLIBOPENCONNECTION*)lParam;
	struct NetlibUser *nlu=(struct NetlibUser*)wParam;
	struct NetlibConnection *nlc;

	Netlib_Logf(nlu,"Connecting to %s:%d (Flags %x)....", nloc->szHost, nloc->wPort, nloc->flags);

	EnterCriticalSection(&csNetlibUser);
	if(iUPnPCleanup==0) {
		forkthread(NetlibUPnPCleanup, 0, NULL);
		iUPnPCleanup = 1;
	}
	LeaveCriticalSection(&csNetlibUser);
	if(GetNetlibHandleType(nlu)!=NLH_USER || !(nlu->user.flags&NUF_OUTGOING) || nloc==NULL 
		|| !(nloc->cbSize==NETLIBOPENCONNECTION_V1_SIZE||nloc->cbSize==sizeof(NETLIBOPENCONNECTION)) || nloc->szHost==NULL || nloc->wPort==0) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	nlc=(struct NetlibConnection*)mir_calloc(sizeof(struct NetlibConnection));
	nlc->handleType=NLH_CONNECTION;
	nlc->nlu=nlu;
	nlc->nloc = *nloc;
	nlc->nloc.szHost = mir_strdup(nloc->szHost);
	nlc->s = INVALID_SOCKET;
	nlc->s2 = INVALID_SOCKET;

	InitializeCriticalSection(&nlc->csHttpSequenceNums);
	nlc->hOkToCloseEvent=CreateEvent(NULL,TRUE,TRUE,NULL);
	nlc->dontCloseNow=0;
	NetlibInitializeNestedCS(&nlc->ncsSend);
	NetlibInitializeNestedCS(&nlc->ncsRecv);

	if (!NetlibDoConnect(nlc))
	{
		FreePartiallyInitedConnection(nlc);		
		return 0;
	}

	return (INT_PTR)nlc;
}

INT_PTR NetlibStartSsl(WPARAM wParam,LPARAM lParam)
{
	struct NetlibConnection *nlc = (struct NetlibConnection*)wParam;
	NETLIBSSL *sp = (NETLIBSSL*)lParam;

	nlc->hSsl = si.connect(nlc->s, sp ? sp->host : nlc->nloc.szHost, nlc->nlu->settings.validateSSL);

	if (nlc->hSsl == NULL)
		Netlib_Logf(nlc->nlu,"(%d %s) Failure to negotiate SSL connection",nlc->s, sp ? sp->host : nlc->nloc.szHost);

	return nlc->hSsl != NULL;
}

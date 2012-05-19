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

extern HANDLE hConnectionHeaderMutex,hSendEvent,hRecvEvent;

INT_PTR NetlibSend(WPARAM wParam,LPARAM lParam)
{
	struct NetlibConnection *nlc=(struct NetlibConnection*)wParam;
	NETLIBBUFFER *nlb=(NETLIBBUFFER*)lParam;
	INT_PTR result;

	if ( nlb == NULL ) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	if ( !NetlibEnterNestedCS( nlc, NLNCS_SEND ))
		return SOCKET_ERROR;

	if ( nlc->usingHttpGateway && !( nlb->flags & MSG_RAW )) {
		if ( !( nlb->flags & MSG_NOHTTPGATEWAYWRAP ) && nlc->nlu->user.pfnHttpGatewayWrapSend ) {
			NetlibDumpData( nlc, ( PBYTE )nlb->buf, nlb->len, 1, nlb->flags );
			result = nlc->nlu->user.pfnHttpGatewayWrapSend(( HANDLE )nlc, ( PBYTE )nlb->buf, nlb->len, nlb->flags | MSG_NOHTTPGATEWAYWRAP, NetlibSend );
		}
		else result = NetlibHttpGatewayPost( nlc, nlb->buf, nlb->len, nlb->flags );
	}
	else {
		NetlibDumpData( nlc, ( PBYTE )nlb->buf, nlb->len, 1, nlb->flags );
		if (nlc->hSsl)
			result = si.write( nlc->hSsl, nlb->buf, nlb->len );
		else
			result = send( nlc->s, nlb->buf, nlb->len, nlb->flags & 0xFFFF );
	}
	NetlibLeaveNestedCS( &nlc->ncsSend );

	if ((( THook* )hSendEvent)->subscriberCount ) {
		NETLIBNOTIFY nln = { nlb, result };
		CallHookSubscribers( hSendEvent, (WPARAM)&nln, (LPARAM)&nlc->nlu->user );
	}
	return result;
}

INT_PTR NetlibRecv(WPARAM wParam,LPARAM lParam)
{
	struct NetlibConnection *nlc = (struct NetlibConnection*)wParam;
	NETLIBBUFFER* nlb = ( NETLIBBUFFER* )lParam;
	int recvResult;

	if ( nlb == NULL ) {
		SetLastError( ERROR_INVALID_PARAMETER );
		return SOCKET_ERROR;
	}

	if ( !NetlibEnterNestedCS( nlc, NLNCS_RECV ))
		return SOCKET_ERROR;

	if ( nlc->usingHttpGateway && !( nlb->flags & MSG_RAW ))
		recvResult = NetlibHttpGatewayRecv( nlc, nlb->buf, nlb->len, nlb->flags );
	else
	{
		if (nlc->hSsl)
			recvResult = si.read( nlc->hSsl, nlb->buf, nlb->len, (nlb->flags & MSG_PEEK) != 0 );
		else
			recvResult = recv( nlc->s, nlb->buf, nlb->len, nlb->flags & 0xFFFF );
	}
	NetlibLeaveNestedCS( &nlc->ncsRecv );
	if (recvResult <= 0) 
		return recvResult;

	NetlibDumpData(nlc, (PBYTE)nlb->buf, recvResult, 0, nlb->flags);

	if ((nlb->flags & MSG_PEEK) == 0 && ((THook*)hRecvEvent)->subscriberCount)
	{
		NETLIBNOTIFY nln = { nlb, recvResult };
		CallHookSubscribers(hRecvEvent, (WPARAM)&nln, (LPARAM)&nlc->nlu->user);
	}
	return recvResult;
}

static int ConnectionListToSocketList(HANDLE *hConns, fd_set *fd, int& pending)
{
	struct NetlibConnection *nlcCheck;
	int i;

	FD_ZERO(fd);
	for(i=0;hConns[i] && hConns[i]!=INVALID_HANDLE_VALUE && i<FD_SETSIZE;i++) {
		nlcCheck=(struct NetlibConnection*)hConns[i];
		if (nlcCheck->handleType!=NLH_CONNECTION && nlcCheck->handleType!=NLH_BOUNDPORT) {
			SetLastError(ERROR_INVALID_DATA);
			return 0;
		}
		FD_SET(nlcCheck->s,fd);
		if ( si.pending( nlcCheck->hSsl ))
			pending++;
	}
	return 1;
}

INT_PTR NetlibSelect(WPARAM,LPARAM lParam)
{
	NETLIBSELECT *nls=(NETLIBSELECT*)lParam;
	if (nls==NULL || nls->cbSize!=sizeof(NETLIBSELECT)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	TIMEVAL tv;
	tv.tv_sec=nls->dwTimeout/1000;
	tv.tv_usec=(nls->dwTimeout%1000)*1000;

	int pending = 0;
	fd_set readfd, writefd, exceptfd;
	WaitForSingleObject(hConnectionHeaderMutex,INFINITE);
	if (!ConnectionListToSocketList(nls->hReadConns,&readfd,pending)
	   || !ConnectionListToSocketList(nls->hWriteConns,&writefd,pending)
	   || !ConnectionListToSocketList(nls->hExceptConns,&exceptfd,pending)) {
		ReleaseMutex(hConnectionHeaderMutex);
		return SOCKET_ERROR;
	}
	ReleaseMutex(hConnectionHeaderMutex);
	if (pending)
		return 1;

	return select(0,&readfd,&writefd,&exceptfd,nls->dwTimeout==INFINITE?NULL:&tv);
}

INT_PTR NetlibSelectEx(WPARAM, LPARAM lParam)
{
	NETLIBSELECTEX *nls=(NETLIBSELECTEX*)lParam;
	if (nls==NULL || nls->cbSize!=sizeof(NETLIBSELECTEX)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}

	TIMEVAL tv;
	tv.tv_sec=nls->dwTimeout/1000;
	tv.tv_usec=(nls->dwTimeout%1000)*1000;
	WaitForSingleObject(hConnectionHeaderMutex,INFINITE);

	int pending = 0;
	fd_set readfd,writefd,exceptfd;
	if (!ConnectionListToSocketList(nls->hReadConns,&readfd,pending)
	   || !ConnectionListToSocketList(nls->hWriteConns,&writefd,pending)
	   || !ConnectionListToSocketList(nls->hExceptConns,&exceptfd,pending)) {
		ReleaseMutex(hConnectionHeaderMutex);
		return SOCKET_ERROR;
	}
	ReleaseMutex(hConnectionHeaderMutex);

	int rc = (pending) ? pending : select(0,&readfd,&writefd,&exceptfd,nls->dwTimeout==INFINITE?NULL:&tv);

	WaitForSingleObject(hConnectionHeaderMutex,INFINITE);
	/* go thru each passed HCONN array and grab its socket handle, then give it to FD_ISSET()
	to see if an event happened for that socket, if it has it will be returned as TRUE (otherwise not)
	This happens for read/write/except */
	struct NetlibConnection *conn=NULL;
	int j;
	for (j=0; j<FD_SETSIZE; j++) {
		conn=(struct NetlibConnection*)nls->hReadConns[j];
		if (conn==NULL || conn==INVALID_HANDLE_VALUE) break;

		if (si.pending(conn->hSsl))
			nls->hReadStatus[j] = TRUE;
		if (conn->usingHttpGateway && conn->nlhpi.szHttpGetUrl == NULL && conn->dataBuffer == NULL)
			nls->hReadStatus[j] = (conn->pHttpProxyPacketQueue != NULL);
		else
			nls->hReadStatus[j] = FD_ISSET(conn->s,&readfd);
	}
	for (j=0; j<FD_SETSIZE; j++) {
		conn=(struct NetlibConnection*)nls->hWriteConns[j];
		if (conn==NULL || conn==INVALID_HANDLE_VALUE) break;
		nls->hWriteStatus[j] = FD_ISSET(conn->s,&writefd);
	}
	for (j=0; j<FD_SETSIZE; j++) {
		conn=(struct NetlibConnection*)nls->hExceptConns[j];
		if (conn==NULL || conn==INVALID_HANDLE_VALUE) break;
		nls->hExceptStatus[j] = FD_ISSET(conn->s,&exceptfd);
	}
	ReleaseMutex(hConnectionHeaderMutex);
	return rc;
}

bool NetlibStringToAddress(const char* str, PSOCKADDR_INET addr)
{
	if (!str) return false;

	if (MyWSAStringToAddress)
	{
		int len = sizeof(SOCKADDR_INET);
		return !MyWSAStringToAddress((char*)str, AF_INET6, NULL, (LPSOCKADDR)addr, &len);
	}
	else
	{
		unsigned iaddr = inet_addr(str);
		if (!iaddr) return false;

		addr->Ipv4.sin_addr.s_addr = iaddr;
		addr->si_family = AF_INET;
		return true; 
	}
}

char* NetlibAddressToString(PSOCKADDR_INET addr)
{
	char saddr[128];

	if (MyWSAAddressToString)
	{
		DWORD len = sizeof(saddr);
		if (!MyWSAAddressToString((LPSOCKADDR)addr, sizeof(*addr), NULL, saddr, &len))
			return mir_strdup(saddr);
	}
	else if (addr->si_family == AF_INET)
	{
		char *szIp = inet_ntoa(addr->Ipv4.sin_addr);
		if (addr->Ipv4.sin_port != 0)
		{
			mir_snprintf(saddr, sizeof(saddr), "%s:%d", szIp, htons(addr->Ipv4.sin_port));
			return mir_strdup(saddr);
		}
		else 
			return mir_strdup(szIp);
	}
	return NULL;
}

void NetlibGetConnectionInfo(NetlibConnection* nlc, NETLIBCONNINFO *connInfo)
{
	if (!nlc || !connInfo || connInfo->cbSize < sizeof(NETLIBCONNINFO)) return;

	SOCKADDR_INET sin = {0};
	int len = sizeof(sin);

	if (!getsockname(nlc->s, (PSOCKADDR)&sin, &len))
	{
		connInfo->wPort = ntohs(sin.Ipv4.sin_port);
		connInfo->dwIpv4 = sin.si_family == AF_INET ? htonl(sin.Ipv4.sin_addr.s_addr) : 0;

		char *szTmp = NetlibAddressToString(&sin);
		strncpy(connInfo->szIpPort, szTmp, sizeof(connInfo->szIpPort));
		connInfo->szIpPort[sizeof(connInfo->szIpPort) - 1] = 0;
		mir_free(szTmp);
	}
}

static PSOCKADDR_INET GetMyIpv6(void)
{
	addrinfo *air = NULL, *ai;
	const char *szMyHost = ""; 

	if (MyGetaddrinfo(szMyHost, NULL, NULL, &air))
		return NULL;

	unsigned n = 0;
	for (ai = air; ai; ai = ai->ai_next)
	{
		if (ai->ai_family == AF_INET || ai->ai_family == AF_INET6)
			++n;
	}

	PSOCKADDR_INET addr = (PSOCKADDR_INET)mir_calloc((n + 1) * sizeof(SOCKADDR_INET));

	unsigned i = 0;
	for (ai = air; ai; ai = ai->ai_next)
	{
		if (ai->ai_family == AF_INET)
		{
			addr[i].si_family = AF_INET;
			addr[i].Ipv4 = *(PSOCKADDR_IN)ai->ai_addr;
			++i;
		}
		else if (ai->ai_family == AF_INET6)
		{
			addr[i].si_family = AF_INET6;
			addr[i].Ipv6 = *(PSOCKADDR_IN6)ai->ai_addr;
			++i;
		}
	}
	MyFreeaddrinfo(air);
	return addr;
}

static PSOCKADDR_INET GetMyIpv4(void)
{
	char hostname[256] = "";

	gethostname(hostname, sizeof(hostname));
	PHOSTENT he = gethostbyname(hostname);

	unsigned n;
	for (n = 0; he->h_addr_list[n]; ++n) ;

	PSOCKADDR_INET addr = (PSOCKADDR_INET)mir_calloc((n + 1) * sizeof(SOCKADDR_INET));

	for (unsigned i = 0; i < n; ++i) 
	{
		addr[i].si_family     = AF_INET;
		addr[i].Ipv4.sin_addr = *(PIN_ADDR)he->h_addr_list[i];
	}

	return addr;
}

PSOCKADDR_INET GetMyIp(void)
{
	return (MyGetaddrinfo && MyFreeaddrinfo) ? GetMyIpv6() : GetMyIpv4();
}

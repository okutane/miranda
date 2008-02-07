/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-08  George Hazan

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

BOOL CJabberProto::WsInit( void )
{
	NETLIBUSER nlu = {0};
	char name[128];

	sprintf( name, "%s %s", m_szModuleName, JTranslate( "connection" ));

	nlu.cbSize = sizeof( nlu );
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;	// | NUF_HTTPGATEWAY;
	nlu.szDescriptiveName = name;
	nlu.szSettingsModule = m_szProtoName;
	//nlu.szHttpGatewayHello = "http://http.proxy.icq.com/hello";
	//nlu.szHttpGatewayUserAgent = "Mozilla/4.08 [en] ( WinNT; U ;Nav )";
	//nlu.pfnHttpGatewayInit = JabberHttpGatewayInit;
	//nlu.pfnHttpGatewayBegin = JabberHttpGatewayBegin;
	//nlu.pfnHttpGatewayWrapSend = JabberHttpGatewayWrapSend;
	//nlu.pfnHttpGatewayUnwrapRecv = JabberHttpGatewayUnwrapRecv;
	m_hNetlibUser = ( HANDLE ) JCallService( MS_NETLIB_REGISTERUSER, 0, ( LPARAM )&nlu );

	return ( m_hNetlibUser!=NULL )?TRUE:FALSE;
}

void CJabberProto::WsUninit( void )
{
	Netlib_CloseHandle( m_hNetlibUser );
	m_hNetlibUser = NULL;
}

JABBER_SOCKET CJabberProto::WsConnect( char* host, WORD port )
{
	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.cbSize = sizeof( nloc );
	nloc.szHost = host;
	nloc.wPort = port;
	return ( HANDLE )JCallService( MS_NETLIB_OPENCONNECTION, ( WPARAM ) m_hNetlibUser, ( LPARAM )&nloc );
}

int CJabberProto::WsSend( JABBER_SOCKET hConn, char* data, int datalen, int flags )
{
	int len;

	if (( len = Netlib_Send( hConn, data, datalen, flags )) == SOCKET_ERROR || len != datalen ) {
		Log( "Netlib_Send() failed, error=%d", WSAGetLastError());
		return SOCKET_ERROR;
	}
	return len;
}

int CJabberProto::WsRecv( JABBER_SOCKET hConn, char* data, long datalen, int flags )
{
	int ret;

	ret = Netlib_Recv( hConn, data, datalen, flags );
	if ( ret == SOCKET_ERROR ) {
		Log( "Netlib_Recv() failed, error=%d", WSAGetLastError());
		return 0;
	}
	if ( ret == 0 ) {
		Log( "Connection closed gracefully" );
		return 0;
	}
	return ret;
}

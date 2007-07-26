/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan
Copyright ( C ) 2007     Maxim Mluhov

XEP-0146 support for Miranda IM

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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_privacy.cpp,v $
Revision       : $Revision: 5337 $
Last change on : $Date: 2007-04-28 13:26:31 +0300 (��, 28 ��� 2007) $
Last change by : $Author: ghazan $

*/

#ifndef _JABBER_RC_H_
#define _JABBER_RC_H_

class CJabberAdhocSession;

void JabberHandleAdhocCommandRequest( XmlNode* iqNode, void* userdata, CJabberIqInfo* pInfo );
int JabberAdhocSetStatusHandler( XmlNode *iqNode, void *usedata, CJabberIqInfo* pInfo, CJabberAdhocSession* pSession );
int JabberAdhocOptionsHandler( XmlNode *iqNode, void *usedata, CJabberIqInfo* pInfo, CJabberAdhocSession* pSession );
int JabberAdhocForwardHandler( XmlNode *iqNode, void *usedata, CJabberIqInfo* pInfo, CJabberAdhocSession* pSession );

#define JABBER_ADHOC_HANDLER_STATUS_EXECUTING            1
#define JABBER_ADHOC_HANDLER_STATUS_COMPLETED            2
#define JABBER_ADHOC_HANDLER_STATUS_CANCEL               3
#define JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION       4
typedef int ( *JABBER_ADHOC_HANDLER )( XmlNode *iqNode, void *usedata, CJabberIqInfo* pInfo, CJabberAdhocSession* pSession );

// 5 minutes to fill out form :)
#define JABBER_ADHOC_SESSION_EXPIRE_TIME                 300000

class CJabberAdhocSession
{
protected:
	TCHAR* m_szSessionId;
	CJabberAdhocSession* m_pNext;
	DWORD m_dwStartTime;

	void* m_pUserData;
	BOOL m_bAutofreeUserData;

	DWORD m_dwStage;
public:
	CJabberAdhocSession()
	{
		ZeroMemory( this, sizeof(CJabberAdhocSession) );
		TCHAR szId[ 128 ];
		mir_sntprintf( szId, SIZEOF(szId), _T("%u%u"), JabberSerialNext(), GetTickCount() );
		m_szSessionId = mir_tstrdup( szId );
		m_dwStartTime = GetTickCount();
	}
	~CJabberAdhocSession()
	{
		if ( m_szSessionId )
			mir_free( m_szSessionId );
		if ( m_bAutofreeUserData && m_pUserData )
			mir_free( m_pUserData );
		if ( m_pNext )
			delete m_pNext;
	}
	CJabberAdhocSession* GetNext()
	{
		return m_pNext;
	}
	CJabberAdhocSession* SetNext( CJabberAdhocSession *pNext )
	{
		CJabberAdhocSession *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	DWORD GetSessionStartTime()
	{
		return m_dwStartTime;
	}
	TCHAR* GetSessionId()
	{
		return m_szSessionId;
	}
	BOOL SetUserData( void* pUserData, BOOL bAutofree = FALSE )
	{
		if ( m_bAutofreeUserData && m_pUserData )
			mir_free( m_pUserData );
		m_pUserData = pUserData;
		m_bAutofreeUserData = bAutofree;
		return TRUE;
	}
	DWORD SetStage( DWORD dwStage )
	{
		DWORD dwRetVal = m_dwStage;
		m_dwStage = dwStage;
		return dwRetVal;
	}
	DWORD GetStage()
	{
		return m_dwStage;
	}
};

class CJabberAdhocNode;
class CJabberAdhocNode
{
protected:
	TCHAR* m_szJid;
	TCHAR* m_szNode;
	TCHAR* m_szName;
	CJabberAdhocNode* m_pNext;
	JABBER_ADHOC_HANDLER m_pHandler;
public:
	CJabberAdhocNode( TCHAR* szJid, TCHAR* szNode, TCHAR* szName, JABBER_ADHOC_HANDLER pHandler )
	{
		ZeroMemory( this, sizeof( CJabberAdhocNode ));
		replaceStr( m_szJid, szJid );
		replaceStr( m_szNode, szNode );
		replaceStr( m_szName, szName );
		m_pHandler = pHandler;
	}
	~CJabberAdhocNode()
	{
		if ( m_szJid) 
			mir_free( m_szJid );
		if ( m_szNode )
			mir_free( m_szNode );
		if ( m_szName )
			mir_free( m_szName );
		if ( m_pNext )
			delete m_pNext;
	}
	CJabberAdhocNode* GetNext()
	{
		return m_pNext;
	}
	CJabberAdhocNode* SetNext( CJabberAdhocNode *pNext )
	{
		CJabberAdhocNode *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	TCHAR* GetJid()
	{
		return m_szJid;
	}
	TCHAR* GetNode()
	{
		return m_szNode;
	}
	TCHAR* GetName()
	{
		return m_szName;
	}
	BOOL CallHandler( XmlNode* iqNode, void* usedata, CJabberIqInfo* pInfo, CJabberAdhocSession* pSession )
	{
		if ( !m_pHandler )
			return FALSE;
		return m_pHandler( iqNode, usedata, pInfo, pSession );
	}
};

class CJabberAdhocManager
{
protected:
	CJabberAdhocNode* m_pNodes;
	CJabberAdhocSession* m_pSessions;
	CRITICAL_SECTION m_cs;

	CJabberAdhocSession* FindSession( TCHAR* szSession )
	{
		CJabberAdhocSession* pSession = m_pSessions;
		while ( pSession ) {
			if ( !_tcscmp( pSession->GetSessionId(), szSession ))
				return pSession;
			pSession = pSession->GetNext();
		}
		return NULL;
	}

	CJabberAdhocSession* AddNewSession()
	{
		CJabberAdhocSession* pSession = new CJabberAdhocSession();
		if ( !pSession )
			return NULL;

		pSession->SetNext( m_pSessions );
		m_pSessions = pSession;

		return pSession;
	}

	CJabberAdhocNode* FindNode( TCHAR* szNode )
	{
		CJabberAdhocNode* pNode = m_pNodes;
		while ( pNode ) {
			if ( !_tcscmp( pNode->GetNode(), szNode ))
				return pNode;
			pNode = pNode->GetNext();
		}
		return NULL;
	}

	BOOL RemoveSession( CJabberAdhocSession* pSession )
	{
		if ( !m_pSessions )
			return FALSE;

		if ( pSession == m_pSessions ) {
			m_pSessions = m_pSessions->GetNext();
			pSession->SetNext( NULL );
			delete pSession;
			return TRUE;
		}

		CJabberAdhocSession* pTmp = m_pSessions;
		while ( pTmp->GetNext() ) {
			if ( pTmp->GetNext() == pSession ) {
				pTmp->SetNext( pSession->GetNext() );
				pSession->SetNext( NULL );
				delete pSession;
				return TRUE;
			}
			pTmp = pTmp->GetNext();
		}
		return FALSE;
	}

	BOOL _ExpireSession( DWORD dwExpireTime )
	{
		if ( !m_pSessions )
			return FALSE;

		CJabberAdhocSession* pSession = m_pSessions;
		if ( pSession->GetSessionStartTime() < dwExpireTime ) {
			m_pSessions = pSession->GetNext();
			pSession->SetNext( NULL );
			delete pSession;
			return TRUE;
		}

		while ( pSession->GetNext() ) {
			if ( pSession->GetNext()->GetSessionStartTime() < dwExpireTime ) {
				CJabberAdhocSession* pRetVal = pSession->GetNext();
				pSession->SetNext( pSession->GetNext()->GetNext() );
				pRetVal->SetNext( NULL );
				delete pRetVal;
				return TRUE;
			}
			pSession = pSession->GetNext();
		}
		return FALSE;
	}

public:
	CJabberAdhocManager()
	{
		ZeroMemory( this, sizeof( CJabberAdhocManager ));
		InitializeCriticalSection( &m_cs );
	}
	~CJabberAdhocManager()
	{
		if ( m_pNodes )
			delete m_pNodes;
		if ( m_pSessions )
			delete m_pSessions;
		DeleteCriticalSection( &m_cs );
	}
	void Lock()
	{
		EnterCriticalSection( &m_cs );
	}
	void Unlock()
	{
		LeaveCriticalSection( &m_cs );
	}
	BOOL FillDefaultNodes()
	{
		AddNode( NULL, _T(JABBER_FEAT_RC_SET_STATUS), _T("Set status"), JabberAdhocSetStatusHandler );
		AddNode( NULL, _T(JABBER_FEAT_RC_SET_OPTIONS), _T("Set options"), JabberAdhocOptionsHandler );
		AddNode( NULL, _T(JABBER_FEAT_RC_FORWARD), _T("Forward unread messages"), JabberAdhocForwardHandler );
		return TRUE;
	}
	BOOL AddNode( TCHAR* szJid, TCHAR* szNode, TCHAR* szName, JABBER_ADHOC_HANDLER pHandler )
	{
		CJabberAdhocNode* pNode = new CJabberAdhocNode( szJid, szNode, szName, pHandler );
		if ( !pNode )
			return FALSE;

		Lock();
		if ( !m_pNodes )
			m_pNodes = pNode;
		else {
			CJabberAdhocNode* pTmp = m_pNodes;
			while ( pTmp->GetNext() )
				pTmp = pTmp->GetNext();
			pTmp->SetNext( pNode );
		}
		Unlock();

		return TRUE;
	}
	CJabberAdhocNode* GetFirstNode()
	{
		return m_pNodes;
	}
	BOOL HandleItemsRequest( XmlNode* iqNode, void* userdata, CJabberIqInfo* pInfo, TCHAR* szNode );
	BOOL HandleInfoRequest( XmlNode* iqNode, void* userdata, CJabberIqInfo* pInfo, TCHAR* szNode );
	BOOL HandleCommandRequest( XmlNode* iqNode, void* userdata, CJabberIqInfo* pInfo, TCHAR* szNode );

	BOOL ExpireSessions()
	{
		Lock();
		DWORD dwExpireTime = GetTickCount() - JABBER_ADHOC_SESSION_EXPIRE_TIME;
		while ( _ExpireSession( dwExpireTime ));
		Unlock();
		return TRUE;
	}
};

extern CJabberAdhocManager g_JabberAdhocManager;

#endif //_JABBER_RC_H_

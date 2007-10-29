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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_caps.cpp,v $
Revision       : $Revision: 5337 $
Last change on : $Date: 2007-04-28 13:26:31 +0300 (��, 28 ��� 2007) $
Last change by : $Author: ghazan $

*/

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "version.h"

void JabberProcessIqResultVersion( XmlNode* node, void* userdata, CJabberIqInfo *pInfo );

JabberFeatCapPair g_JabberFeatCapPairs[] = {
	{	_T(JABBER_FEAT_DISCO_INFO),       JABBER_CAPS_DISCO_INFO,        _T("Client supports Service Discovery info"), },
	{	_T(JABBER_FEAT_DISCO_ITEMS),      JABBER_CAPS_DISCO_ITEMS,       _T("Client supports Service Discovery items list"), },
	{	_T(JABBER_FEAT_ENTITY_CAPS),      JABBER_CAPS_ENTITY_CAPS,       _T("Client can inform about its Jabber capabilities"), },
	{	_T(JABBER_FEAT_SI),               JABBER_CAPS_SI,                _T("Client supports stream initiation (for filetransfers for ex.)"), },
	{	_T(JABBER_FEAT_SI_FT),            JABBER_CAPS_SI_FT,             _T("Client supports stream initiation for file transfers"), },
	{	_T(JABBER_FEAT_BYTESTREAMS),      JABBER_CAPS_BYTESTREAMS,       _T("Client supports file transfers via SOCKS5 Bytestreams"), },
	{	_T(JABBER_FEAT_IBB),              JABBER_CAPS_IBB,               _T("Client supports file transfers via In-Band Bytestreams"), },
	{	_T(JABBER_FEAT_OOB),              JABBER_CAPS_OOB,               _T("Client supports file transfers via Out-of-Band Bytestreams"), },
	{	_T(JABBER_FEAT_COMMANDS),         JABBER_CAPS_COMMANDS,          _T("Client supports execution of Ad-Hoc commands"), },
	{	_T(JABBER_FEAT_REGISTER),         JABBER_CAPS_REGISTER,          _T("Client supports in-band registration"), },
	{	_T(JABBER_FEAT_MUC),              JABBER_CAPS_MUC,               _T("Client supports multi-user chat"), },
	{	_T(JABBER_FEAT_CHATSTATES),       JABBER_CAPS_CHATSTATES,        _T("Client can report chat state in a chat session"), },
	{	_T(JABBER_FEAT_LAST_ACTIVITY),    JABBER_CAPS_LAST_ACTIVITY,     _T("Client can report information about the last activity of the user"), },
	{	_T(JABBER_FEAT_VERSION),          JABBER_CAPS_VERSION,           _T("Client can report own version information"), },
	{	_T(JABBER_FEAT_ENTITY_TIME),      JABBER_CAPS_ENTITY_TIME,       _T("Client can report local time of the user"), },
	{	_T(JABBER_FEAT_PING),             JABBER_CAPS_PING,              _T("Client can send and receive ping requests"), },
	{	_T(JABBER_FEAT_DATA_FORMS),       JABBER_CAPS_DATA_FORMS,        _T("Client supports data forms"), },
	{	_T(JABBER_FEAT_MESSAGE_EVENTS),   JABBER_CAPS_MESSAGE_EVENTS,    _T("Client can request and respond to events relating to the delivery, display, and composition of messages"), },
	{	_T(JABBER_FEAT_VCARD_TEMP),       JABBER_CAPS_VCARD_TEMP,        _T("Client supports vCard"), },
	{	_T(JABBER_FEAT_AVATAR),           JABBER_CAPS_AVATAR,            _T("Client supports iq-based avatars"), },
	{	_T(JABBER_FEAT_XHTML),            JABBER_CAPS_XHTML,             _T("Client supports xHTML formatting of chat messages"), },
	{	_T(JABBER_FEAT_AGENTS),           JABBER_CAPS_AGENTS,            _T("Client supports Jabber Browsing"), },
	{	_T(JABBER_FEAT_BROWSE),           JABBER_CAPS_BROWSE,            _T("Client supports Jabber Browsing"), },
	{	_T(JABBER_FEAT_FEATURE_NEG),      JABBER_CAPS_FEATURE_NEG,       _T("Client can negotiate options for specific features"), },
	{	_T(JABBER_FEAT_AMP),              JABBER_CAPS_AMP,               _T("Client can request advanced processing of message stanzas"), },
	{	_T(JABBER_FEAT_USER_MOOD),        JABBER_CAPS_USER_MOOD,         _T("Client can report information about user moods"), },
	{	_T(JABBER_FEAT_USER_MOOD_NOTIFY), JABBER_CAPS_USER_MOOD_NOTIFY,  _T("Client receives information about user moods"), },
	{	_T(JABBER_FEAT_PUBSUB),			  JABBER_CAPS_PUBSUB,            _T("Client supports generic publish-subscribe functionality"), },
	{	_T(JABBER_FEAT_SECUREIM),         JABBER_CAPS_SECUREIM,          _T("Client supports SecureIM plugin for Miranda IM"), },
	{	_T(JABBER_FEAT_PRIVACY_LISTS),	  JABBER_CAPS_PRIVACY_LISTS,     _T("Client can block communications from particular other users using Privacy lists"), },
	{	_T(JABBER_FEAT_MESSAGE_RECEIPTS), JABBER_CAPS_MESSAGE_RECEIPTS,  _T("Client supports Message Receipts"), },
	{	_T(JABBER_FEAT_USER_TUNE),        JABBER_CAPS_USER_TUNE,         _T("Client can report information about the music to which a user is listening"), },
	{	_T(JABBER_FEAT_USER_TUNE_NOTIFY), JABBER_CAPS_USER_TUNE_NOTIFY,  _T("Client receives information about the music to which a user is listening"), },
	{	_T(JABBER_FEAT_PRIVATE_STORAGE),  JABBER_CAPS_PRIVATE_STORAGE,   _T("Client supports private XML Storage (for bookmakrs and other)"), },
	{	NULL,                             0                            }
};

JabberFeatCapPair g_JabberFeatCapPairsExt[] = {
	{	_T(JABBER_EXT_SECUREIM),          JABBER_CAPS_SECUREIM         },
	{	_T(JABBER_EXT_COMMANDS),          JABBER_CAPS_COMMANDS         },
	{	_T(JABBER_EXT_USER_MOOD),         JABBER_CAPS_USER_MOOD_NOTIFY },
	{	_T(JABBER_EXT_USER_TUNE),         JABBER_CAPS_USER_TUNE_NOTIFY },
	{	_T(__VERSION_STRING),             JABBER_CAPS_MIRANDA_PARTIAL  },
	{	NULL,                             0                            }
};

CJabberClientCapsManager g_JabberClientCapsManager;

static void JabberIqResultCapsDiscoInfo( XmlNode* iqNode, void* userdata, CJabberIqInfo* pInfo )
{
	JABBER_RESOURCE_STATUS *r = JabberResourceInfoFromJID( pInfo->GetFrom() );

	XmlNode* query = pInfo->GetChildNode();
	if ( pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT && query ) {
		JabberCapsBits jcbCaps = 0;
		XmlNode *feature;
		for ( int i = 1; ( feature = JabberXmlGetNthChild( query, "feature", i )) != NULL; i++ ) {
			TCHAR *featureName = JabberXmlGetAttrValue( feature, "var" );
			if ( featureName ) {
				for ( int i = 0; g_JabberFeatCapPairs[i].szFeature; i++ ) {
					if ( !_tcscmp( g_JabberFeatCapPairs[i].szFeature, featureName )) {
						jcbCaps |= g_JabberFeatCapPairs[i].jcbCap;
						break;
		}	}	}	}

		// no version info support and no XEP-0115 support?
		if ( r && r->dwVersionRequestTime == -1 && !r->version && !r->software && !r->szCapsNode ) {
			r->jcbCachedCaps = jcbCaps;
			r->dwDiscoInfoRequestTime = -1;
			return;
		}

		g_JabberClientCapsManager.SetClientCaps( pInfo->GetIqId(), jcbCaps );
		JabberUserInfoUpdate( pInfo->GetHContact() );
	}
	else {
		// no version info support and no XEP-0115 support?
		if ( r && r->dwVersionRequestTime == -1 && !r->version && !r->software && !r->szCapsNode ) {
			r->jcbCachedCaps = JABBER_RESOURCE_CAPS_NONE;
			r->dwDiscoInfoRequestTime = -1;
			return;
		}
		g_JabberClientCapsManager.SetClientCaps( pInfo->GetIqId(), JABBER_RESOURCE_CAPS_ERROR );
	}
}

JabberCapsBits JabberGetTotalJidCapabilites( TCHAR *jid )
{
	if ( !jid )
		return JABBER_RESOURCE_CAPS_NONE;

	TCHAR szBareJid[ JABBER_MAX_JID_LEN ];
	JabberStripJid( jid, szBareJid, SIZEOF( szBareJid ));

	JabberCapsBits jcbToReturn = JabberGetResourceCapabilites( szBareJid, FALSE );
	if ( jcbToReturn & JABBER_RESOURCE_CAPS_ERROR)
		jcbToReturn = JABBER_RESOURCE_CAPS_NONE;
	
	JABBER_LIST_ITEM *item = JabberListGetItemPtr( LIST_ROSTER, szBareJid );
	if ( !item )
		item = JabberListGetItemPtr( LIST_VCARD_TEMP, szBareJid );
	if ( item ) {
		for ( int i = 0; i < item->resourceCount; i++ ) {
			TCHAR szFullJid[ JABBER_MAX_JID_LEN ];
			mir_sntprintf( szFullJid, JABBER_MAX_JID_LEN, _T("%s/%s"), szBareJid, item->resource[i].resourceName );
			JabberCapsBits jcb = JabberGetResourceCapabilites( szFullJid, FALSE );
			if ( !( jcb & JABBER_RESOURCE_CAPS_ERROR ))
				jcbToReturn |= jcb;
		}
	}
	return jcbToReturn;
}

JabberCapsBits JabberGetResourceCapabilites( TCHAR *jid, BOOL appendBestResource /*= TRUE*/ )
{
	TCHAR fullJid[ 512 ];
	if ( appendBestResource )
		JabberGetClientJID( jid, fullJid, SIZEOF( fullJid ));
	else
		_tcsncpy( fullJid, jid, SIZEOF( fullJid ));

	JABBER_RESOURCE_STATUS *r = JabberResourceInfoFromJID( fullJid );
	if ( r == NULL ) return JABBER_RESOURCE_CAPS_ERROR;

	// XEP-0115 mode
	if ( r->szCapsNode && r->szCapsVer ) {
		JabberCapsBits jcbCaps = 0, jcbExtCaps = 0;
		BOOL bRequestSent = FALSE;
		JabberCapsBits jcbMainCaps = g_JabberClientCapsManager.GetClientCaps( r->szCapsNode, r->szCapsVer );

		if ( jcbMainCaps == JABBER_RESOURCE_CAPS_TIMEOUT && !r->dwDiscoInfoRequestTime )
			jcbMainCaps = JABBER_RESOURCE_CAPS_ERROR;

		if ( jcbMainCaps == JABBER_RESOURCE_CAPS_ERROR ) {
			// send disco#info query

			CJabberIqInfo *pInfo = g_JabberIqManager.AddHandler( JabberIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE, -1, NULL, 0, JABBER_RESOURCE_CAPS_QUERY_TIMEOUT );
			g_JabberClientCapsManager.SetClientCaps( r->szCapsNode, r->szCapsVer, JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId() );
			r->dwDiscoInfoRequestTime = pInfo->GetRequestTime();
			
			XmlNodeIq iq( pInfo );
			XmlNode* query = iq.addQuery( JABBER_FEAT_DISCO_INFO );

			TCHAR queryNode[512];
			mir_sntprintf( queryNode, SIZEOF(queryNode), _T("%s#%s"), r->szCapsNode, r->szCapsVer );
			query->addAttr( "node", queryNode );
			jabberThreadInfo->send( iq );

			bRequestSent = TRUE;
		}
		else if ( jcbMainCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS )
			bRequestSent = TRUE;
		else if ( jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT )
			jcbCaps |= jcbMainCaps;

		if (( jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT ) && r->szCapsExt ) {
			TCHAR *caps = mir_tstrdup( r->szCapsExt );

			TCHAR *token = _tcstok( caps, _T(" ") );
			while ( token ) {
				jcbExtCaps = g_JabberClientCapsManager.GetClientCaps( r->szCapsNode, token );
				if ( jcbExtCaps == JABBER_RESOURCE_CAPS_ERROR ) {
					// send disco#info query

					CJabberIqInfo* pInfo = g_JabberIqManager.AddHandler( JabberIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE, -1, NULL, 0, JABBER_RESOURCE_CAPS_QUERY_TIMEOUT );
					g_JabberClientCapsManager.SetClientCaps( r->szCapsNode, token, JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId() );

					XmlNodeIq iq( pInfo );
					XmlNode* query = iq.addQuery( JABBER_FEAT_DISCO_INFO );

					TCHAR queryNode[512];
					mir_sntprintf( queryNode, SIZEOF(queryNode), _T("%s#%s"), r->szCapsNode, token );
					query->addAttr( "node", queryNode );
					jabberThreadInfo->send( iq );

					bRequestSent = TRUE;
				}
				else if ( jcbExtCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS )
					bRequestSent = TRUE;
				else
					jcbCaps |= jcbExtCaps;

				token = _tcstok( NULL, _T(" ") );
			}

			mir_free(caps);
		}

		if ( bRequestSent )
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;

		return jcbCaps | r->jcbManualDiscoveredCaps;
	}

	// capability mode (version request + service discovery)

	// no version info:
	if ( !r->version && !r->software ) {
		// version request not sent:
		if ( !r->dwVersionRequestTime ) {
			// send version query

			CJabberIqInfo *pInfo = g_JabberIqManager.AddHandler( JabberProcessIqResultVersion, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_CHILD_TAG_NODE, -1, NULL, 0, JABBER_RESOURCE_CAPS_QUERY_TIMEOUT );
			r->dwVersionRequestTime = pInfo->GetRequestTime();
			
			XmlNodeIq iq( pInfo );
			XmlNode* query = iq.addQuery( JABBER_FEAT_VERSION );
			jabberThreadInfo->send( iq );
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;
		}
		// version not received:
		else if ( r->dwVersionRequestTime != -1 ) {
			// no timeout?
			if ( GetTickCount() - r->dwVersionRequestTime < JABBER_RESOURCE_CAPS_QUERY_TIMEOUT )
				return JABBER_RESOURCE_CAPS_IN_PROGRESS;

			// timeout
			r->dwVersionRequestTime = -1;
		}
		// no version information, try direct service discovery
		if ( !r->dwDiscoInfoRequestTime ) {
			// send disco#info query

			CJabberIqInfo *pInfo = g_JabberIqManager.AddHandler( JabberIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE, -1, NULL, 0, JABBER_RESOURCE_CAPS_QUERY_TIMEOUT );
			r->dwDiscoInfoRequestTime = pInfo->GetRequestTime();

			XmlNodeIq iq( pInfo );
			XmlNode* query = iq.addQuery( JABBER_FEAT_DISCO_INFO );
			jabberThreadInfo->send( iq );

			return JABBER_RESOURCE_CAPS_IN_PROGRESS;
		}
		else if ( r->dwDiscoInfoRequestTime == -1 )
			return r->jcbCachedCaps | r->jcbManualDiscoveredCaps;
		else if ( GetTickCount() - r->dwDiscoInfoRequestTime < JABBER_RESOURCE_CAPS_QUERY_TIMEOUT )
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;
		else
			r->dwDiscoInfoRequestTime = -1;
		// version request timeout:
		return JABBER_RESOURCE_CAPS_NONE;
	}

	// version info available:
	if ( r->software && r->version ) {
		JabberCapsBits jcbMainCaps = g_JabberClientCapsManager.GetClientCaps( r->software, r->version );
		if ( jcbMainCaps == JABBER_RESOURCE_CAPS_ERROR ) {
			// Bombus hack:
			if ( !_tcscmp( r->software, _T( "Bombus" )) || !_tcscmp( r->software, _T( "BombusMod" )) ) {
				jcbMainCaps = JABBER_CAPS_SI|JABBER_CAPS_SI_FT|JABBER_CAPS_IBB|JABBER_CAPS_MESSAGE_EVENTS|JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY|JABBER_CAPS_DATA_FORMS|JABBER_CAPS_LAST_ACTIVITY|JABBER_CAPS_VERSION|JABBER_CAPS_COMMANDS|JABBER_CAPS_VCARD_TEMP;
				g_JabberClientCapsManager.SetClientCaps( r->software, r->version, jcbMainCaps );
			}
			// Neos hack:
			else if ( !_tcscmp( r->software, _T( "neos" ))) {
				jcbMainCaps = JABBER_CAPS_OOB|JABBER_CAPS_MESSAGE_EVENTS|JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY|JABBER_CAPS_LAST_ACTIVITY|JABBER_CAPS_VERSION;
				g_JabberClientCapsManager.SetClientCaps( r->software, r->version, jcbMainCaps );
			}
			// sim hack:
			else if ( !_tcscmp( r->software, _T( "sim" ))) {
				jcbMainCaps = JABBER_CAPS_OOB|JABBER_CAPS_VERSION|JABBER_CAPS_MESSAGE_EVENTS|JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY;
				g_JabberClientCapsManager.SetClientCaps( r->software, r->version, jcbMainCaps );
		}	}

		if ( jcbMainCaps == JABBER_RESOURCE_CAPS_ERROR ) {
			// send disco#info query

			CJabberIqInfo *pInfo = g_JabberIqManager.AddHandler( JabberIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE, -1, NULL, 0, JABBER_RESOURCE_CAPS_QUERY_TIMEOUT );
			g_JabberClientCapsManager.SetClientCaps( r->software, r->version, JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId() );
			r->dwDiscoInfoRequestTime = pInfo->GetRequestTime();

			XmlNodeIq iq( pInfo );
			XmlNode* query = iq.addQuery( JABBER_FEAT_DISCO_INFO );
			jabberThreadInfo->send( iq );

			jcbMainCaps = JABBER_RESOURCE_CAPS_IN_PROGRESS;
		}
		return jcbMainCaps | r->jcbManualDiscoveredCaps;
	}

	return JABBER_RESOURCE_CAPS_NONE;
}

CJabberClientPartialCaps::CJabberClientPartialCaps( TCHAR *szVer )
{
	m_szVer = mir_tstrdup( szVer );
	m_jcbCaps = JABBER_RESOURCE_CAPS_ERROR;
	m_pNext = NULL;
	m_nIqId = -1;
	m_dwRequestTime = 0;
}

CJabberClientPartialCaps::~CJabberClientPartialCaps()
{
	mir_free( m_szVer );
	if ( m_pNext )
		delete m_pNext;
}

CJabberClientPartialCaps* CJabberClientPartialCaps::SetNext( CJabberClientPartialCaps *pCaps )
{
	CJabberClientPartialCaps *pRetVal = m_pNext;
	m_pNext = pCaps;
	return pRetVal;
}

void CJabberClientPartialCaps::SetCaps( JabberCapsBits jcbCaps, int nIqId /*= -1*/ )
{
	if ( jcbCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS )
		m_dwRequestTime = GetTickCount();
	else
		m_dwRequestTime = 0;
	m_jcbCaps = jcbCaps;
	m_nIqId = nIqId;
}

JabberCapsBits CJabberClientPartialCaps::GetCaps()
{
	if ( m_jcbCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS && GetTickCount() - m_dwRequestTime > JABBER_RESOURCE_CAPS_QUERY_TIMEOUT ) {
		m_jcbCaps = JABBER_RESOURCE_CAPS_TIMEOUT;
		m_dwRequestTime = 0;
	}
	return m_jcbCaps;
}

CJabberClientPartialCaps* CJabberClientCaps::FindByVersion( TCHAR *szVer )
{
	if ( !m_pCaps || !szVer )
		return NULL;

	CJabberClientPartialCaps *pCaps = m_pCaps;
	while ( pCaps ) {
		if ( !_tcscmp( szVer, pCaps->GetVersion()))
			break;
		pCaps = pCaps->GetNext();
	}
	return pCaps;
}

CJabberClientPartialCaps* CJabberClientCaps::FindById( int nIqId )
{
	if ( !m_pCaps || nIqId == -1 )
		return NULL;

	CJabberClientPartialCaps *pCaps = m_pCaps;
	while ( pCaps ) {
		if ( pCaps->GetIqId() == nIqId )
			break;
		pCaps = pCaps->GetNext();
	}
	return pCaps;
}

CJabberClientCaps::CJabberClientCaps( TCHAR *szNode )
{
	m_szNode = mir_tstrdup( szNode );
	m_pCaps = NULL;
	m_pNext= NULL;
}

CJabberClientCaps::~CJabberClientCaps() {
	mir_free( m_szNode );
	if ( m_pCaps )
		delete m_pCaps;
	if ( m_pNext )
		delete m_pNext;
}

CJabberClientCaps* CJabberClientCaps::SetNext( CJabberClientCaps *pClient )
{
	CJabberClientCaps *pRetVal = m_pNext;
	m_pNext = pClient;
	return pRetVal;
}

JabberCapsBits CJabberClientCaps::GetPartialCaps( TCHAR *szVer ) {
	CJabberClientPartialCaps *pCaps = FindByVersion( szVer );
	if ( !pCaps )
		return JABBER_RESOURCE_CAPS_ERROR;
	return pCaps->GetCaps();
}

BOOL CJabberClientCaps::SetPartialCaps( TCHAR *szVer, JabberCapsBits jcbCaps, int nIqId /*= -1*/ ) {
	CJabberClientPartialCaps *pCaps = FindByVersion( szVer );
	if ( !pCaps ) {
		pCaps = new CJabberClientPartialCaps( szVer );
		if ( !pCaps )
			return FALSE;
		pCaps->SetNext( m_pCaps );
		m_pCaps = pCaps;
	}
	pCaps->SetCaps( jcbCaps, nIqId );
	return TRUE;
}

BOOL CJabberClientCaps::SetPartialCaps( int nIqId, JabberCapsBits jcbCaps ) {
	CJabberClientPartialCaps *pCaps = FindById( nIqId );
	if ( !pCaps )
		return FALSE;
	pCaps->SetCaps( jcbCaps, -1 );
	return TRUE;
}

CJabberClientCapsManager::CJabberClientCapsManager()
{
	InitializeCriticalSection( &m_cs );
	m_pClients = NULL;
}

CJabberClientCapsManager::~CJabberClientCapsManager()
{
	if ( m_pClients )
		delete m_pClients;
	DeleteCriticalSection( &m_cs );
}

CJabberClientCaps * CJabberClientCapsManager::FindClient( TCHAR *szNode )
{
	if ( !m_pClients || !szNode )
		return NULL;

	CJabberClientCaps *pClient = m_pClients;
	while ( pClient ) {
		if ( !_tcscmp( szNode, pClient->GetNode()))
			break;
		pClient = pClient->GetNext();
	}
	return pClient;
}

void CJabberClientCapsManager::AddDefaultCaps() {
	char* version = JabberGetVersionText();
	TCHAR *tversion = mir_a2t( version );
	mir_free( version );
	SetClientCaps( _T(JABBER_CAPS_MIRANDA_NODE), tversion, JABBER_CAPS_MIRANDA_ALL );
	mir_free( tversion );

	for ( int i = 0; g_JabberFeatCapPairsExt[i].szFeature; i++ )
		SetClientCaps( _T(JABBER_CAPS_MIRANDA_NODE), g_JabberFeatCapPairsExt[i].szFeature, g_JabberFeatCapPairsExt[i].jcbCap );
}

JabberCapsBits CJabberClientCapsManager::GetClientCaps( TCHAR *szNode, TCHAR *szVer )
{
	Lock();
	CJabberClientCaps *pClient = FindClient( szNode );
	if ( !pClient ) {
		Unlock();
		JabberLog( "CAPS: get no caps for: " TCHAR_STR_PARAM ", " TCHAR_STR_PARAM, szNode, szVer );
		return JABBER_RESOURCE_CAPS_ERROR;
	}
	JabberCapsBits jcbCaps = pClient->GetPartialCaps( szVer );
	Unlock();
	JabberLog( "CAPS: get caps %I64x for: " TCHAR_STR_PARAM ", " TCHAR_STR_PARAM, jcbCaps, szNode, szVer );
	return jcbCaps;
}

BOOL CJabberClientCapsManager::SetClientCaps( TCHAR *szNode, TCHAR *szVer, JabberCapsBits jcbCaps, int nIqId /*= -1*/ )
{
	Lock();
	CJabberClientCaps *pClient = FindClient( szNode );
	if (!pClient) {
		pClient = new CJabberClientCaps( szNode );
		if ( !pClient ) {
			Unlock();
			return FALSE;
		}
		pClient->SetNext( m_pClients );
		m_pClients = pClient;
	}
	BOOL bOk = pClient->SetPartialCaps( szVer, jcbCaps, nIqId );
	Unlock();
	JabberLog( "CAPS: set caps %I64x for: " TCHAR_STR_PARAM ", " TCHAR_STR_PARAM, jcbCaps, szNode, szVer );
	return bOk;
}

BOOL CJabberClientCapsManager::SetClientCaps( int nIqId, JabberCapsBits jcbCaps )
{
	Lock();
	if ( !m_pClients ) {
		Unlock();
		return FALSE;
	}
	BOOL bOk = FALSE;
	CJabberClientCaps *pClient = m_pClients;
	while ( pClient ) {
		if ( pClient->SetPartialCaps( nIqId, jcbCaps )) {
			JabberLog( "CAPS: set caps %I64x for iq %d", jcbCaps, nIqId );
			bOk = TRUE;
			break;
		}
		pClient = pClient->GetNext();
	}
	Unlock();
	return bOk;
}

BOOL CJabberClientCapsManager::HandleInfoRequest( XmlNode* iqNode, void* userdata, CJabberIqInfo* pInfo, TCHAR* szNode )
{
	JabberCapsBits jcb = 0;

	if ( szNode ) {
		for ( int i = 0; g_JabberFeatCapPairsExt[i].szFeature; i++ ) {
			TCHAR szExtCap[ 512 ];
			mir_sntprintf( szExtCap, SIZEOF(szExtCap), _T("%s#%s"), _T(JABBER_CAPS_MIRANDA_NODE), g_JabberFeatCapPairsExt[i].szFeature );
			if ( !_tcscmp( szNode, szExtCap )) {
				jcb = g_JabberFeatCapPairsExt[i].jcbCap;
				break;
			}
		}
		// unknown node, not XEP-0115 request
		if ( !jcb )
			return FALSE;
	}
	else
		jcb = JABBER_CAPS_MIRANDA_ALL;

	XmlNodeIq iq( "result", pInfo );

	XmlNode* query = iq.addChild( "query" );
	query->addAttr( "xmlns", _T(JABBER_FEAT_DISCO_INFO) );
	if ( szNode )
		query->addAttr( "node", szNode );

	XmlNode* identity = query->addChild( "identity" );
	identity->addAttr( "category", "client" );
	identity->addAttr( "type", "pc" );
	identity->addAttr( "name", "Miranda" );

	for ( int i = 0; g_JabberFeatCapPairs[i].szFeature; i++ ) 
		if ( jcb & g_JabberFeatCapPairs[i].jcbCap ) {
			XmlNode* feature = query->addChild( "feature" );
			feature->addAttr( "var", g_JabberFeatCapPairs[i].szFeature );
		}

	jabberThreadInfo->send( iq );
	
	return TRUE;
}

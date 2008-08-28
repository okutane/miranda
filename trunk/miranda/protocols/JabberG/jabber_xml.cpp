/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-08  George Hazan
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

File name      : $URL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

*/

#include "jabber.h"

#define TAG_MAX_LEN 128
#define ATTR_MAX_LEN 8192

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNodeIq class members

XmlNodeIq::XmlNodeIq( const TCHAR* type, int id, LPCTSTR to ) :
	XmlNode( _T( "iq" ))
{
	if ( type != NULL ) *this << XATTR( _T("type"), type );
	if ( to   != NULL ) *this << XATTR( _T("to"),   to );
	if ( id   != -1   ) *this << XATTRID( id );
}

XmlNodeIq::XmlNodeIq( const TCHAR* type, LPCTSTR idStr, LPCTSTR to ) :
	XmlNode( _T( "iq" ))
{
	if ( type  != NULL ) *this << XATTR( _T("type"), type  );
	if ( to    != NULL ) *this << XATTR( _T("to"),   to    );
	if ( idStr != NULL ) *this << XATTR( _T("id"),   idStr );
}

XmlNodeIq::XmlNodeIq( const TCHAR* type, HXML node, LPCTSTR to ) :
	XmlNode( _T( "iq" ))
{
	if ( type  != NULL ) *this << XATTR( _T("type"), type  );
	if ( to    != NULL ) *this << XATTR( _T("to"),   to    );
	if ( node  != NULL ) {
		const TCHAR *iqId = xmlGetAttrValue( *this, _T( "id" ));
		if ( iqId != NULL ) *this << XATTR( _T("id"), iqId );
	}
}

XmlNodeIq::XmlNodeIq( CJabberIqInfo* pInfo ) :
	XmlNode( _T( "iq" ))
{
	if ( pInfo ) {
		if ( pInfo->GetCharIqType() != NULL ) *this << XATTR( _T("type"), _A2T(pInfo->GetCharIqType()));
		if ( pInfo->GetReceiver()   != NULL ) *this << XATTR( _T("to"), pInfo->GetReceiver() );
		if ( pInfo->GetIqId()       != -1 )   *this << XATTRID( pInfo->GetIqId() );
	}
}

XmlNodeIq::XmlNodeIq( const TCHAR* type, CJabberIqInfo* pInfo ) :
	XmlNode( _T( "iq" ))
{
	if ( type != NULL ) *this << XATTR( _T("type"), type );
	if ( pInfo ) {
		if ( pInfo->GetFrom()  != NULL ) *this << XATTR( _T("to"), pInfo->GetFrom() );
		if ( pInfo->GetIdStr() != NULL ) *this << XATTR( _T("id"), pInfo->GetIdStr() );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNode class members

XmlNode::XmlNode( LPCTSTR pszName )
{
	m_hXml = xi.createNode( pszName, NULL, 0 );
}

XmlNode::XmlNode( LPCTSTR pszName, LPCTSTR ptszText )
{
	m_hXml = xi.createNode( pszName, ptszText, 0 );
}

XmlNode::XmlNode( const XmlNode& n )
{
	m_hXml = xi.copyNode( n );
}

XmlNode& XmlNode::operator =( const XmlNode& n )
{	
	if ( m_hXml )
		xi.destroyNode( m_hXml );
	m_hXml = xi.copyNode( n );
	return *this;
}

XmlNode::~XmlNode()
{
	if ( m_hXml ) {
		xi.destroyNode( m_hXml );
		m_hXml = NULL;
}	}

/////////////////////////////////////////////////////////////////////////////////////////

HXML __fastcall operator<<( HXML node, XCHILDNS& child )
{
	HXML res = xmlAddChild( node, child.name );
	xmlAddAttr( res, _T("xmlns"), child.ns );
	return res;
}

HXML __fastcall operator<<( HXML node, XQUERY& child )
{
	HXML n = xmlAddChild( node, _T("query"));
	if ( n )
		xmlAddAttr( n, _T("xmlns"), child.ns );
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////

void __fastcall xmlAddAttr( HXML hXml, LPCTSTR name, LPCTSTR value )
{
	if ( value )
		xi.addAttr( hXml, name, value );
}

void __fastcall xmlAddAttr( HXML hXml, LPCTSTR pszName, int value )
{
	xi.addAttrInt( hXml, pszName, value );
}

void __fastcall xmlAddAttrID( HXML hXml, int id )
{
	TCHAR text[ 100 ];
	mir_sntprintf( text, SIZEOF(text), _T("mir_%d"), id );
	xmlAddAttr( hXml, _T("id"), text );
}

/////////////////////////////////////////////////////////////////////////////////////////

LPCTSTR __fastcall xmlGetAttr( HXML hXml, int n )
{
	return xi.getAttr( hXml, n );
}

int __fastcall xmlGetAttrCount( HXML hXml )
{
	return xi.getAttrCount( hXml );
}

LPCTSTR __fastcall xmlGetAttrName( HXML hXml, int n )
{
	return xi.getAttrName( hXml, n );
}

/////////////////////////////////////////////////////////////////////////////////////////

void __fastcall xmlAddChild( HXML hXml, HXML n )
{
	xi.addChild2( n, hXml );
}

HXML __fastcall xmlAddChild( HXML hXml, LPCTSTR name )
{
	return xi.addChild( hXml, name, NULL );
}

HXML __fastcall xmlAddChild( HXML hXml, LPCTSTR name, LPCTSTR value )
{
	return xi.addChild( hXml, name, value );
}

HXML __fastcall xmlAddChild( HXML hXml, LPCTSTR name, int value )
{
	TCHAR buf[40];
	_itot( value, buf, 10 );
	return xi.addChild( hXml, name, buf );
}

/////////////////////////////////////////////////////////////////////////////////////////

LPCTSTR __fastcall xmlGetAttrValue( HXML hXml, LPCTSTR key )
{
	return xi.getAttrValue( hXml, key );
}

HXML __fastcall xmlGetChild( HXML hXml, int n )
{
	return xi.getChild( hXml, n );
}

HXML __fastcall xmlGetChild( HXML hXml, LPCTSTR key )
{
	return xi.getNthChild( hXml, key, 0 );
}

#if defined( _UNICODE )
HXML __fastcall xmlGetChild( HXML hXml, LPCSTR key )
{
	LPTSTR wszKey = mir_a2t( key );
	HXML result = xi.getNthChild( hXml, wszKey, 0 );
	mir_free( wszKey );
	return result;
}

HXML __fastcall xmlGetChildByTag( HXML hXml, LPCTSTR key, LPCTSTR attrName, LPCTSTR attrValue )
{
	return xi.getChildByAttrValue( hXml, key, attrName, attrValue );
}
#endif

HXML __fastcall xmlGetChildByTag( HXML hXml, LPCSTR key, LPCSTR attrName, LPCTSTR attrValue )
{
	LPTSTR wszKey = mir_a2t( key ), wszName = mir_a2t( attrName );
	HXML result = xi.getChildByAttrValue( hXml, wszKey, wszName, attrValue );
	mir_free( wszKey ), mir_free( wszName );
	return result;
}

int __fastcall xmlGetChildCount( HXML hXml )
{
	return xi.getChildCount( hXml );
}

HXML __fastcall xmlGetNthChild( HXML hXml, LPCTSTR tag, int nth )
{
	int i, num;

	if ( !hXml || tag == NULL || _tcslen( tag ) <= 0 || nth < 1 )
		return NULL;

	num = 1;
	for ( i=0; ; i++ ) {
		HXML n = xi.getChild( hXml, i );
		if ( !n )
			break;
		if ( !lstrcmp( tag, xmlGetName( n ))) {
			if ( num == nth )
				return n;

			num++;
	}	}

	return NULL;
}

LPCTSTR __fastcall xmlGetName( HXML xml )
{
	return xi.getName( xml );
}

LPCTSTR __fastcall xmlGetText( HXML xml )
{
	return xi.getText( xml );
}

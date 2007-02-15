/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-06  George Hazan
Copyright ( C ) 2007     Artem Shpynov

Module implements a search according to XEP-0055: Jabber Search
http://www.xmpp.org/extensions/xep-0055.html

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

File name      : $Source: /cvsroot/miranda/miranda/protocols/JabberG/jabber_proxy.cpp,v $
Revision       : $Revision: 2866 $
Last change on : $Date: 2006-05-16 20:39:40 +0400 (��, 16 ��� 2006) $
Last change by : $Author: ghazan $

*/

#include <CommCtrl.h>
#include "jabber.h"
#include "jabber_iq.h"
#include "resource.h"
#include "jabber_search.h"

///////////////////////////////////////////////////////////////////////////////
// Subclassing of IDC_FRAME to implement more user-friendly fields scrolling  
//
static int JabberSearchFrameProc(HWND hwnd, int msg, WPARAM wParam, LPARAM lParam)
{
	if ( msg == WM_COMMAND && lParam != 0 ) {
		HWND hwndDlg = GetParent( hwnd );		
		JabberSearchData* dat = ( JabberSearchData* )GetWindowLong( hwndDlg, GWL_USERDATA );
		if ( dat && lParam ) {
			int pos = dat->curPos;
			RECT MineRect;
			RECT FrameRect;
			GetWindowRect(GetDlgItem( hwndDlg, IDC_FRAME ),&FrameRect);
			GetWindowRect((HWND)lParam, &MineRect);
			if ( MineRect.top-10 < FrameRect.top ) {
				pos=dat->curPos+(MineRect.top-14-FrameRect.top);
				if (pos<0) pos=0;  
			}
			else if ( MineRect.bottom > FrameRect.bottom ) {
				pos=dat->curPos+(MineRect.bottom-FrameRect.bottom);
				if (dat->frameHeight+pos>dat->CurrentHeight)
					pos=dat->CurrentHeight-dat->frameHeight;
			}
			if ( pos != dat->curPos ) {
				ScrollWindow( GetDlgItem( hwndDlg, IDC_FRAME ), 0, dat->curPos - pos, NULL,  &( dat->frameRect ));
				SetScrollPos( GetDlgItem( hwndDlg, IDC_VSCROLL ), SB_CTL, pos, TRUE );
				RECT Invalid=dat->frameRect;
				if (dat->curPos - pos >0)
					Invalid.bottom=Invalid.top+(dat->curPos - pos);
				else
					Invalid.top=Invalid.bottom+(dat->curPos - pos);

				RedrawWindow(GetDlgItem( hwndDlg, IDC_FRAME ), NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW |RDW_ALLCHILDREN);
				dat->curPos = pos;
	}	}	}		

	if ( msg == WM_PAINT ) {
		PAINTSTRUCT ps;
		HDC hdc=BeginPaint(hwnd, &ps);
		FillRect(hdc,&(ps.rcPaint),GetSysColorBrush(COLOR_BTNFACE));
		EndPaint(hwnd, &ps);
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

///////////////////////////////////////////////////////////////////////////////
//  Add Search field to form
//

static int JabberSearchAddField(HWND hwndDlg, TCHAR *Label, TCHAR * Var, int Order )
{
	if ( !Label || !Var )
		return FALSE;

	HFONT hFont = ( HFONT ) SendMessage( hwndDlg, WM_GETFONT, 0, 0 );
	HWND hwndParent=GetDlgItem(hwndDlg,IDC_FRAME);
	LONG frameExStyle = GetWindowLong( hwndParent, GWL_EXSTYLE );
	frameExStyle |= WS_EX_CONTROLPARENT;
	SetWindowLong( hwndParent, GWL_EXSTYLE, frameExStyle );
	SetWindowLong(GetDlgItem(hwndDlg,IDC_FRAME),GWL_WNDPROC,(LONG)JabberSearchFrameProc);

	int CornerX=1;
	int CornerY=1;
	RECT rect;
	GetClientRect(hwndParent,&rect);
	int width=rect.right-5-CornerX;

	HWND hwndLabel=CreateWindowEx(WS_EX_NOPARENTNOTIFY,_T("STATIC"),(LPCTSTR)TranslateTS(Label),WS_CHILD, CornerX, CornerY + Order*40, width, 13,hwndParent,NULL,hInst,0);
	HWND hwndVar=CreateWindowEx(WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE,_T("EDIT"),(LPCTSTR)NULL,WS_CHILD|WS_TABSTOP, CornerX+5, CornerY + Order*40+14, width ,20,hwndParent,NULL,hInst,0);
	SendMessage(hwndLabel, WM_SETFONT, (WPARAM)hFont,0);
	SendMessage(hwndVar, WM_SETFONT, (WPARAM)hFont,0);
	ShowWindow(hwndLabel,SW_SHOW);
	ShowWindow(hwndVar,SW_SHOW);
	//remade list
	//reallocation
	JabberSearchData* dat = ( JabberSearchData* )GetWindowLong( hwndDlg, GWL_USERDATA );
	if ( dat ) {
		dat->pJSInf = (JabberSearchFieldsInfo*) realloc(dat->pJSInf, sizeof(JabberSearchFieldsInfo)*(dat->nJSInfCount+1));
		dat->pJSInf[dat->nJSInfCount].hwndCaptionItem=hwndLabel;		
		dat->pJSInf[dat->nJSInfCount].hwndValueItem=hwndVar;
		dat->pJSInf[dat->nJSInfCount].szFieldCaption=_tcsdup(Label);
		dat->pJSInf[dat->nJSInfCount].szFieldName=_tcsdup(Var);
		dat->nJSInfCount++;
	}
	return CornerY + Order*40+14 +20;
}

////////////////////////////////////////////////////////////////////////////////
// Available search field request result handler  (XEP-0055. Examples 2, 7)

static void JabberIqResultGetSearchFields( XmlNode *iqNode, void *userdata )
{
	if  ( !searchHandleDlg )
		return;

	TCHAR* type = JabberXmlGetAttrValue( iqNode, "type" );
	if ( !type )
		return;

	if ( !lstrcmp( type, _T("result"))) {
		XmlNode* queryNode = JabberXmlGetNthChild(iqNode,"query",1);
		XmlNode* xNode = JabberXmlGetChildWithGivenAttrValue( queryNode, "x", "xmlns", _T("jabber:x:data"));
		int formHeight=0;
		ShowWindow(searchHandleDlg,SW_HIDE);
		if ( xNode ) {
			//1. Form
			int i=1;
			int Order=0;
			XmlNode* xcNode =JabberXmlGetNthChild( xNode, "title", 1 );
			if ( xcNode )
				SetDlgItemText( searchHandleDlg, IDC_INSTRUCTIONS, xcNode->text);
			
			while ( xcNode = JabberXmlGetNthChild( xNode, "field", i )) {
				TCHAR* label = JabberXmlGetAttrValue( xcNode, "label" );
				TCHAR* var = JabberXmlGetAttrValue( xcNode, "var" );
				if ( label && var ) {
					Data *MyData = ( Data* )malloc( sizeof( Data ));
					MyData->Label = mir_tstrdup( label );
					MyData->Var = mir_tstrdup( var );
					MyData->Order = Order;
					PostMessage( searchHandleDlg, WM_USER+10, ( WPARAM )TRUE, ( LPARAM )MyData );
					Order++;
				}
				i++;
				ShowWindow(searchHandleDlg,SW_SHOW);
			}		
		}
		else {
			int Order = 0;
			for ( int i = 0; i < queryNode->numChild; i++ ) {
				XmlNode* chNode = queryNode->child[i];
				if (!strcmpi(chNode->name, "instructions") && chNode->text)
					SetDlgItemText(searchHandleDlg,IDC_INSTRUCTIONS,TranslateTS(chNode->text));
				else if ( chNode->name ) {
					Data* MyData = ( Data* )malloc( sizeof( Data ));
					MyData->Label = a2t( chNode->name );
					MyData->Var = a2t( chNode->name );
					MyData->Order = Order;
					PostMessage( searchHandleDlg, WM_USER+10, ( WPARAM )FALSE, ( LPARAM )MyData );
					Order++;
			}	}		

			ShowWindow(searchHandleDlg,SW_SHOW);
		}

		TCHAR* szFrom = JabberXmlGetAttrValue( iqNode, "from" );
		if ( szFrom )
			JabberSearchAddToRecent( szFrom, searchHandleDlg );
	}
	else if ( !lstrcmp( type, _T("error"))) {
		TCHAR* code=NULL;
		TCHAR* description=NULL;
		TCHAR buff[255];
		XmlNode* errorNode=JabberXmlGetChild(iqNode,"error");
		if ( errorNode ) {
			code = JabberXmlGetAttrValue(errorNode,"code");
			description = errorNode->text;
		}
		_sntprintf(buff,SIZEOF(buff),TranslateT("Error %s %s\r\nPlease select other server"),code ? code : _T(""),description?description:_T(""));
		SetDlgItemText( searchHandleDlg, IDC_INSTRUCTIONS, buff );
	}
	else SetDlgItemText( searchHandleDlg, IDC_INSTRUCTIONS, TranslateT( "Error Unknown reply recieved\r\nPlease select other server" ));
}

////////////////////////////////////////////////////////////////////////////////
// Search field request result handler  (XEP-0055. Examples 3, 8)

int TCharKeyCmp( TCHAR* a, TCHAR* b)
{
	return _tcsicmp(a,b);
}

typedef UNIQUE_MAP<TCHAR,TCharKeyCmp> U_TCHAR_MAP;

static void JabberIqResultAdvancedSearch( XmlNode *iqNode, void *userdata )
{
	TCHAR* type;
	int id;
	BOOL bFreeColumnNames=FALSE;

	U_TCHAR_MAP  mColumnsNames(10);
	U_TCHAR_MAP* pUserColumn;
	LIST<void>   SearchResults(2);
	U_TCHAR_MAP  mReportedColumns(10);

	if ((( id = JabberGetPacketID( iqNode )) == -1 ) || (( type = JabberXmlGetAttrValue( iqNode, "type" )) == NULL )) {
		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
		return;
	}

	if ( !lstrcmp( type, _T("result"))) {
		XmlNode* queryNode = JabberXmlGetNthChild( iqNode, "query", 1 );
		XmlNode* xNode = JabberXmlGetChildWithGivenAttrValue( queryNode, "x", "xmlns", _T("jabber:x:data"));
		if (xNode) {
			//1. Form
			int i=1;
			XmlNode* itemNode;
			while ( itemNode = JabberXmlGetNthChild( xNode, "item", i )) {
				pUserColumn = new U_TCHAR_MAP(10);
				int j = 1;
				XmlNode* fieldNode;
				while ( fieldNode = JabberXmlGetNthChild( itemNode, "field", j )) {
					TCHAR* var = JabberXmlGetAttrValue( fieldNode, "var" );
					if ( var ) {
						TCHAR* Text = JabberXmlGetChild( fieldNode, "value" )->text;
						if ( Text ) {
							mColumnsNames.insert( var, var );
							pUserColumn->insert( var, Text );
					}	}
					j++;
				}
				SearchResults.insert(( void* )pUserColumn );
				i++;
			}

			XmlNode* reportNode = JabberXmlGetNthChild( xNode, "reported", 1 );
			if ( reportNode ) {
				i = 1;
				while(XmlNode* fieldNode = JabberXmlGetNthChild( reportNode, "field", i++ )) {
					TCHAR* var = JabberXmlGetAttrValue( fieldNode, "var" );
					if ( var && mColumnsNames.getIndex( var ) >= 0 ) {
						TCHAR * Label=JabberXmlGetAttrValue(fieldNode,"label");
						mReportedColumns.insert(var, (Label!=NULL) ? Label : var);
			}	}	}
		}
		else {
			//2. Field list
			int i=1;
			while ( XmlNode* itemNode = JabberXmlGetNthChild( queryNode, "item", i++ )) {
				pUserColumn = new U_TCHAR_MAP(10);
				bFreeColumnNames = TRUE;
				
				TCHAR* jid = JabberXmlGetAttrValue( itemNode, "jid" );
				int index = mReportedColumns.getIndex( _T("jid"));
				if ( index < 0 ) {
					TCHAR* szTJid = a2t("jid");
					mReportedColumns.insert( szTJid, szTJid );
					mColumnsNames.insert( szTJid, szTJid );
					pUserColumn->insert( szTJid, jid );
				}
				else pUserColumn->insert( _T("jid"), jid );

				for ( int j=0; j < itemNode->numChild; j++ ) {
					XmlNode* child = itemNode->child[j];
					if ( child->name ) {
						TCHAR* szColumnName = a2t( child->name );
						TCHAR* szWorkName;
						index = mReportedColumns.getIndex( szColumnName );
						if ( index < 0 ) {
							mReportedColumns.insert( szColumnName, szColumnName ); //this item should be freed
							szWorkName = szColumnName;
						}
						else {
							index = mReportedColumns.getIndex( szColumnName );
							szWorkName = mReportedColumns.getKeyName( index );
							mir_free( szColumnName );
						}
						if ( child->text && child->text[0] != _T('\0')) {
							mColumnsNames.insert( szWorkName, szWorkName );
							pUserColumn->insert( szWorkName, child->text );
				}	}	}

				SearchResults.insert((void*)pUserColumn);
		}	}
	}
	else if ( !lstrcmp( type, _T("error")))
	{
		TCHAR* code=NULL;
		TCHAR* description=NULL;
		TCHAR buff[255];
		XmlNode* errorNode = JabberXmlGetChild(iqNode,"error");
		if ( errorNode ) {
			code = JabberXmlGetAttrValue(errorNode,"code");
			description = errorNode->text;
		}
		_sntprintf( buff, SIZEOF( buff ), TranslateT( "Error %s %s\r\nTry to specify more detailed"),
			code ? code : _T(""), description ? description : _T(""));
		JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE ) id, 0 );
		if ( searchHandleDlg )
			SetDlgItemText( searchHandleDlg, IDC_INSTRUCTIONS, buff );
		else 
			MessageBox( NULL, buff, TranslateT( "Search error" ), MB_OK|MB_ICONSTOP);
		return;
	}

	U_TCHAR_MAP* workingColumnMap;
	if ( mReportedColumns.getCount() > 0 ) 
		workingColumnMap = &mReportedColumns;
	else 
		workingColumnMap = &mColumnsNames;
	int nCount = mColumnsNames.getCount();
	int nUserFound = SearchResults.getCount();
	int i, currcolumn;

	JABBER_CUSTOMSEARCHRESULTS Results = { 0 };
	Results.nSize = sizeof(Results);
	Results.pszFields = ( TCHAR** )mir_alloc(sizeof(TCHAR*)*nCount);
	Results.nFieldCount = nCount;

	for ( i=currcolumn=0; i < workingColumnMap->getCount() && currcolumn < nCount; i++ ) {
		TCHAR* columnName = workingColumnMap->getUnOrderedKeyName( i );
		if ( mColumnsNames.getIndex( columnName ) >= 0 ) //if field exists in common columns
			Results.pszFields[ currcolumn++ ] = workingColumnMap->getUnOrdered( i );
	}

	// sending column names
	Results.jsr.hdr.cbSize = 0;
	JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, ( HANDLE ) id, (LPARAM) &Results );

	//send set info (the row items info)
	Results.jsr.hdr.cbSize = sizeof( Results.jsr );
	for ( i=0; i < nUserFound; i++ ) {
		U_TCHAR_MAP* mInfo = ( U_TCHAR_MAP* )SearchResults[i];
		if ( mInfo ) {
			TCHAR* jid;
			for ( int column = 0, currcolumn = 0; column < workingColumnMap->getCount() && currcolumn < nCount; column++ ) {
				TCHAR* szValue = NULL;
				TCHAR* szVar = workingColumnMap->getUnOrderedKeyName(column);
				if ( szVar ) {
					szValue = mInfo->operator [](szVar);				
					if ( !_tcsicmp( szVar,_T("jid")))
						jid = szValue;
				}
				Results.pszFields[ currcolumn++ ] = szValue;
			}
			_tcsncpy(Results.jsr.jid,jid,SIZEOF(Results.jsr.jid));
			//send set info (the row data for contact)  
			JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, ( HANDLE ) id, (LPARAM) &Results );
			delete mInfo;
	}	}

	mir_free( Results.pszFields );
	if ( bFreeColumnNames ) {
		i=0;
		while ( TCHAR* name = mColumnsNames.getKeyName( i++ ))
			mir_free( name );
	}

	//send success to finish searching
	JSendBroadcast( NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, ( HANDLE )id, 0 );
}

static void JabberSearchFreeData(HWND hwndDlg, JabberSearchData * dat)
{
	//lock
	if ( dat->nJSInfCount && dat->pJSInf ) {
		for ( int i=0; i < dat->nJSInfCount; i++ ) {
			if (dat->pJSInf[i].hwndValueItem)
				DestroyWindow(dat->pJSInf[i].hwndValueItem);			
			if (dat->pJSInf[i].hwndCaptionItem)
				DestroyWindow(dat->pJSInf[i].hwndCaptionItem);
			if (dat->pJSInf[i].szFieldCaption) 
				free(dat->pJSInf[i].szFieldCaption);
			if (dat->pJSInf[i].szFieldName) 
				free(dat->pJSInf[i].szFieldName);
		}
		free(dat->pJSInf);
		dat->pJSInf=NULL;
		dat->nJSInfCount=0;
	}
	ShowWindow( GetDlgItem( hwndDlg, IDC_VSCROLL ), SW_HIDE );
	SetDlgItemText(hwndDlg, IDC_INSTRUCTIONS, TranslateT( "Select/type search service URL above and press <Go>" ));
	//unlock
}

static void JabberSearchRefreshFrameScroll(HWND hwndDlg, JabberSearchData * dat)
{
	HWND hFrame = GetDlgItem( hwndDlg, IDC_FRAME );
	HWND hwndScroll = GetDlgItem( hwndDlg, IDC_VSCROLL );
	RECT rc;
	GetClientRect( hFrame, &rc );
	GetClientRect( hFrame, &dat->frameRect );
	dat->frameHeight = rc.bottom-rc.top;
	if ( dat->frameHeight < dat->CurrentHeight ) {
		ShowWindow( hwndScroll, SW_SHOW );
		EnableWindow( hwndScroll, TRUE );
	}
	else ShowWindow( hwndScroll, SW_HIDE );

	SetScrollRange( hwndScroll, SB_CTL, 0, dat->CurrentHeight-dat->frameHeight, FALSE );
}

static int JabberSearchRenewFields( HWND hwndDlg, JabberSearchData* dat )
{
	char szServerName[100];
	EnableWindow( GetDlgItem(hwndDlg, IDC_GO), FALSE );
	GetDlgItemTextA( hwndDlg, IDC_SERVER, szServerName, sizeof( szServerName ));
	dat->CurrentHeight = 0;
	JabberSearchRefreshFrameScroll( hwndDlg, dat );
	JabberSearchFreeData( hwndDlg, dat );

	if ( jabberOnline ) 
		SetDlgItemText( hwndDlg, IDC_INSTRUCTIONS, TranslateT( "Please wait...\r\nConnecting search server..." ));
	else
		SetDlgItemText( hwndDlg, IDC_INSTRUCTIONS, TranslateT( "You have to be connected to server" ));

	if ( !jabberOnline )
		return 0;

	searchHandleDlg = hwndDlg;

	int iqId = JabberSerialNext();
	XmlNodeIq iq( "get", iqId, szServerName );
	XmlNode* query = iq.addChild( "query" );
	query->addAttr( "xmlns", "jabber:iq:search" );
	JabberIqAdd( iqId, IQ_PROC_GETSEARCHFIELDS, JabberIqResultGetSearchFields );
	jabberThreadInfo->send( iq );
	return iqId;
}

static void JabberSearchAddUrlToRecentCombo( HWND hwndDlg, TCHAR* szAddr )
{
	int lResult = SendMessage( GetDlgItem(hwndDlg,IDC_SERVER), (UINT) CB_FINDSTRING, 0, (LPARAM)szAddr );
	if ( lResult == -1 )
		SendDlgItemMessage( hwndDlg,IDC_SERVER,CB_ADDSTRING,0,(LPARAM)szAddr);
}

static void JabberSearchDeleteFromRecent(TCHAR * szAddr,BOOL deleteLastFromDB=TRUE)
{
	DBVARIANT dbv;
	char key[30];
	//search in recent
	for ( int i=0; i<10; i++ ) {
		sprintf( key, "RecentlySearched_%d", i );
		if ( !JGetStringT( NULL, key, &dbv )) {
			if ( !_tcsicmp( szAddr, dbv.ptszVal )) {
				JFreeVariant( &dbv );	
				for ( int j=i; j < 10; j++ ) {
					sprintf( key, "RecentlySearched_%d", j+1 );
					if ( !JGetStringT( NULL, key, &dbv )) {
						sprintf( key, "RecentlySearched_%d", j );	
						JSetStringT( NULL, key, dbv.ptszVal );
						JFreeVariant( &dbv );
					}
					else if ( deleteLastFromDB ) {
						sprintf(key,"RecentlySearched_%d",j);
						JDeleteSetting(NULL,key);
					}
					break;
				}
				break;
			}
			else JFreeVariant( &dbv );
}	}	}

static void JabberSearchAddToRecent(TCHAR * szAddr, HWND hwndDialog=NULL)
{
	DBVARIANT dbv;
	char key[30];
	JabberSearchDeleteFromRecent( szAddr );
	for ( int j=9; j > 0; j-- ) {
		sprintf( key, "RecentlySearched_%d", j-1 );
		if ( !JGetStringT( NULL, key, &dbv )) {
			sprintf(key,"RecentlySearched_%d",j);	
			JSetStringT(NULL,key,dbv.ptszVal);
			JFreeVariant(&dbv);
	}	}

	sprintf( key, "RecentlySearched_%d", 0 );
	JSetStringT( NULL, key, szAddr );
	if ( hwndDialog )
		JabberSearchAddUrlToRecentCombo( hwndDialog, szAddr );
}

static BOOL CALLBACK JabberSearchAdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberSearchData* dat = ( JabberSearchData* )GetWindowLong( hwndDlg, GWL_USERDATA );
	switch (msg) {	
	case WM_INITDIALOG:
		{
			TranslateDialogDefault( hwndDlg );
			dat = ( JabberSearchData* )malloc( sizeof( JabberSearchData ));
			memset( dat, 0, sizeof( JabberSearchData ));
			SetWindowLong( hwndDlg, GWL_USERDATA, ( LPARAM )dat );

			/* Server Combo box */
			char szServerName[100];
			if ( JGetStaticString( "Jud", NULL, szServerName, sizeof szServerName ))
				strcpy( szServerName, "users.jabber.org" );
			SetDlgItemTextA( hwndDlg, IDC_SERVER, szServerName );
			SendDlgItemMessageA( hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, ( LPARAM )szServerName );
			//TO DO: Add Transports here
			int transpCount = jabberTransports.getCount();
			for ( int i=0; i < transpCount; i++) {
				TCHAR* szTransp = jabberTransports[ i ];
				if ( szTransp )
					JabberSearchAddUrlToRecentCombo( hwndDlg, szTransp );
			}

			for ( int j=0; j < 10; j++ ) {
				DBVARIANT dbv;
				char key[30];
				sprintf( key, "RecentlySearched_%d", j );
				if ( !JGetStringT( NULL, key, &dbv )) {
					JabberSearchAddUrlToRecentCombo( hwndDlg, dbv.ptszVal );
					JFreeVariant( &dbv );
			}	}

			//TO DO: Add 4 recently used			
			dat->lastRequestIq = JabberSearchRenewFields( hwndDlg, dat );
			return TRUE;
		}

	case WM_COMMAND:
		if ( LOWORD( wParam ) == IDC_SERVER ) {
			switch ( HIWORD( wParam )) {
			case CBN_EDITCHANGE:
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO),TRUE);
				return TRUE;

			case CBN_EDITUPDATE:
				JabberSearchFreeData(hwndDlg, dat);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO),TRUE);
				return TRUE;

			case CBN_SELENDOK:
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO),TRUE);
				PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_GO,BN_CLICKED),0);
				return TRUE;
			}
		}
		else if ( LOWORD( wParam ) == IDC_GO && HIWORD( wParam ) == BN_CLICKED ) {
			JabberSearchRenewFields( hwndDlg, dat );
			return TRUE;
		}
		break;

	case WM_SIZE:
		{
			//Resize IDC_FRAME to take full size
			RECT rcForm;
			GetWindowRect(hwndDlg,  &rcForm);
			RECT rcFrame;
			GetWindowRect(GetDlgItem(hwndDlg,IDC_FRAME),  &rcFrame);
			rcFrame.bottom=rcForm.bottom;
			SetWindowPos(GetDlgItem(hwndDlg,IDC_FRAME),NULL,0,0,rcFrame.right-rcFrame.left,rcFrame.bottom-rcFrame.top,SWP_NOZORDER|SWP_NOMOVE);
			GetWindowRect(GetDlgItem(hwndDlg,IDC_VSCROLL), &rcForm);
			SetWindowPos(GetDlgItem(hwndDlg,IDC_VSCROLL),NULL,0,0,rcForm.right-rcForm.left,rcFrame.bottom-rcFrame.top,SWP_NOZORDER|SWP_NOMOVE);
			JabberSearchRefreshFrameScroll(hwndDlg, dat);
		}
		return TRUE;

	case WM_USER+10:
		{			
			Data* MyDat = ( Data* )lParam;
			dat->fSearchRequestIsXForm = ( BOOL )wParam;
			dat->CurrentHeight = JabberSearchAddField( hwndDlg, MyDat->Label, MyDat->Var, MyDat->Order );
			mir_free( MyDat->Label );
			mir_free( MyDat->Var );
			free( MyDat );
			JabberSearchRefreshFrameScroll( hwndDlg, dat );
			return TRUE;
		}
	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if ( zDelta ) {
				int nScrollLines = 0;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, ( void* )&nScrollLines, 0 );
				for (int i=0; i < (nScrollLines+1)/2; i++ )
					SendMessage( hwndDlg, WM_VSCROLL, ( zDelta < 0 ) ? SB_LINEDOWN : SB_LINEUP, 0 );
		}	}
		return TRUE;

	case WM_VSCROLL:
		if ( dat != NULL ) {
			int pos = dat->curPos;
			switch ( LOWORD( wParam )) {
			case SB_LINEDOWN:
				pos += 10;
				break;
			case SB_LINEUP:
				pos -= 10;
				break;
			case SB_PAGEDOWN:
				pos += ( dat->CurrentHeight - 10 );
				break;
			case SB_PAGEUP:
				pos -= ( dat->CurrentHeight - 10 );
				break;
			case SB_THUMBTRACK:
				pos = HIWORD( wParam );
				break;
			}
			if ( pos > ( dat->CurrentHeight - dat->frameHeight ))
				pos = dat->CurrentHeight - dat->frameHeight;
			if ( pos < 0 )
				pos = 0;
			if ( dat->curPos != pos ) {
				ScrollWindow( GetDlgItem( hwndDlg, IDC_FRAME ), 0, dat->curPos - pos, NULL ,  &( dat->frameRect ));
				SetScrollPos( GetDlgItem( hwndDlg, IDC_VSCROLL ), SB_CTL, pos, TRUE );
				RECT Invalid=dat->frameRect;
				if ( dat->curPos - pos > 0 )
					Invalid.bottom = Invalid.top + ( dat->curPos - pos );
				else
					Invalid.top = Invalid.bottom + ( dat->curPos - pos );

				RedrawWindow( GetDlgItem( hwndDlg, IDC_FRAME ), NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN );
				dat->curPos = pos;
		}	}
		return TRUE;

	case WM_DESTROY:
		JabberSearchFreeData(hwndDlg,dat);
		dat=NULL;
		return TRUE;
	}

	return FALSE;
}

int JabberSearchCreateAdvUI(WPARAM wParam, LPARAM lParam)
{
	if ( lParam && hInst ) {
		HWND hwnd = CreateDialog( hInst, MAKEINTRESOURCE(IDD_SEARCHUSER), (HWND)lParam, JabberSearchAdvancedDlgProc );
		return (int)hwnd;
	}
	return 0; // Failure
}

//////////////////////////////////////////////////////////////////////////
// The function formats request to server 

int JabberSearchByAdvanced( WPARAM wParam, LPARAM lParam )
{
	if ( !jabberOnline || !lParam)
		return 0;	//error

	HWND hwndDlg = ( HWND )lParam;
	JabberSearchData* dat = ( JabberSearchData* )GetWindowLong( hwndDlg, GWL_USERDATA );
	if ( !dat )
		return 0; //error

	// check if server connected (at least one field exists)
	if ( dat->nJSInfCount == 0 ) return 0;	

	// formating request
	BOOL fRequestNotEmpty=FALSE;

	// get server name
	char szServerName[100];
	GetDlgItemTextA(hwndDlg,IDC_SERVER, szServerName, sizeof(szServerName));

	// formating query
	int iqId = JabberSerialNext();
	XmlNodeIq iq( "set", iqId, szServerName );
	XmlNode* query = iq.addChild( "query" ), *field, *x;	
	iq.addAttr( "xml:lang", "en" ); //? not sure is it needed ?
	query->addAttr( "xmlns", "jabber:iq:search" );

	// next can be 2 cases:
	// Forms: XEP-0055 Example 7
	if ( dat->fSearchRequestIsXForm ) {
		x = query->addChild( "x" );
		x->addAttr( "xmlns", "jabber:x:data" ); 
		x->addAttr( "type", "submit" );

		// Next will be the cycle through fields
		for ( int i=0; i < dat->nJSInfCount; i++ ) {
			TCHAR szFieldValue[100];
			GetWindowText( dat->pJSInf[i].hwndValueItem, szFieldValue, SIZEOF( szFieldValue ));
			if ( szFieldValue[ 0 ] != '\0' ) {
				field = x->addChild( "field" ); 
				//field->addAttr( "var", dat->pJSInf[i].szFieldName );
				field->addChild( "value", szFieldValue );
				fRequestNotEmpty=TRUE;
		}	}
	}
	else { //and Simple fields: XEP-0055 Example 3 	
		for ( int i=0; i < dat->nJSInfCount; i++ ) {
			TCHAR szFieldValue[100];
			GetWindowText( dat->pJSInf[i].hwndValueItem, szFieldValue, SIZEOF( szFieldValue ));
			if ( szFieldValue[0] != 0 ) {
				char* szTemp = t2a( dat->pJSInf[i].szFieldName );
				field = query->addChild( szTemp, szFieldValue ); 
				mir_free( szTemp );
				fRequestNotEmpty = TRUE;
	}	}	}

	if ( fRequestNotEmpty ) {
		// register search request result handler
		JabberIqAdd( iqId, IQ_PROC_GETSEARCH, JabberIqResultAdvancedSearch );		
		// send request
		jabberThreadInfo->send( iq );
		return iqId;
	}
	return 0;
}

/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005     George Hazan

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

*/

#include "jabber.h"
#include <commctrl.h>
#include "jabber_list.h"
#include "resource.h"

static BOOL CALLBACK JabberUserInfoDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );
static BOOL CALLBACK JabberUserPhotoDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

int JabberUserInfoInit( WPARAM wParam, LPARAM lParam )
{
	if ( !JCallService( MS_PROTO_ISPROTOCOLLOADED, 0, ( LPARAM )jabberProtoName ))
		return 0;

	HANDLE hContact = ( HANDLE )lParam;
	char* szProto = ( char* )JCallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
	if ( szProto != NULL && !strcmp( szProto, jabberProtoName )) {
		OPTIONSDIALOGPAGE odp = {0};
		odp.cbSize = sizeof( odp );
		odp.hIcon = NULL;
		odp.hInstance = hInst;
		odp.pfnDlgProc = JabberUserInfoDlgProc;
		odp.position = -2000000000;
		odp.pszTemplate = MAKEINTRESOURCE( IDD_INFO_JABBER );
		odp.pszTitle = jabberModuleName;
		JCallService( MS_USERINFO_ADDPAGE, wParam, ( LPARAM )&odp );
		odp.pfnDlgProc = JabberUserPhotoDlgProc;
		odp.position = 2000000000;
		odp.pszTemplate = MAKEINTRESOURCE( IDD_VCARD_PHOTO );
		odp.pszTitle = JTranslate( "Photo" );
		JCallService( MS_USERINFO_ADDPAGE, wParam, ( LPARAM )&odp );
	}
	return 0;
}

static BOOL CALLBACK JabberUserInfoDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		// lParam is hContact
		TranslateDialogDefault( hwndDlg );
		SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG )( HANDLE ) lParam );
		SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
		return TRUE;
	case WM_JABBER_REFRESH:
		{
			DBVARIANT dbv;
			int count, index;
			JABBER_LIST_ITEM *item;
			JABBER_RESOURCE_STATUS *r;
			char* localResource;

			HWND hwndList = GetDlgItem( hwndDlg, IDC_INFO_RESOURCE );
			SendMessage( hwndList, LB_RESETCONTENT, 0, 0 );
			SetDlgItemText( hwndDlg, IDC_INFO_JID, "" );
			SetDlgItemText( hwndDlg, IDC_SUBSCRIPTION, "" );
			SetDlgItemText( hwndDlg, IDC_SOFTWARE, JTranslate( "<click resource to view>" ));
			SetDlgItemText( hwndDlg, IDC_VERSION, JTranslate( "<click resource to view>" ));
			SetDlgItemText( hwndDlg, IDC_SYSTEM, JTranslate( "<click resource to view>" ));
			EnableWindow( GetDlgItem( hwndDlg, IDC_SOFTWARE ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_VERSION ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, IDC_SYSTEM ), FALSE );

			HANDLE hContact = ( HANDLE ) GetWindowLong( hwndDlg, GWL_USERDATA );
			if ( !DBGetContactSetting( hContact, jabberProtoName, "jid", &dbv )) {
				char* jid = dbv.pszVal;
				SetDlgItemText( hwndDlg, IDC_INFO_JID, jid );
				if ( jabberOnline ) {
					if (( item=JabberListGetItemPtr( LIST_ROSTER, jid )) != NULL ) {
						if (( r=item->resource ) != NULL ) {
							count = item->resourceCount;
							for ( int i=0; i<count; i++ ) {
								localResource = JabberTextDecode( r[i].resourceName );
								index = SendMessage( hwndList, LB_ADDSTRING, 0, ( LPARAM )localResource );
								SendMessage( hwndList, LB_SETITEMDATA, index, ( LPARAM )r[i].resourceName );
								free( localResource );
						}	}

						switch ( item->subscription ) {
						case SUB_BOTH:
							SetDlgItemText( hwndDlg, IDC_SUBSCRIPTION, JTranslate( "both" ));
							break;
						case SUB_TO:
							SetDlgItemText( hwndDlg, IDC_SUBSCRIPTION, JTranslate( "to" ));
							break;
						case SUB_FROM:
							SetDlgItemText( hwndDlg, IDC_SUBSCRIPTION, JTranslate( "from" ));
							break;
						default:
							SetDlgItemText( hwndDlg, IDC_SUBSCRIPTION, JTranslate( "none" ));
							break;
					}	}
					else SetDlgItemText( hwndDlg, IDC_SUBSCRIPTION, JTranslate( "none ( not on roster )" ));
				}
				else EnableWindow( hwndList, FALSE );
				JFreeVariant( &dbv );
		}	}
		break;
	case WM_NOTIFY:
		switch (( ( LPNMHDR )lParam )->idFrom ) {
		case 0:
			switch (( ( LPNMHDR )lParam )->code ) {
			case PSN_INFOCHANGED:
				{
					HANDLE hContact = ( HANDLE ) (( LPPSHNOTIFY ) lParam )->lParam;
					SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, ( LPARAM )hContact );
				}
				break;
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_INFO_RESOURCE:
			switch ( HIWORD( wParam )) {
			case LBN_SELCHANGE:
				{
					DBVARIANT dbv;
					char* jid;

					HWND hwndList = GetDlgItem( hwndDlg, IDC_INFO_RESOURCE );
					HANDLE hContact = ( HANDLE ) GetWindowLong( hwndDlg, GWL_USERDATA );
					if ( !DBGetContactSetting( hContact, jabberProtoName, "jid", &dbv )) {
						jid = dbv.pszVal;
						int nItem = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
						char* szResource = ( char* )SendMessage( hwndList, LB_GETITEMDATA, ( WPARAM ) nItem, 0 );
						JABBER_LIST_ITEM* item = JabberListGetItemPtr( LIST_ROSTER, jid );
						JABBER_RESOURCE_STATUS *r;

						if ( szResource != ( char* )LB_ERR && item != NULL && ( r=item->resource ) != NULL ) {
							for ( int i=0; i<item->resourceCount && strcmp( r[i].resourceName, szResource ); i++ );
							if ( i < item->resourceCount ) {
								if ( r[i].software != NULL ) {
									SetDlgItemText( hwndDlg, IDC_SOFTWARE, r[i].software );
									EnableWindow( GetDlgItem( hwndDlg, IDC_SOFTWARE ), TRUE );
								}
								else {
									SetDlgItemText( hwndDlg, IDC_SOFTWARE, JTranslate( "<not specified>" ));
									EnableWindow( GetDlgItem( hwndDlg, IDC_SOFTWARE ), FALSE );
								}
								if ( r[i].version != NULL ) {
									SetDlgItemText( hwndDlg, IDC_VERSION, r[i].version );
									EnableWindow( GetDlgItem( hwndDlg, IDC_VERSION ), TRUE );
								}
								else {
									SetDlgItemText( hwndDlg, IDC_VERSION, JTranslate( "<not specified>" ));
									EnableWindow( GetDlgItem( hwndDlg, IDC_VERSION ), FALSE );
								}
								if ( r[i].system != NULL ) {
									SetDlgItemText( hwndDlg, IDC_SYSTEM, r[i].system );
									EnableWindow( GetDlgItem( hwndDlg, IDC_SYSTEM ), TRUE );
								}
								else {
									SetDlgItemText( hwndDlg, IDC_SYSTEM, JTranslate( "<not specified>" ));
									EnableWindow( GetDlgItem( hwndDlg, IDC_SYSTEM ), FALSE );
						}	}	}
						JFreeVariant( &dbv );
			}	}	}
			break;
		}
		break;
	}
	return FALSE;
}

typedef struct {
	HANDLE hContact;
	HBITMAP hBitmap;
} USER_PHOTO_INFO;

static BOOL CALLBACK JabberUserPhotoDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	USER_PHOTO_INFO *photoInfo;

	photoInfo = ( USER_PHOTO_INFO * ) GetWindowLong( hwndDlg, GWL_USERDATA );

	switch ( msg ) {
	case WM_INITDIALOG:
		// lParam is hContact
		TranslateDialogDefault( hwndDlg );
		photoInfo = ( USER_PHOTO_INFO * ) malloc( sizeof( USER_PHOTO_INFO ));
		photoInfo->hContact = ( HANDLE ) lParam;
		photoInfo->hBitmap = NULL;
		SetWindowLong( hwndDlg, GWL_USERDATA, ( LONG ) photoInfo );
		SendMessage( GetDlgItem( hwndDlg, IDC_SAVE ), BM_SETIMAGE, IMAGE_ICON, ( LPARAM )LoadImage( hInst, MAKEINTRESOURCE( IDI_SAVE ), IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), 0 ));
		ShowWindow( GetDlgItem( hwndDlg, IDC_LOAD ), SW_HIDE );
		ShowWindow( GetDlgItem( hwndDlg, IDC_DELETE ), SW_HIDE );
		SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
		return TRUE;
	case WM_NOTIFY:
		switch (( ( LPNMHDR )lParam )->idFrom ) {
		case 0:
			switch (( ( LPNMHDR )lParam )->code ) {
			case PSN_INFOCHANGED:
				SendMessage( hwndDlg, WM_JABBER_REFRESH, 0, 0 );
				break;
			}
			break;
		}
		break;
	case WM_JABBER_REFRESH:
		{
			JABBER_LIST_ITEM *item;
			DBVARIANT dbv;
			char* jid;

			if ( photoInfo->hBitmap ) {
				DeleteObject( photoInfo->hBitmap );
				photoInfo->hBitmap = NULL;
			}
			ShowWindow( GetDlgItem( hwndDlg, IDC_SAVE ), SW_HIDE );
			if ( !DBGetContactSetting( photoInfo->hContact, jabberProtoName, "jid", &dbv )) {
				jid = dbv.pszVal;
				if (( item=JabberListGetItemPtr( LIST_ROSTER, jid )) != NULL ) {
					if ( item->photoFileName ) {
						JabberLog( "Showing picture from %s", item->photoFileName );
						photoInfo->hBitmap = ( HBITMAP ) JCallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )item->photoFileName );
						ShowWindow( GetDlgItem( hwndDlg, IDC_SAVE ), SW_SHOW );
					}
				}
				JFreeVariant( &dbv );
			}
			InvalidateRect( hwndDlg, NULL, TRUE );
			UpdateWindow( hwndDlg );
		}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_SAVE:
			{
				DBVARIANT dbv;
				JABBER_LIST_ITEM *item;
				HANDLE hFile;
				OPENFILENAME ofn;
				static char szFilter[512];
				unsigned char buffer[3];
				char szFileName[_MAX_PATH];
				char* jid;
				DWORD n;

				if ( DBGetContactSetting( photoInfo->hContact, jabberProtoName, "jid", &dbv ))
					break;
				jid = dbv.pszVal;
				if (( item=JabberListGetItemPtr( LIST_ROSTER, jid )) != NULL ) {
					if (( hFile=CreateFile( item->photoFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL )) != INVALID_HANDLE_VALUE ) {
						if ( ReadFile( hFile, buffer, 3, &n, NULL ) && n==3 ) {
							if ( !strncmp(( char* )buffer, "BM", 2 )) {
								mir_snprintf( szFilter, sizeof( szFilter ), "BMP %s ( *.bmp )", JTranslate( "format" ));
								n = strlen( szFilter );
								strncpy( szFilter+n+1, "*.BMP", sizeof( szFilter )-n-2 );
							}
							else if ( !strncmp(( char* )buffer, "GIF", 3 )) {
								mir_snprintf( szFilter, sizeof( szFilter ), "GIF %s ( *.gif )", JTranslate( "format" ));
								n = strlen( szFilter );
								strncpy( szFilter+n+1, "*.GIF", sizeof( szFilter )-n-2 );
							}
							else if ( buffer[0]==0xff && buffer[1]==0xd8 && buffer[2]==0xff ) {
								mir_snprintf( szFilter, sizeof( szFilter ), "JPEG %s ( *.jpg;*.jpeg )", JTranslate( "format" ));
								n = strlen( szFilter );
								strncpy( szFilter+n+1, "*.JPG;*.JPEG", sizeof( szFilter )-n-2 );
							}
							else {
								mir_snprintf( szFilter, sizeof( szFilter ), "%s ( *.* )", JTranslate( "Unknown format" ));
								n = strlen( szFilter );
								strncpy( szFilter+n+1, "*.*", sizeof( szFilter )-n-2 );
							}
							szFilter[sizeof( szFilter )-1] = '\0';

#ifndef OPENFILENAME_SIZE_VERSION_400
							ofn.lStructSize = sizeof( OPENFILENAME );
#else
							ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#endif
							ofn.hwndOwner = hwndDlg;
							ofn.hInstance = NULL;
							ofn.lpstrFilter = szFilter;
							ofn.lpstrCustomFilter = NULL;
							ofn.nMaxCustFilter = 0;
							ofn.nFilterIndex = 0;
							ofn.lpstrFile = szFileName;
							ofn.nMaxFile = _MAX_PATH;
							ofn.lpstrFileTitle = NULL;
							ofn.nMaxFileTitle = 0;
							ofn.lpstrInitialDir = NULL;
							ofn.lpstrTitle = NULL;
							ofn.Flags = OFN_OVERWRITEPROMPT;
							ofn.nFileOffset = 0;
							ofn.nFileExtension = 0;
							ofn.lpstrDefExt = NULL;
							ofn.lCustData = 0L;
							ofn.lpfnHook = NULL;
							ofn.lpTemplateName = NULL;
							szFileName[0] = '\0';
							if ( GetSaveFileName( &ofn )) {
								JabberLog( "File selected is %s", szFileName );
								CopyFile( item->photoFileName, szFileName, FALSE );
							}
						}
						CloseHandle( hFile );
					}
				}
				JFreeVariant( &dbv );

			}
			break;
		}
		break;
	case WM_PAINT:
		if ( !jabberOnline ) {
			SetDlgItemText( hwndDlg, IDC_CANVAS, JTranslate( "<Photo not available while offline>" ));
		}
		else if ( !photoInfo->hBitmap ) {
			SetDlgItemText( hwndDlg, IDC_CANVAS, JTranslate( "<No photo>" ));
		}
		else {
			BITMAP bm;
			HDC hdcMem;
			HWND hwndCanvas;
			HDC hdcCanvas;
			POINT ptSize, ptOrg, pt, ptFitSize;
			RECT rect;
			HBITMAP hBitmap;

			SetDlgItemText( hwndDlg, IDC_CANVAS, "" );
			hBitmap = photoInfo->hBitmap;
			hwndCanvas = GetDlgItem( hwndDlg, IDC_CANVAS );
			hdcCanvas = GetDC( hwndCanvas );
			hdcMem = CreateCompatibleDC( hdcCanvas );
			SelectObject( hdcMem, hBitmap );
			SetMapMode( hdcMem, GetMapMode( hdcCanvas ));
			GetObject( hBitmap, sizeof( BITMAP ), ( LPVOID ) &bm );
			ptSize.x = bm.bmWidth;
			ptSize.y = bm.bmHeight;
			DPtoLP( hdcCanvas, &ptSize, 1 );
			ptOrg.x = ptOrg.y = 0;
			DPtoLP( hdcMem, &ptOrg, 1 );
			GetClientRect( hwndCanvas, &rect );
			InvalidateRect( hwndCanvas, NULL, TRUE );
			UpdateWindow( hwndCanvas );
			if ( ptSize.x<=rect.right && ptSize.y<=rect.bottom ) {
				pt.x = ( rect.right - ptSize.x )/2;
				pt.y = ( rect.bottom - ptSize.y )/2;
				BitBlt( hdcCanvas, pt.x, pt.y, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCCOPY );
			}
			else {
				if (( ( float )( ptSize.x-rect.right ))/ptSize.x > (( float )( ptSize.y-rect.bottom ))/ptSize.y ) {
					ptFitSize.x = rect.right;
					ptFitSize.y = ( ptSize.y*rect.right )/ptSize.x;
					pt.x = 0;
					pt.y = ( rect.bottom - ptFitSize.y )/2;
				}
				else {
					ptFitSize.x = ( ptSize.x*rect.bottom )/ptSize.y;
					ptFitSize.y = rect.bottom;
					pt.x = ( rect.right - ptFitSize.x )/2;
					pt.y = 0;
				}
				SetStretchBltMode( hdcCanvas, COLORONCOLOR );
				StretchBlt( hdcCanvas, pt.x, pt.y, ptFitSize.x, ptFitSize.y, hdcMem, ptOrg.x, ptOrg.y, ptSize.x, ptSize.y, SRCCOPY );
			}
			DeleteDC( hdcMem );
		}
		break;
	case WM_DESTROY:
		if ( photoInfo->hBitmap ) {
			JabberLog( "Delete bitmap" );
			DeleteObject( photoInfo->hBitmap );
		}
		if ( photoInfo ) free( photoInfo );
		break;
	}
	return FALSE;
}
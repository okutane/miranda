/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright (c) 2003-5 George Hazan.
Copyright (c) 2002-3 Richard Hughes (original version).

This file uses the 'Webhost sample' code
Copyright(C) 2002 Chris Becke (http://www.mvps.org/user32/webhost.cab)

Miranda IM: the free icq client for MS Windows
Copyright (C) 2000-2002 Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Loads PNG2DIB library to handle PNG images. If DLL isn't found or can't be loaded,
// a special error dialog appears.

static HMODULE sttPngLib = NULL;
pfnConvertPng2dib png2dibConvertor = NULL;
pfnConvertDib2png dib2pngConvertor = NULL;
pfnGetVer         getver = NULL;

static BOOL CALLBACK LoadPng2dibProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{
			RECT tRect;
			GetWindowRect( hwndDlg, &tRect );
			HDC tHDC  = GetDC( hwndDlg );
			int tXXXX = GetDeviceCaps( tHDC, HORZRES );
			int tYYYY = GetDeviceCaps( tHDC, VERTRES );
			ReleaseDC( hwndDlg, tHDC );
			SetWindowPos( hwndDlg, HWND_TOP, tXXXX/2 - ( tRect.right-tRect.left )/2 - 1, tYYYY/2 - ( tRect.bottom-tRect.top )/2 - 1, 0, 0, SWP_NOSIZE );
		}
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_BTN_INSTALL:
				MSN_CallService( MS_UTILS_OPENURL, TRUE, ( LPARAM )"http://www.postman.ru/~ghazan/files/png2dib.mir" );
				EndDialog( hwndDlg, 1 );
				break;

			case IDC_BTN_DOWNLOAD:
				MSN_CallService( MS_UTILS_OPENURL, TRUE, ( LPARAM )"http://www.postman.ru/~ghazan/files/png2dib.zip" );
				EndDialog( hwndDlg, 2 );
				break;

			case IDC_BTN_CANCEL:
				EndDialog( hwndDlg, 3 );
				break;
	}	}	}

	return FALSE;
}

BOOL __stdcall MSN_LoadPngModule()
{
	if ( sttPngLib == NULL ) {
		if (( sttPngLib = LoadLibrary( "png2dib.dll" )) == NULL ) {
			char tDllPath[ MAX_PATH ];
			GetModuleFileName( hInst, tDllPath, sizeof( tDllPath ));
			{
				char* p = strrchr( tDllPath, '\\' );
				if ( p != NULL )
					strcpy( p+1, "png2dib.dll" );
				else
					strcpy( tDllPath, "png2dib.dll" );
			}

			if (( sttPngLib = LoadLibrary( tDllPath )) == NULL ) {
LBL_Error:
				DialogBox( hInst, MAKEINTRESOURCE( IDD_GET_PNG2DIB ), NULL, LoadPng2dibProc );
				return FALSE;
		}	}

		png2dibConvertor = ( pfnConvertPng2dib )GetProcAddress( sttPngLib, "mempng2dib" );
		dib2pngConvertor = ( pfnConvertDib2png )GetProcAddress( sttPngLib, "dib2mempng" );
		getver           = ( pfnGetVer )        GetProcAddress( sttPngLib, "getver" );
		if ( png2dibConvertor == NULL || dib2pngConvertor == NULL || getver == NULL ) {
			FreeLibrary( sttPngLib ); sttPngLib = NULL;
			MessageBox( NULL,
				MSN_Translate( "Your png2dib.dll is either obsolete or damaged. Press Ok to download the latest version" ),
				MSN_Translate( "Error" ),
				MB_OK | MB_ICONSTOP );

			goto LBL_Error;
	}	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN contact option page dialog procedure.

struct MsnDlgProcData
{
	HANDLE hContact;
	HANDLE hEventHook;
};

#define HM_REBIND_AVATAR  ( WM_USER + 1024 )

BOOL CALLBACK MsnDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{
			MsnDlgProcData* pData = new MsnDlgProcData;
			pData->hContact = ( HANDLE )lParam;
			pData->hEventHook = HookEventMessage( ME_PROTO_ACK, hwndDlg, HM_REBIND_AVATAR );
			SetWindowLong( hwndDlg, GWL_USERDATA, LONG( pData ));

			char tBuffer[ MAX_PATH ];
			if ( MSN_GetStaticString( "OnMobile", pData->hContact, tBuffer, sizeof tBuffer ))
				strcpy( tBuffer, "N" );
			SetDlgItemText( hwndDlg, IDC_MOBILE, tBuffer );

			if ( MSN_GetStaticString( "OnMsnMobile", pData->hContact, tBuffer, sizeof tBuffer ))
				strcpy( tBuffer, "N" );
			SetDlgItemText( hwndDlg, IDC_MSN_MOBILE, tBuffer );

			MSN_GetAvatarFileName(( HANDLE )lParam, tBuffer, sizeof tBuffer );

			if ( access( tBuffer, 0 )) {
LBL_Reread:
				DBWriteContactSettingString( pData->hContact, "ContactPhoto", "File", tBuffer );
				p2p_invite( pData->hContact, MSN_APPID_AVATAR );
				return TRUE;
			}

			char tSavedContext[ 256 ], tNewContext[ 256 ];
			if ( MSN_GetStaticString( "PictContext", pData->hContact, tNewContext, sizeof( tNewContext )) ||
				  MSN_GetStaticString( "PictSavedContext", pData->hContact, tSavedContext, sizeof( tSavedContext )))
				goto LBL_Reread;

			if ( stricmp( tNewContext, tSavedContext ))
				goto LBL_Reread;

			SendDlgItemMessage( hwndDlg, IDC_MSN_PICT, STM_SETIMAGE, IMAGE_BITMAP,
				( LPARAM )LoadImage( ::GetModuleHandle(NULL), tBuffer, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE ));
		}
		return TRUE;

	case HM_REBIND_AVATAR:
		{	MsnDlgProcData* pData = ( MsnDlgProcData* )GetWindowLong( hwndDlg, GWL_USERDATA );

			ACKDATA* ack = ( ACKDATA* )lParam;
			if ( ack->type == ACKTYPE_AVATAR && ack->hContact == pData->hContact ) {
				if ( ack->result == ACKRESULT_SUCCESS ) {
					PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )ack->hProcess;
					SendDlgItemMessage( hwndDlg, IDC_MSN_PICT, STM_SETIMAGE, IMAGE_BITMAP,
						( LPARAM )LoadImage( ::GetModuleHandle(NULL), AI->filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE ));
				}
				else if ( ack->result == ACKRESULT_STATUS )
					p2p_invite( ack->hContact, MSN_APPID_AVATAR );
		}	}
		break;

	case WM_DESTROY:
		MsnDlgProcData* pData = ( MsnDlgProcData* )GetWindowLong( hwndDlg, GWL_USERDATA );
		UnhookEvent( pData->hEventHook );
		delete pData;
	}

	return FALSE;
}

int MsnOnDetailsInit( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact = ( HANDLE )lParam;

	char* szProto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact, 0 );
	if (( szProto == NULL || strcmp( szProto, msnProtocolName )) && hContact )
		return 0;

	if ( !MSN_GetByte( "EnableAvatars", 0 ))
		return 0;

	if ( !MSN_GetDword( hContact, "FlagBits", 0 ))
		return 0;

	OPTIONSDIALOGPAGE odp;
	odp.cbSize = sizeof(odp);
	odp.hIcon = NULL;
	odp.hInstance = hInst;
	odp.pfnDlgProc = MsnDlgProc;
	odp.position = -1900000000;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_USEROPTS);
	odp.pszTitle = Translate(msnProtocolName);

	CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

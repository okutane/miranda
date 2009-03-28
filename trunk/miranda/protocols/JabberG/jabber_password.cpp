/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-09  George Hazan

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
#include "jabber_iq.h"
#include "jabber_caps.h"

static INT_PTR CALLBACK JabberChangePasswordDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam );

int __cdecl CJabberProto::OnMenuHandleChangePassword( WPARAM, LPARAM )
{
	if ( IsWindow( m_hwndJabberChangePassword ))
		SetForegroundWindow( m_hwndJabberChangePassword );
	else
		m_hwndJabberChangePassword = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_CHANGEPASSWORD ), NULL, JabberChangePasswordDlgProc, ( LPARAM )this );

	return 0;
}

static INT_PTR CALLBACK JabberChangePasswordDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	CJabberProto* ppro = (CJabberProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	switch ( msg ) {
	case WM_INITDIALOG:
		ppro = (CJabberProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, ( LONG_PTR )lParam );

		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )ppro->LoadIconEx( "key" ));
		TranslateDialogDefault( hwndDlg );
		if ( ppro->m_bJabberOnline && ppro->m_ThreadInfo!=NULL ) {
			TCHAR text[128];
			mir_sntprintf( text, SIZEOF( text ), _T("%s %s@") _T(TCHAR_STR_PARAM), TranslateT( "Set New Password for" ), ppro->m_ThreadInfo->username, ppro->m_ThreadInfo->server );
			SetWindowText( hwndDlg, text );
		}
		return TRUE;
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
			if ( ppro->m_bJabberOnline && ppro->m_ThreadInfo!=NULL ) {
				char newPasswd[128], text[128];
				GetDlgItemTextA( hwndDlg, IDC_NEWPASSWD, newPasswd, SIZEOF( newPasswd ));
				GetDlgItemTextA( hwndDlg, IDC_NEWPASSWD2, text, SIZEOF( text ));
				if ( strcmp( newPasswd, text )) {
					MessageBox( hwndDlg, TranslateT( "New password does not match." ), TranslateT( "Change Password" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
					break;
				}
				GetDlgItemTextA( hwndDlg, IDC_OLDPASSWD, text, SIZEOF( text ));
				if ( strcmp( text, ppro->m_ThreadInfo->password )) {
					MessageBox( hwndDlg, TranslateT( "Current password is incorrect." ), TranslateT( "Change Password" ), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND );
					break;
				}
				strncpy( ppro->m_ThreadInfo->newPassword, newPasswd, SIZEOF( ppro->m_ThreadInfo->newPassword ));

				int iqId = ppro->SerialNext();
				ppro->IqAdd( iqId, IQ_PROC_NONE, &CJabberProto::OnIqResultSetPassword );

				XmlNodeIq iq( _T("set"), iqId, _A2T(ppro->m_ThreadInfo->server));
				HXML q = iq << XQUERY( _T(JABBER_FEAT_REGISTER));
				q << XCHILD( _T("username"), ppro->m_ThreadInfo->username );
				q << XCHILD( _T("password"), _A2T(newPasswd));
				ppro->m_ThreadInfo->send( iq );
			}
			DestroyWindow( hwndDlg );
			break;
		case IDCANCEL:
			DestroyWindow( hwndDlg );
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;
	case WM_DESTROY:
		ppro->m_hwndJabberChangePassword = NULL;
		break;
	}

	return FALSE;
}

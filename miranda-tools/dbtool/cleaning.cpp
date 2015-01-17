/*
Miranda Database Tool
Miranda Database Tool
Copyright 2000-2015 Miranda IM project, 
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
#include "dbtool.h"

INT_PTR CALLBACK CleaningDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	INT_PTR bReturn;

	if(DoMyControlProcessing(hdlg,message,wParam,lParam,&bReturn)) return bReturn;
	switch(message) {
		case WM_INITDIALOG:
			CheckDlgButton(hdlg,IDC_ERASEHISTORY,opts.bEraseHistory);
			EnableWindow(GetDlgItem(hdlg,IDC_ERASEHISTORY),!opts.bAggressive);
			CheckDlgButton(hdlg,IDC_MARKREAD,opts.bMarkRead);
			CheckDlgButton(hdlg,IDC_CONVERTUTF,opts.bConvertUtf);
			TranslateDialog(hdlg);
			return TRUE;
		case WZN_PAGECHANGING:
			opts.bEraseHistory=IsDlgButtonChecked(hdlg,IDC_ERASEHISTORY)&&!opts.bAggressive;
			opts.bMarkRead=IsDlgButtonChecked(hdlg,IDC_MARKREAD);
			opts.bConvertUtf=IsDlgButtonChecked(hdlg,IDC_CONVERTUTF);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_BACK:
					if(opts.bCheckOnly)
						SendMessage(GetParent(hdlg),WZM_GOTOPAGE,IDD_SELECTDB,(LPARAM)SelectDbDlgProc);
					else
						SendMessage(GetParent(hdlg),WZM_GOTOPAGE,IDD_FILEACCESS,(LPARAM)FileAccessDlgProc);
					break;
				case IDOK:
					if (!opts.hFile) {
						opts.hFile = CreateFile( opts.filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
						if ( opts.hFile == INVALID_HANDLE_VALUE ) {
							opts.hFile = NULL;
							opts.error = GetLastError();
							SendMessage( GetParent(hdlg), WZM_GOTOPAGE, IDD_OPENERROR, ( LPARAM )OpenErrorDlgProc );
							break;
						}
					}
					SendMessage(GetParent(hdlg),WZM_GOTOPAGE,IDD_PROGRESS,(LPARAM)ProgressDlgProc);
					break;
			}
			break;
	}
	return FALSE;
}

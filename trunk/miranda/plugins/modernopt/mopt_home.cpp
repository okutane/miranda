/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2007 Artem Shpynov
Copyright 2000-2007 Miranda ICQ/IM project,

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
#include "modernopt.h"

extern HMODULE hInst;

INT_PTR CALLBACK ModernOptHome_DlgProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		for (i = 0; i < SIZEOF(g_ModernOptPages); ++i) {
			if (g_ModernOptPages[i].idcButton) {
				HWND hwndCtrl = GetDlgItem(hwndDlg, g_ModernOptPages[i].idcButton);
				if (g_ModernOptPages[i].bShow) {
					MDescButton_SetIconShared(hwndCtrl, LoadImage(hInst, MAKEINTRESOURCE(g_ModernOptPages[i].iIcon), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
					MDescButton_SetTitle(hwndCtrl, TranslateTS(g_ModernOptPages[i].lpzTitle));
					MDescButton_SetDescription(hwndCtrl, TranslateTS(g_ModernOptPages[i].lpzDescription));
				} 
				else ShowWindow(hwndCtrl, SW_HIDE);
		}	}

		return FALSE;

	case WM_COMMAND:
		switch ( LOWORD(wParam)) {
		case IDC_BTN_CLASSICOPT:
			PostMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
			{
				OPENOPTIONSDIALOG ood = {0};
				ood.cbSize = sizeof(ood);
				CallService(MS_OPT_OPENOPTIONS, 0, (LPARAM)&ood);
			}
			break;

		case IDC_BTN_HELP:
			ShellExecuteA(hwndDlg, "open", "http://www.miranda-im.org/", "", "", SW_SHOW);
			break;

		default:
			for (i = 0; i < SIZEOF(g_ModernOptPages); ++i) {
				if (g_ModernOptPages[i].idcButton == LOWORD(wParam))
				{
					CallService(MS_MODERNOPT_SELECTPAGE, i, 0);
					return TRUE;
		}	}	}

	case WM_DESTROY:
		return FALSE;
	}
	return FALSE;
}

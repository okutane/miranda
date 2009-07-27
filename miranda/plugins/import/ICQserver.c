/*

Import plugin for Miranda IM

Copyright (C) 2001-2005 Martin �berg, Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

// ==============
// == INCLUDES ==
// ==============

#include "import.h"

#include "ICQserver.h"
#include "resource.h"

// ====================
// ====================
// == IMPLEMENTATION ==
// ====================
// ====================

BOOL CALLBACK ICQserverPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		SendMessage(GetParent(hdlg),WIZM_DISABLEBUTTON,0,0);
		SendMessage(GetParent(hdlg),WIZM_ENABLEBUTTON,1,0);
		SendMessage(GetParent(hdlg),WIZM_DISABLEBUTTON,2,0);
		TranslateDialogDefault(hdlg);
		ICQserverImport();
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_FINISHED,(LPARAM)FinishedPageProc);
			break;
		case IDCANCEL:
			PostMessage(GetParent(hdlg),WM_CLOSE,0,0);
			break;
		}
		break;
	}
	return FALSE;
}

static void ICQserverImport()
{
	// Clear last update stamp
	DBDeleteContactSetting(NULL, szICQModuleName[ iICQAccount ], "SrvLastUpdate");
	DBDeleteContactSetting(NULL, szICQModuleName[ iICQAccount ], "SrvRecordCount");

	// Enable contacts downloading
	DBWriteContactSettingByte(NULL, szICQModuleName[ iICQAccount ], "UseServerCList", 1);
	DBWriteContactSettingByte(NULL, szICQModuleName[ iICQAccount ], "AddServerNew", 1);
	DBWriteContactSettingByte(NULL, szICQModuleName[ iICQAccount ], "UseServerNicks", 1);
	DBWriteContactSettingByte(NULL, szICQModuleName[ iICQAccount ], "ServerAddRemove", 1);
}

/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
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
#include "m_clc.h"
#include "clc.h"
extern BOOL InvalidateRectZ(HWND hWnd, CONST RECT* lpRect,BOOL bErase );
extern ON_SETALLEXTRAICON_CYCLE;
extern BYTE LOCK_REPAINTING;
extern BOOL CLM_AUTOREBUILD_WAS_POSTED;
#define UNLOCK_REPAINT 13315
//processing of all the CLM_ messages incoming

extern LRESULT ( *saveProcessExternalMessages )(HWND hwnd,struct ClcData *dat,UINT msg,WPARAM wParam,LPARAM lParam);

LRESULT ProcessExternalMessages(HWND hwnd,struct ClcData *dat,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case CLM_AUTOREBUILD:
		KillTimer(hwnd,TIMERID_REBUILDAFTER);
		// Give some time to gather other events too		
		SetTimer(hwnd,TIMERID_REBUILDAFTER,500,NULL);
		CLM_AUTOREBUILD_WAS_POSTED=FALSE;
		return 0;

	case CLM_SETEXTRACOLUMNSSPACE:
		dat->extraColumnSpacing=(int)wParam;
		InvalidateRectZ(hwnd,NULL,FALSE);
		return 0;

	case CLM_SETFONT:
		if(HIWORD(lParam)<0 || HIWORD(lParam)>FONTID_MAX) return 0;
		dat->fontInfo[HIWORD(lParam)].hFont=(HFONT)wParam;
		dat->fontInfo[HIWORD(lParam)].changed=1;

		RowHeights_GetMaxRowHeight(dat, hwnd);

		if(LOWORD(lParam))
			InvalidateRectZ(hwnd,NULL,FALSE);
		return 0;

	case CLM_SETHIDEEMPTYGROUPS:
		{
			BOOL old=((GetWindowLong(hwnd,GWL_STYLE)&CLS_HIDEEMPTYGROUPS)!=0);
			BOOL newval=old;
			if(wParam) SetWindowLong(hwnd,GWL_STYLE,GetWindowLong(hwnd,GWL_STYLE)|CLS_HIDEEMPTYGROUPS);
			else SetWindowLong(hwnd,GWL_STYLE,GetWindowLong(hwnd,GWL_STYLE)&~CLS_HIDEEMPTYGROUPS);
			newval=((GetWindowLong(hwnd,GWL_STYLE)&CLS_HIDEEMPTYGROUPS)!=0);
			if (newval!=old)
				SendMessage(hwnd,CLM_AUTOREBUILD,0,0);
		}
		return 0;

	case CLM_SETTEXTCOLOR:
		if(wParam<0 || wParam>FONTID_MAX) break;
		dat->fontInfo[wParam].colour=lParam;
		dat->force_in_dialog=TRUE;
		if (wParam==FONTID_CONTACTS) {
			dat->fontInfo[FONTID_SECONDLINE].colour=lParam;
			dat->fontInfo[FONTID_THIRDLINE].colour=lParam;
		}

		return 0;
	}

	return saveProcessExternalMessages(hwnd, dat, msg, wParam, lParam);
}

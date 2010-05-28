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

------------------------

implements a simple status floater as a layered (and skinnable) window with
a minimalistic UI (change status, access main menu). It also may hold a copy
of the event area.

Also implementes floating contacts (FLT_*() functions)

*/

#include "commonheaders.h"

#define SNAP_SCREEN_TOLERANCE	10
#define SNAP_FLT_TOLERANCE		10
#define	TOOLTIP_TIMER			1

#define MS_TOOLTIP_SHOWTIP		"mToolTip/ShowTip"
#define MS_TOOLTIP_HIDETIP		"mToolTip/HideTip"


BYTE __forceinline percent_to_byte(UINT32 percent)
{
    return(BYTE) ((FLOAT) (((FLOAT) percent) / 100) * 255);
}

void FLT_Update(struct ClcData *dat, struct ClcContact *contact);
void FLT_ShowHideAll(int showCmd);
void FLT_SnapToEdges(HWND hwnd);
void FLT_SnapToFloater(HWND hwnd);

HWND g_hwndSFL = 0;
HDC g_SFLCachedDC = 0;
HBITMAP g_SFLhbmOld = 0, g_SFLhbm = 0;
struct ContactFloater *pFirstFloater = 0;
BOOL hover = FALSE;
BOOL tooltip = FALSE;
int hTooltipTimer = 0;
POINT start_pos;


extern StatusItems_t *StatusItems;
extern BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
extern BOOL (WINAPI *MyUpdateLayeredWindow)(HWND hwnd, HDC hdcDst, POINT *pptDst,SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
extern int g_nextExtraCacheEntry, g_padding_y;
extern struct ExtraCache *g_ExtraCache;

extern struct CluiData g_CluiData;
extern HIMAGELIST hCListImages;
extern HWND g_hwndEventArea;
extern struct ClcData *g_clcData;
extern HDC g_HDC;

extern int g_list_avatars;

FLOATINGOPTIONS g_floatoptions;

static UINT padctrlIDs[] = { IDC_FLT_PADLEFTSPIN, IDC_FLT_PADRIGHTSPIN, IDC_FLT_PADTOPSPIN,
							 IDC_FLT_PADBOTTOMSPIN, 0 };

/*
 * floating contacts support functions
 * simple linked list of allocated ContactFloater* structs
 */

static struct ContactFloater *FLT_AddToList(struct ContactFloater *pFloater) {
    struct ContactFloater *pCurrent = pFirstFloater;

    if (!pFirstFloater) {
        pFirstFloater = pFloater;
        pFirstFloater->pNextFloater = NULL;
        return pFirstFloater;
    } else {
        while (pCurrent->pNextFloater != 0)
            pCurrent = pCurrent->pNextFloater;
        pCurrent->pNextFloater = pFloater;
        pFloater->pNextFloater = NULL;
        return pCurrent;
    }
}

static struct ContactFloater *FLT_RemoveFromList(struct ContactFloater *pFloater) {
    struct ContactFloater *pCurrent = pFirstFloater;

    if (pFloater == pFirstFloater) {
        if(pFloater->pNextFloater != NULL)
            pFirstFloater = pFloater->pNextFloater;
        else
            pFirstFloater = NULL;
        return pFirstFloater;
    }

    do {
        if (pCurrent->pNextFloater == pFloater) {
            pCurrent->pNextFloater = pCurrent->pNextFloater->pNextFloater;
            return 0;
        }
    } while (pCurrent = pCurrent->pNextFloater);

    return NULL;
}

void FLT_SnapToEdges(HWND hwnd)
{
	RECT dr;
	MONITORINFO monInfo;
	RECT rcWindow;
	HMONITOR curMonitor;
	
	if ( MyMonitorFromWindow == NULL )
		return;

	curMonitor = MyMonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

	monInfo.cbSize = sizeof(monInfo);
	MyGetMonitorInfo(curMonitor, &monInfo);

	dr = monInfo.rcWork;
	GetWindowRect(hwnd, &rcWindow);
	
	if (rcWindow.left < dr.left + SNAP_SCREEN_TOLERANCE){
		SetWindowPos(hwnd, HWND_TOPMOST, 0, rcWindow.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		GetWindowRect(hwnd, &rcWindow);
	}

	if (rcWindow.top < dr.top + SNAP_SCREEN_TOLERANCE){
		SetWindowPos(hwnd, HWND_TOPMOST, rcWindow.left, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		GetWindowRect(hwnd, &rcWindow);
	}

	if (rcWindow.right > dr.right - SNAP_SCREEN_TOLERANCE)
		SetWindowPos(hwnd, HWND_TOPMOST, dr.right - (rcWindow.right - rcWindow.left), rcWindow.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	if (rcWindow.bottom > dr.bottom - SNAP_SCREEN_TOLERANCE)
		SetWindowPos(hwnd, HWND_TOPMOST, rcWindow.left, dr.bottom - (rcWindow.bottom - rcWindow.top), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void FLT_SnapToFloater(HWND hwnd)
{
	struct ContactFloater *pCurrent = pFirstFloater;
	RECT rcWindow, rcBase;
	int minTop = 0xFFFFFF, minBottom = 0xFFFFFF, minRight = 0xFFFFFF, minLeft = 0xFFFFFF;
	int posTop = 0, posBottom = 0, posRight = 0, posLeft = 0;

	GetWindowRect(hwnd, &rcBase);

	//find the closest floater
	while(pCurrent) {
		GetWindowRect(pCurrent->hwnd, &rcWindow);
		//top
		if((rcWindow.top - rcBase.bottom > -SNAP_FLT_TOLERANCE) && (rcWindow.top - rcBase.bottom < minTop)){
			posTop = rcWindow.top;
			minTop = rcWindow.top - rcBase.bottom;
		}
		//bottom
		if((rcBase.top - rcWindow.bottom > -SNAP_FLT_TOLERANCE) && (rcBase.top - rcWindow.bottom  < minBottom)){
			posBottom = rcWindow.bottom;
			minBottom = rcBase.top - rcWindow.bottom;
		}
		//left
		if((rcWindow.left - rcBase.right > -SNAP_FLT_TOLERANCE) && (rcWindow.left - rcBase.right < minLeft)){
			posLeft= rcWindow.left;
			minLeft = rcWindow.left - rcBase.right;
		}
		//right
		if((rcBase.left - rcWindow.right > -SNAP_FLT_TOLERANCE) && (rcBase.left - rcWindow.right  < minRight)){
			posRight= rcWindow.right;
			minRight = rcBase.left - rcWindow.right;
		}
		pCurrent = pCurrent->pNextFloater;
	}

	//snap to the closest floater if spacing is under SNAP_FLT_TOLERANCE
	if (posTop && (rcBase.bottom > posTop - SNAP_FLT_TOLERANCE))
		SetWindowPos(hwnd, HWND_TOPMOST, rcBase.left, posTop - (rcBase.bottom - rcBase.top), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	if (posBottom && (rcBase.top < posBottom + SNAP_FLT_TOLERANCE))
		SetWindowPos(hwnd, HWND_TOPMOST, rcBase.left, posBottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	
	if (posLeft && (rcBase.right > posLeft - SNAP_FLT_TOLERANCE))
		SetWindowPos(hwnd, HWND_TOPMOST, posLeft - (rcBase.right - rcBase.left), rcBase.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	
	if (posRight && (rcBase.left < posRight + SNAP_FLT_TOLERANCE))
		SetWindowPos(hwnd, HWND_TOPMOST, posRight, rcBase.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

/*
 * dialog procedure for the floating contacts option page
 */

LRESULT CALLBACK DlgProcFloatingContacts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			{
				DWORD dwFlags = g_floatoptions.dwFlags;
				int i = 0;

				TranslateDialogDefault(hwndDlg);

				CheckDlgButton(hwndDlg, IDC_FLT_ENABLED, g_floatoptions.enabled);
				SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_FLT_ENABLED, 0);
				CheckDlgButton(hwndDlg, IDC_FLT_SIMPLELAYOUT, dwFlags & FLT_SIMPLE);
				SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_FLT_SIMPLELAYOUT, 0);
				CheckDlgButton(hwndDlg, IDC_FLT_AVATARS, dwFlags & FLT_AVATARS);
				CheckDlgButton(hwndDlg, IDC_FLT_DUALROWS, dwFlags & FLT_DUALROW);
				CheckDlgButton(hwndDlg, IDC_FLT_EXTRAICONS, dwFlags & FLT_EXTRAICONS);
				CheckDlgButton(hwndDlg, IDC_FLT_SYNCED, dwFlags & FLT_SYNCWITHCLIST);
				CheckDlgButton(hwndDlg, IDC_FLT_AUTOHIDE, dwFlags & FLT_AUTOHIDE);
				CheckDlgButton(hwndDlg, IDC_FLT_SNAP, dwFlags & FLT_SNAP);
				CheckDlgButton(hwndDlg, IDC_FLT_BORDER, dwFlags & FLT_BORDER);
				SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_FLT_BORDER, 0);
				CheckDlgButton(hwndDlg, IDC_FLT_ROUNDED, dwFlags & FLT_ROUNDED);
                CheckDlgButton(hwndDlg, IDC_FLT_FILLSTD, dwFlags & FLT_FILLSTDCOLOR);

				SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_FLT_ROUNDED, 0);

                if (ServiceExists(MS_TOOLTIP_SHOWTIP))
                {
                    CheckDlgButton(hwndDlg, IDC_FLT_SHOWTOOLTIPS, dwFlags & FLT_SHOWTOOLTIPS);
                    SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_FLT_SHOWTOOLTIPS, 0);
                    CheckDlgButton(hwndDlg, IDC_FLT_DEFHOVERTIME, g_floatoptions.def_hover_time);
                    SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_FLT_DEFHOVERTIME, 0);
                } 
                else
                {
                    CheckDlgButton(hwndDlg, IDC_FLT_SHOWTOOLTIPS, 0);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_SHOWTOOLTIPS), 0);
                }

				for(i = 0; padctrlIDs[i] != 0; i++)
					SendDlgItemMessage(hwndDlg, padctrlIDs[i], UDM_SETRANGE, 0, MAKELONG(20, 0));
				SendDlgItemMessage(hwndDlg, IDC_FLT_WIDTHSPIN, UDM_SETRANGE, 0, MAKELONG(200, 50));
				SendDlgItemMessage(hwndDlg, IDC_FLT_RADIUSSPIN, UDM_SETRANGE, 0, MAKELONG(20, 1));
                SendDlgItemMessage(hwndDlg, IDC_FLT_HOVERTIMESPIN, UDM_SETRANGE, 0, MAKELONG(5000, 1));


				SendDlgItemMessage(hwndDlg, IDC_FLT_PADLEFTSPIN, UDM_SETPOS, 0, (LPARAM)g_floatoptions.pad_left);
				SendDlgItemMessage(hwndDlg, IDC_FLT_PADRIGHTSPIN, UDM_SETPOS, 0, (LPARAM)g_floatoptions.pad_right);
				SendDlgItemMessage(hwndDlg, IDC_FLT_PADTOPSPIN, UDM_SETPOS, 0, (LPARAM)g_floatoptions.pad_top);
				SendDlgItemMessage(hwndDlg, IDC_FLT_PADBOTTOMSPIN, UDM_SETPOS, 0, (LPARAM)g_floatoptions.pad_top);
				SendDlgItemMessage(hwndDlg, IDC_FLT_WIDTHSPIN, UDM_SETPOS, 0, (LPARAM)g_floatoptions.width);
				SendDlgItemMessage(hwndDlg, IDC_FLT_RADIUSSPIN, UDM_SETPOS, 0, (LPARAM)g_floatoptions.radius);
                SendDlgItemMessage(hwndDlg, IDC_FLT_HOVERTIMESPIN, UDM_SETPOS, 0, (LPARAM)g_floatoptions.hover_time);

				SendDlgItemMessage(hwndDlg, IDC_FLT_ACTIVEOPACITY, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
				SendDlgItemMessage(hwndDlg, IDC_FLT_ACTIVEOPACITY, TBM_SETPOS, TRUE, g_floatoptions.act_trans);
				SendDlgItemMessage(hwndDlg, IDC_FLT_OPACITY, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
				SendDlgItemMessage(hwndDlg, IDC_FLT_OPACITY, TBM_SETPOS, TRUE, g_floatoptions.trans);
				SendMessage(hwndDlg, WM_HSCROLL, 0, 0);

				SendDlgItemMessage(hwndDlg, IDC_FLT_BORDERCOLOUR, CPM_SETDEFAULTCOLOUR, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_FLT_BORDERCOLOUR, CPM_SETCOLOUR, 0, g_floatoptions.border_colour);  

				FLT_ShowHideAll(SW_SHOWNOACTIVATE);

				return TRUE;
			}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_FLT_ENABLED:
					{
						int isEnabled = IsDlgButtonChecked(hwndDlg, IDC_FLT_ENABLED);
						int isSimple = IsDlgButtonChecked(hwndDlg, IDC_FLT_SIMPLELAYOUT);
						int isBorder = IsDlgButtonChecked(hwndDlg, IDC_FLT_BORDER);
						int isRounded = IsDlgButtonChecked(hwndDlg, IDC_FLT_ROUNDED);
                        int isTooltip = IsDlgButtonChecked(hwndDlg, IDC_FLT_SHOWTOOLTIPS);
                        int isDefHoverTime = IsDlgButtonChecked(hwndDlg, IDC_FLT_DEFHOVERTIME);

						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_SIMPLELAYOUT), isEnabled);		
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_SYNCED), isEnabled);	
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_AUTOHIDE), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_SNAP), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_ACTIVEOPACITY), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_OPACITY), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADLEFTSPIN), isEnabled);		
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADRIGHTSPIN), isEnabled);	
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADTOPSPIN), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADLEFT), isEnabled);		
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADRIGHT), isEnabled);	
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADTOP), isEnabled);
						//EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADBOTTOMSPIN), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_PADBOTTOM), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_WIDTHSPIN), isEnabled);	
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_WIDTH), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_BORDER), isEnabled);	
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_ROUNDED), isEnabled);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_BORDERCOLOUR), isEnabled & isBorder);	
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_RADIUS), isEnabled & isRounded);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_RADIUSSPIN), isEnabled & isRounded);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_SHOWTOOLTIPS), isEnabled & ServiceExists(MS_TOOLTIP_SHOWTIP));	
                        EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_DEFHOVERTIME), isEnabled & isTooltip);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_HOVERTIME), isEnabled & isTooltip & !isDefHoverTime);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_HOVERTIMESPIN), isEnabled & isTooltip & !isDefHoverTime);

						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_AVATARS), isEnabled & !isSimple);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_EXTRAICONS), isEnabled & !isSimple);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_DUALROWS), isEnabled & !isSimple);
					}
					break;
				case IDC_FLT_SIMPLELAYOUT:
					{
						if (IsDlgButtonChecked(hwndDlg, IDC_FLT_ENABLED)){
							int isSimple = IsDlgButtonChecked(hwndDlg, IDC_FLT_SIMPLELAYOUT);
							EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_AVATARS), !isSimple);
							EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_EXTRAICONS), !isSimple);
							EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_DUALROWS), !isSimple);
						}
					}
					break;
				case IDC_FLT_BORDER:
					{
						if (IsDlgButtonChecked(hwndDlg, IDC_FLT_ENABLED)){
							int isBorder = IsDlgButtonChecked(hwndDlg, IDC_FLT_BORDER);
							EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_BORDERCOLOUR), isBorder);
						}
					}
					break;
				case IDC_FLT_ROUNDED:
					{
						if (IsDlgButtonChecked(hwndDlg, IDC_FLT_ENABLED)){
							int isRounded = IsDlgButtonChecked(hwndDlg, IDC_FLT_ROUNDED);
							EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_RADIUS), isRounded);
							EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_RADIUSSPIN), isRounded);
						}
					}
					break;
                case IDC_FLT_SHOWTOOLTIPS:
                    {
                        if (IsDlgButtonChecked(hwndDlg, IDC_FLT_ENABLED)){
                            int isTooltip = IsDlgButtonChecked(hwndDlg, IDC_FLT_SHOWTOOLTIPS);
                            int isDefHoverTime = IsDlgButtonChecked(hwndDlg, IDC_FLT_DEFHOVERTIME);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_DEFHOVERTIME), isTooltip);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_HOVERTIME), isTooltip & !isDefHoverTime);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_HOVERTIMESPIN), isTooltip & !isDefHoverTime);
                        }
                    }
                    break;
                case IDC_FLT_DEFHOVERTIME:
                    {
                        if (IsDlgButtonChecked(hwndDlg, IDC_FLT_ENABLED) && IsDlgButtonChecked(hwndDlg, IDC_FLT_SHOWTOOLTIPS)){
                            int isDefHoverTime = IsDlgButtonChecked(hwndDlg, IDC_FLT_DEFHOVERTIME);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_HOVERTIME), !isDefHoverTime);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_FLT_HOVERTIMESPIN), !isDefHoverTime);
                        }
                    }
                    break;

				case IDC_FLT_PADTOP:
					{
						if(HIWORD(wParam) == EN_CHANGE){
							int value = SendDlgItemMessage(hwndDlg, IDC_FLT_PADTOPSPIN, UDM_GETPOS, 0, 0);
							SendDlgItemMessage(hwndDlg, IDC_FLT_PADBOTTOMSPIN, UDM_SETPOS, 0, (LPARAM)value);
						}
					}
					break;
			break;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_HSCROLL:
			{
				char str[10];
				wsprintfA(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_FLT_ACTIVEOPACITY, TBM_GETPOS, 0, 0) / 255);
				SetDlgItemTextA(hwndDlg, IDC_FLT_ACTIVEOPACITYVALUE, str);
				wsprintfA(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_FLT_OPACITY, TBM_GETPOS, 0, 0) / 255);
				SetDlgItemTextA(hwndDlg, IDC_FLT_OPACITYVALUE, str);

				if (lParam)
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->idFrom) {
                case 0:
                    switch (((LPNMHDR) lParam)->code) {
                        case PSN_APPLY:
                            {
								g_floatoptions.enabled = IsDlgButtonChecked(hwndDlg, IDC_FLT_ENABLED) ? 1 : 0;
								g_floatoptions.dwFlags = 0;

								if(IsDlgButtonChecked(hwndDlg, IDC_FLT_SIMPLELAYOUT))
									g_floatoptions.dwFlags = FLT_SIMPLE;

								g_floatoptions.dwFlags |= (IsDlgButtonChecked(hwndDlg, IDC_FLT_AVATARS) ? FLT_AVATARS : 0) |
														  (IsDlgButtonChecked(hwndDlg, IDC_FLT_DUALROWS) ? FLT_DUALROW : 0) |
														  (IsDlgButtonChecked(hwndDlg, IDC_FLT_EXTRAICONS) ? FLT_EXTRAICONS : 0) |
														  (IsDlgButtonChecked(hwndDlg, IDC_FLT_SYNCED) ? FLT_SYNCWITHCLIST : 0) |
														  (IsDlgButtonChecked(hwndDlg, IDC_FLT_AUTOHIDE) ? FLT_AUTOHIDE : 0) |
														  (IsDlgButtonChecked(hwndDlg, IDC_FLT_SNAP) ? FLT_SNAP : 0) |
														  (IsDlgButtonChecked(hwndDlg, IDC_FLT_BORDER) ? FLT_BORDER : 0) |
                                                          (IsDlgButtonChecked(hwndDlg, IDC_FLT_FILLSTD) ? FLT_FILLSTDCOLOR : 0) |
                                                          (IsDlgButtonChecked(hwndDlg, IDC_FLT_SHOWTOOLTIPS) ? FLT_SHOWTOOLTIPS : 0) |
														  (IsDlgButtonChecked(hwndDlg, IDC_FLT_ROUNDED) ? FLT_ROUNDED : 0);
								
								g_floatoptions.act_trans = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_ACTIVEOPACITY, TBM_GETPOS, 0, 0);
								g_floatoptions.trans = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_OPACITY, TBM_GETPOS, 0, 0);
								g_floatoptions.pad_left = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_PADLEFTSPIN, UDM_GETPOS, 0, 0);
								g_floatoptions.pad_right = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_PADRIGHTSPIN, UDM_GETPOS, 0, 0);
								g_floatoptions.pad_top = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_PADTOPSPIN, UDM_GETPOS, 0, 0);
								g_floatoptions.pad_bottom = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_PADBOTTOMSPIN, UDM_GETPOS, 0, 0);
								g_floatoptions.width = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_WIDTHSPIN, UDM_GETPOS, 0, 0);
								g_floatoptions.radius = (BYTE)SendDlgItemMessage(hwndDlg, IDC_FLT_RADIUSSPIN, UDM_GETPOS, 0, 0);
								g_floatoptions.border_colour = SendDlgItemMessage(hwndDlg, IDC_FLT_BORDERCOLOUR, CPM_GETCOLOUR, 0, 0);

                                g_floatoptions.def_hover_time= IsDlgButtonChecked(hwndDlg, IDC_FLT_DEFHOVERTIME) ? 1 : 0;
                                if (g_floatoptions.def_hover_time)
                                    g_floatoptions.hover_time = DBGetContactSettingWord(NULL, "CLC", "InfoTipHoverTime", 200); 
                                else
                                    g_floatoptions.hover_time = (WORD)SendDlgItemMessage(hwndDlg, IDC_FLT_HOVERTIMESPIN, UDM_GETPOS, 0, 0);

								FLT_WriteOptions();
								FLT_RefreshAll();
								return TRUE;
							}
                    }
                    break;
            }
            break;
	}
	return FALSE;
}

void FLT_ReadOptions()
{
	DWORD dwPad;

	ZeroMemory(&g_floatoptions, sizeof(FLOATINGOPTIONS));

	g_floatoptions.enabled = DBGetContactSettingByte(NULL, "CList", "flt_enabled", 0);
	g_floatoptions.dwFlags = DBGetContactSettingDword(NULL, "CList", "flt_flags", FLT_SIMPLE);
	dwPad = DBGetContactSettingDword(NULL, "CList", "flt_padding", 0);
	
	g_floatoptions.pad_top = LOBYTE(LOWORD(dwPad));
	g_floatoptions.pad_right = HIBYTE(LOWORD(dwPad));
	g_floatoptions.pad_bottom = LOBYTE(HIWORD(dwPad));
	g_floatoptions.pad_left = HIBYTE(HIWORD(dwPad));

	g_floatoptions.width = DBGetContactSettingDword(NULL, "CList", "flt_width", 100);
	g_floatoptions.act_trans = DBGetContactSettingByte(NULL, "CList", "flt_acttrans", 255);
	g_floatoptions.trans = DBGetContactSettingByte(NULL, "CList", "flt_trans", 255);
	g_floatoptions.radius = DBGetContactSettingByte(NULL, "CList", "flt_radius", 3);
	g_floatoptions.border_colour = DBGetContactSettingDword(NULL, "CList", "flt_bordercolour", 0);
    g_floatoptions.def_hover_time = DBGetContactSettingByte(NULL, "CList", "flt_defhovertime", 1);

    if (g_floatoptions.def_hover_time)
        g_floatoptions.hover_time = DBGetContactSettingWord(NULL, "CLC", "InfoTipHoverTime", 200); 
    else
        g_floatoptions.hover_time = DBGetContactSettingWord(NULL, "CList", "flt_hovertime", 200); 

}

void FLT_WriteOptions()
{
	DWORD dwPad;

	DBWriteContactSettingByte(NULL, "CList", "flt_enabled", g_floatoptions.enabled);
	DBWriteContactSettingDword(NULL, "CList", "flt_flags", g_floatoptions.dwFlags);
	dwPad = MAKELONG(MAKEWORD(g_floatoptions.pad_top, g_floatoptions.pad_right), 
					 MAKEWORD(g_floatoptions.pad_bottom, g_floatoptions.pad_left));
	DBWriteContactSettingDword(NULL, "CList", "flt_padding", dwPad);
	DBWriteContactSettingDword(NULL, "CList", "flt_width", g_floatoptions.width);
	DBWriteContactSettingByte(NULL, "CList", "flt_acttrans", g_floatoptions.act_trans);
	DBWriteContactSettingByte(NULL, "CList", "flt_trans", g_floatoptions.trans);
	DBWriteContactSettingByte(NULL, "CList", "flt_radius", g_floatoptions.radius);
	DBWriteContactSettingDword(NULL, "CList", "flt_bordercolour", g_floatoptions.border_colour);
    DBWriteContactSettingByte(NULL, "CList", "flt_defhovertime", g_floatoptions.def_hover_time);
    if (!g_floatoptions.def_hover_time)
        DBWriteContactSettingWord(NULL, "CList", "flt_hovertime", g_floatoptions.hover_time);

}

LRESULT CALLBACK StatusFloaterClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_DESTROY:
			Utils_SaveWindowPosition(hwnd, 0, "CLUI", "sfl");
			if(g_SFLCachedDC) {
				SelectObject(g_SFLCachedDC, g_SFLhbmOld);
				DeleteObject(g_SFLhbm);
				DeleteDC(g_SFLCachedDC);
				g_SFLCachedDC = 0;
			}
			break;
		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
			{
				HDC hdc;
				PAINTSTRUCT ps;

				hdc = BeginPaint(hwnd, &ps);
				ps.fErase = FALSE;
				EndPaint(hwnd, &ps);
				return TRUE;
			}
        case WM_LBUTTONDOWN:
            {
                POINT ptMouse;
				RECT rcWindow;

				GetCursorPos(&ptMouse);
				GetWindowRect(hwnd, &rcWindow);
				rcWindow.right = rcWindow.left + 25;
				if(!PtInRect(&rcWindow, ptMouse))
					return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(ptMouse.x, ptMouse.y));
				break;
			}
		case WM_LBUTTONUP:
			{
                HMENU hmenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
				RECT rcWindow;
				POINT pt;

				GetCursorPos(&pt);
				GetWindowRect(hwnd, &rcWindow);
				if(g_CluiData.bUseFloater & CLUI_FLOATER_EVENTS) {
					if(pt.y > rcWindow.top + ((rcWindow.bottom - rcWindow.top) / 2))
						SendMessage(g_hwndEventArea, WM_COMMAND, MAKEWPARAM(IDC_NOTIFYBUTTON, 0), 0);
					else
						TrackPopupMenu(hmenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON, rcWindow.left, rcWindow.bottom, 0, pcli->hwndContactList, NULL);
				}
				else
					TrackPopupMenu(hmenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON, rcWindow.left, rcWindow.bottom, 0, pcli->hwndContactList, NULL);
				return 0;
			}
		case WM_CONTEXTMENU:
			{
                HMENU hmenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
				RECT rcWindow;

				GetWindowRect(hwnd, &rcWindow);
                TrackPopupMenu(hmenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON, rcWindow.left, rcWindow.bottom, 0, pcli->hwndContactList, NULL);
				return 0;
			}

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CALLBACK ShowTooltip(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime){
	struct ContactFloater *pCurrent = pFirstFloater;
	POINT pt;
	CLCINFOTIP ti = {0};

	KillTimer(hwnd, TOOLTIP_TIMER);
	hTooltipTimer = 0;

	GetCursorPos(&pt);
	if ((abs(pt.x - start_pos.x) > 3) && (abs(pt.y - start_pos.y) > 3)) return;
					
	while(pCurrent->hwnd != hwnd)
		pCurrent = pCurrent->pNextFloater;
	
	ti.cbSize = sizeof(ti);
	ti.isGroup = 0;
	ti.isTreeFocused = 0;
	ti.hItem = pCurrent->hContact;
	ti.ptCursor = pt;
	CallService(MS_TOOLTIP_SHOWTIP, 0, (LPARAM)&ti);
	tooltip = TRUE;
}

LRESULT CALLBACK ContactFloaterClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR iEntry = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	struct ExtraCache *centry = NULL;

	if(iEntry >= 0 && iEntry < g_nextExtraCacheEntry)
		centry = &g_ExtraCache[iEntry];

	switch(msg) {
		case WM_NCCREATE:
			{
				CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
				iEntry = (int)cs->lpCreateParams;
				if(iEntry >= 0 && iEntry < g_nextExtraCacheEntry)
					centry = &g_ExtraCache[iEntry];
				return TRUE;
			}
		case WM_DESTROY:
			if(centry) {
                WINDOWPLACEMENT wp = {0};

				SelectObject(centry->floater->hdc, centry->floater->hbmOld);
				DeleteObject(centry->floater->hbm);
				DeleteDC(centry->floater->hdc);
				FLT_RemoveFromList(centry->floater);
				free(centry->floater);
				centry->floater = 0;
				Utils_SaveWindowPosition(hwnd, centry->hContact, "CList", "flt");
				break;
			}
		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
			{
				HDC hdc;
				PAINTSTRUCT ps;

				hdc = BeginPaint(hwnd, &ps);
				ps.fErase = FALSE;
				EndPaint(hwnd, &ps);
				return TRUE;
			}
		case WM_LBUTTONDBLCLK:
			if(centry)
				CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)centry->hContact, 0);
			return 0;
		case WM_LBUTTONDOWN:
			{
				POINT ptMouse;
				RECT rcWindow;

				GetCursorPos(&ptMouse);
				GetWindowRect(hwnd, &rcWindow);
				rcWindow.right = rcWindow.left + 25;
				if(!PtInRect(&rcWindow, ptMouse))
					return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(ptMouse.x, ptMouse.y));
				break;
			}
		case WM_MOUSEMOVE:
			if( MyTrackMouseEvent && !hover ) {
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_HOVER | TME_LEAVE;
				tme.hwndTrack = hwnd;
				tme.dwHoverTime = 5;
				MyTrackMouseEvent(&tme);
				hover = TRUE;
			}
			if ( ServiceExists( MS_TOOLTIP_SHOWTIP )) {
				if ((g_floatoptions.dwFlags & FLT_SHOWTOOLTIPS) && !tooltip) {
					GetCursorPos(&start_pos);
					if (hTooltipTimer) KillTimer(hwnd, TOOLTIP_TIMER);
					hTooltipTimer = SetTimer(hwnd, TOOLTIP_TIMER, g_floatoptions.hover_time, ShowTooltip);					
				}
			}

			return FALSE;

		case WM_MOUSEHOVER:
			{
				struct ClcContact *contact = NULL;
				struct ContactFloater *pCurrent = pFirstFloater;
				int oldTrans = g_floatoptions.trans;

				while(pCurrent->hwnd != hwnd)
					pCurrent = pCurrent->pNextFloater;

				if(FindItem(pcli->hwndContactTree, g_clcData, pCurrent->hContact, &contact, NULL, 0)){
					g_floatoptions.trans = g_floatoptions.act_trans;
					FLT_Update(g_clcData, contact);
					g_floatoptions.trans = oldTrans;
				}

				break;
			}
		case WM_MOUSELEAVE:
			{
				struct ClcContact *contact = NULL;
				struct ContactFloater *pCurrent = pFirstFloater;

				while(pCurrent->hwnd != hwnd)
					pCurrent = pCurrent->pNextFloater;

				if(FindItem(pcli->hwndContactTree, g_clcData, pCurrent->hContact, &contact, NULL, 0))
					FLT_Update(g_clcData, contact);

                if (hTooltipTimer)
                { 
                    KillTimer(hwnd, TOOLTIP_TIMER);
                    hTooltipTimer = 0;		
                }

                if (tooltip) CallService(MS_TOOLTIP_HIDETIP, 0, 0);

                hover = FALSE;
                tooltip = FALSE;


				break;
			}

		case WM_MOVE:
			{
				if (g_floatoptions.dwFlags & FLT_SNAP)
					FLT_SnapToEdges(hwnd);

				if(GetKeyState(VK_CONTROL) < 0)
					FLT_SnapToFloater(hwnd);

				break;
			}
		case WM_MEASUREITEM:
			return(CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam));
		case WM_DRAWITEM:
			return(CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam));
		case WM_COMMAND:
			return(CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKELONG(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)centry->hContact));
		case WM_CONTEXTMENU:
			{
				if(centry) {
					HMENU hContactMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)centry->hContact, 0);
					RECT rcWindow;

					GetWindowRect(hwnd, &rcWindow);
			        TrackPopupMenu(hContactMenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON, rcWindow.left, rcWindow.bottom, 0, hwnd, NULL);
					DestroyMenu(hContactMenu);
					return 0;
				}
				break;
			}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void SFL_RegisterWindowClass()
{
	WNDCLASS wndclass;

    wndclass.style = 0;
    wndclass.lpfnWndProc = StatusFloaterClassProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = g_hInst;
    wndclass.hIcon = 0;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) (COLOR_3DFACE);
    wndclass.lpszMenuName = 0;
    wndclass.lpszClassName = _T("StatusFloaterClass");
    RegisterClass(&wndclass);

    wndclass.style = CS_DBLCLKS;
	wndclass.lpszClassName = _T("ContactFloaterClass");
    wndclass.lpfnWndProc = ContactFloaterClassProc;
    RegisterClass(&wndclass);
}

void SFL_UnregisterWindowClass()
{
	UnregisterClass(_T("StatusFloaterClass"), g_hInst);
	UnregisterClass(_T("ContactFloaterClass"), g_hInst);
}

void SFL_Destroy()
{
	if(g_hwndSFL)
		DestroyWindow(g_hwndSFL);
	g_hwndSFL = 0;
}

static HICON sfl_hIcon = (HICON)-1;
static int sfl_iIcon = -1;
static TCHAR sfl_statustext[100] = _T("");

void SFL_Update(HICON hIcon, int iIcon, HIMAGELIST hIml, const TCHAR *szText, BOOL refresh)
{
	RECT rcClient, rcWindow;
	POINT ptDest, ptSrc = {0};
	SIZE szDest, szT;
	BLENDFUNCTION bf = {0};
	HFONT hOldFont;
	StatusItems_t *item = &StatusItems[ID_EXTBKSTATUSFLOATER - ID_STATUS_OFFLINE];
	RECT rcStatusArea;
	LONG cy;

	if(g_hwndSFL == 0)
		return;

	GetClientRect(g_hwndSFL, &rcClient);
	GetWindowRect(g_hwndSFL, &rcWindow);

	ptDest.x = rcWindow.left;
	ptDest.y = rcWindow.top;
	szDest.cx = rcWindow.right - rcWindow.left;
	szDest.cy = rcWindow.bottom - rcWindow.top;

	if(item->IGNORED) {
		FillRect(g_SFLCachedDC, &rcClient, GetSysColorBrush(COLOR_3DFACE));
		SetTextColor(g_SFLCachedDC, GetSysColor(COLOR_BTNTEXT));
	}
	else {
		FillRect(g_SFLCachedDC, &rcClient, GetSysColorBrush(COLOR_3DFACE));
		DrawAlpha(g_SFLCachedDC, &rcClient, item->COLOR, 100, item->COLOR2, item->COLOR2_TRANSPARENT,
			item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
		SetTextColor(g_SFLCachedDC, item->TEXTCOLOR);
	}
	bf.BlendOp = AC_SRC_OVER;
	bf.AlphaFormat = 0;
	bf.SourceConstantAlpha = item->IGNORED ? 255 : percent_to_byte(item->ALPHA);

	rcStatusArea = rcClient;

	if(g_CluiData.bUseFloater & CLUI_FLOATER_EVENTS)
		rcStatusArea.bottom = 20;

	cy = rcStatusArea.bottom - rcStatusArea.top;

	if(szText != NULL && refresh) {
		_tcsncpy(sfl_statustext, szText, 100);
		sfl_statustext[99] = 0;
	}

	if(!hIcon) {
		HICON p_hIcon;

		if(refresh)
			sfl_iIcon = iIcon;
		if(sfl_iIcon != -1) {
			p_hIcon = ImageList_ExtractIcon(0, hCListImages, sfl_iIcon);
			DrawIconEx(g_SFLCachedDC, 5, (cy - 16) / 2, p_hIcon, 16, 16, 0, 0, DI_NORMAL);
			DestroyIcon(p_hIcon);
		}
	}
	else {
		if(refresh)
			sfl_hIcon = hIcon;
		if(sfl_hIcon != (HICON)-1)
			DrawIconEx(g_SFLCachedDC, 5, (cy - 16) / 2, sfl_hIcon, 16, 16, 0, 0, DI_NORMAL);
	}

	hOldFont = SelectObject(g_SFLCachedDC, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(g_SFLCachedDC, TRANSPARENT);
	GetTextExtentPoint32(g_SFLCachedDC, sfl_statustext, lstrlen(sfl_statustext), &szT);
	TextOut(g_SFLCachedDC, 24, (cy - szT.cy) / 2, sfl_statustext, lstrlen(sfl_statustext));

	if(g_CluiData.bUseFloater & CLUI_FLOATER_EVENTS) {
		RECT rcNA = rcClient;

		rcNA.top = 18;
		PaintNotifyArea(g_SFLCachedDC, &rcNA);
	}

	SelectObject(g_SFLCachedDC, hOldFont);

	if(MyUpdateLayeredWindow)
		MyUpdateLayeredWindow(g_hwndSFL, 0, &ptDest, &szDest, g_SFLCachedDC, &ptSrc, GetSysColor(COLOR_3DFACE), &bf, ULW_ALPHA | ULW_COLORKEY);
}

/*
 * set the floater
 * mode = 0/1 forced hide/show
 * OR -1 to set it depending on the clist state (visible/hidden) (this is actually reversed, because the function
 * is called *before* the clist is shown or hidden)
 */

void SFL_SetState(int uMode)
{
	BYTE bClistState;

	if(g_hwndSFL == 0 || !(g_CluiData.bUseFloater & CLUI_USE_FLOATER))
		return;

	if(uMode == -1) {
		if(g_CluiData.bUseFloater & CLUI_FLOATER_AUTOHIDE) {
			bClistState = DBGetContactSettingByte(NULL, "CList", "State", SETTING_STATE_NORMAL);
			ShowWindow(g_hwndSFL, bClistState == SETTING_STATE_NORMAL ? SW_SHOW : SW_HIDE);
		}
		else
			ShowWindow(g_hwndSFL, SW_SHOW);
	}
	else
		ShowWindow(g_hwndSFL, uMode ? SW_SHOW : SW_HIDE);
}

// XXX improve size calculations for the floater window.

void SFL_SetSize()
{
	HDC hdc;
	LONG lWidth;
	RECT rcWindow;
	SIZE sz;
	char *szStatusMode;
	HFONT oldFont;
	int i;

	GetWindowRect(g_hwndSFL, &rcWindow);
	lWidth = rcWindow.right - rcWindow.left;

	hdc = GetDC(g_hwndSFL);
	oldFont = SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		szStatusMode = Translate((char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)i, 0));
		GetTextExtentPoint32A(hdc, szStatusMode, lstrlenA(szStatusMode), &sz);
		lWidth = max(lWidth, sz.cx + 16 + 8);
	}
	SetWindowPos(g_hwndSFL, HWND_TOPMOST, rcWindow.left, rcWindow.top, lWidth, max(g_CluiData.bUseFloater & CLUI_FLOATER_EVENTS ? 36 : 20, sz.cy + 4), SWP_SHOWWINDOW);
	GetWindowRect(g_hwndSFL, &rcWindow);

	if(g_SFLCachedDC) {
		SelectObject(g_SFLCachedDC, g_SFLhbmOld);
		DeleteObject(g_SFLhbm);
		DeleteDC(g_SFLCachedDC);
		g_SFLCachedDC = 0;
	}

	g_SFLCachedDC = CreateCompatibleDC(hdc);
	g_SFLhbm = CreateCompatibleBitmap(hdc, lWidth, rcWindow.bottom - rcWindow.top);
	g_SFLhbmOld = SelectObject(g_SFLCachedDC, g_SFLhbm);

	ReleaseDC(g_hwndSFL, hdc);
	CluiProtocolStatusChanged(0, 0);
}

void SFL_Create()
{
	if(g_hwndSFL == 0 && g_CluiData.bUseFloater & CLUI_USE_FLOATER && MyUpdateLayeredWindow != NULL)
		g_hwndSFL = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, _T("StatusFloaterClass"), _T("sfl"), WS_VISIBLE, 0, 0, 0, 0, 0, 0, g_hInst, 0);
	else
		return;

	SetWindowLong(g_hwndSFL, GWL_STYLE, GetWindowLong(g_hwndSFL, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_OVERLAPPEDWINDOW | WS_POPUPWINDOW));

	Utils_RestoreWindowPosition(g_hwndSFL, 0, "CLUI", "sfl");
	SFL_SetSize();
}

void FLT_SetSize(struct ExtraCache *centry, LONG width, LONG height)
{
	HDC hdc;
	RECT rcWindow;
	HFONT oldFont;
	int flags = SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOACTIVATE;

	int iVis = pcli->pfnGetWindowVisibleState(pcli->hwndContactList, 0, 0);
	if((g_floatoptions.dwFlags & FLT_AUTOHIDE) && (iVis == 2 || iVis == 4)) //2 = GWVS_VISIBLE, 4 = GWVS_PARTIALLY_COVERED
		flags = SWP_NOMOVE | SWP_NOACTIVATE;

	if(centry->floater) {
		hdc = GetDC(centry->floater->hwnd);
		oldFont = SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

		SetWindowPos(centry->floater->hwnd, HWND_TOPMOST, 0, 0, width, height, flags);
		GetWindowRect(centry->floater->hwnd, &rcWindow);

		if(centry->floater->hdc) {
			SelectObject(centry->floater->hdc, centry->floater->hbmOld);
			DeleteObject(centry->floater->hbm);
			DeleteDC(centry->floater->hdc);
			centry->floater->hdc = 0;
		}

		centry->floater->hdc = CreateCompatibleDC(hdc);
		centry->floater->hbm = CreateCompatibleBitmap(hdc, width, rcWindow.bottom - rcWindow.top);
		centry->floater->hbmOld= SelectObject(centry->floater->hdc, centry->floater->hbm);

		ReleaseDC(centry->floater->hwnd, hdc);
	}
}

void FLT_Create(int iEntry)
{
	struct ExtraCache *centry = NULL;

	if(iEntry >= 0 && iEntry < g_nextExtraCacheEntry) {
		struct ClcContact *contact = NULL;
		struct ClcGroup *group = NULL;

        centry = &g_ExtraCache[iEntry];
		if(centry->floater == 0 && MyUpdateLayeredWindow != NULL) {

			centry->floater = (struct ContactFloater *)malloc(sizeof(struct ContactFloater));
			if(centry->floater == NULL)
				return;
			FLT_AddToList(centry->floater);
			centry->floater->hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, _T("ContactFloaterClass"), _T("sfl"), WS_VISIBLE, 0, 0, 0, 0, 0, 0, g_hInst, (LPVOID)iEntry);
			centry->floater->hContact = centry->hContact;
		}
		else if(centry->floater != NULL) {
			ShowWindow(centry->floater->hwnd, SW_SHOWNOACTIVATE);
			return;
		}

		SetWindowLong(centry->floater->hwnd, GWL_STYLE, GetWindowLong(centry->floater->hwnd, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_OVERLAPPEDWINDOW | WS_POPUPWINDOW));

        if(Utils_RestoreWindowPosition(centry->floater->hwnd, centry->hContact, "CList", "flt"))
			if(Utils_RestoreWindowPositionNoMove(centry->floater->hwnd, centry->hContact, "CList", "flt"))
				SetWindowPos(centry->floater->hwnd, 0, 50, 50, 150, 30, SWP_NOZORDER | SWP_NOACTIVATE);

		//FLT_SetSize(centry, 100, 20);
		ShowWindow(centry->floater->hwnd, SW_SHOWNOACTIVATE);
		if(FindItem(pcli->hwndContactTree, g_clcData, centry->hContact, &contact, &group, 0)) {
			if(contact)
				FLT_Update(g_clcData, contact);
		}
	}
}

extern HDC hdcAV;
extern HDC hdcTempAV;
extern HBITMAP hbmTempAV, hbmTempOldAV;
extern LONG g_maxAV_X, g_maxAV_Y;

void FLT_Update(struct ClcData *dat, struct ClcContact *contact)
{
	RECT rcClient, rcWindow;
	POINT ptDest, ptSrc = {0};
	SIZE szDest;
	BLENDFUNCTION bf = {0};
	HWND hwnd;
	HDC hdc;
	BOOL firstDrawn = TRUE;
	struct ClcGroup *group = NULL;
	struct ClcContact *newContact = NULL;
	HRGN rgn;
	HANDLE hbrBorder;
    COLORREF clrKey;
    HBRUSH   brKey;
    float    greyLevel;

	if(contact == NULL || dat == NULL)
		return;

	if(contact->extraCacheEntry < 0 || contact->extraCacheEntry >= g_nextExtraCacheEntry)
		return;

	if(g_ExtraCache[contact->extraCacheEntry].floater == NULL)
		return;

	FLT_SetSize(&g_ExtraCache[contact->extraCacheEntry], g_floatoptions.width, RowHeights_GetFloatingRowHeight(dat, pcli->hwndContactTree, contact, g_floatoptions.dwFlags) + (2*g_floatoptions.pad_top));

	hwnd = g_ExtraCache[contact->extraCacheEntry].floater->hwnd;
	hdc = g_ExtraCache[contact->extraCacheEntry].floater->hdc;

	if(hwnd == 0)
		return;

	GetClientRect(hwnd, &rcClient);
	GetWindowRect(hwnd, &rcWindow);

	ptDest.x = rcWindow.left;
	ptDest.y = rcWindow.top;
	szDest.cx = rcWindow.right - rcWindow.left;
	szDest.cy = rcWindow.bottom - rcWindow.top;

    /*
     * fill with a DESATURATED representation of the clist bg color and use this later as a color key 
     */

    greyLevel = (float)(GetRValue(g_clcData->bkColour) * 0.299 + GetGValue(g_clcData->bkColour) * 0.587 + GetBValue(g_clcData->bkColour) * 0.144);
    if (greyLevel > 255)
        greyLevel = 255;

    clrKey = RGB((BYTE)greyLevel, (BYTE)greyLevel, (BYTE)greyLevel);
    brKey = CreateSolidBrush(clrKey);
	FillRect(hdc, &rcClient, brKey);
    DeleteObject(brKey);

	SetBkMode(hdc, TRANSPARENT);

	if(g_floatoptions.dwFlags & FLT_ROUNDED){
		rgn = CreateRoundRectRgn(0, 0, rcClient.right, rcClient.bottom, g_floatoptions.radius, g_floatoptions.radius);
		SelectClipRgn(hdc, rgn);
        if(g_floatoptions.dwFlags & FLT_FILLSTDCOLOR) {
            HBRUSH br = CreateSolidBrush(g_clcData->bkColour);
            FillRect(hdc, &rcClient, br);
            DeleteObject(br);
        }
	}

	if(FindItem(pcli->hwndContactTree, dat, contact->hContact, &newContact, &group, 0)) {
		DWORD oldFlags = g_CluiData.dwFlags;
		BYTE oldPadding = g_CluiData.avatarPadding;
		DWORD oldExtraImageMask = g_ExtraCache[contact->extraCacheEntry].dwXMask;
		struct avatarCacheEntry *ace_old = contact->ace;
		BYTE oldDualRow = contact->bSecondLine;

		int oldLeftMargin = dat->leftMargin;
		int oldRightMargin = dat->rightMargin;

		if(g_floatoptions.dwFlags & FLT_SIMPLE) {
			contact->ace = 0;
			contact->bSecondLine = MULTIROW_NEVER;
			g_CluiData.dwFlags &= ~(CLUI_SHOWCLIENTICONS | CLUI_SHOWVISI);
			g_ExtraCache[contact->extraCacheEntry].dwXMask = 0;
		}
		else{
			if(!(g_floatoptions.dwFlags & FLT_AVATARS)) {
                contact->ace = 0;
                g_list_avatars = 0;
            }
            else
                g_list_avatars = 1;

            if(!(g_floatoptions.dwFlags & FLT_DUALROW))
                contact->bSecondLine = MULTIROW_NEVER;
			else
				contact->bSecondLine = MULTIROW_ALWAYS;

			if(!(g_floatoptions.dwFlags & FLT_EXTRAICONS)) {
				g_CluiData.dwFlags &= ~(CLUI_SHOWCLIENTICONS | CLUI_SHOWVISI);
				g_ExtraCache[contact->extraCacheEntry].dwXMask = 0;
			}
		}

		//g_CluiData.avatarPadding = g_floatoptions.pad_top;
		dat->leftMargin = g_floatoptions.pad_left;
		dat->rightMargin = g_floatoptions.pad_right;

		g_HDC = hdc;

        hdcTempAV = CreateCompatibleDC(g_HDC);
        hdcAV = CreateCompatibleDC(g_HDC);
        hbmTempAV = CreateCompatibleBitmap(g_HDC, g_maxAV_X, g_maxAV_Y);
        hbmTempOldAV = SelectObject(hdcTempAV, hbmTempAV);

        g_padding_y = g_floatoptions.pad_top;
		PaintItem(hdc, group, contact, 0, 0, dat, -4, pcli->hwndContactTree, 0, &rcClient, &firstDrawn, 0, rcClient.bottom - rcClient.top);
        g_padding_y = 0;

        SelectObject(hdcTempAV, hbmTempOldAV);
        DeleteObject(hbmTempAV);
        DeleteDC(hdcTempAV);
        DeleteDC(hdcAV);

		g_CluiData.dwFlags = oldFlags;
		g_CluiData.avatarPadding = oldPadding;
		contact->ace = ace_old;
		contact->bSecondLine = oldDualRow;
		g_ExtraCache[contact->extraCacheEntry].dwXMask = oldExtraImageMask;

		dat->leftMargin = oldLeftMargin;
		dat->rightMargin = oldRightMargin;
	}
 
	if(g_floatoptions.dwFlags & FLT_BORDER){
		hbrBorder = CreateSolidBrush(g_floatoptions.border_colour);
		if(g_floatoptions.dwFlags & FLT_ROUNDED)
			FrameRgn(hdc, rgn, hbrBorder, 1, 1);
		else
			FrameRect(hdc, &rcClient, hbrBorder);

		DeleteObject(hbrBorder);
	}
	
	if(g_floatoptions.dwFlags & FLT_ROUNDED)
		DeleteObject(rgn);

	bf.BlendOp = AC_SRC_OVER;
	bf.AlphaFormat = 0;
	bf.SourceConstantAlpha = g_floatoptions.trans;

	if(MyUpdateLayeredWindow)
		MyUpdateLayeredWindow(hwnd, 0, &ptDest, &szDest, hdc, &ptSrc, clrKey /*GetSysColor(COLOR_3DFACE)*/, &bf, ULW_COLORKEY | ULW_ALPHA);
}

/*
 * syncs the floating contacts with clist contact visibility.
 * will hide all floating contacts which are not visible on the list
 * needed after a list rebuild
 */

void FLT_SyncWithClist()
{
	struct ClcContact *contact;
	struct ContactFloater *pCurrent = pFirstFloater;
	HWND hwnd;
    int iVis = pcli->pfnGetWindowVisibleState(pcli->hwndContactList, 0, 0);

	if(g_floatoptions.dwFlags & FLT_SYNCWITHCLIST){
		while(pCurrent) {
			hwnd = pCurrent->hwnd;
			if(hwnd && IsWindow(hwnd)){
				if(FindItem(pcli->hwndContactTree, g_clcData, pCurrent->hContact, &contact, NULL, 0)) {
                    FLT_Update(g_clcData, contact);
                    if(((g_floatoptions.dwFlags & FLT_AUTOHIDE) && (iVis == 2 || iVis == 4)) || !(g_floatoptions.dwFlags & FLT_AUTOHIDE))
                        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
                    else
                        ShowWindow(hwnd, SW_HIDE);
                }
				else
					ShowWindow(hwnd, SW_HIDE);
			}
			pCurrent = pCurrent->pNextFloater;
		}
	}
}

/*
 * quickly show or hide all floating contacts
 * used by autohide/show feature
 */

void FLT_ShowHideAll(int showCmd)
{
	struct ClcContact *contact;
	struct ContactFloater *pCurrent = pFirstFloater;
	HWND hwnd;

	if(g_floatoptions.dwFlags & FLT_AUTOHIDE){
		while(pCurrent) {
			hwnd = pCurrent->hwnd;
			if(hwnd && IsWindow(hwnd)){
				if(showCmd == SW_SHOWNOACTIVATE && FindItem(pcli->hwndContactTree, g_clcData, pCurrent->hContact, &contact, NULL, 0))
					ShowWindow(hwnd, SW_SHOWNOACTIVATE);
				else if(showCmd != SW_SHOWNOACTIVATE)
					ShowWindow(hwnd, showCmd);
			}
			pCurrent = pCurrent->pNextFloater;
		}
	}
}

/*
 * update/repaint all contact floaters
 */

void FLT_RefreshAll()
{
	struct ClcContact *contact = NULL;
	struct ContactFloater *pCurrent = pFirstFloater;

	while(pCurrent) {
		if(FindItem(pcli->hwndContactTree, g_clcData, pCurrent->hContact, &contact, NULL, 0)) {
			HWND hwnd = pCurrent->hwnd;
			if(hwnd && IsWindow(hwnd))
				FLT_Update(g_clcData, contact);
		}
		pCurrent = pCurrent->pNextFloater;
	}
}


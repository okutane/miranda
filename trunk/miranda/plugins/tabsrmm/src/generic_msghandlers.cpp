/*
astyle --force-indent=tab=4 --brackets=linux --indent-switches
--pad=oper --one-line=keep-blocks  --unpad=paren

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

$Id$

*/

/*
* these are generic message handlers which are used by the message dialog window procedure.
* calling them directly instead of using SendMessage() is faster.
* also contains various callback functions for custom buttons
*/

#include "commonheaders.h"

extern RECT	  rcLastStatusBarClick;

/*
* action and callback procedures for the stock button objects
*/

static void BTN_StockAction(ButtonItem *item, HWND hwndDlg, struct _MessageWindowData *dat, HWND hwndBtn)
{
	if (item->dwStockFlags & SBI_HANDLEBYCLIENT && IsWindow(hwndDlg) && dat)
		SendMessage(hwndDlg, WM_COMMAND, MAKELONG(item->uId, BN_CLICKED), (LPARAM)hwndBtn);
	else {
		switch (item->uId) {
			case IDC_SBAR_CANCEL:
				PostMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_SAVE, BN_CLICKED), (LPARAM)hwndBtn);
				break;
			case IDC_SBAR_SLIST:
				SendMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, WM_LBUTTONUP);
				break;
			case IDC_SBAR_FAVORITES: {
				POINT pt;
				int iSelection;
				GetCursorPos(&pt);
				iSelection = TrackPopupMenu(PluginConfig.g_hMenuFavorites, TPM_RETURNCMD, pt.x, pt.y, 0, PluginConfig.g_hwndHotkeyHandler, NULL);
				HandleMenuEntryFromhContact(iSelection);
				break;
			}
			case IDC_SBAR_RECENT: {
				POINT pt;
				int iSelection;
				GetCursorPos(&pt);
				iSelection = TrackPopupMenu(PluginConfig.g_hMenuRecent, TPM_RETURNCMD, pt.x, pt.y, 0, PluginConfig.g_hwndHotkeyHandler, NULL);
				HandleMenuEntryFromhContact(iSelection);
				break;
			}
			case IDC_SBAR_USERPREFS: {
				HANDLE hContact = 0;
				SendMessage(hwndDlg, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
				if (hContact != 0)
					CallService(MS_TABMSG_SETUSERPREFS, (WPARAM)hContact, 0);
				break;
			}
			case IDC_SBAR_TOGGLEFORMAT: {
				if (dat) {
					if (IsDlgButtonChecked(hwndDlg, IDC_SBAR_TOGGLEFORMAT) == BST_UNCHECKED) {
						dat->SendFormat = 0;
						GetSendFormat(hwndDlg, dat, 0);
					} else {
						dat->SendFormat = SENDFORMAT_BBCODE;
						GetSendFormat(hwndDlg, dat, 0);
					}
				}
				break;
			}
		}
	}
}

static void BTN_StockCallback(ButtonItem *item, HWND hwndDlg, struct _MessageWindowData *dat, HWND hwndBtn)
{
}

/*
* predefined button objects for customizeable buttons
*/

static struct SIDEBARITEM sbarItems[] = {
	IDC_SBAR_SLIST, SBI_TOP, &PluginConfig.g_sideBarIcons[0], &PluginConfig.g_sideBarIcons[0], &PluginConfig.g_sideBarIcons[0], _T("t_slist"), BTN_StockAction, BTN_StockCallback, _T("Open session list"),
	IDC_SBAR_FAVORITES, SBI_TOP, &PluginConfig.g_sideBarIcons[1], &PluginConfig.g_sideBarIcons[1], &PluginConfig.g_sideBarIcons[1], _T("t_fav"), BTN_StockAction, BTN_StockCallback, _T("Open favorites"),
	IDC_SBAR_RECENT, SBI_TOP, &PluginConfig.g_sideBarIcons[2],  &PluginConfig.g_sideBarIcons[2], &PluginConfig.g_sideBarIcons[2], _T("t_recent"), BTN_StockAction, BTN_StockCallback, _T("Open recent contacts"),
	IDC_SBAR_USERPREFS, SBI_TOP, &PluginConfig.g_sideBarIcons[4], &PluginConfig.g_sideBarIcons[4], &PluginConfig.g_sideBarIcons[4], _T("t_prefs"), BTN_StockAction, BTN_StockCallback, _T("Contact preferences"),
	IDC_SBAR_TOGGLEFORMAT, SBI_TOP | SBI_TOGGLE, &PluginConfig.g_buttonBarIcons[20], &PluginConfig.g_buttonBarIcons[20], &PluginConfig.g_buttonBarIcons[20], _T("t_tformat"), BTN_StockAction, BTN_StockCallback, _T("Formatting"),
	IDC_SBAR_SETUP, SBI_BOTTOM, &PluginConfig.g_sideBarIcons[3], &PluginConfig.g_sideBarIcons[3], &PluginConfig.g_sideBarIcons[3], _T("t_setup"), BTN_StockAction, BTN_StockCallback, _T("Miranda options"),
	IDOK, SBI_TOP | SBI_HANDLEBYCLIENT, &PluginConfig.g_buttonBarIcons[9], &PluginConfig.g_buttonBarIcons[9], &PluginConfig.g_buttonBarIcons[9], _T("t_send"), BTN_StockAction, BTN_StockCallback, _T("Send message"),
	IDC_SBAR_CANCEL, SBI_TOP, &PluginConfig.g_buttonBarIcons[6], &PluginConfig.g_buttonBarIcons[6], &PluginConfig.g_buttonBarIcons[6], _T("t_close"), BTN_StockAction, BTN_StockCallback, _T("Close session"),
	IDC_SMILEYBTN, SBI_TOP | SBI_HANDLEBYCLIENT, &PluginConfig.g_buttonBarIcons[11], &PluginConfig.g_buttonBarIcons[11], &PluginConfig.g_buttonBarIcons[11], _T("t_emoticon"), BTN_StockAction, BTN_StockCallback, _T("Emoticon"),
	IDC_NAME, SBI_TOP | SBI_HANDLEBYCLIENT, &PluginConfig.g_buttonBarIcons[16], &PluginConfig.g_buttonBarIcons[16], &PluginConfig.g_buttonBarIcons[16], _T("t_menu"), BTN_StockAction, BTN_StockCallback, _T("User menu"),
	IDC_PROTOCOL, SBI_TOP | SBI_HANDLEBYCLIENT, &PluginConfig.g_buttonBarIcons[4], &PluginConfig.g_buttonBarIcons[4], &PluginConfig.g_buttonBarIcons[4], _T("t_details"), BTN_StockAction, BTN_StockCallback, _T("User details"),
	0, 0, 0, 0, 0, _T(""), NULL, NULL, _T("")
};

int BTN_GetStockItem(ButtonItem *item, const TCHAR *szName)
{
	int i = 0;

	while (sbarItems[i].uId) {
		if (!_tcsicmp(sbarItems[i].szName, szName)) {
			item->uId = sbarItems[i].uId;
			//item->dwFlags |= BUTTON_ISSIDEBAR;
			//myGlobals.m_SideBarEnabled = TRUE;
			if (item->dwFlags & BUTTON_ISSIDEBAR) {
				if (sbarItems[i].dwFlags & SBI_TOP)
					item->yOff = 0;
				else if (sbarItems[i].dwFlags & SBI_BOTTOM)
					item->yOff = -1;
			}
			item->dwStockFlags = sbarItems[i].dwFlags;
			item->dwFlags = sbarItems[i].dwFlags & SBI_TOGGLE ? item->dwFlags | BUTTON_ISTOGGLE : item->dwFlags & ~BUTTON_ISTOGGLE;
			item->pfnAction = sbarItems[i].pfnAction;
			item->pfnCallback = sbarItems[i].pfnCallback;
			lstrcpyn(item->szTip, sbarItems[i].tszTip, 256);
			item->szTip[255] = 0;
			if (sbarItems[i].hIcon) {
				item->normalGlyphMetrics[0] = (LONG_PTR)sbarItems[i].hIcon;
				item->dwFlags |= BUTTON_NORMALGLYPHISICON;
			}
			if (sbarItems[i].hIconPressed) {
				item->pressedGlyphMetrics[0] = (LONG_PTR)sbarItems[i].hIconPressed;
				item->dwFlags |= BUTTON_PRESSEDGLYPHISICON;
			}
			if (sbarItems[i].hIconHover) {
				item->hoverGlyphMetrics[0] = (LONG_PTR)sbarItems[i].hIconHover;
				item->dwFlags |= BUTTON_HOVERGLYPHISICON;
			}
			return 1;
		}
		i++;
	}
	return 0;
}

/*
* set the states of defined database action buttons (only if button is a toggle)
*/

void DM_SetDBButtonStates(HWND hwndChild, struct _MessageWindowData *dat)
{
	ButtonItem *buttonItem = dat->pContainer->buttonItems;
	HANDLE hContact = dat->hContact, hFinalContact = 0;
	char *szModule, *szSetting;
	HWND hwndContainer = dat->pContainer->hwnd;;

	while (buttonItem) {
		BOOL result = FALSE;
		HWND hWnd = GetDlgItem(hwndContainer, buttonItem->uId);

		if (buttonItem->pfnCallback)
			buttonItem->pfnCallback(buttonItem, hwndChild, dat, hWnd);

		if (!(buttonItem->dwFlags & BUTTON_ISTOGGLE && buttonItem->dwFlags & BUTTON_ISDBACTION)) {
			buttonItem = buttonItem->nextItem;
			continue;
		}
		szModule = buttonItem->szModule;
		szSetting = buttonItem->szSetting;
		if (buttonItem->dwFlags & BUTTON_DBACTIONONCONTACT || buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION) {
			if (hContact == 0) {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
				buttonItem = buttonItem->nextItem;
				continue;
			}
			if (buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION)
				szModule = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			hFinalContact = hContact;
		} else
			hFinalContact = 0;

		if (buttonItem->type == DBVT_ASCIIZ) {
			DBVARIANT dbv = {0};

			if (!DBGetContactSettingString(hFinalContact, szModule, szSetting, &dbv)) {
				result = !strcmp((char *)buttonItem->bValuePush, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
		} else {
			switch (buttonItem->type) {
				case DBVT_BYTE: {
					BYTE val = M->GetByte(hFinalContact, szModule, szSetting, 0);
					result = (val == buttonItem->bValuePush[0]);
					break;
				}
				case DBVT_WORD: {
					WORD val = DBGetContactSettingWord(hFinalContact, szModule, szSetting, 0);
					result = (val == *((WORD *) & buttonItem->bValuePush));
					break;
				}
				case DBVT_DWORD: {
					DWORD val = M->GetDword(hFinalContact, szModule, szSetting, 0);
					result = (val == *((DWORD *) & buttonItem->bValuePush));
					break;
				}
			}
		}
		SendMessage(hWnd, BM_SETCHECK, (WPARAM)result, 0);
		buttonItem = buttonItem->nextItem;
	}
}

LRESULT DM_ScrollToBottom(HWND hwndDlg, struct _MessageWindowData *dat, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si = { 0 };

	if (dat == NULL)
		dat = (struct _MessageWindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	if (dat) {

		if (dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED)
			return 0;

		if (IsIconic(dat->pContainer->hwnd))
			dat->dwFlags |= MWF_DEFERREDSCROLL;

		if (dat->hwndIEView) {
			PostMessage(hwndDlg, DM_SCROLLIEVIEW, 0, 0);
			return 0;
		} else if (dat->hwndHPP) {
			SendMessage(hwndDlg, DM_SCROLLIEVIEW, 0, 0);
			return 0;
		} else {
			HWND hwnd = GetDlgItem(hwndDlg, dat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG);

			if (lParam)
				SendMessage(hwnd, WM_SIZE, 0, 0);

			if (wParam == 1 && lParam == 1) {
				RECT rc;
				int len;

				GetClientRect(hwnd, &rc);
				len = GetWindowTextLengthA(hwnd);
				SendMessage(hwnd, EM_SETSEL, len - 1, len - 1);
			}
			if (wParam)
				SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			else
				PostMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			if (lParam)
				InvalidateRect(hwnd, NULL, FALSE);
		}
	}
	return 0;
}

LRESULT DM_LoadLocale(_MessageWindowData *dat)
{
	/*
	* set locale if saved to contact
	*/

	if (dat) {
		if (dat->dwFlags & MWF_WASBACKGROUNDCREATE)
			return 0;

		if (PluginConfig.m_AutoLocaleSupport && dat->hContact != 0) {
			DBVARIANT dbv;
			int res;
			char szKLName[KL_NAMELENGTH+1];
			UINT flags = KLF_ACTIVATE;

			res = DBGetContactSettingString(dat->hContact, SRMSGMOD_T, "locale", &dbv);
			if (res == 0) {
				dat->hkl = LoadKeyboardLayoutA(dbv.pszVal, KLF_ACTIVATE);
				GetLocaleID(dat, dbv.pszVal);
				PostMessage(dat->hwnd, DM_SETLOCALE, 0, 0);
				DBFreeVariant(&dbv);
			} else {
				GetKeyboardLayoutNameA(szKLName);
				dat->hkl = LoadKeyboardLayoutA(szKLName, 0);
				DBWriteContactSettingString(dat->hContact, SRMSGMOD_T, "locale", szKLName);
				GetLocaleID(dat, szKLName);
			}
			UpdateReadChars(dat);
		}
	}
	return 0;
}

LRESULT DM_RecalcPictureSize(_MessageWindowData *dat)
{
	BITMAP bminfo;
	HBITMAP hbm;

	if (dat) {
		hbm = ((dat->dwFlagsEx & MWF_SHOW_INFOPANEL) && PluginConfig.m_AvatarMode != 5) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown);

		if (hbm == 0) {
			dat->pic.cy = dat->pic.cx = 60;
			return 0;
		}
		GetObject(hbm, sizeof(bminfo), &bminfo);
		CalcDynamicAvatarSize(dat, &bminfo);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
	return 0;
}

LRESULT DM_UpdateLastMessage(const _MessageWindowData *dat)
{
	if (dat) {
		if (dat->pContainer->hwndStatus == 0)
			return 0;
		if (dat->pContainer->hwndActive != dat->hwnd)
			return 0;
		if (dat->showTyping) {
			TCHAR szBuf[80];

			mir_sntprintf(szBuf, safe_sizeof(szBuf), TranslateT("%s is typing..."), dat->szNickname);
			SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM) szBuf);
			SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM) PluginConfig.g_buttonBarIcons[5]);
			if (dat->pContainer->hwndSlist)
				SendMessage(dat->pContainer->hwndSlist, BM_SETIMAGE, IMAGE_ICON, (LPARAM)PluginConfig.g_buttonBarIcons[5]);
			return 0;
		}
		if (dat->lastMessage || dat->pContainer->dwFlags & CNT_UINSTATUSBAR) {
			DBTIMETOSTRINGT dbtts;
			TCHAR date[64], time[64];

			if (!(dat->pContainer->dwFlags & CNT_UINSTATUSBAR)) {
				dbtts.szFormat = _T("d");
				dbtts.cbDest = safe_sizeof(date);
				dbtts.szDest = date;
				CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, dat->lastMessage, (LPARAM) & dbtts);
				if (dat->pContainer->dwFlags & CNT_UINSTATUSBAR && lstrlen(date) > 6)
					date[lstrlen(date) - 5] = 0;
				dbtts.szFormat = _T("t");
				dbtts.cbDest = safe_sizeof(time);
				dbtts.szDest = time;
				CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, dat->lastMessage, (LPARAM) & dbtts);
			}
			if (dat->pContainer->dwFlags & CNT_UINSTATUSBAR) {
				char fmt[100];
				char *uidName = (char *)CallProtoService(dat->szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
				if (!uidName) uidName = Translate("ID");
				mir_snprintf(fmt, sizeof(fmt), "%s: %s", uidName, dat->uin);
				SendMessageA(dat->pContainer->hwndStatus, SB_SETTEXTA, 0, (LPARAM) fmt);
			} else {
				TCHAR fmt[100];
				mir_sntprintf(fmt, safe_sizeof(fmt), TranslateT("Last received: %s at %s"), date, time);
				SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM) fmt);
			}
			SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)(nen_options.bFloaterInWin ? PluginConfig.g_buttonBarIcons[16] : 0));
			if (dat->pContainer->hwndSlist)
				SendMessage(dat->pContainer->hwndSlist, BM_SETIMAGE, IMAGE_ICON, (LPARAM)PluginConfig.g_buttonBarIcons[16]);
		} else {
			SendMessageA(dat->pContainer->hwndStatus, SB_SETTEXTA, 0, (LPARAM) "");
			SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)(nen_options.bFloaterInWin ? PluginConfig.g_buttonBarIcons[16] : 0));
			if (dat->pContainer->hwndSlist)
				SendMessage(dat->pContainer->hwndSlist, BM_SETIMAGE, IMAGE_ICON, (LPARAM)PluginConfig.g_buttonBarIcons[16]);
		}
	}
	return 0;
}

/*
* save current keyboard layout for the given contact
*/

LRESULT DM_SaveLocale(_MessageWindowData *dat, WPARAM wParam, LPARAM lParam)
{
	if (dat) {
		if (PluginConfig.m_AutoLocaleSupport && dat->hContact && dat->pContainer->hwndActive == dat->hwnd) {
			char szKLName[KL_NAMELENGTH + 1];
			if ((HKL)lParam != dat->hkl) {
				dat->hkl = (HKL)lParam;
				ActivateKeyboardLayout(dat->hkl, 0);
				GetKeyboardLayoutNameA(szKLName);
				DBWriteContactSettingString(dat->hContact, SRMSGMOD_T, "locale", szKLName);
				GetLocaleID(dat, szKLName);
				UpdateReadChars(dat);
			}
		}
	}
	return 0;
}

/*
* generic handler for the WM_COPY message in message log/chat history richedit control(s).
* it filters out the invisible event boundary markers from the text copied to the clipboard.
*/

LRESULT DM_WMCopyHandler(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = CallWindowProc(oldWndProc, hwnd, WM_COPY, wParam, lParam);

	if (OpenClipboard(hwnd)) {
#if defined(_UNICODE)
		HANDLE hClip = GetClipboardData(CF_UNICODETEXT);
#else
		HANDLE hClip = GetClipboardData(CF_TEXT);
#endif
		if (hClip) {
			HGLOBAL hgbl;
			TCHAR *tszLocked;
			TCHAR *tszText = (TCHAR *)malloc((lstrlen((TCHAR *)hClip) + 2) * sizeof(TCHAR));

			lstrcpy(tszText, (TCHAR *)hClip);
			FilterEventMarkers(tszText);
			EmptyClipboard();

			hgbl = GlobalAlloc(GMEM_MOVEABLE, (lstrlen(tszText) + 1) * sizeof(TCHAR));
			tszLocked = (TCHAR *)GlobalLock(hgbl);
			lstrcpy(tszLocked, tszText);
			GlobalUnlock(hgbl);
#if defined(_UNICODE)
			SetClipboardData(CF_UNICODETEXT, hgbl);
#else
			SetClipboardData(CF_TEXT, hgbl);
#endif
			if (tszText)
				free(tszText);
		}
		CloseClipboard();
	}
	return result;
}

/*
* create embedded contact list control
*/

HWND DM_CreateClist(HWND hwndParent, struct _MessageWindowData *dat)
{
	HWND hwndClist = CreateWindowExA(0, "CListControl", "", WS_TABSTOP | WS_VISIBLE | WS_CHILD | 0x248, 184, 0, 30, 30, hwndParent, (HMENU)IDC_CLIST, g_hInst, NULL);

	//MAD: fix for little bug, when following code didn't work (another hack :) )
	HANDLE hItem;
	SendMessage(hwndClist, WM_TIMER, 14, 0);
	//
	hItem = (HANDLE) SendDlgItemMessage(hwndParent, IDC_CLIST, CLM_FINDCONTACT, (WPARAM) dat->hContact, 0);

	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) & ~CLS_EX_TRACKSELECT);
	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) | (CLS_EX_NOSMOOTHSCROLLING | CLS_EX_NOTRANSLUCENTSEL));
	//MAD: show offline contacts in multi-send
	if (!PluginConfig.m_AllowOfflineMultisend)
		SetWindowLongPtr(hwndClist, GWL_STYLE, GetWindowLongPtr(hwndClist, GWL_STYLE) | CLS_HIDEOFFLINE);
	//
	if (hItem)
		SendMessage(hwndClist, CLM_SETCHECKMARK, (WPARAM) hItem, 1);

	if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_DISABLEGROUPS && !M->GetByte("CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
		SendMessage(hwndClist, CLM_SETUSEGROUPS, (WPARAM) FALSE, 0);
	else
		SendMessage(hwndClist, CLM_SETUSEGROUPS, (WPARAM) TRUE, 0);
	if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_HIDEEMPTYGROUPS && M->GetByte("CList", "HideEmptyGroups", SETTING_USEGROUPS_DEFAULT))
		SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, (WPARAM) TRUE, 0);
	else
		SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, (WPARAM) FALSE, 0);
	SendMessage(hwndClist, CLM_FIRST + 106, 0, 1);
	SendMessage(hwndClist, CLM_AUTOREBUILD, 0, 0);

	return hwndClist;
}

LRESULT DM_MouseWheelHandler(HWND hwnd, HWND hwndParent, struct _MessageWindowData *mwdat, WPARAM wParam, LPARAM lParam)
{
	RECT rc, rc1;
	POINT pt;
	TCHITTESTINFO hti;
	HWND hwndTab;
	UINT uID = mwdat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG;

	GetCursorPos(&pt);
	GetWindowRect(hwnd, &rc);
	if (PtInRect(&rc, pt))
		return 1;
	if (mwdat->pContainer->dwFlags & CNT_SIDEBAR) {
		GetWindowRect(GetDlgItem(mwdat->pContainer->hwnd, IDC_SIDEBARUP), &rc);
		GetWindowRect(GetDlgItem(mwdat->pContainer->hwnd, IDC_SIDEBARDOWN), &rc1);
		rc.bottom = rc1.bottom;
		if (PtInRect(&rc, pt)) {
			short amount = (short)(HIWORD(wParam));
			SendMessage(mwdat->pContainer->hwnd, WM_COMMAND, MAKELONG(amount > 0 ? IDC_SIDEBARUP : IDC_SIDEBARDOWN, 0), 0);
			return 0;
		}
	}
	if (mwdat->hwndIEView)
		GetWindowRect(mwdat->hwndIEView, &rc);
	else if (mwdat->hwndHPP)
		GetWindowRect(mwdat->hwndHPP, &rc);
	else
		GetWindowRect(GetDlgItem(hwndParent, uID), &rc);
	if (PtInRect(&rc, pt)) {
		HWND hwnd = (mwdat->hwndIEView || mwdat->hwndHPP) ? mwdat->hwndIWebBrowserControl : GetDlgItem(hwndParent, uID);
		short wDirection = (short)HIWORD(wParam);

		if (hwnd == 0)
			hwnd = WindowFromPoint(pt);

		if (LOWORD(wParam) & MK_SHIFT || M->GetByte("fastscroll", 0)) {
			if (wDirection < 0)
				SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
			else if (wDirection > 0)
				SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
		} else
			SendMessage(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return 0;
	}
	hwndTab = GetDlgItem(mwdat->pContainer->hwnd, IDC_MSGTABS);
	GetCursorPos(&hti.pt);
	ScreenToClient(hwndTab, &hti.pt);
	hti.flags = 0;
	if (TabCtrl_HitTest(hwndTab, &hti) != -1) {
		SendMessage(hwndTab, WM_MOUSEWHEEL, wParam, -1);
		return 0;
	}
	return 1;
}

void DM_FreeTheme(_MessageWindowData *dat)
{
	if(dat) {
		if (CMimAPI::m_pfnCloseThemeData) {
			if(dat->hTheme) {
				CMimAPI::m_pfnCloseThemeData(dat->hTheme);
				dat->hTheme = 0;
			}
			if(dat->hThemeIP) {
				CMimAPI::m_pfnCloseThemeData(dat->hThemeIP);
				dat->hThemeIP = 0;
			}
			if(dat->hThemeToolbar) {
				CMimAPI::m_pfnCloseThemeData(dat->hThemeToolbar);
				dat->hThemeToolbar = 0;
			}
		}
	}
}

LRESULT DM_ThemeChanged(_MessageWindowData *dat)
{
	CSkinItem *item_log = &SkinItems[ID_EXTBKHISTORY];
	CSkinItem *item_msg = &SkinItems[ID_EXTBKINPUTAREA];

	dat->bFlatMsgLog = M->GetByte("flatlog", 1);

	HWND	hwnd = dat->hwnd;

	if (!dat->bFlatMsgLog)
		dat->hTheme = (M->isVSAPIState() && CMimAPI::m_pfnOpenThemeData) ? CMimAPI::m_pfnOpenThemeData(hwnd, L"EDIT") : 0;
	else
		dat->hTheme = 0;

	if (dat->bType == SESSIONTYPE_IM) {
		if (dat->bFlatMsgLog || dat->hTheme != 0 || (dat->pContainer->bSkinned && !item_log->IGNORED))
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_LOG), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_LOG), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
		if (dat->bFlatMsgLog || dat->hTheme != 0 || (dat->pContainer->bSkinned && !item_msg->IGNORED))
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_MESSAGE), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_MESSAGE), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
	} else {
		if (dat->bFlatMsgLog || dat->hTheme != 0 || (dat->pContainer->bSkinned && !item_log->IGNORED)) {
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_LOG), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_LOG), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_LIST), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_LIST), GWL_EXSTYLE) & ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
		}
		if (dat->bFlatMsgLog || dat->hTheme != 0 || (dat->pContainer->bSkinned && !item_msg->IGNORED))
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_MESSAGE), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_MESSAGE), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
	}
	dat->hThemeIP = M->isAero() ? CMimAPI::m_pfnOpenThemeData(hwnd, L"ButtonStyle") : 0;
	dat->hThemeToolbar = M->isAero() ? CMimAPI::m_pfnOpenThemeData(hwnd, L"REBAR") : 0;

	return 0;
}

/*
* status icon stuff (by sje, used for indicating encryption status in the status bar
* this is now part of the message window api
*/

static HANDLE hHookIconPressedEvt;
struct StatusIconListNode *status_icon_list = 0;
int status_icon_list_size = 0;

static INT_PTR SI_AddStatusIcon(WPARAM wParam, LPARAM lParam)
{
	StatusIconData *sid = (StatusIconData *)lParam;
	struct StatusIconListNode *siln = (struct StatusIconListNode *)mir_alloc(sizeof(struct StatusIconListNode));

	siln->sid.cbSize = sid->cbSize;
	siln->sid.szModule = mir_strdup(sid->szModule);
	siln->sid.dwId = sid->dwId;
	siln->sid.hIcon = sid->hIcon;
	siln->sid.hIconDisabled = sid->hIconDisabled;
	siln->sid.flags = sid->flags;
	if (sid->szTooltip) siln->sid.szTooltip = mir_strdup(sid->szTooltip);
	else siln->sid.szTooltip = 0;

	siln->next = status_icon_list;
	status_icon_list = siln;
	status_icon_list_size++;

	M->BroadcastMessage(DM_STATUSICONCHANGE, 0, 0);
	return 0;
}

static INT_PTR SI_RemoveStatusIcon(WPARAM wParam, LPARAM lParam)
{
	StatusIconData *sid = (StatusIconData *)lParam;
	struct StatusIconListNode *current = status_icon_list, *prev = 0;

	while (current) {
		if (strcmp(current->sid.szModule, sid->szModule) == 0 && current->sid.dwId == sid->dwId) {
			if (prev) prev->next = current->next;
			else status_icon_list = current->next;

			status_icon_list_size--;

			mir_free(current->sid.szModule);
			DestroyIcon(current->sid.hIcon);
			if (current->sid.hIconDisabled) DestroyIcon(current->sid.hIconDisabled);
			if (current->sid.szTooltip) mir_free(current->sid.szTooltip);
			mir_free(current);
			M->BroadcastMessage(DM_STATUSICONCHANGE, 0, 0);
			return 0;
		}

		prev = current;
		current = current->next;
	}
	return 1;
}

static void SI_RemoveAllStatusIcons(void)
{
	struct StatusIconListNode *current;

	while (status_icon_list) {
		current = status_icon_list;
		status_icon_list = status_icon_list->next;
		status_icon_list_size--;

		mir_free(current->sid.szModule);
		DestroyIcon(current->sid.hIcon);
		if (current->sid.hIconDisabled) DestroyIcon(current->sid.hIconDisabled);
		if (current->sid.szTooltip) mir_free(current->sid.szTooltip);
		mir_free(current);
	}
	M->BroadcastMessage(DM_STATUSICONCHANGE, 0, 0);
}

static INT_PTR SI_ModifyStatusIcon(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;

	StatusIconData *sid = (StatusIconData *)lParam;
	struct StatusIconListNode *current = status_icon_list;

	while (current) {
		if (strcmp(current->sid.szModule, sid->szModule) == 0 && current->sid.dwId == sid->dwId) {
			if (!hContact) {
				current->sid.flags = sid->flags;
				if (sid->hIcon) {
					DestroyIcon(current->sid.hIcon);
					current->sid.hIcon = sid->hIcon;
				}
				if (sid->hIconDisabled) {
					DestroyIcon(current->sid.hIconDisabled);
					current->sid.hIconDisabled = sid->hIconDisabled;
				}
				if (sid->szTooltip) {
					if (current->sid.szTooltip) mir_free(current->sid.szTooltip);
					current->sid.szTooltip = mir_strdup(sid->szTooltip);
				}

				M->BroadcastMessage(DM_STATUSICONCHANGE, 0, 0);
			} else {
				char buff[256];
				HWND hwnd;
				if (!(sid->flags&MBF_OWNERSTATE)) {
					sprintf(buff, "SRMMStatusIconFlags%d", (int)sid->dwId);
					M->WriteByte(hContact, sid->szModule, buff, (BYTE)sid->flags);
				}
				if ((hwnd = M->FindWindow(hContact))) {
					if (sid->flags&MBF_OWNERSTATE) {

						struct StatusIconListNode *siln = NULL;
						struct _MessageWindowData *dat = (struct _MessageWindowData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
						struct StatusIconListNode *psi = dat->pSINod;
						while (psi) {
							if (strcmp(psi->sid.szModule, sid->szModule) == 0 && psi->sid.dwId == sid->dwId) {
								siln = psi;
								break;
							}
							psi = psi->next;
						}
						if (!siln) {
							siln = (struct StatusIconListNode *)mir_alloc(sizeof(struct StatusIconListNode));
							siln->sid.szModule = mir_strdup(sid->szModule);
							siln->sid.dwId = sid->dwId;
							siln->sid.hIcon = sid->hIcon;
							siln->sid.hIconDisabled = sid->hIconDisabled;
							siln->sid.flags = sid->flags;

							if (sid->szTooltip) siln->sid.szTooltip = mir_strdup(sid->szTooltip);
							else siln->sid.szTooltip = 0;

							siln->next = dat->pSINod;
							dat->pSINod = siln;
						} else {
							siln->sid.hIcon = sid->hIcon;
							siln->sid.hIconDisabled = sid->hIconDisabled;
							siln->sid.flags = sid->flags;
							if (siln->sid.szTooltip) mir_free(siln->sid.szTooltip);

							if (sid->szTooltip) siln->sid.szTooltip = mir_strdup(sid->szTooltip);
							else siln->sid.szTooltip = 0;

						}


						PostMessage(hwnd, DM_STATUSICONCHANGE, 0, 0);
					} else
						PostMessage(hwnd, DM_STATUSICONCHANGE, 0, 0);
				}
			}
			return 0;
		}
		current = current->next;
	}

	return 1;
}

void DrawStatusIcons(struct _MessageWindowData *dat, HDC hDC, RECT r, int gap)
{
	struct StatusIconListNode *current = status_icon_list;
	HICON hIcon = NULL;
	char buff[256];
	int flags;
	int x = r.left;
	SetBkMode(hDC, TRANSPARENT);
	while (current) {
		if (current->sid.flags&MBF_OWNERSTATE) {
			struct StatusIconListNode *currentSIN = dat->pSINod;
			flags = current->sid.flags;
			hIcon = current->sid.hIcon;
			while (currentSIN) {
				if (strcmp(currentSIN->sid.szModule, current->sid.szModule) == 0 && currentSIN->sid.dwId == current->sid.dwId) {
					flags = currentSIN->sid.flags;
					hIcon = currentSIN->sid.hIcon;
					break;
				}
				currentSIN = currentSIN->next;
			}
		} else {
			sprintf(buff, "SRMMStatusIconFlags%d", (int)current->sid.dwId);
			flags = M->GetByte(dat->hContact, current->sid.szModule, buff, current->sid.flags);
		}

		if (!(flags & MBF_HIDDEN)) {
			if (!(flags&MBF_OWNERSTATE) && (flags & MBF_DISABLED) && current->sid.hIconDisabled)
				hIcon = current->sid.hIconDisabled;
			else if (!(flags&MBF_OWNERSTATE))
				hIcon = current->sid.hIcon;

			if (flags & MBF_DISABLED && current->sid.hIconDisabled == (HICON)0)
				CSkin::DrawDimmedIcon(hDC, x, (r.top + r.bottom - PluginConfig.m_smcxicon) >> 1, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, hIcon, 50);
			else
				DrawIconEx(hDC, x, (r.top + r.bottom - PluginConfig.m_smcxicon) >> 1, hIcon, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);

			x += PluginConfig.m_smcxicon + gap;
		}
		current = current->next;
	}
	DrawIconEx(hDC, x, (r.top + r.bottom - PluginConfig.m_smcxicon) >> 1, dat->pContainer->dwFlags & CNT_NOSOUND ? PluginConfig.g_buttonBarIcons[23] : PluginConfig.g_buttonBarIcons[22], PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);
	x += PluginConfig.m_smcxicon + gap;
	if (dat->bType == SESSIONTYPE_IM)
		DrawIconEx(hDC, x, (r.top + r.bottom - PluginConfig.m_smcxicon) >> 1, M->GetByte(dat->hContact, SRMSGMOD, SRMSGSET_TYPING, M->GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)) ? PluginConfig.g_buttonBarIcons[12] : PluginConfig.g_buttonBarIcons[13], PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);
	else
		CSkin::DrawDimmedIcon(hDC, x, (r.top + r.bottom - PluginConfig.m_smcxicon) >> 1, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon,
					   M->GetByte(dat->hContact, SRMSGMOD, SRMSGSET_TYPING, M->GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)) ? PluginConfig.g_buttonBarIcons[12] : PluginConfig.g_buttonBarIcons[13], 50);
}

void SI_CheckStatusIconClick(struct _MessageWindowData *dat, HWND hwndFrom, POINT pt, RECT r, int gap, int code)
{
	StatusIconClickData sicd;
	struct StatusIconListNode *current = status_icon_list;
	struct StatusIconListNode *clicked = NULL;

	unsigned int iconNum = (pt.x - (r.left + 0)) / (PluginConfig.m_smcxicon + gap);
	unsigned int list_icons = 0;
	char         buff[100];
	DWORD		 flags;

	if (dat && (code == NM_CLICK || code == NM_RCLICK)) {
		POINT	ptScreen;

		GetCursorPos(&ptScreen);
		if (!PtInRect(&rcLastStatusBarClick, ptScreen))
			return;
	}
	while (current && dat) {
		if (current->sid.flags&MBF_OWNERSTATE) {
			struct StatusIconListNode *currentSIN = dat->pSINod;
			flags = current->sid.flags;
			while (currentSIN) {
				if (strcmp(currentSIN->sid.szModule, current->sid.szModule) == 0 && currentSIN->sid.dwId == current->sid.dwId) {
					flags = currentSIN->sid.flags;
					break;
				}
				currentSIN = currentSIN->next;
			}
		} else  {
			sprintf(buff, "SRMMStatusIconFlags%d", (int)current->sid.dwId);
			flags = M->GetByte(dat->hContact, current->sid.szModule, buff, current->sid.flags);
		}
		if (!(flags & MBF_HIDDEN)) {
			if (list_icons++ == iconNum)
				clicked = current;
		}
		current = current->next;
	}

	if ((int)iconNum == list_icons && code != NM_RCLICK) {
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			struct ContainerWindowData *piContainer = pFirstContainer;

			while (piContainer) {
				piContainer->dwFlags = ((dat->pContainer->dwFlags & CNT_NOSOUND) ? piContainer->dwFlags | CNT_NOSOUND : piContainer->dwFlags & ~CNT_NOSOUND);
				InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
				piContainer = piContainer->pNextContainer;
			}
		} else {
			dat->pContainer->dwFlags ^= CNT_NOSOUND;
			InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
		}
	} else if ((int)iconNum == list_icons + 1 && code != NM_RCLICK && dat->bType == SESSIONTYPE_IM) {
		SendMessage(dat->pContainer->hwndActive, WM_COMMAND, IDC_SELFTYPING, 0);
		InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
	} else if (clicked) {
		sicd.cbSize = sizeof(StatusIconClickData);
		GetCursorPos(&sicd.clickLocation);
		sicd.dwId = clicked->sid.dwId;
		sicd.szModule = clicked->sid.szModule;
		sicd.flags = (code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0);
		NotifyEventHooks(hHookIconPressedEvt, (WPARAM)dat->hContact, (LPARAM)&sicd);
		InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
	}
}

static HANDLE SI_hServiceIcon[3];

int SI_InitStatusIcons()
{
	SI_hServiceIcon[0] = CreateServiceFunction(MS_MSG_ADDICON, SI_AddStatusIcon);
	SI_hServiceIcon[1] = CreateServiceFunction(MS_MSG_REMOVEICON, SI_RemoveStatusIcon);
	SI_hServiceIcon[2] = CreateServiceFunction(MS_MSG_MODIFYICON, SI_ModifyStatusIcon);
	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);

	return 0;
}


int SI_DeinitStatusIcons()
{
	int i;
	DestroyHookableEvent(hHookIconPressedEvt);
	for (i = 0; i < 3; i++)
		DestroyServiceFunction(SI_hServiceIcon[i]);
	SI_RemoveAllStatusIcons();
	return 0;
}

int SI_GetStatusIconsCount()
{
	return status_icon_list_size;
}

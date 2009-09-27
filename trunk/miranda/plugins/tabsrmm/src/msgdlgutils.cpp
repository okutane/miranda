/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id$
 *
 * Helper functions for the message dialog.
 *
 */

#include "commonheaders.h"
#pragma hdrstop

//#include "m_MathModule.h"

extern      NEN_OPTIONS nen_options;
extern      LOGFONTA logfonts[MSGDLGFONTCOUNT + 2];
extern      COLORREF fontcolors[MSGDLGFONTCOUNT + 2];
extern      TemplateSet LTR_Active, RTL_Active;

static struct RTFColorTable _rtf_ctable[] = {
	_T("red"), RGB(255, 0, 0), 0, ID_FONT_RED,
	_T("blue"), RGB(0, 0, 255), 0, ID_FONT_BLUE,
	_T("green"), RGB(0, 255, 0), 0, ID_FONT_GREEN,
	_T("magenta"), RGB(255, 0, 255), 0, ID_FONT_MAGENTA,
	_T("yellow"), RGB(255, 255, 0), 0, ID_FONT_YELLOW,
	_T("cyan"), RGB(0, 255, 255), 0, ID_FONT_CYAN,
	_T("black"), 0, 0, ID_FONT_BLACK,
	_T("white"), RGB(255, 255, 255), 0, ID_FONT_WHITE,
	_T(""), 0, 0, 0
};

struct RTFColorTable *rtf_ctable = 0;
unsigned int g_ctable_size;

#ifndef SHVIEW_THUMBNAIL
#define SHVIEW_THUMBNAIL 0x702D
#endif

#define EVENTTYPE_NICKNAME_CHANGE       9001
#define EVENTTYPE_STATUSMESSAGE_CHANGE  9002
#define EVENTTYPE_AVATAR_CHANGE         9003
#define EVENTTYPE_CONTACTLEFTCHANNEL    9004
#define EVENTTYPE_WAT_ANSWER            9602
#define EVENTTYPE_JABBER_CHATSTATES     2000
#define EVENTTYPE_JABBER_PRESENCE       2001

static int g_status_events[] = {
	EVENTTYPE_STATUSCHANGE,
	EVENTTYPE_CONTACTLEFTCHANNEL,
	EVENTTYPE_WAT_ANSWER,
	EVENTTYPE_JABBER_CHATSTATES,
	EVENTTYPE_JABBER_PRESENCE

};

static int g_status_events_size = 0;
#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )

BOOL TSAPI IsStatusEvent(int eventType)
{
	int i;

	if (g_status_events_size == 0)
		g_status_events_size = MAX_REGS(g_status_events);

	for (i = 0; i < g_status_events_size; i++) {
		if (eventType == g_status_events[i])
			return TRUE;
	}
	return FALSE;
}

/*
 * init default color table. the table may grow when using custom colors via bbcodes
 */

void TSAPI RTF_CTableInit()
{
	int iSize = sizeof(struct RTFColorTable) * RTF_CTABLE_DEFSIZE;

	rtf_ctable = (struct RTFColorTable *) malloc(iSize);
	ZeroMemory(rtf_ctable, iSize);
	CopyMemory(rtf_ctable, _rtf_ctable, iSize);
	PluginConfig.rtf_ctablesize = g_ctable_size = RTF_CTABLE_DEFSIZE;
}

/*
 * add a color to the global rtf color table
 */

void RTF_ColorAdd(const TCHAR *tszColname, size_t length)
{
	TCHAR *stopped;
	COLORREF clr;

	PluginConfig.rtf_ctablesize++;
	g_ctable_size++;
	rtf_ctable = (struct RTFColorTable *)realloc(rtf_ctable, sizeof(struct RTFColorTable) * g_ctable_size);
	clr = _tcstol(tszColname, &stopped, 16);
	mir_sntprintf(rtf_ctable[g_ctable_size - 1].szName, length + 1, _T("%06x"), clr);
	rtf_ctable[g_ctable_size - 1].menuid = rtf_ctable[g_ctable_size - 1].index = 0;

	clr = _tcstol(tszColname, &stopped, 16);
	rtf_ctable[g_ctable_size - 1].clr = (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)));
}

/*
 * reorder tabs within a container. fSavePos indicates whether the new position should be saved to the
 * contacts db record (if so, the container will try to open the tab at the saved position later)
 */

void TSAPI RearrangeTab(HWND hwndDlg, const _MessageWindowData *dat, int iMode, BOOL fSavePos)
{
	TCITEM item = {0};
	HWND hwndTab = GetParent(hwndDlg);
	int newIndex;
	TCHAR oldText[512];
	item.mask = TCIF_IMAGE | TCIF_TEXT | TCIF_PARAM;
	item.pszText = oldText;
	item.cchTextMax = 500;

	if (dat == NULL || !IsWindow(hwndDlg))
		return;

	TabCtrl_GetItem(hwndTab, dat->iTabID, &item);
	newIndex = LOWORD(iMode);

	if (newIndex >= 0 && newIndex <= TabCtrl_GetItemCount(hwndTab)) {
		TabCtrl_DeleteItem(hwndTab, dat->iTabID);
		item.lParam = (LPARAM)hwndDlg;
		TabCtrl_InsertItem(hwndTab, newIndex, &item);
		BroadCastContainer(dat->pContainer, DM_REFRESHTABINDEX, 0, 0);
		ActivateTabFromHWND(hwndTab, hwndDlg);
		if (fSavePos)
			M->WriteDword(dat->hContact, SRMSGMOD_T, "tabindex", newIndex * 100);
	}
}

/*
 * subclassing for the save as file dialog (needed to set it to thumbnail view on Windows 2000
 * or later
 */

static BOOL CALLBACK OpenFileSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG: {
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
			break;
		}
		case WM_NOTIFY: {
			OPENFILENAMEA *ofn = (OPENFILENAMEA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			HWND hwndParent = GetParent(hwnd);
			HWND hwndLv = FindWindowEx(hwndParent, NULL, _T("SHELLDLL_DefView"), NULL) ;

			if (hwndLv != NULL && *((DWORD *)(ofn->lCustData))) {
				SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
				*((DWORD *)(ofn->lCustData)) = 0;
			}
			break;
		}
	}
	return FALSE;
}

/*
 * saves a contact picture to disk
 * takes hbm (bitmap handle) and bool isOwnPic (1 == save the picture as your own avatar)
 * requires AVS and ADVAIMG services (Miranda 0.7+)
 */

static void SaveAvatarToFile(_MessageWindowData *dat, HBITMAP hbm, int isOwnPic)
{
	TCHAR szFinalPath[MAX_PATH];
	TCHAR szFinalFilename[MAX_PATH];
	TCHAR szBaseName[MAX_PATH];
	TCHAR szTimestamp[100];
	OPENFILENAME ofn = {0};
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	static TCHAR *forbiddenCharacters = _T("%/\\'");
	int i, j;
	DWORD setView = 1;

	mir_sntprintf(szTimestamp, 100, _T("%04u %02u %02u_%02u%02u"), lt->tm_year + 1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min);

#if defined(_UNICODE)
	TCHAR *szMetaProto = mir_a2u(dat->szMetaProto);
	TCHAR *szProto = mir_a2u(dat->szProto);
#else
	TCHAR *szMetaProto = dat->szMetaProto;
	TCHAR *szProto = dat->szProto;
#endif
	mir_sntprintf(szFinalPath, MAX_PATH, _T("%s\\%s"), M->getSavedAvatarPath(), dat->bIsMeta ? szMetaProto : szProto);

#if defined(_UNICODE)
	mir_free(szProto);
	mir_free(szMetaProto);
#endif

	if (CreateDirectory(szFinalPath, 0) == 0) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			MessageBox(0, CTranslator::get(CTranslator::GEN_MSG_SAVE_NODIR),
					   CTranslator::get(CTranslator::GEN_MSG_SAVE), MB_OK | MB_ICONSTOP);
			return;
		}
	}
	if (isOwnPic)
		mir_sntprintf(szBaseName, MAX_PATH,_T("My Avatar_%s"), szTimestamp);
	else
		mir_sntprintf(szBaseName, MAX_PATH, _T("%s_%s"), dat->szNickname, szTimestamp);

	mir_sntprintf(szFinalFilename, MAX_PATH, _T("%s.png"), szBaseName);
	ofn.lpstrDefExt = _T("png");

	/*
	 * do not allow / or \ or % in the filename
	 */
	for (i = 0; i < lstrlen(forbiddenCharacters); i++) {
		for(j = 0; j < lstrlen(szFinalFilename); j++) {
			if(szFinalFilename[j] == forbiddenCharacters[i])
				szFinalFilename[j] = '_';
		}
	}

	ofn.lpstrFilter = _T("Image files\0*.bmp;*.png;*.jpg;*.gif\0\0");
	if (IsWinVer2000Plus()) {
		ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
		ofn.lpfnHook = (LPOFNHOOKPROC)OpenFileSubclass;
		ofn.lStructSize = sizeof(ofn);
	} else {
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.Flags = OFN_HIDEREADONLY;
	}
	ofn.hwndOwner = 0;
	ofn.lpstrFile = szFinalFilename;
	ofn.lpstrInitialDir = szFinalPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lCustData = (LPARAM) & setView;
	if (GetSaveFileName(&ofn)) {
		if (PathFileExists(szFinalFilename)) {
			if (MessageBox(0, CTranslator::get(CTranslator::GEN_MSG_SAVE_FILE_EXISTS),
						   CTranslator::get(CTranslator::GEN_MSG_SAVE), MB_YESNO | MB_ICONQUESTION) == IDNO)
				return;
		}
		IMGSRVC_INFO ii;
		ii.cbSize = sizeof(ii);
#if defined(_UNICODE)
		ii.wszName = szFinalFilename;
#else
		ii.szName = szFinalFilename;
#endif
		ii.hbm = hbm;
		ii.dwMask = IMGI_HBITMAP;
		ii.fif = FIF_UNKNOWN;			// get the format from the filename extension. png is default.
		CallService(MS_IMG_SAVE, (WPARAM)&ii, IMGL_TCHAR);
	}
}

/*
 * flash a tab icon if mode = true, otherwise restore default icon
 * store flashing state into bState
 */

void TSAPI FlashTab(struct _MessageWindowData *dat, HWND hwndTab, int iTabindex, BOOL *bState, BOOL mode, HICON origImage)
{
	TCITEM item;

	ZeroMemory((void *)&item, sizeof(item));
	item.mask = TCIF_IMAGE;

	if (mode)
		*bState = !(*bState);
	else
		dat->hTabIcon = origImage;
	item.iImage = 0;
	TabCtrl_SetItem(hwndTab, iTabindex, &item);
	if(dat->pContainer->dwFlags & CNT_SIDEBAR)
		dat->pContainer->SideBar->updateSession(dat);
}

/*
 * calculates avatar layouting, based on splitter position to find the optimal size
 * for the avatar w/o disturbing the toolbar too much.
 */

void TSAPI CalcDynamicAvatarSize(_MessageWindowData *dat, BITMAP *bminfo)
{
	RECT rc;
	double aspect = 0, newWidth = 0, picAspect = 0;
	double picProjectedWidth = 0;
	BOOL bBottomToolBar=dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR;
	BOOL bToolBar=dat->pContainer->dwFlags & CNT_HIDETOOLBAR?0:1;
	bool fInfoPanel = dat->Panel->isActive();

	if (PluginConfig.g_FlashAvatarAvail) {
		FLASHAVATAR fa = {0};
		fa.cProto = dat->szProto;
		fa.id = 25367;
		fa.hContact = fInfoPanel ? NULL : dat->hContact;
		CallService(MS_FAVATAR_GETINFO, (WPARAM)&fa, 0);
		if (fa.hWindow) {
			bminfo->bmHeight = FAVATAR_HEIGHT;
			bminfo->bmWidth = FAVATAR_WIDTH;
		}
	}
	GetClientRect(dat->hwnd, &rc);

	if (dat->dwFlags & MWF_WASBACKGROUNDCREATE || dat->pContainer->dwFlags & CNT_DEFERREDCONFIGURE || dat->pContainer->dwFlags & CNT_CREATE_MINIMIZED || IsIconic(dat->pContainer->hwnd))
		return;                 // at this stage, the layout is not yet ready...

	if (bminfo->bmWidth == 0 || bminfo->bmHeight == 0)
		picAspect = 1.0;
	else
		picAspect = (double)(bminfo->bmWidth / (double)bminfo->bmHeight);
	picProjectedWidth = (double)((dat->dynaSplitter-((bBottomToolBar && bToolBar)? DPISCALEX(24):0) + ((dat->showUIElements != 0) ? DPISCALEX(28) : DPISCALEX(2)))) * picAspect;

	//MaD: by changing dat->iButtonBarReallyNeeds to dat->iButtonBarNeeds
	//	   we can change resize priority to buttons, if needed...
	if (((rc.right) - (int)picProjectedWidth) > (dat->iButtonBarReallyNeeds) && !PluginConfig.m_AlwaysFullToolbarWidth && bToolBar)
		dat->iRealAvatarHeight = dat->dynaSplitter + 3 /*4 */ + (dat->showUIElements ? DPISCALEY(28):DPISCALEY(2));
	else
		dat->iRealAvatarHeight = dat->dynaSplitter + DPISCALEY(6);
	//mad
	dat->iRealAvatarHeight-=((bBottomToolBar&&bToolBar)? DPISCALEY(22):0);

	if (PluginConfig.m_LimitStaticAvatarHeight > 0)
		dat->iRealAvatarHeight = min(dat->iRealAvatarHeight, PluginConfig.m_LimitStaticAvatarHeight);

	if (M->GetByte(dat->hContact, "dontscaleavatars", M->GetByte("dontscaleavatars", 0)))
		dat->iRealAvatarHeight = min(bminfo->bmHeight, dat->iRealAvatarHeight);

	if (bminfo->bmHeight != 0)
		aspect = (double)dat->iRealAvatarHeight / (double)bminfo->bmHeight;
	else
		aspect = 1;

	newWidth = (double)bminfo->bmWidth * aspect;
	if (newWidth > (double)(rc.right) * 0.8)
		newWidth = (double)(rc.right) * 0.8;
	dat->pic.cy = dat->iRealAvatarHeight + 2;
	dat->pic.cx = (int)newWidth + 2;
}

/*
 + returns TRUE if the contact is handled by the MetaContacts protocol
 */

int TSAPI IsMetaContact(const _MessageWindowData *dat)
{
	if (dat->hContact == 0 || dat->szProto == NULL)
		return 0;

	return (PluginConfig.g_MetaContactsAvail && !strcmp(dat->szProto, PluginConfig.szMetaName));
}

/*
 * retrieve the current (active) protocol of a given metacontact. Usually, this is the
 * "most online" protocol, unless a specific protocol is forced
 */

char* TSAPI GetCurrentMetaContactProto(_MessageWindowData *dat)
{
	dat->hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)dat->hContact, 0);
	if (dat->hSubContact) {
		dat->szMetaProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)dat->hSubContact, 0);
		if (dat->szMetaProto)
			dat->wMetaStatus = DBGetContactSettingWord(dat->hSubContact, dat->szMetaProto, "Status", ID_STATUS_OFFLINE);
		else
			dat->wMetaStatus = ID_STATUS_OFFLINE;
		return dat->szMetaProto;
	} else
		return dat->szProto;
}

void TSAPI WriteStatsOnClose(_MessageWindowData *dat)
{
	DBEVENTINFO dbei;
	char buffer[450];
	HANDLE hNewEvent;
	time_t now = time(NULL);
	now = now - dat->stats.started;

	return;

	if (dat->hContact != 0 &&(PluginConfig.m_LogStatusChanges != 0) && dat->dwFlags & MWF_LOG_STATUSCHANGES) {
		mir_snprintf(buffer, sizeof(buffer), "Session close - active for: %d:%02d:%02d, Sent: %d (%d), Rcvd: %d (%d)", now / 3600, now / 60, now % 60, dat->stats.iSent, dat->stats.iSentBytes, dat->stats.iReceived, dat->stats.iReceivedBytes);
		dbei.cbSize = sizeof(dbei);
		dbei.pBlob = (PBYTE) buffer;
		dbei.cbBlob = (int)(strlen(buffer)) + 1;
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = DBEF_READ;
		dbei.timestamp = time(NULL);
		dbei.szModule = dat->szProto;
		hNewEvent = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) dat->hContact, (LPARAM) & dbei);
		if (dat->hDbEventFirst == NULL) {
			dat->hDbEventFirst = hNewEvent;
			SendMessage(dat->hwnd, DM_REMAKELOG, 0, 0);
		}
	}
}

int TSAPI MsgWindowUpdateMenu(_MessageWindowData *dat, HMENU submenu, int menuID)
{
	HWND	hwndDlg = dat->hwnd;
	bool 	fInfoPanel = dat->Panel->isActive();

	if (menuID == MENU_TABCONTEXT) {
		SESSION_INFO *si = (SESSION_INFO *)dat->si;
		int iTabs = TabCtrl_GetItemCount(GetParent(hwndDlg));

		EnableMenuItem(submenu, ID_TABMENU_ATTACHTOCONTAINER, M->GetByte("useclistgroups", 0) || M->GetByte("singlewinmode", 0) ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(submenu, ID_TABMENU_CLEARSAVEDTABPOSITION, (M->GetDword(dat->hContact, "tabindex", -1) != -1) ? MF_ENABLED : MF_GRAYED);
	} else if (menuID == MENU_PICMENU) {
		MENUITEMINFO mii = {0};
		TCHAR *szText = NULL;
		char  avOverride = (char)M->GetByte(dat->hContact, "hideavatar", -1);
		HMENU visMenu = GetSubMenu(submenu, 0);
		BOOL picValid = fInfoPanel ? (dat->hOwnPic != 0) : (dat->ace && dat->ace->hbmPic && dat->ace->hbmPic != PluginConfig.g_hbmUnknown);
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STRING;

		EnableMenuItem(submenu, ID_PICMENU_SAVETHISPICTUREAS, MF_BYCOMMAND | (picValid ? MF_ENABLED : MF_GRAYED));

		CheckMenuItem(visMenu, ID_VISIBILITY_DEFAULT, MF_BYCOMMAND | (avOverride == -1 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_HIDDENFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 0 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_VISIBLEFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 1 ? MF_CHECKED : MF_UNCHECKED));

		CheckMenuItem(submenu, ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH, MF_BYCOMMAND | (PluginConfig.m_AlwaysFullToolbarWidth ? MF_CHECKED : MF_UNCHECKED));
		if (!fInfoPanel) {
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | (ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED));
			szText = const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_AVATAR_SETTINGS));
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_ENABLED);
		} else {
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | ((ServiceExists(MS_AV_SETMYAVATAR) && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)(dat->bIsMeta ? dat->szMetaProto : dat->szProto), 0)) ? MF_ENABLED : MF_GRAYED));
			szText = const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_AVATAR_SETOWN));
		}
		mii.dwTypeData = szText;
		mii.cch = lstrlen(szText) + 1;
		SetMenuItemInfo(submenu, ID_PICMENU_SETTINGS, FALSE, &mii);
	} else if (menuID == MENU_PANELPICMENU) {
		HMENU visMenu = GetSubMenu(submenu, 0);
		char  avOverride = (char)M->GetByte(dat->hContact, "hideavatar", -1);

		CheckMenuItem(visMenu, ID_VISIBILITY_DEFAULT, MF_BYCOMMAND | (avOverride == -1 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_HIDDENFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 0 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_VISIBLEFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 1 ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | (ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(submenu, ID_PANELPICMENU_SAVETHISPICTUREAS, MF_BYCOMMAND | ((ServiceExists(MS_AV_SAVEBITMAP) && dat->ace && dat->ace->hbmPic && dat->ace->hbmPic != PluginConfig.g_hbmUnknown) ? MF_ENABLED : MF_GRAYED));
	}
	return 0;
}

int TSAPI MsgWindowMenuHandler(_MessageWindowData *dat, int selection, int menuId)
{
	if(dat == 0)
		return(0);

	HWND	hwndDlg = dat->hwnd;

	if (menuId == MENU_PICMENU || menuId == MENU_PANELPICMENU || menuId == MENU_TABCONTEXT) {
		switch (selection) {
			case ID_TABMENU_ATTACHTOCONTAINER:
				CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SELECTCONTAINER), hwndDlg, SelectContainerDlgProc, (LPARAM) hwndDlg);
				return 1;
			case ID_TABMENU_CONTAINEROPTIONS:
				if (dat->pContainer->hWndOptions == 0)
					CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), hwndDlg, DlgProcContainerOptions, (LPARAM) dat->pContainer);
				return 1;
			case ID_TABMENU_CLOSECONTAINER:
				SendMessage(dat->pContainer->hwnd, WM_CLOSE, 0, 0);
				return 1;
			case ID_TABMENU_CLOSETAB:
				SendMessage(hwndDlg, WM_CLOSE, 1, 0);
				return 1;
			case ID_TABMENU_SAVETABPOSITION:
				M->WriteDword(dat->hContact, SRMSGMOD_T, "tabindex", dat->iTabID * 100);
				break;
			case ID_TABMENU_CLEARSAVEDTABPOSITION:
				DBDeleteContactSetting(dat->hContact, SRMSGMOD_T, "tabindex");
				break;
			case ID_TABMENU_LEAVECHATROOM: {
				if (dat && dat->bType == SESSIONTYPE_CHAT) {
					SESSION_INFO *si = (SESSION_INFO *)dat->si;
					if ( (si != NULL) && (dat->hContact != NULL) ) {
						char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) dat->hContact, 0);
						if ( szProto )
							CallProtoService( szProto, PS_LEAVECHAT, (WPARAM)dat->hContact, 0 );
					}
				}
				return 1;
			}
			case ID_VISIBILITY_DEFAULT:
			case ID_VISIBILITY_HIDDENFORTHISCONTACT:
			case ID_VISIBILITY_VISIBLEFORTHISCONTACT: {
				BYTE avOverrideMode;

				if (selection == ID_VISIBILITY_DEFAULT)
					avOverrideMode = -1;
				else if (selection == ID_VISIBILITY_HIDDENFORTHISCONTACT)
					avOverrideMode = 0;
				else if (selection == ID_VISIBILITY_VISIBLEFORTHISCONTACT)
					avOverrideMode = 1;
				M->WriteByte(dat->hContact, SRMSGMOD_T, "hideavatar", avOverrideMode);
				dat->panelWidth = -1;
				ShowPicture(dat, FALSE);
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
				DM_ScrollToBottom(dat, 0, 1);
				return 1;
			}
			case ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH:
				PluginConfig.m_AlwaysFullToolbarWidth = !PluginConfig.m_AlwaysFullToolbarWidth;
				M->WriteByte(SRMSGMOD_T, "alwaysfulltoolbar", (BYTE)PluginConfig.m_AlwaysFullToolbarWidth);
				M->BroadcastMessage(DM_CONFIGURETOOLBAR, 0, 1);
				break;
			case ID_PICMENU_SAVETHISPICTUREAS:
				if (dat->Panel->isActive()) {
					if (dat)
						SaveAvatarToFile(dat, dat->hOwnPic, 1);
				} else {
					if (dat && dat->ace)
						SaveAvatarToFile(dat, dat->ace->hbmPic, 0);
				}
				break;
			case ID_PANELPICMENU_SAVETHISPICTUREAS:
				if (dat && dat->ace)
					SaveAvatarToFile(dat, dat->ace->hbmPic, 0);
				break;
			case ID_PICMENU_SETTINGS: {
				if (menuId == MENU_PANELPICMENU)
					CallService(MS_AV_CONTACTOPTIONS, (WPARAM)dat->hContact, 0);
				else if (menuId == MENU_PICMENU) {
					if (dat->Panel->isActive()) {
						if (ServiceExists(MS_AV_SETMYAVATAR) && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)(dat->bIsMeta ? dat->szMetaProto : dat->szProto), 0))
							CallService(MS_AV_SETMYAVATAR, (WPARAM)(dat->bIsMeta ? dat->szMetaProto : dat->szProto), 0);
					} else
						CallService(MS_AV_CONTACTOPTIONS, (WPARAM)dat->hContact, 0);
				}
			}
			return 1;
		}
	} else if (menuId == MENU_LOGMENU) {
		int iLocalTime = 0;
		int iRtl = (PluginConfig.m_RTLDefault == 0 ? M->GetByte(dat->hContact, "RTL", 0) : M->GetByte(dat->hContact, "RTL", 1));
		int iLogStatus = (PluginConfig.m_LogStatusChanges != 0) && dat->dwFlags & MWF_LOG_STATUSCHANGES;

		DWORD dwOldFlags = dat->dwFlags;

		switch (selection) {
			case ID_MESSAGELOGSETTINGS_GLOBAL: {
				OPENOPTIONSDIALOG	ood = {0};

				ood.cbSize = sizeof(OPENOPTIONSDIALOG);
				ood.pszGroup = NULL;
				ood.pszPage = "Message Sessions";
				ood.pszTab = NULL;
				M->WriteByte(SRMSGMOD_T, "opage", 3);			// force 3th tab to appear
				CallService (MS_OPT_OPENOPTIONS, 0, (LPARAM)&ood);
				return 1;
			}
			case ID_MESSAGELOGSETTINGS_FORTHISCONTACT:
				CallService(MS_TABMSG_SETUSERPREFS, (WPARAM)dat->hContact, (LPARAM)0);
				return 1;

			case ID_MESSAGELOG_EXPORTMESSAGELOGSETTINGS: {
				const TCHAR *szFilename = GetThemeFileName(1);
				if (szFilename != NULL)
					WriteThemeToINI(szFilename, dat);
				return 1;
			}
			case ID_MESSAGELOG_IMPORTMESSAGELOGSETTINGS: {
				const TCHAR *szFilename = GetThemeFileName(0);
				DWORD dwFlags = THEME_READ_FONTS;
				int   result;

				if (szFilename != NULL) {
					result = MessageBox(0, CTranslator::get(CTranslator::GEN_WARNING_LOADTEMPLATES),
										CTranslator::get(CTranslator::GEN_TITLE_LOADTHEME), MB_YESNOCANCEL);
					if (result == IDCANCEL)
						return 1;
					else if (result == IDYES)
						dwFlags |= THEME_READ_TEMPLATES;
					ReadThemeFromINI(szFilename, 0, 0, dwFlags);
					CacheLogFonts();
					CacheMsgLogIcons();
					M->BroadcastMessage(DM_OPTIONSAPPLIED, 1, 0);
					M->BroadcastMessage(DM_FORCEDREMAKELOG, (WPARAM)hwndDlg, (LPARAM)(dat->dwFlags & MWF_LOG_ALL));
				}
				return 1;
			}
		}
	}
	return 0;
}

/*
 * update the status bar field which displays the number of characters in the input area
 * and various indicators (caps lock, num lock, insert mode).
 */

void TSAPI UpdateReadChars(const _MessageWindowData *dat)
{

	if(dat && (dat->pContainer->hwndStatus && dat->pContainer->hwndActive == dat->hwnd)) {
		TCHAR 	buf[128];
		int 	len;
		TCHAR 	szIndicators[20];
		BOOL 	fCaps, fNum;

		szIndicators[0] = 0;
		if (dat->bType == SESSIONTYPE_CHAT)
			len = GetWindowTextLength(GetDlgItem(dat->hwnd, IDC_CHAT_MESSAGE));
		else {
#if defined(_UNICODE)
			/*
			 * retrieve text length in UTF8 bytes, because this is the relevant length for most protocols
			 */
			GETTEXTLENGTHEX gtxl = {0};
			gtxl.codepage = CP_UTF8;
			gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMBYTES;

			len = SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETTEXTLENGTHEX, (WPARAM) & gtxl, 0);
#else
			len = GetWindowTextLength(GetDlgItem(dat->hwnd, IDC_MESSAGE));
#endif
		}

		fCaps = (GetKeyState(VK_CAPITAL) & 1);
		fNum = (GetKeyState(VK_NUMLOCK) & 1);

		if (dat->fInsertMode)
			lstrcat(szIndicators, _T("O"));
		if (fCaps)
			lstrcat(szIndicators, _T("C"));
		if (fNum)
			lstrcat(szIndicators, _T("N"));
		if (dat->fInsertMode || fCaps || fNum)
			lstrcat(szIndicators, _T(" | "));

		mir_sntprintf(buf, safe_sizeof(buf), _T("%s%s %d/%d"), szIndicators, dat->lcID, dat->iOpenJobs, len);
		SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 1, (LPARAM) buf);
	}
}

/*
 * update all status bar fields and force a redraw of the status bar.
 */

void TSAPI UpdateStatusBar(const _MessageWindowData *dat)
{
	if (dat && dat->pContainer->hwndStatus && dat->pContainer->hwndActive == dat->hwnd) {
		if (dat->bType == SESSIONTYPE_IM) {
			if(dat->szStatusBar[0]) {
				SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[5]);
				SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)dat->szStatusBar);
			}
			else {
				SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, 0);
				DM_UpdateLastMessage(dat);
			}
		}
		else
			SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, 0);
		UpdateReadChars(dat);
		InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
		SendMessage(dat->pContainer->hwndStatus, WM_USER + 101, 0, (LPARAM)dat);
	}
}

/*
 * provide user feedback via icons on tabs. Used to indicate "send in progress" or
 * any error state.
 *
 * NOT used for typing notification feedback as this is handled directly from the
 * MTN handler.
 */

void TSAPI HandleIconFeedback(_MessageWindowData *dat, HICON iIcon)
{
	TCITEM item = {0};
	HICON iOldIcon = dat->hTabIcon;

	if (iIcon == (HICON) - 1) { // restore status image
		if (dat->dwFlags & MWF_ERRORSTATE) {
			dat->hTabIcon = PluginConfig.g_iconErr;
		} else {
			dat->hTabIcon = dat->hTabStatusIcon;
		}
	} else
		dat->hTabIcon = iIcon;
	item.iImage = 0;
	item.mask = TCIF_IMAGE;
	if(dat->pContainer->dwFlags & CNT_SIDEBAR)
		dat->pContainer->SideBar->updateSession(dat);
	else
		TabCtrl_SetItem(GetDlgItem(dat->pContainer->hwnd, IDC_MSGTABS), dat->iTabID, &item);
}

/*
 * retrieve the visiblity of the avatar window, depending on the global setting
 * and local mode
 */

int TSAPI GetAvatarVisibility(HWND hwndDlg, struct _MessageWindowData *dat)
{
	BYTE bAvatarMode = PluginConfig.m_AvatarMode;
	BYTE bOwnAvatarMode = PluginConfig.m_OwnAvatarMode;
	char hideOverride = (char)M->GetByte(dat->hContact, "hideavatar", -1);
	// infopanel visible, consider own avatar display

	dat->showPic = 0;

	if (dat->Panel->isActive() && bAvatarMode != 5) {
		if (bOwnAvatarMode)
			dat->showPic = FALSE;
		else {
			FLASHAVATAR fa = {0};
			if (PluginConfig.g_FlashAvatarAvail) {
				fa.cProto = dat->szProto;
				fa.id = 25367;
				fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);

				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				if (fa.hWindow != NULL&&dat->hwndContactPic) {
					DestroyWindow(dat->hwndContactPic);
					dat->hwndContactPic = NULL;
				}
				dat->showPic = ((dat->hOwnPic && dat->hOwnPic != PluginConfig.g_hbmUnknown) || (fa.hWindow != NULL)) ? 1 : 0;
			} else
				dat->showPic = (dat->hOwnPic && dat->hOwnPic != PluginConfig.g_hbmUnknown) ? 1 : 0;

			if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == NULL && !dat->hwndContactPic)
				dat->hwndContactPic =CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);

		}
		switch (bAvatarMode) {
			case 0:
			case 1:
				dat->showInfoPic = 1;
				break;
			case 4:
				dat->showInfoPic = 0;
				break;
			case 3: {
				FLASHAVATAR fa = {0};
				HBITMAP hbm = ((dat->ace && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) ? dat->ace->hbmPic : 0);

				if (PluginConfig.g_FlashAvatarAvail) {
					fa.cProto = dat->szProto;
					fa.id = 25367;
					fa.hContact = dat->hContact;
					fa.hParentWindow = dat->hwndPanelPicParent;

					CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
					if (fa.hWindow != NULL && dat->hwndPanelPic) {
						DestroyWindow(dat->hwndPanelPic);
						dat->hwndPanelPic = NULL;
						ShowWindow(dat->hwndPanelPicParent, SW_SHOW);
						EnableWindow(dat->hwndPanelPicParent, TRUE);
					}
				}
				if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == NULL && !dat->hwndPanelPic) {
					dat->hwndPanelPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, dat->hwndPanelPicParent, (HMENU)7000, NULL, NULL);
					if(dat->hwndPanelPic)
						SendMessage(dat->hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, TRUE);
				}
				if ((hbm && hbm != PluginConfig.g_hbmUnknown) || (fa.hWindow != NULL))
					dat->showInfoPic = 1;
				else
					dat->showInfoPic = 0;
				break;
			}
		}
		if (dat->showInfoPic)
			dat->showInfoPic = hideOverride == 0 ? 0 : dat->showInfoPic;
		else
			dat->showInfoPic = hideOverride == 1 ? 1 : dat->showInfoPic;
		//Bolshevik: reloads avatars
		if (dat->showInfoPic) {
			/** panel and contact is shown, reloads contact's avatar -> panel
			*  user avatar -> bottom picture
			*/
			SendMessage(dat->hwndPanelPic, AVATAR_SETCONTACT, (WPARAM)0, (LPARAM)dat->hContact);
			if (dat->hwndContactPic)
				SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, (WPARAM)0, (LPARAM)dat->szProto);
		}
		else {
			//show only user picture(contact is unaccessible)
			if (dat->hwndContactPic)
				SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, (WPARAM)0, (LPARAM)dat->szProto);
		}
//Bolshevik_
	} else {
		dat->showInfoPic = 0;

		switch (bAvatarMode) {
			case 0:             // globally on
				dat->showPic = 1;
				break;
			case 4:             // globally OFF
				dat->showPic = 0;
				break;
			case 5:
			case 3:             // on, if present
			case 2:
			case 1: {
				FLASHAVATAR fa = {0};
				HBITMAP hbm = (dat->ace && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) ? dat->ace->hbmPic : 0;

				if (PluginConfig.g_FlashAvatarAvail) {
					fa.cProto = dat->szProto;
					fa.id = 25367;
					fa.hContact = dat->hContact;
					fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);

					CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);

					if (fa.hWindow != NULL&&dat->hwndContactPic) {
						DestroyWindow(dat->hwndContactPic);
						dat->hwndContactPic = NULL;
					}
				}
				if (!PluginConfig.g_bDisableAniAvatars&&fa.hWindow == NULL&&!dat->hwndContactPic)
					dat->hwndContactPic =CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);

				if ((hbm && hbm != PluginConfig.g_hbmUnknown) || (fa.hWindow != NULL))
					dat->showPic = 1;
				else
					dat->showPic = 0;
				break;
			}
		}
		if (dat->showPic)
			dat->showPic = hideOverride == 0 ? 0 : dat->showPic;
		else
			dat->showPic = hideOverride == 1 ? 1 : dat->showPic;
		//Bolshevik: reloads avatars
		if (dat->showPic)
			//shows contact or user picture, depending on panel visibility
			if (dat->hwndPanelPic)
				SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, (WPARAM)0, (LPARAM)dat->szProto);
		if (dat->hwndContactPic)
			SendMessage(dat->hwndContactPic, AVATAR_SETCONTACT, (WPARAM)0, (LPARAM)dat->hContact);

	}
	return dat->showPic;
}

/*
 * checks, if there is a valid smileypack installed for the given protocol
 */

int TSAPI CheckValidSmileyPack(const char *szProto, HANDLE hContact, HICON *hButtonIcon)
{
	SMADD_INFO2 smainfo = {0};

	if (PluginConfig.g_SmileyAddAvail) {
		smainfo.cbSize = sizeof(smainfo);
		smainfo.Protocolname = const_cast<char *>(szProto);
		smainfo.hContact = hContact;
		CallService(MS_SMILEYADD_GETINFO2, 0, (LPARAM)&smainfo);
		if (hButtonIcon)
			*hButtonIcon = smainfo.ButtonIcon;
		return smainfo.NumberOfVisibleSmileys;
	} else
		return 0;
}

/*
 * return value MUST be free()'d by caller.
 */

TCHAR* TSAPI QuoteText(const TCHAR *text, int charsPerLine, int removeExistingQuotes)
{
	int inChar, outChar, lineChar;
	int justDoneLineBreak, bufSize;
	TCHAR *strout;

#ifdef _UNICODE
	bufSize = lstrlenW(text) + 23;
#else
	bufSize = strlen(text) + 23;
#endif
	strout = (TCHAR*)malloc(bufSize * sizeof(TCHAR));
	inChar = 0;
	justDoneLineBreak = 1;
	for (outChar = 0, lineChar = 0;text[inChar];) {
		if (outChar >= bufSize - 8) {
			bufSize += 20;
			strout = (TCHAR *)realloc(strout, bufSize * sizeof(TCHAR));
		}
		if (justDoneLineBreak && text[inChar] != '\r' && text[inChar] != '\n') {
			if (removeExistingQuotes)
				if (text[inChar] == '>') {
					while (text[++inChar] != '\n');
					inChar++;
					continue;
				}
			strout[outChar++] = '>';
			strout[outChar++] = ' ';
			lineChar = 2;
		}
		if (lineChar == charsPerLine && text[inChar] != '\r' && text[inChar] != '\n') {
			int decreasedBy;
			for (decreasedBy = 0;lineChar > 10;lineChar--, inChar--, outChar--, decreasedBy++)
				if (strout[outChar] == ' ' || strout[outChar] == '\t' || strout[outChar] == '-') break;
			if (lineChar <= 10) {
				lineChar += decreasedBy;
				inChar += decreasedBy;
				outChar += decreasedBy;
			} else inChar++;
			strout[outChar++] = '\r';
			strout[outChar++] = '\n';
			justDoneLineBreak = 1;
			continue;
		}
		strout[outChar++] = text[inChar];
		lineChar++;
		if (text[inChar] == '\n' || text[inChar] == '\r') {
			if (text[inChar] == '\r' && text[inChar+1] != '\n')
				strout[outChar++] = '\n';
			justDoneLineBreak = 1;
			lineChar = 0;
		} else justDoneLineBreak = 0;
		inChar++;
	}
	strout[outChar++] = '\r';
	strout[outChar++] = '\n';
	strout[outChar] = 0;
	return strout;
}


void TSAPI AdjustBottomAvatarDisplay(_MessageWindowData *dat)
{
	if(dat) {
		bool fInfoPanel = dat->Panel->isActive();
		HBITMAP hbm = (fInfoPanel && PluginConfig.m_AvatarMode != 5) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown);
		HWND	hwndDlg = dat->hwnd;

		if (PluginConfig.g_FlashAvatarAvail) {
			FLASHAVATAR fa = {0};

			fa.hContact = dat->hContact;
			fa.hWindow = 0;
			fa.id = 25367;
			fa.cProto = dat->szProto;
			CallService(MS_FAVATAR_GETINFO, (WPARAM)&fa, 0);
			if (fa.hWindow) {
				dat->hwndFlash = fa.hWindow;
				SetParent(dat->hwndFlash, fInfoPanel ? dat->hwndPanelPicParent : GetDlgItem(dat->hwnd, IDC_CONTACTPIC));
			}
			fa.hContact = 0;
			fa.hWindow = 0;
			if (fInfoPanel) {
				fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);
				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				if (fa.hWindow) {
					SetParent(fa.hWindow, GetDlgItem(hwndDlg, IDC_CONTACTPIC));
					ShowWindow(fa.hWindow, SW_SHOW);
				}
			} else {
				CallService(MS_FAVATAR_GETINFO, (WPARAM)&fa, 0);
				if (fa.hWindow)
					ShowWindow(fa.hWindow, SW_HIDE);
			}
		}

		if (hbm) {
			dat->showPic = GetAvatarVisibility(hwndDlg, dat);
			if (dat->dynaSplitter == 0 || dat->splitterY == 0)
				LoadSplitter(dat);
			dat->dynaSplitter = dat->splitterY - DPISCALEY(34);
			DM_RecalcPictureSize(dat);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CONTACTPIC), dat->showPic ? SW_SHOW : SW_HIDE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), NULL, TRUE);
		} else {
			dat->showPic = GetAvatarVisibility(hwndDlg, dat);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CONTACTPIC), dat->showPic ? SW_SHOW : SW_HIDE);
			dat->pic.cy = dat->pic.cx = DPISCALEY(60);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), NULL, TRUE);
		}
	}
}

void TSAPI ShowPicture(_MessageWindowData *dat, BOOL showNewPic)
{
	DBVARIANT 	dbv = {0};
	RECT 		rc;
	HWND		hwndDlg = dat->hwnd;

	if (!dat->Panel->isActive())
		dat->pic.cy = dat->pic.cx = DPISCALEY(60);

	if (showNewPic) {
		if (dat->Panel->isActive() && PluginConfig.m_AvatarMode != 5) {
			if (!dat->hwndPanelPic) {
				dat->panelWidth = -1;
				InvalidateRect(dat->hwnd, NULL, TRUE);
				UpdateWindow(dat->hwnd);
				SendMessage(dat->hwnd, WM_SIZE, 0, 0);
			}
			return;
		}
		AdjustBottomAvatarDisplay(dat);
	} else {
		dat->showPic = dat->showPic ? 0 : 1;
		DBWriteContactSettingByte(dat->hContact, SRMSGMOD_T, "MOD_ShowPic", (BYTE)dat->showPic);
	}
	GetWindowRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), &rc);
	if (dat->minEditBoxSize.cy + DPISCALEY(3)> dat->splitterY)
		SendMessage(hwndDlg, DM_SPLITTERMOVED, (WPARAM)rc.bottom - dat->minEditBoxSize.cy, (LPARAM)GetDlgItem(hwndDlg, IDC_SPLITTER));
	if (!showNewPic)
		SetDialogToType(hwndDlg);
	else
		SendMessage(hwndDlg, WM_SIZE, 0, 0);

	SendMessage(hwndDlg, DM_CALCMINHEIGHT, 0, 0);
	SendMessage(dat->pContainer->hwnd, DM_REPORTMINHEIGHT, (WPARAM) hwndDlg, (LPARAM) dat->uMinHeight);
}

void TSAPI FlashOnClist(HWND hwndDlg, struct _MessageWindowData *dat, HANDLE hEvent, DBEVENTINFO *dbei)
{
	CLISTEVENT cle;

	dat->dwTickLastEvent = GetTickCount();
	if ((GetForegroundWindow() != dat->pContainer->hwnd || dat->pContainer->hwndActive != hwndDlg) && !(dbei->flags & DBEF_SENT) && dbei->eventType == EVENTTYPE_MESSAGE) {
		UpdateTrayMenu(dat, (WORD)(dat->bIsMeta ? dat->wMetaStatus : dat->wStatus), dat->bIsMeta ? dat->szMetaProto : dat->szProto, dat->szStatus, dat->hContact, 0L);
		if (nen_options.bTraySupport == TRUE && PluginConfig.m_WinVerMajor >= 5)
			return;
	}
	if (hEvent == 0)
		return;

	if (!PluginConfig.m_FlashOnClist)
		return;
	if ((GetForegroundWindow() != dat->pContainer->hwnd || dat->pContainer->hwndActive != hwndDlg) && !(dbei->flags & DBEF_SENT) && dbei->eventType == EVENTTYPE_MESSAGE && !(dat->dwFlagsEx & MWF_SHOW_FLASHCLIST)) {
		ZeroMemory(&cle, sizeof(cle));
		cle.cbSize = sizeof(cle);
		cle.hContact = (HANDLE) dat->hContact;
		cle.hDbEvent = hEvent;
		cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = "SRMsg/ReadMessage";
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
		dat->dwFlagsEx |= MWF_SHOW_FLASHCLIST;
		dat->hFlashingEvent = hEvent;
	}
}

/*
 * callback function for text streaming
 */

static DWORD CALLBACK Message_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	static DWORD dwRead;
	char ** ppText = (char **) dwCookie;

	if (*ppText == NULL) {
		*ppText = (char *)malloc(cb + 2);
		CopyMemory(*ppText, pbBuff, cb);
		*pcb = cb;
		dwRead = cb;
		*(*ppText + cb) = '\0';
	} else {
		char  *p = (char *)realloc(*ppText, dwRead + cb + 2);
		CopyMemory(p + dwRead, pbBuff, cb);
		*ppText = p;
		*pcb = cb;
		dwRead += cb;
		*(*ppText + dwRead) = '\0';
	}
	return 0;
}

/*
 * retrieve contents of the richedit control by streaming. Used to get the
 * typed message before sending it.
 * caller must free the returned pointer.
 * UNICODE version returns UTF-8 encoded string.
 */

char* TSAPI Message_GetFromStream(HWND hwndRtf, const _MessageWindowData* dat, DWORD dwPassedFlags)
{
	EDITSTREAM stream;
	char *pszText = NULL;
	DWORD dwFlags = 0;

	if (hwndRtf == 0 || dat == 0)
		return NULL;

	ZeroMemory(&stream, sizeof(stream));
	stream.pfnCallback = Message_StreamCallback;
	stream.dwCookie = (DWORD_PTR) & pszText; // pass pointer to pointer
#if defined(_UNICODE)
	if (dwPassedFlags == 0)
		dwFlags = (CP_UTF8 << 16) | (SF_RTFNOOBJS | SFF_PLAINRTF | SF_USECODEPAGE);
	else
		dwFlags = (CP_UTF8 << 16) | dwPassedFlags;
#else
	if (dwPassedFlags == 0)
		dwFlags = SF_RTF | SF_RTFNOOBJS | SFF_PLAINRTF;
	else
		dwFlags = dwPassedFlags;
#endif
	SendMessage(hwndRtf, EM_STREAMOUT, (WPARAM)dwFlags, (LPARAM) & stream);

	return pszText; // pszText contains the text
}

static void CreateColorMap(TCHAR *Text)
{
	TCHAR * pszText = Text;
	TCHAR * p1;
	TCHAR * p2;
	TCHAR * pEnd;
	int iIndex = 1, i = 0;
	COLORREF default_color;

	static const TCHAR *lpszFmt = _T("\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];");
	TCHAR szRed[10], szGreen[10], szBlue[10];

	p1 = _tcsstr(pszText, _T("\\colortbl"));
	if (!p1)
		return;

	pEnd = _tcschr(p1, '}');

	p2 = _tcsstr(p1, _T("\\red"));

	for (i = 0; i < RTF_CTABLE_DEFSIZE; i++)
		rtf_ctable[i].index = 0;

	default_color = (COLORREF)M->GetDword(FONTMODULE, "Font16Col", 0);

	while (p2 && p2 < pEnd) {
		if (_stscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0) {
			int i;
			for (i = 0; i < RTF_CTABLE_DEFSIZE; i++) {
				if (rtf_ctable[i].clr == RGB(_ttoi(szRed), _ttoi(szGreen), _ttoi(szBlue)))
					rtf_ctable[i].index = iIndex;
			}
		}
		iIndex++;
		p1 = p2;
		p1 ++;

		p2 = _tcsstr(p1, _T("\\red"));
	}
	return ;
}

int RTFColorToIndex(int iCol)
{
	int i = 0;
	for (i = 0; i < RTF_CTABLE_DEFSIZE; i++) {
		if (rtf_ctable[i].index == iCol)
			return i + 1;
	}
	return 0;
}

/*
 * convert rich edit code to bbcode (if wanted). Otherwise, strip all RTF formatting
 * tags and return plain text
 */

BOOL TSAPI DoRtfToTags(TCHAR * pszText, const _MessageWindowData *dat)
{
	TCHAR * p1;
	BOOL bJustRemovedRTF = TRUE;
	BOOL bTextHasStarted = FALSE;
	LOGFONTA lf;
	COLORREF color;
	static int inColor = 0;

	if (!pszText)
		return FALSE;

	/*
	 * used to filter out attributes which are already set for the default message input area
	 * font
	 */

	lf = dat->theme.logFonts[MSGFONTID_MESSAGEAREA];
	color = dat->theme.fontColors[MSGFONTID_MESSAGEAREA];

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table

	CreateColorMap(pszText);
	// scan the file for rtf commands and remove or parse them
	inColor = 0;
	p1 = _tcsstr(pszText, _T("\\pard"));
	if (p1) {
		size_t iRemoveChars;
		TCHAR InsertThis[50];
		p1 += 5;

		MoveMemory(pszText, p1, (lstrlen(p1) + 1) * sizeof(TCHAR));
		p1 = pszText;
		// iterate through all characters, if rtf control character found then take action
		while (*p1 != (TCHAR) '\0') {
			_sntprintf(InsertThis, 50, _T(""));
			iRemoveChars = 0;

			switch (*p1) {
				case (TCHAR) '\\':
					if (p1 == _tcsstr(p1, _T("\\cf"))) { // foreground color
						TCHAR szTemp[20];
						int iCol = _ttoi(p1 + 3);
						int iInd = RTFColorToIndex(iCol);
						bJustRemovedRTF = TRUE;

						_sntprintf(szTemp, 20, _T("%d"), iCol);
						iRemoveChars = 3 + lstrlen(szTemp);
						if (bTextHasStarted || iCol)
							_sntprintf(InsertThis, sizeof(InsertThis) / sizeof(TCHAR), (iInd > 0) ? (inColor ? _T("[/color][color=%s]") : _T("[color=%s]")) : (inColor ? _T("[/color]") : _T("")), rtf_ctable[iInd - 1].szName);
						inColor = iInd > 0 ? 1 : 0;
					} else if (p1 == _tcsstr(p1, _T("\\highlight"))) { //background color
						TCHAR szTemp[20];
						int iCol = _ttoi(p1 + 10);
						//int iInd = RTFColorToIndex(pIndex, iCol, dat);
						bJustRemovedRTF = TRUE;

						_sntprintf(szTemp, 20, _T("%d"), iCol);
						iRemoveChars = 10 + lstrlen(szTemp);
						//if(bTextHasStarted || iInd >= 0)
						//	_snprintf(InsertThis, sizeof(InsertThis), ( iInd >= 0 ) ? _T("%%f%02u") : _T("%%F"), iInd);
					} else if (p1 == _tcsstr(p1, _T("\\par"))) { // newline
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = 4;
						//_sntprintf(InsertThis, sizeof(InsertThis), _T("\n"));
					} else if (p1 == _tcsstr(p1, _T("\\line"))) { // soft line break;
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = 5;
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("\n"));
					} else if (p1 == _tcsstr(p1, _T("\\emdash"))) {
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = 7;
#if defined(_UNICODE)
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("%c"), 0x2014);
#else
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("-"));
#endif
					} else if (p1 == _tcsstr(p1, _T("\\bullet"))) {
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = 7;
#if defined(_UNICODE)
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("%c"), 0x2022);
#else
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("*"));
#endif
					} else if (p1 == _tcsstr(p1, _T("\\ldblquote"))) {
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = 10;
#if defined(_UNICODE)
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("%c"), 0x201C);
#else
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("\""));
#endif
					} else if (p1 == _tcsstr(p1, _T("\\rdblquote"))) {
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = 10;
#if defined(_UNICODE)
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("%c"), 0x201D);
#else
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("\""));
#endif
					} else if (p1 == _tcsstr(p1, _T("\\b"))) { //bold
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = (p1[2] != (TCHAR) '0') ? 2 : 3;
						if (!(lf.lfWeight == FW_BOLD)) { // only allow bold if the font itself isn't a bold one, otherwise just strip it..
							if (dat->SendFormat)
								_sntprintf(InsertThis, safe_sizeof(InsertThis), (p1[2] != (TCHAR) '0') ? _T("[b]") : _T("[/b]"));
						}

					} else if (p1 == _tcsstr(p1, _T("\\i"))) { // italics
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = (p1[2] != (TCHAR) '0') ? 2 : 3;
						if (!lf.lfItalic) { // same as for bold
							if (dat->SendFormat)
								_sntprintf(InsertThis, safe_sizeof(InsertThis), (p1[2] != (TCHAR) '0') ? _T("[i]") : _T("[/i]"));
						}

					} else if (p1 == _tcsstr(p1, _T("\\strike"))) { // strike-out
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = (p1[7] != (TCHAR) '0') ? 7 : 8;
						if (!lf.lfStrikeOut) { // same as for bold
							if (dat->SendFormat)
								_sntprintf(InsertThis, safe_sizeof(InsertThis), (p1[7] != (TCHAR) '0') ? _T("[s]") : _T("[/s]"));
						}

					} else if (p1 == _tcsstr(p1, _T("\\ul"))) { // underlined
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						if (p1[3] == (TCHAR) 'n')
							iRemoveChars = 7;
						else if (p1[3] == (TCHAR) '0')
							iRemoveChars = 4;
						else
							iRemoveChars = 3;
						if (!lf.lfUnderline) { // same as for bold
							if (dat->SendFormat)
								_sntprintf(InsertThis, safe_sizeof(InsertThis), (p1[3] != (TCHAR) '0' && p1[3] != (TCHAR) 'n') ? _T("[u]") : _T("[/u]"));
						}

					} else if (p1 == _tcsstr(p1, _T("\\tab"))) { // tab
						bTextHasStarted = TRUE;
						bJustRemovedRTF = TRUE;
						iRemoveChars = 4;
#if defined(_UNICODE)
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("%c"), 0x09);
#else
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T(" "));
#endif
					} else if (p1[1] == (TCHAR) '\\' || p1[1] == (TCHAR) '{' || p1[1] == (TCHAR) '}') { // escaped characters
						bTextHasStarted = TRUE;
						//bJustRemovedRTF = TRUE;
						iRemoveChars = 2;
						_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("%c"), p1[1]);
					} else if (p1[1] == (TCHAR) '\'') { // special character
						bTextHasStarted = TRUE;
						bJustRemovedRTF = FALSE;
						if (p1[2] != (TCHAR) ' ' && p1[2] != (TCHAR) '\\') {
							int iLame = 0;
							TCHAR * p3;
							TCHAR *stoppedHere;

							if (p1[3] != (TCHAR) ' ' && p1[3] != (TCHAR) '\\') {
								_tcsncpy(InsertThis, p1 + 2, 3);
								iRemoveChars = 4;
								InsertThis[2] = 0;
							} else {
								_tcsncpy(InsertThis, p1 + 2, 2);
								iRemoveChars = 3;
								InsertThis[2] = 0;
							}
							// convert string containing char in hex format to int.
							p3 = InsertThis;
							iLame = _tcstol(p3, &stoppedHere, 16);
							_sntprintf(InsertThis, safe_sizeof(InsertThis), _T("%c"), (TCHAR) iLame);

						} else
							iRemoveChars = 2;
					} else { // remove unknown RTF command
						int j = 1;
						bJustRemovedRTF = TRUE;
						while (!_tcschr(_T(" !$%()#*\"'"), p1[j]) && p1[j] != (TCHAR) '�' && p1[j] != (TCHAR) '\\' && p1[j] != (TCHAR) '\0')
							//                    while(!_tcschr(_T(" !�$%&()#*"), p1[j]) && p1[j] != (TCHAR)'\\' && p1[j] != (TCHAR)'\0')
							j++;
						//					while(p1[j] != (TCHAR)' ' && p1[j] != (TCHAR)'\\' && p1[j] != (TCHAR)'\0')
						//						j++;
						iRemoveChars = j;
					}
					break;

				case (TCHAR) '{': // other RTF control characters
				case (TCHAR) '}':
					iRemoveChars = 1;
					break;

				case (TCHAR) ' ': // remove spaces following a RTF command
					if (bJustRemovedRTF)
						iRemoveChars = 1;
					bJustRemovedRTF = FALSE;
					bTextHasStarted = TRUE;
					break;

				default: // other text that should not be touched
					bTextHasStarted = TRUE;
					bJustRemovedRTF = FALSE;

					break;
			}

			// move the memory and paste in new commands instead of the old RTF
			if (lstrlen(InsertThis) || iRemoveChars) {
				MoveMemory(p1 + lstrlen(InsertThis), p1 + iRemoveChars, (lstrlen(p1) - iRemoveChars + 1) * sizeof(TCHAR));
				CopyMemory(p1, InsertThis, lstrlen(InsertThis) * sizeof(TCHAR));
				p1 += lstrlen(InsertThis);
			} else
				p1++;
		}

	} else {
		return FALSE;
	}
	return TRUE;
}

/*
 * trims the output from DoRtfToTags(), removes trailing newlines and whitespaces...
 */

void TSAPI DoTrimMessage(TCHAR *msg)
{
	size_t iLen = lstrlen(msg);
	size_t i = iLen;

	while (i && (msg[i-1] == '\r' || msg[i-1] == '\n') || msg[i-1] == ' ') {
		i--;
	}
	if (i < iLen)
		msg[i] = '\0';
}

/*
 * saves message to the input history.
 * its using streamout in UTF8 format - no unicode "issues" and all RTF formatting is saved aswell,
 * so restoring a message from the input history will also restore its formatting
 */

void TSAPI SaveInputHistory(HWND hwndDlg, _MessageWindowData *dat, WPARAM wParam, LPARAM lParam)
{
	int iLength = 0, iStreamLength = 0;
	int oldTop = 0;
	char *szFromStream = NULL;

	if (wParam) {
		oldTop = dat->iHistoryTop;
		dat->iHistoryTop = (int)wParam;
	}

	szFromStream = Message_GetFromStream(GetDlgItem(hwndDlg, IDC_MESSAGE), dat, (CP_UTF8 << 16) | (SF_RTFNOOBJS | SFF_PLAINRTF | SF_USECODEPAGE));

	iLength = iStreamLength = (lstrlenA(szFromStream) + 1);

	if (iLength > 0 && dat->history != NULL) {
		if ((dat->iHistoryTop == dat->iHistorySize) && oldTop == 0) {         // shift the stack down...
			struct InputHistory ihTemp = dat->history[0];
			dat->iHistoryTop--;
			MoveMemory((void *)&dat->history[0], (void *)&dat->history[1], (dat->iHistorySize - 1) * sizeof(struct InputHistory));
			dat->history[dat->iHistoryTop] = ihTemp;
		}
		if (iLength > dat->history[dat->iHistoryTop].lLen) {
			if (dat->history[dat->iHistoryTop].szText == NULL) {
				if (iLength < HISTORY_INITIAL_ALLOCSIZE)
					iLength = HISTORY_INITIAL_ALLOCSIZE;
				dat->history[dat->iHistoryTop].szText = (TCHAR *)malloc(iLength);
				dat->history[dat->iHistoryTop].lLen = iLength;
			} else {
				if (iLength > dat->history[dat->iHistoryTop].lLen) {
					dat->history[dat->iHistoryTop].szText = (TCHAR *)realloc(dat->history[dat->iHistoryTop].szText, iLength);
					dat->history[dat->iHistoryTop].lLen = iLength;
				}
			}
		}
		CopyMemory(dat->history[dat->iHistoryTop].szText, szFromStream, iStreamLength);
		if (!oldTop) {
			if (dat->iHistoryTop < dat->iHistorySize) {
				dat->iHistoryTop++;
				dat->iHistoryCurrent = dat->iHistoryTop;
			}
		}
	}
	if (szFromStream)
		free(szFromStream);

	if (oldTop)
		dat->iHistoryTop = oldTop;
}

/*
 * retrieve both buddys and my own UIN for a message session and store them in the message window *dat
 * respects metacontacts and uses the current protocol if the contact is a MC
 */

void TSAPI GetContactUIN(_MessageWindowData *dat)
{
	CONTACTINFO ci;

	ZeroMemory((void *)&ci, sizeof(ci));

	if (dat->bIsMeta) {
		ci.hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)dat->hContact, 0);
		ci.szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ci.hContact, 0);
	} else {
		ci.hContact = dat->hContact;
		ci.szProto = dat->szProto;
	}
	ci.cbSize = sizeof(ci);
	ci.dwFlag = CNF_DISPLAYUID | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		switch (ci.type) {
			case CNFT_ASCIIZ:
				mir_sntprintf(dat->uin, safe_sizeof(dat->uin), _T("%s"), reinterpret_cast<TCHAR *>(ci.pszVal));
				mir_free((void *)ci.pszVal);
				break;
			case CNFT_DWORD:
				mir_sntprintf(dat->uin, safe_sizeof(dat->uin), _T("%u"), ci.dVal);
				break;
			default:
				dat->uin[0] = 0;
				break;
		}
	} else
		dat->uin[0] = 0;

	// get own uin...
	ci.hContact = 0;

	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		switch (ci.type) {
			case CNFT_ASCIIZ:
				mir_sntprintf(dat->myUin, safe_sizeof(dat->myUin), _T("%s"), reinterpret_cast<TCHAR *>(ci.pszVal));
				mir_free((void *)ci.pszVal);
				break;
			case CNFT_DWORD:
				mir_sntprintf(dat->myUin, safe_sizeof(dat->myUin), _T("%u"), ci.dVal);
				break;
			default:
				dat->uin[0] = 0;
				break;
		}
	} else
		dat->myUin[0] = 0;
}

static int g_IEViewAvail = -1;
static int g_HPPAvail = -1;

UINT TSAPI GetIEViewMode(HWND hwndDlg, HANDLE hContact)
{
	int iWantIEView = 0, iWantHPP = 0;

	if (g_IEViewAvail == -1)
		g_IEViewAvail = ServiceExists(MS_IEVIEW_WINDOW);

	if (g_HPPAvail == -1)
		g_HPPAvail = ServiceExists("History++/ExtGrid/NewWindow");

	PluginConfig.g_WantIEView = g_IEViewAvail && M->GetByte("default_ieview", 0);
	PluginConfig.g_WantHPP = g_HPPAvail && M->GetByte("default_hpp", 0);

	iWantIEView = (PluginConfig.g_WantIEView) || (M->GetByte(hContact, "ieview", 0) == 1 && g_IEViewAvail);
	iWantIEView = (M->GetByte(hContact, "ieview", 0) == (BYTE) - 1) ? 0 : iWantIEView;

	iWantHPP = (PluginConfig.g_WantHPP) || (M->GetByte(hContact, "hpplog", 0) == 1 && g_HPPAvail);
	iWantHPP = (M->GetByte(hContact, "hpplog", 0) == (BYTE) - 1) ? 0 : iWantHPP;

	return iWantHPP ? WANT_HPP_LOG : (iWantIEView ? WANT_IEVIEW_LOG : 0);
}

static void CheckAndDestroyHPP(struct _MessageWindowData *dat)
{
	if (dat->hwndHPP) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = dat->hwndHPP;
		//MAD: restore old wndProc
		if(dat->oldIEViewProc){
			SetWindowLongPtr(dat->hwndHPP, GWLP_WNDPROC, (LONG_PTR)dat->oldIEViewProc);
			dat->oldIEViewProc=0;
			}
		//MAD_
		CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndHPP = 0;
	}
}

void TSAPI CheckAndDestroyIEView(_MessageWindowData *dat)
{
	if (dat->hwndIEView) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = dat->hwndIEView;
		if (dat->oldIEViewProc){
			SetWindowLongPtr(dat->hwndIEView, GWLP_WNDPROC, (LONG_PTR)dat->oldIEViewProc);
			dat->oldIEViewProc =0;
		}
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		dat->oldIEViewProc = 0;
		dat->hwndIEView = 0;
	}
}

void TSAPI SetMessageLog(_MessageWindowData *dat)
{
	HWND		 hwndDlg = dat->hwnd;
	unsigned int iLogMode = GetIEViewMode(hwndDlg, dat->hContact);

	if (iLogMode == WANT_IEVIEW_LOG && dat->hwndIEView == 0) {
		IEVIEWWINDOW ieWindow;

		ZeroMemory(&ieWindow, sizeof(ieWindow));
		CheckAndDestroyHPP(dat);
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwFlags = 0;
		ieWindow.dwMode = IEWM_TABSRMM;
		ieWindow.parent = hwndDlg;
		ieWindow.x = 0;
		ieWindow.y = 0;
		ieWindow.cx = 200;
		ieWindow.cy = 200;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndIEView = ieWindow.hwnd;
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOG), SW_HIDE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOG), FALSE);
	} else if (iLogMode == WANT_HPP_LOG && dat->hwndHPP == 0) {
		IEVIEWWINDOW ieWindow;

		CheckAndDestroyIEView(dat);
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwFlags = 0;
		ieWindow.dwMode = IEWM_TABSRMM;
		ieWindow.parent = hwndDlg;
		ieWindow.x = 0;
		ieWindow.y = 0;
		ieWindow.cx = 10;
		ieWindow.cy = 10;
		CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndHPP = ieWindow.hwnd;
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOG), SW_HIDE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOG), FALSE);
	} else {
		if (iLogMode != WANT_IEVIEW_LOG)
			CheckAndDestroyIEView(dat);
		if (iLogMode != WANT_HPP_LOG)
			CheckAndDestroyHPP(dat);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOG), SW_SHOW);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOG), TRUE);
		dat->hwndIEView = 0;
		dat->hwndIWebBrowserControl = 0;
		dat->hwndHPP = 0;
	}
}

void TSAPI SwitchMessageLog(_MessageWindowData *dat, int iMode)
{
	HWND hwndDlg = dat->hwnd;

	if (iMode) {           // switch from rtf to IEview or hpp
		SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOG), FALSE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOG), SW_HIDE);
		SetMessageLog(dat);
	} else                    // switch from IEView or hpp to rtf
		SetMessageLog(dat);

	SetDialogToType(hwndDlg);
	SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);

	if (dat->hwndIEView) {
 		if (M->GetByte("subclassIEView", 0)&&dat->oldIEViewProc == 0) {
 			WNDPROC wndProc = (WNDPROC)SetWindowLongPtr(dat->hwndIEView, GWLP_WNDPROC, (LONG_PTR)IEViewSubclassProc);
 			dat->oldIEViewProc = wndProc;
 		}
	} else if (dat->hwndHPP) {
		if (dat->oldIEViewProc == 0) {
			WNDPROC wndProc = (WNDPROC)SetWindowLongPtr(dat->hwndHPP, GWLP_WNDPROC, (LONG_PTR)HPPKFSubclassProc);
			dat->oldIEViewProc = wndProc;
		}
	}
}

void TSAPI FindFirstEvent(_MessageWindowData *dat)
{
	int historyMode = (int)M->GetByte(dat->hContact, SRMSGMOD, SRMSGSET_LOADHISTORY, -1);
	if (historyMode == -1)
		historyMode = (int)M->GetByte(SRMSGMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);

	dat->hDbEventFirst = (HANDLE) CallService(MS_DB_EVENT_FINDFIRSTUNREAD, (WPARAM) dat->hContact, 0);
	switch (historyMode) {
		case LOADHISTORY_COUNT: {
			int i;
			HANDLE hPrevEvent;
			DBEVENTINFO dbei = { 0};
			dbei.cbSize = sizeof(dbei);
			//MAD: ability to load only current session's history
			if (dat->bActualHistory)
				i=dat->messageCount;
			else i = DBGetContactSettingWord(NULL, SRMSGMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT);
			//
			for (; i > 0; i--) {
				if (dat->hDbEventFirst == NULL)
					hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->hContact, 0);
				else
					hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) dat->hDbEventFirst, 0);
				if (hPrevEvent == NULL)
					break;
				dbei.cbBlob = 0;
				dat->hDbEventFirst = hPrevEvent;
				CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
				if (!DbEventIsShown(dat, &dbei))
					i++;
			}
			break;
		}
		case LOADHISTORY_TIME: {
			HANDLE hPrevEvent;
			DBEVENTINFO dbei = { 0};
			DWORD firstTime;

			dbei.cbSize = sizeof(dbei);
			if (dat->hDbEventFirst == NULL)
				dbei.timestamp = time(NULL);
			else
				CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
			firstTime = dbei.timestamp - 60 * DBGetContactSettingWord(NULL, SRMSGMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
			for (;;) {
				if (dat->hDbEventFirst == NULL)
					hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->hContact, 0);
				else
					hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) dat->hDbEventFirst, 0);
				if (hPrevEvent == NULL)
					break;
				dbei.cbBlob = 0;
				CallService(MS_DB_EVENT_GET, (WPARAM) hPrevEvent, (LPARAM) & dbei);
				if (dbei.timestamp < firstTime)
					break;
				dat->hDbEventFirst = hPrevEvent;
			}
			break;
		}
		default: {
			break;
		}
	}
}

void TSAPI SaveSplitter(_MessageWindowData *dat)
{
	/*
	 * group chats save their normal splitter position independently
	 */

	if(dat->bType == SESSIONTYPE_CHAT)
		return;

	if (dat->splitterY < DPISCALEY_S(MINSPLITTERY) || dat->splitterY < 0)
		dat->splitterY = DPISCALEY_S(MINSPLITTERY);

	if (dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE)
		M->WriteDword(dat->hContact, SRMSGMOD_T, "splitsplity", dat->splitterY);
	else {
		if(!(dat->pContainer->dwPrivateFlags & CNT_GLOBALSETTINGS))
			dat->pContainer->splitterPos = dat->splitterY;
		else
			M->WriteDword(SRMSGMOD_T, "splitsplity", dat->splitterY);
	}
}

void TSAPI LoadSplitter(_MessageWindowData *dat)
{
	if (!(dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE))
		if(dat->pContainer->dwPrivateFlags & CNT_GLOBALSETTINGS)
			dat->splitterY = (int)M->GetDword("splitsplity", (DWORD) 60);
		else
			dat->splitterY = dat->pContainer->splitterPos;
	else
		dat->splitterY = (int)M->GetDword(dat->hContact, "splitsplity", M->GetDword("splitsplity", (DWORD) 60));

	if (dat->splitterY < DPISCALEY_S(MINSPLITTERY) || dat->splitterY < 0)
		dat->splitterY = 150;
}

void TSAPI PlayIncomingSound(const ContainerWindowData *pContainer, HWND hwnd)
{
	int iPlay = 0;
	DWORD dwFlags = pContainer->dwFlags;

	if (nen_options.iNoSounds)
		return;

	if (dwFlags & CNT_NOSOUND)
		iPlay = FALSE;
	else if (dwFlags & CNT_SYNCSOUNDS) {
		iPlay = !MessageWindowOpened(0, (LPARAM)hwnd);
	} else
		iPlay = TRUE;

	if (iPlay) {
		if (GetForegroundWindow() == pContainer->hwnd && pContainer->hwndActive == hwnd)
			SkinPlaySound("RecvMsgActive");
		else
			SkinPlaySound("RecvMsgInactive");
	}
}

/*
 * reads send format and configures the toolbar buttons
 * if mode == 0, int only configures the buttons and does not change send format
 */

void TSAPI GetSendFormat(_MessageWindowData *dat, int mode)
{
	UINT controls[5] = {IDC_FONTBOLD, IDC_FONTITALIC, IDC_FONTUNDERLINE,IDC_FONTSTRIKEOUT, IDC_FONTFACE};
	int i;

	if (mode) {
		dat->SendFormat = M->GetDword(dat->hContact, "sendformat", PluginConfig.m_SendFormat);
		if (dat->SendFormat == -1)          // per contact override to disable it..
			dat->SendFormat = 0;
		else if (dat->SendFormat == 0)
			dat->SendFormat = PluginConfig.m_SendFormat ? 1 : 0;
	}
	for (i = 0; i < 5; i++) {
		EnableWindow(GetDlgItem(dat->hwnd, controls[i]), dat->SendFormat != 0 ? TRUE : FALSE);
		ShowWindow(GetDlgItem(dat->hwnd, controls[i]), (dat->SendFormat != 0 /*&& !(dat->pContainer->dwFlags & CNT_HIDETOOLBAR)*/ )? SW_SHOW : SW_HIDE);
	}
	return;
}

/*
 * get 2-digit locale id from current keyboard layout
 */

void TSAPI GetLocaleID(_MessageWindowData *dat, const TCHAR *szKLName)
{
	TCHAR szLI[20], *stopped = NULL;
	USHORT langID;
	WORD   wCtype2[3];
	_PARAFORMAT2 pf2;
	BOOL fLocaleNotSet;
	char szTest[4] = {(char)0xe4, (char)0xf6, (char)0xfc, 0 };

	LCTYPE infoType;

	if(PluginConfig.m_bIsVista)
		infoType = LOCALE_SISO639LANGNAME2;
	else
		infoType = LOCALE_SISO639LANGNAME;

	szLI[0] = 0;

	ZeroMemory(&pf2, sizeof(_PARAFORMAT2));
	langID = (USHORT)_tcstol(szKLName, &stopped, 16);
	dat->lcid = MAKELCID(langID, 0);
	GetLocaleInfo(dat->lcid, infoType , szLI, 10);
	fLocaleNotSet = (dat->lcID[0] == 0 && dat->lcID[1] == 0);
	_tcsupr(szLI);
	mir_sntprintf(dat->lcID, safe_sizeof(dat->lcID), szLI);
	GetStringTypeA(dat->lcid, CT_CTYPE2, szTest, 3, wCtype2);
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA;
	SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
	if (FindRTLLocale(dat) && fLocaleNotSet) {
		if (wCtype2[0] == C2_RIGHTTOLEFT || wCtype2[1] == C2_RIGHTTOLEFT || wCtype2[2] == C2_RIGHTTOLEFT) {
			ZeroMemory(&pf2, sizeof(pf2));
			pf2.dwMask = PFM_RTLPARA;
			pf2.cbSize = sizeof(pf2);
			pf2.wEffects = PFE_RTLPARA;
			SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		} else {
			ZeroMemory(&pf2, sizeof(pf2));
			pf2.dwMask = PFM_RTLPARA;
			pf2.cbSize = sizeof(pf2);
			pf2.wEffects = 0;
			SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		}
		SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_SETLANGOPTIONS, 0, (LPARAM) SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
	}
}

void TSAPI LoadContactAvatar(_MessageWindowData *dat)
{
	if (ServiceExists(MS_AV_GETAVATARBITMAP) && dat)
		dat->ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)dat->hContact, 0);
	else if (dat)
		dat->ace = NULL;

	if (dat && (!(dat->Panel->isActive()) || PluginConfig.m_AvatarMode == 5)) {
		BITMAP bm;
		dat->iRealAvatarHeight = 0;

		if (dat->ace && dat->ace->hbmPic) {
			AdjustBottomAvatarDisplay(dat);
			GetObject(dat->ace->hbmPic, sizeof(bm), &bm);
			CalcDynamicAvatarSize(dat, &bm);
			PostMessage(dat->hwnd, WM_SIZE, 0, 0);
		} else if (dat->ace == NULL) {
			AdjustBottomAvatarDisplay(dat);
			GetObject(PluginConfig.g_hbmUnknown, sizeof(bm), &bm);
			CalcDynamicAvatarSize(dat, &bm);
			PostMessage(dat->hwnd, WM_SIZE, 0, 0);
		}
	}
}

void TSAPI LoadOwnAvatar(_MessageWindowData *dat)
{
	AVATARCACHEENTRY *ace = NULL;

	if (ServiceExists(MS_AV_GETMYAVATAR))
		ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)(dat->bIsMeta ? dat->szMetaProto : dat->szProto));

	if (ace) {
		dat->hOwnPic = ace->hbmPic;
		dat->ownAce = ace;
	} else {
		dat->hOwnPic = PluginConfig.g_hbmUnknown;
		dat->ownAce = NULL;
	}
	if (dat->Panel->isActive() && PluginConfig.m_AvatarMode != 5) {
		BITMAP bm;

		dat->iRealAvatarHeight = 0;
		AdjustBottomAvatarDisplay(dat);
		GetObject(dat->hOwnPic, sizeof(bm), &bm);
		CalcDynamicAvatarSize(dat, &bm);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
}

void TSAPI LoadTimeZone(_MessageWindowData *dat)
{
	if(ServiceExists("CLN/GetTimeOffset")) {
		dat->timezone = 1;
		dat->timediff = (DWORD)CallService("CLN/GetTimeOffset", (WPARAM)dat->hContact, 0);
	}
	else {
		dat->timezone = (DWORD)(M->GetByte(dat->hContact, "UserInfo", "Timezone", M->GetByte(dat->hContact, dat->szProto, "Timezone", -1)));
		if (dat->timezone != -1) {
			DWORD contact_gmt_diff;
			contact_gmt_diff = dat->timezone > 128 ? 256 - dat->timezone : 0 - dat->timezone;
			dat->timediff = (int)PluginConfig.local_gmt_diff - (int)contact_gmt_diff * 60 * 60 / 2;
		}
	}
}

/*
 * paste contents of the clipboard into the message input area and send it immediately
 */

void TSAPI HandlePasteAndSend(const _MessageWindowData *dat)
{
	if (!PluginConfig.m_PasteAndSend) {
		SendMessage(dat->hwnd, DM_ACTIVATETOOLTIP, IDC_MESSAGE, (LPARAM)CTranslator::get(CTranslator::GEN_WARNING_PASTEANDSEND_DISABLED));
		return;                                     // feature disabled
	}

	SendMessage(GetDlgItem(dat->hwnd, IDC_MESSAGE), EM_PASTESPECIAL, CF_TEXT, 0);
	if (GetWindowTextLengthA(GetDlgItem(dat->hwnd, IDC_MESSAGE)) > 0)
		SendMessage(dat->hwnd, WM_COMMAND, IDOK, 0);
}

/*
 * draw various elements of the message window, like avatar(s), info panel fields
 * and the color formatting menu
 */

int TSAPI MsgWindowDrawHandler(WPARAM wParam, LPARAM lParam, HWND hwndDlg, _MessageWindowData *dat)
{
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;

	if (!dat)
		return 0;

	bool	fAero = M->isAero();

	if (dis->CtlType == ODT_MENU && dis->hwndItem == (HWND)GetSubMenu(PluginConfig.g_hMenuContext, 7)) {
		RECT rc = { 0 };
		HBRUSH old, col;
		COLORREF clr;
		switch (dis->itemID) {
			case ID_FONT_RED:
				clr = RGB(255, 0, 0);
				break;
			case ID_FONT_BLUE:
				clr = RGB(0, 0, 255);
				break;
			case ID_FONT_GREEN:
				clr = RGB(0, 255, 0);
				break;
			case ID_FONT_MAGENTA:
				clr = RGB(255, 0, 255);
				break;
			case ID_FONT_YELLOW:
				clr = RGB(255, 255, 0);
				break;
			case ID_FONT_WHITE:
				clr = RGB(255, 255, 255);
				break;
			case ID_FONT_DEFAULTCOLOR:
				clr = GetSysColor(COLOR_MENU);
				break;
			case ID_FONT_CYAN:
				clr = RGB(0, 255, 255);
				break;
			case ID_FONT_BLACK:
				clr = RGB(0, 0, 0);
				break;
			default:
				clr = 0;
		}
		col = (HBRUSH)CreateSolidBrush(clr);
		old = (HBRUSH)SelectObject(dis->hDC, col);
		rc.left = 2;
		rc.top = dis->rcItem.top - 5;
		rc.right = 15;
		rc.bottom = dis->rcItem.bottom + 4;
		Rectangle(dis->hDC, rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1);
		FillRect(dis->hDC, &rc, col);
		SelectObject(dis->hDC, old);
		DeleteObject(col);
		return TRUE;
	}
	/*
	 * parent window of the infopanel ACC control
	 */
	else if (dis->hwndItem == dat->hwndPanelPicParent) {
		RECT 	rc = dis->rcItem;

		if(!IsWindowEnabled(dat->hwndPanelPicParent) || !dat->Panel->isActive())
			return(TRUE);

		if(fAero) {
			HDC		hdc;
			HBITMAP hbm, hbmOld;
			LONG	cx = rc.right - rc.left;
			LONG	cy = rc.bottom - rc.top;

			rc.left -= 3; rc.right += 3;
			hdc = CreateCompatibleDC(dis->hDC);
			hbm = CSkin::CreateAeroCompatibleBitmap(rc, dis->hDC);
			hbmOld = (HBITMAP)SelectObject(hdc, hbm);

			if(CSkin::m_pCurrentAeroEffect == 0)
				FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			else {
				if(CSkin::m_pCurrentAeroEffect->m_finalAlpha == 0)
					CSkin::ApplyAeroEffect(hdc, &rc, CSkin::AERO_EFFECT_AREA_INFOPANEL, 0);
				else {
					FillRect(hdc, &rc, CSkin::m_BrushBack);
					CSkin::ApplyAeroEffect(hdc, &rc, CSkin::AERO_EFFECT_AREA_INFOPANEL, 0);
				}
			}
			BitBlt(dis->hDC, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);
			SelectObject(hdc, hbmOld);
			DeleteObject(hbm);
			DeleteDC(hdc);
		}
		else {
			rc.left -= 3; rc.right += 3;
			dat->Panel->renderBG(dis->hDC, rc, &SkinItems[ID_EXTBKINFOPANELBG], false);
		}
		if(CSkin::m_bAvatarBorderType == 1) {
			HRGN clipRgn = 0;

			if(dat->hwndPanelPic) {
				RECT	rcPic;
				GetClientRect(dat->hwndPanelPic, &rcPic);
				LONG ix = ((dis->rcItem.right - dis->rcItem.left) - rcPic.right) / 2 - 1;
				LONG iy = ((dis->rcItem.bottom - dis->rcItem.top) - rcPic.bottom) / 2 - 1;

				clipRgn = CreateRectRgn(ix, iy, ix + rcPic.right + 2, iy + rcPic.bottom + 2);
			}
			else
				clipRgn = CreateRectRgn(dis->rcItem.left, dis->rcItem.top, dis->rcItem.right,
										dis->rcItem.bottom);
			HBRUSH hbr = CreateSolidBrush(CSkin::m_avatarBorderClr);
			FrameRgn(dis->hDC, clipRgn, hbr, 1, 1);
			DeleteObject(hbr);
			DeleteObject(clipRgn);
		}
		return(TRUE);
	}
	else if ((dis->hwndItem == GetDlgItem(hwndDlg, IDC_CONTACTPIC) && (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown) && dat->showPic) || (dis->hwndItem == hwndDlg && dat->Panel->isActive() && (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown))) {
		HBRUSH hOldBrush;
		BITMAP bminfo;
		double dAspect = 0, dNewWidth = 0, dNewHeight = 0;
		DWORD iMaxHeight = 0, top, cx, cy;
		RECT rc, rcClient, rcFrame;
		HDC hdcDraw;
		HBITMAP hbmDraw, hbmOld;
		BOOL bPanelPic = dis->hwndItem == hwndDlg;
		DWORD aceFlags = 0;
		HPEN hPenBorder = 0, hPenOld = 0;
		HRGN clipRgn = 0;
		int  iRad = PluginConfig.m_WinVerMajor >= 5 ? 4 : 6;
		BOOL flashAvatar = FALSE;
		bool fInfoPanel = dat->Panel->isActive();
		COLORREF clr2 = 0;

		if (PluginConfig.g_FlashAvatarAvail && (!bPanelPic || (bPanelPic && dat->showInfoPic == 1))) {
			FLASHAVATAR fa = {0};

			fa.id = 25367;
			fa.cProto = dat->szProto;
			if (!bPanelPic && fInfoPanel) {
				fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);
				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONTACTPIC), fa.hWindow != 0);
			} else {
				fa.hContact = dat->hContact;
				fa.hParentWindow = fInfoPanel ? dat->hwndPanelPicParent : GetDlgItem(hwndDlg, IDC_CONTACTPIC);
				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				if (fInfoPanel) {
					if (fa.hWindow != NULL&&dat->hwndPanelPic) {
						DestroyWindow(dat->hwndPanelPic);
						dat->hwndPanelPic = NULL;
					}
					if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == 0 && dat->hwndPanelPic == 0) {
						dat->hwndPanelPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, dat->hwndPanelPicParent, (HMENU)7000, NULL, NULL);
						SendMessage(dat->hwndPanelPic, AVATAR_SETCONTACT, (WPARAM)0, (LPARAM)dat->hContact);
					}
				} else {
					if (fa.hWindow != NULL && dat->hwndContactPic) {
						DestroyWindow(dat->hwndContactPic);
						dat->hwndContactPic = NULL;
					}
					if(!PluginConfig.g_bDisableAniAvatars && fa.hWindow == 0 && dat->hwndContactPic == 0) {
						dat->hwndContactPic =CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);
						SendMessage(dat->hwndContactPic, AVATAR_SETCONTACT, (WPARAM)0, (LPARAM)dat->hContact);
					}
				}
				dat->hwndFlash = fa.hWindow;
			}
			if (fa.hWindow != 0) {
				bminfo.bmHeight = FAVATAR_HEIGHT;
				bminfo.bmWidth = FAVATAR_WIDTH;
				CallService(MS_FAVATAR_SETBKCOLOR, (WPARAM)&fa, (LPARAM)GetSysColor(COLOR_3DFACE));
				flashAvatar = TRUE;
			}
		}

		if (bPanelPic) {
			if (!flashAvatar) {
				GetObject(dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown, sizeof(bminfo), &bminfo);
			}
			if ((dat->ace && dat->showInfoPic && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) || dat->showInfoPic)
				aceFlags = dat->ace ? dat->ace->dwFlags : 0;
			else {
				if (dat->panelWidth) {
					dat->panelWidth = -1;
					if(!CSkin::m_skinEnabled)
						SendMessage(hwndDlg, WM_SIZE, 0, 0);
				}
				return TRUE;
			}
		} else  {
			if (!fInfoPanel || PluginConfig.m_AvatarMode == 5) {
				if (dat->ace)
					aceFlags = dat->ace->dwFlags;
			} else if (dat->ownAce)
				aceFlags = dat->ownAce->dwFlags;

			GetObject((fInfoPanel && PluginConfig.m_AvatarMode != 5) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown), sizeof(bminfo), &bminfo);
		}

		GetClientRect(hwndDlg, &rc);
		if(bPanelPic) {
			rcClient = dis->rcItem;
			cx = (rcClient.right - rcClient.left);
			cy = (rcClient.bottom - rcClient.top) + 1;
		} else {
			GetClientRect(dis->hwndItem, &rcClient);
			cx = rcClient.right;
			cy = rcClient.bottom;
		}

		if (cx < 5 || cy < 5)
			return TRUE;

		if (bPanelPic) {
			if (bminfo.bmHeight > bminfo.bmWidth) {
				if (bminfo.bmHeight > 0)
					dAspect = (double)(cy /*- 2*/) / (double)bminfo.bmHeight;
				else
					dAspect = 1.0;
				dNewWidth = (double)bminfo.bmWidth * dAspect;
				dNewHeight = cy;// - 2;
			} else {
				if (bminfo.bmWidth > 0)
					dAspect = (double)(cy /*- 2*/) / (double)bminfo.bmWidth;
				else
					dAspect = 1.0;
				dNewHeight = (double)bminfo.bmHeight * dAspect;
				dNewWidth = cy;// - 2;
			}
			if (dat->panelWidth == -1) {
				dat->panelWidth = (int)dNewWidth;
				return(0);
			}
		} else {

			if (bminfo.bmHeight > 0)
				dAspect = (double)dat->iRealAvatarHeight / (double)bminfo.bmHeight;
			else
				dAspect = 1.0;
			dNewWidth = (double)bminfo.bmWidth * dAspect;
			if (dNewWidth > (double)(rc.right) * 0.8)
				dNewWidth = (double)(rc.right) * 0.8;
			iMaxHeight = dat->iRealAvatarHeight;
		}

		if (flashAvatar) {
			SetWindowPos(dat->hwndPanelPicParent, HWND_TOP, rcClient.left, rcClient.top,
						 (int)dNewWidth, (int)dNewHeight, SWP_SHOWWINDOW | SWP_NOCOPYBITS);
			return TRUE;
		}

		hdcDraw = CreateCompatibleDC(dis->hDC);
		hbmDraw = CreateCompatibleBitmap(dis->hDC, cx, cy);
		hbmOld = (HBITMAP)SelectObject(hdcDraw, hbmDraw);

		bool	fAero = M->isAero();

		hOldBrush = (HBRUSH)SelectObject(hdcDraw, fAero ? (HBRUSH)GetStockObject(HOLLOW_BRUSH) : GetSysColorBrush(COLOR_3DFACE));
		rcFrame = rcClient;

		if (!bPanelPic) {
			top = (cy - dat->pic.cy) / 2;
			RECT rcEdge = {0, top, dat->pic.cx, top + dat->pic.cy};
			if (CSkin::m_skinEnabled)
				CSkin::SkinDrawBG(dis->hwndItem, dat->pContainer->hwnd, dat->pContainer, &rcEdge, hdcDraw);
			else {
				if(fAero && CSkin::m_pCurrentAeroEffect) {
					COLORREF clr = PluginConfig.m_tbBackgroundHigh ? PluginConfig.m_tbBackgroundHigh :
									(CSkin::m_pCurrentAeroEffect ? CSkin::m_pCurrentAeroEffect->m_clrToolbar : 0xf0f0f0);

					clr2 = PluginConfig.m_tbBackgroundLow ? PluginConfig.m_tbBackgroundLow :
							(CSkin::m_pCurrentAeroEffect ? CSkin::m_pCurrentAeroEffect->m_clrToolbar2 : CSkin::m_dwmColorRGB);

					HBRUSH br = CreateSolidBrush(clr);
					FillRect(hdcDraw, &rcFrame, br);
					DeleteObject(br);
					//	::DrawAlpha(hdcDraw, &rcFrame, 0xf0f0f0, 40, CSkin::m_dwmColorRGB, 0, 9, 31, 4, 0);
				}
				else
					FillRect(hdcDraw, &rcFrame, GetSysColorBrush(COLOR_3DFACE));
			}

			hPenBorder = CreatePen(PS_SOLID, 1, clr2 ? clr2 : CSkin::m_avatarBorderClr);
			hPenOld = (HPEN)SelectObject(hdcDraw, hPenBorder);

			if (CSkin::m_bAvatarBorderType == 1 || CSkin::m_bAvatarBorderType == 0)
				Rectangle(hdcDraw, rcEdge.left, rcEdge.top, rcEdge.right, rcEdge.bottom);
			else if (CSkin::m_bAvatarBorderType == 2) {
				clipRgn = CreateRoundRectRgn(rcEdge.left, rcEdge.top, rcEdge.right + 1, rcEdge.bottom + 1, iRad, iRad);
				SelectClipRgn(hdcDraw, clipRgn);
			}
		}
		if ((((fInfoPanel && PluginConfig.m_AvatarMode != 5) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown)) && dat->showPic) || bPanelPic) {
			HDC hdcMem = CreateCompatibleDC(dis->hDC);
			HBITMAP hbmAvatar = bPanelPic ? (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown) : ((fInfoPanel && PluginConfig.m_AvatarMode != 5) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown));
			HBITMAP hbmMem = 0;
			if (bPanelPic) {
				bool	bBorder = CSkin::m_bAvatarBorderType ? true : false;

				if(dat->hwndPanelPic && bBorder && CSkin::m_bAvatarBorderType != 1)
					bBorder = false;

				LONG 	height_off = 0;
				LONG	border_off = bBorder ? 1 : 0;

				ResizeBitmap rb;
				rb.size = sizeof(rb);
				rb.fit = RESIZEBITMAP_STRETCH;
				rb.max_height = (int)(dNewHeight - (bBorder ? 2 : 0));
				rb.max_width = (int)(dNewWidth - (bBorder ? 2 : 0));
				rb.hBmp = hbmAvatar;

				HBITMAP hbmNew = (HBITMAP)CallService("IMG/ResizeBitmap", (WPARAM)&rb, 0);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmNew);

				rcFrame.left = rcFrame.top = 0;
				rcFrame.right = (rcClient.right - rcClient.left);
				rcFrame.bottom = (rcClient.bottom - rcClient.top);

				rcFrame.left = rcFrame.right - (LONG)dNewWidth;
				rcFrame.bottom = (LONG)dNewHeight;

				height_off = ((cy) - (rb.max_height + (bBorder ? 2 : 0))) / 2;
				rcFrame.top += height_off;
				rcFrame.bottom += height_off;

				//_DebugTraceA("Panel picture metrics: %d, %d - %f, %f - %d, %d (%d)", cx, cy, dNewWidth, dNewHeight, rcFrame.left, rcFrame.top, dat->panelWidth);

				if(!dat->hwndPanelPic)
					OffsetRect(&rcClient, -2, 0);

				if(dat->hwndPanelPic == 0) {
					if (CSkin::m_bAvatarBorderType == 1)
						clipRgn = CreateRectRgn(rcClient.left + rcFrame.left, rcClient.top + rcFrame.top, rcClient.left + rcFrame.right,
												rcClient.top + rcFrame.bottom);
					else if (CSkin::m_bAvatarBorderType == 2) {
						clipRgn = CreateRoundRectRgn(rcClient.left + rcFrame.left, rcClient.top + rcFrame.top, rcClient.left + rcFrame.right + 1,
													 rcClient.top + rcFrame.bottom + 1, iRad, iRad);
						SelectClipRgn(dis->hDC, clipRgn);
					}
				}
				if(dat->hwndPanelPic) {
					SendMessage(dat->hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, fAero ? TRUE : FALSE);
					SetWindowPos(dat->hwndPanelPic, HWND_TOP, rcFrame.left + border_off, rcFrame.top + border_off,
								 rb.max_width, rb.max_height, SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOSENDCHANGING);
				}
				else {
					if(CMimAPI::m_MyAlphaBlend) {
						CMimAPI::m_MyAlphaBlend(dis->hDC, rcClient.left + rcFrame.left + border_off, rcClient.top + rcFrame.top + border_off,
												rb.max_width, rb.max_height, hdcMem, 0, 0,
												rb.max_width, rb.max_height, CSkin::m_default_bf);
					}
					else
						CSkin::MY_AlphaBlend(dis->hDC, rcClient.left + rcFrame.left + border_off, rcClient.top + rcFrame.top + border_off,
											 rb.max_width, rb.max_height, rb.max_width, rb.max_height, hdcMem);
				}
				SelectObject(hdcMem, hbmMem);
				//DeleteObject(hbmMem);
				DeleteDC(hdcMem);
				if(hbmNew != hbmAvatar)
					DeleteObject(hbmNew);
			} else {
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmAvatar);
				LONG xy_off = 1; //CSkin::m_bAvatarBorderType ? 1 : 0;
				LONG width_off = 0; //= CSkin::m_bAvatarBorderType ? 0 : 2;

				SetStretchBltMode(hdcDraw, HALFTONE);
				if (aceFlags & AVS_PREMULTIPLIED)
					CSkin::MY_AlphaBlend(hdcDraw, xy_off, top + xy_off, (int)dNewWidth + width_off,
										 iMaxHeight + width_off, bminfo.bmWidth, bminfo.bmHeight, hdcMem);
				else
					StretchBlt(hdcDraw, xy_off, top + xy_off, (int)dNewWidth + width_off,
							   iMaxHeight + width_off, hdcMem, 0, 0, bminfo.bmWidth, bminfo.bmHeight, SRCCOPY);
				SelectObject(hdcMem, hbmMem);
				DeleteObject(hbmMem);
				DeleteDC(hdcMem);
			}
			if (clipRgn) {
				HBRUSH hbr = CreateSolidBrush(CSkin::m_avatarBorderClr);
				if(bPanelPic)
					FrameRgn(dis->hDC, clipRgn, hbr, 1, 1);
				else
					FrameRgn(hdcDraw, clipRgn, hbr, 1, 1);
				DeleteObject(hbr);
				DeleteObject(clipRgn);
			}
		}
		SelectObject(hdcDraw, hPenOld);
		SelectObject(hdcDraw, hOldBrush);
		DeleteObject(hPenBorder);
		if(!bPanelPic)
			BitBlt(dis->hDC, 0, 0, cx, cy, hdcDraw, 0, 0, SRCCOPY);
		SelectObject(hdcDraw, hbmOld);
		DeleteObject(hbmDraw);
		DeleteDC(hdcDraw);
		return TRUE;
	} else if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_STATICTEXT) || dis->hwndItem == GetDlgItem(hwndDlg, IDC_LOGFROZENTEXT)) {
		TCHAR szWindowText[256];
		if (CSkin::m_skinEnabled) {
			SetTextColor(dis->hDC, CSkin::m_DefaultFontColor);
			CSkin::SkinDrawBG(dis->hwndItem, dat->pContainer->hwnd, dat->pContainer, &dis->rcItem, dis->hDC);
		} else {
			SetTextColor(dis->hDC, GetSysColor(COLOR_BTNTEXT));
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_3DFACE));
		}
		GetWindowText(dis->hwndItem, szWindowText, 255);
		szWindowText[255] = 0;
		SetBkMode(dis->hDC, TRANSPARENT);
		DrawText(dis->hDC, szWindowText, -1, &dis->rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP | DT_END_ELLIPSIS);
		return TRUE;
	} else if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_STATICERRORICON)) {
		if (CSkin::m_skinEnabled)
			CSkin::SkinDrawBG(dis->hwndItem, dat->pContainer->hwnd, dat->pContainer, &dis->rcItem, dis->hDC);
		else
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_3DFACE));
		DrawIconEx(dis->hDC, (dis->rcItem.right - dis->rcItem.left) / 2 - 8, (dis->rcItem.bottom - dis->rcItem.top) / 2 - 8,
				   PluginConfig.g_iconErr, 16, 16, 0, 0, DI_NORMAL);
		return TRUE;
	}
	return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
}

void TSAPI LoadThemeDefaults(_MessageWindowData *dat)
{
	int i;
	char szTemp[40];
	COLORREF colour;
	dat->theme.statbg = PluginConfig.crStatus;
	dat->theme.oldinbg = PluginConfig.crOldIncoming;
	dat->theme.oldoutbg = PluginConfig.crOldOutgoing;
	dat->theme.inbg = PluginConfig.crIncoming;
	dat->theme.outbg = PluginConfig.crOutgoing;
	dat->theme.bg = PluginConfig.crDefault;
	dat->theme.hgrid = M->GetDword(FONTMODULE, "hgrid", RGB(224, 224, 224));
	dat->theme.left_indent = M->GetDword("IndentAmount", 20) * 15;
	dat->theme.right_indent = M->GetDword("RightIndent", 20) * 15;
	dat->theme.inputbg = M->GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);

	for (i = 1; i <= 5; i++) {
		_snprintf(szTemp, 10, "cc%d", i);
		colour = M->GetDword(szTemp, RGB(224, 224, 224));
		if (colour == 0)
			colour = RGB(1, 1, 1);
		dat->theme.custom_colors[i - 1] = colour;
	}
	dat->theme.logFonts = logfonts;
	dat->theme.fontColors = fontcolors;
	dat->theme.rtfFonts = NULL;
}

void TSAPI LoadOverrideTheme(_MessageWindowData *dat)
{
	BOOL bReadTemplates = ((dat->pContainer->ltr_templates == NULL) || (dat->pContainer->rtl_templates == NULL) ||
						   (dat->pContainer->logFonts == NULL) || (dat->pContainer->fontColors == NULL));

	if (lstrlen(dat->pContainer->szAbsThemeFile) > 1) {
		if (PathFileExists(dat->pContainer->szAbsThemeFile)) {
			if (CheckThemeVersion(dat->pContainer->szAbsThemeFile) == 0) {
				LoadThemeDefaults(dat);
				return;
			}
			if (dat->pContainer->ltr_templates == NULL) {
				dat->pContainer->ltr_templates = (TemplateSet *)malloc(sizeof(TemplateSet));
				CopyMemory(dat->pContainer->ltr_templates, &LTR_Active, sizeof(TemplateSet));
			}
			if (dat->pContainer->rtl_templates == NULL) {
				dat->pContainer->rtl_templates = (TemplateSet *)malloc(sizeof(TemplateSet));
				CopyMemory(dat->pContainer->rtl_templates, &RTL_Active, sizeof(TemplateSet));
			}

			if (dat->pContainer->logFonts == NULL)
				dat->pContainer->logFonts = (LOGFONTA *)malloc(sizeof(LOGFONTA) * (MSGDLGFONTCOUNT + 2));
			if (dat->pContainer->fontColors == NULL)
				dat->pContainer->fontColors = (COLORREF *)malloc(sizeof(COLORREF) * (MSGDLGFONTCOUNT + 2));
			if (dat->pContainer->rtfFonts == NULL)
				dat->pContainer->rtfFonts = (char *)malloc((MSGDLGFONTCOUNT + 2) * RTFCACHELINESIZE);

			dat->ltr_templates = dat->pContainer->ltr_templates;
			dat->rtl_templates = dat->pContainer->rtl_templates;
			dat->theme.logFonts = dat->pContainer->logFonts;
			dat->theme.fontColors = dat->pContainer->fontColors;
			dat->theme.rtfFonts = dat->pContainer->rtfFonts;
			ReadThemeFromINI(dat->pContainer->szAbsThemeFile, dat, bReadTemplates ? 0 : 1, THEME_READ_ALL);
			dat->dwFlags = dat->theme.dwFlags;
			dat->theme.left_indent *= 15;
			dat->theme.right_indent *= 15;
			dat->dwFlags |= MWF_SHOW_PRIVATETHEME;
			return;
		}
	}
	LoadThemeDefaults(dat);
}

void TSAPI ConfigureSmileyButton(_MessageWindowData *dat)
{
	HICON hButtonIcon = 0;
	HWND  hwndDlg = dat->hwnd;
	int nrSmileys = 0;
  	int showToolbar = dat->pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;
	int iItemID = IDC_SMILEYBTN;

	if (PluginConfig.g_SmileyAddAvail) {

		nrSmileys = CheckValidSmileyPack(dat->bIsMeta ? dat->szMetaProto : dat->szProto, dat->bIsMeta ? dat->hSubContact : dat->hContact, &hButtonIcon);

		dat->doSmileys = 1;

		if (hButtonIcon == 0 || PluginConfig.m_SmileyButtonOverride) {
			dat->hSmileyIcon = 0;
			SendDlgItemMessage(hwndDlg, iItemID, BM_SETIMAGE, IMAGE_ICON, (LPARAM) PluginConfig.g_buttonBarIcons[11]);
			if (hButtonIcon)
				DestroyIcon(hButtonIcon);
		} else {
			SendDlgItemMessage(hwndDlg, iItemID, BM_SETIMAGE, IMAGE_ICON, (LPARAM) hButtonIcon);
			dat->hSmileyIcon = hButtonIcon;
		}
	}

	if (nrSmileys == 0 || dat->hContact == 0)
		dat->doSmileys = 0;

	ShowWindow(GetDlgItem(hwndDlg, iItemID), (dat->doSmileys && showToolbar) ? SW_SHOW : SW_HIDE);
	EnableWindow(GetDlgItem(hwndDlg, iItemID), dat->doSmileys ? TRUE : FALSE);
}

HICON TSAPI GetXStatusIcon(const _MessageWindowData *dat)
{
	char szServiceName[128];
	char *szProto = dat->bIsMeta ? dat->szMetaProto : dat->szProto;

	mir_snprintf(szServiceName, 128, "%s/GetXStatusIcon", szProto);

	if (ServiceExists(szServiceName) && dat->xStatus > 0 && dat->xStatus <= 32)
		return (HICON)(CallProtoService(szProto, "/GetXStatusIcon", dat->xStatus, 0));
	return 0;
}

LRESULT TSAPI GetSendButtonState(HWND hwnd)
{
	HWND hwndIDok=GetDlgItem(hwnd, IDOK);

	if(hwndIDok)
		return(SendMessage(hwndIDok, BUTTONSETASFLATBTN + 15, 0, 0));
	else
		return 0;
}

void TSAPI EnableSendButton(const _MessageWindowData *dat, int iMode)
{
	HWND hwndOK;
	SendMessage(GetDlgItem(dat->hwnd, IDOK), BUTTONSETASFLATBTN + 14, iMode, 0);
	SendMessage(GetDlgItem(dat->hwnd, IDC_PIC), BUTTONSETASFLATBTN + 14, dat->fEditNotesActive ? TRUE : !iMode, 0);

	hwndOK = GetDlgItem(GetParent(GetParent(dat->hwnd)), IDOK);

	if (IsWindow(hwndOK))
		SendMessage(hwndOK, BUTTONSETASFLATBTN + 14, iMode, 0);
}

void TSAPI SendNudge(const _MessageWindowData *dat)
{
	char *szProto = dat->bIsMeta ? dat->szMetaProto : dat->szProto;
	char szServiceName[128];
	HANDLE hContact = dat->bIsMeta ? dat->hSubContact : dat->hContact;

	mir_snprintf(szServiceName, 128, "%s/SendNudge", szProto);
	if (ServiceExists(szServiceName) && ServiceExists(MS_NUDGE_SEND))
		CallService(MS_NUDGE_SEND, (WPARAM)hContact, 0);
	else
		SendMessage(dat->hwnd, DM_ACTIVATETOOLTIP, IDC_MESSAGE,
					(LPARAM)CTranslator::get(CTranslator::GEN_WARNING_NUDGE_DISABLED));
}

void TSAPI GetClientIcon(_MessageWindowData *dat)
{
	DBVARIANT dbv = {0};

	if (dat->hClientIcon) {
		DestroyIcon(dat->hClientIcon);
		dat->hClientIcon = 0;
	}

	if (ServiceExists(MS_FP_GETCLIENTICON)) {
		if (!DBGetContactSettingString(dat->hContact, dat->szProto, "MirVer", &dbv)) {
			dat->hClientIcon = (HICON)CallService(MS_FP_GETCLIENTICON, (WPARAM)dbv.pszVal, 0);
			DBFreeVariant(&dbv);
		}
	}
}
void TSAPI GetMaxMessageLength(_MessageWindowData *dat)
{
	HANDLE hContact;
	char   *szProto;

	if (dat->bIsMeta)
		GetCurrentMetaContactProto(dat);

	hContact = dat->bIsMeta ? dat->hSubContact : dat->hContact;
	szProto = dat->bIsMeta ? dat->szMetaProto : dat->szProto;

	if (dat->szProto) {
		int nMax;

		nMax = CallProtoService(szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, (LPARAM)hContact);
		if (nMax) {
			if (M->GetByte("autosplit", 0))
				SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_EXLIMITTEXT, 0, 20000);
			else
				SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_EXLIMITTEXT, 0, (LPARAM)nMax);
			dat->nMax = nMax;
		} else {
			SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_EXLIMITTEXT, 0, (LPARAM)20000);
			dat->nMax = 20000;
		}
	}
}

#define STATUSMSG_XSTATUSNAME 1
#define STATUSMSG_CLIST 2
#define STATUSMSG_YIM 3
#define STATUSMSG_GG 4
#define STATUSMSG_XSTATUS 5

/*
 * read status message from the database. Standard clist status message has priority,
 * but if not available, custom messages like xStatus can be used.
 */

void TSAPI GetCachedStatusMsg(_MessageWindowData *dat)
{
	DBVARIANT dbv = {0};
	HANDLE hContact;
	BYTE bStatusMsgValid = 0;
	char *szProto = dat ? dat->szProto : NULL;

	if (dat == NULL)
		return;

	dat->statusMsg[0] = 0;

	hContact = dat->hContact;

	if (!M->GetTString(hContact, "CList", "StatusMsg", &dbv) && lstrlen(dbv.ptszVal) > 1)
		bStatusMsgValid = STATUSMSG_CLIST;
	else {
		if (szProto) {
			if (!M->GetTString(hContact, szProto, "YMsg", &dbv) && lstrlen(dbv.ptszVal) > 1)
				bStatusMsgValid = STATUSMSG_YIM;
			else if (!M->GetTString(hContact, szProto, "StatusDescr", &dbv) && lstrlen(dbv.ptszVal) > 1)
				bStatusMsgValid = STATUSMSG_GG;
			else if (!M->GetTString(hContact, szProto, "XStatusMsg", &dbv) && lstrlen(dbv.ptszVal) > 1)
				bStatusMsgValid = STATUSMSG_XSTATUS;
		}
	}
	if (bStatusMsgValid == 0) {     // no status msg, consider xstatus name (if available)
		if (!M->GetTString(hContact, szProto, "XStatusName", &dbv) && lstrlen(dbv.ptszVal) > 1) {
			bStatusMsgValid = STATUSMSG_XSTATUSNAME;
			mir_sntprintf(dat->statusMsg, safe_sizeof(dat->statusMsg), _T("%s"), dbv.ptszVal);
			mir_free(dbv.ptszVal);
		} else {
			BYTE bXStatus = M->GetByte(hContact, szProto, "XStatusId", 0);
			if (bXStatus > 0 && bXStatus <= 31) {
				mir_sntprintf(dat->statusMsg, safe_sizeof(dat->statusMsg), _T("%s"), xStatusDescr[bXStatus]);
				bStatusMsgValid = STATUSMSG_XSTATUSNAME;
			}
		}
	}
	if (bStatusMsgValid > STATUSMSG_XSTATUSNAME) {
		int j = 0, i, iLen;
		iLen = lstrlen(dbv.ptszVal);
		for (i = 0; i < iLen && j < 1024; i++) {
			if (dbv.ptszVal[i] == (TCHAR)0x0d)
				continue;
			dat->statusMsg[j] = dbv.ptszVal[i] == (TCHAR)0x0a ? (TCHAR)' ' : dbv.ptszVal[i];
			j++;
		}
		dat->statusMsg[j] = (TCHAR)0;
		mir_free(dbv.ptszVal);
	}
}

/*
 * path utilities (make various paths relative to *PROFILE* directory, not miranda directory.
 * taken and modified from core services
 */

void TSAPI GetMyNick(_MessageWindowData *dat)
{
	CONTACTINFO ci;

	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = dat->bIsMeta ? dat->szMetaProto : dat->szProto;
	ci.dwFlag = CNF_TCHAR | CNF_NICK;

	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (lstrlen(reinterpret_cast<TCHAR *>(ci.pszVal)) < 1 || !_tcscmp(reinterpret_cast<TCHAR *>(ci.pszVal),
																			  CTranslator::get(CTranslator::GEN_UNKNOWN_CONTACT))) {
				mir_sntprintf(dat->szNickname, safe_sizeof(dat->szNickname), _T("%s"), dat->uin);
				if (ci.pszVal) {
					mir_free(ci.pszVal);
					ci.pszVal = NULL;
				}
			} else {
				_tcsncpy(dat->szMyNickname, reinterpret_cast<TCHAR *>(ci.pszVal), 110);
				dat->szMyNickname[109] = 0;
				if (ci.pszVal) {
					mir_free(ci.pszVal);
					ci.pszVal = NULL;
				}
			}
		} else if (ci.type == CNFT_DWORD)
			_ltot(ci.dVal, dat->szMyNickname, 10);
		else
			_tcsncpy(dat->szMyNickname, _T("<undef>"), 110);                // that really should *never* happen
	} else
		_tcsncpy(dat->szMyNickname, _T("<undef>"), 110);                    // same here
	if (ci.pszVal)
		mir_free(ci.pszVal);
}

/*
 * returns != 0 when one of the installed keyboard layouts belongs to an rtl language
 * used to find out whether we need to configure the message input box for bidirectional mode
 */

int TSAPI FindRTLLocale(struct _MessageWindowData *dat)
{
	HKL layouts[20];
	int i, result = 0;
	LCID lcid;
	WORD wCtype2[5];

	if (dat->iHaveRTLLang == 0) {
		ZeroMemory(layouts, 20 * sizeof(HKL));
		GetKeyboardLayoutList(20, layouts);
		for (i = 0; i < 20 && layouts[i]; i++) {
			lcid = MAKELCID(LOWORD(layouts[i]), 0);
			GetStringTypeA(lcid, CT_CTYPE2, "���", 3, wCtype2);
			if (wCtype2[0] == C2_RIGHTTOLEFT || wCtype2[1] == C2_RIGHTTOLEFT || wCtype2[2] == C2_RIGHTTOLEFT)
				result = 1;
		}
		dat->iHaveRTLLang = (result ? 1 : -1);
	} else
		result = dat->iHaveRTLLang == 1 ? 1 : 0;

	return result;
}

HICON TSAPI MY_GetContactIcon(const _MessageWindowData *dat)
{
	if (dat->bIsMeta)
		return CopyIcon(LoadSkinnedProtoIcon(dat->szMetaProto, dat->wMetaStatus));

	if (ServiceExists(MS_CLIST_GETCONTACTICON) && ServiceExists(MS_CLIST_GETICONSIMAGELIST)) {
		HIMAGELIST iml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
		int index = CallService(MS_CLIST_GETCONTACTICON, (WPARAM)dat->hContact, 0);
		if (iml && index >= 0)
			return ImageList_GetIcon(iml, index, ILD_NORMAL);
	}
	return CopyIcon(LoadSkinnedProtoIcon(dat->szProto, dat->wStatus));
}

static void TSAPI MTH_updatePreview(const _MessageWindowData *dat)
{
	TMathWindowInfo mathWndInfo;
	HWND hwndEdit = GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_IM ? IDC_MESSAGE : IDC_CHAT_MESSAGE);
	int len = GetWindowTextLengthA(hwndEdit);
	RECT windRect;
	char * thestr = (char *)malloc(len + 5);

	GetWindowTextA(hwndEdit, thestr, len + 1);
	GetWindowRect(dat->pContainer->hwnd, &windRect);
	mathWndInfo.top = windRect.top;
	mathWndInfo.left = windRect.left;
	mathWndInfo.right = windRect.right;
	mathWndInfo.bottom = windRect.bottom;

	CallService(MTH_SETFORMULA, 0, (LPARAM) thestr);
	CallService(MTH_RESIZE, 0, (LPARAM) &mathWndInfo);
	free(thestr);
}

void TSAPI MTH_updateMathWindow(const _MessageWindowData *dat)
{
	WINDOWPLACEMENT cWinPlace;

	if (!PluginConfig.m_MathModAvail)
		return;

	MTH_updatePreview(dat);
	CallService(MTH_SHOW, 0, 0);
	cWinPlace.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(dat->pContainer->hwnd, &cWinPlace);
	return;
}

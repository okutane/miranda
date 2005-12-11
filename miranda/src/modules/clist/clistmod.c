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
#include "clc.h"

int AddMainMenuItem(WPARAM wParam, LPARAM lParam);
int AddContactMenuItem(WPARAM wParam, LPARAM lParam);
int ContactChangeGroup(WPARAM wParam, LPARAM lParam);
int InitCListEvents(void);
void UninitCListEvents(void);
int ContactSettingChanged(WPARAM wParam, LPARAM lParam);
int ContactAdded(WPARAM wParam, LPARAM lParam);
int ContactDeleted(WPARAM wParam, LPARAM lParam);
int GetContactDisplayName(WPARAM wParam, LPARAM lParam);
int InvalidateDisplayName(WPARAM wParam, LPARAM lParam);
int InitGroupServices(void);
int Docking_IsDocked(WPARAM wParam, LPARAM lParam);
int MenuProcessCommand(WPARAM wParam, LPARAM lParam);
void InitDisplayNameCache(void);
void FreeDisplayNameCache(void);
void InitTray(void);
void LoadCLUIModule();

HANDLE hContactDoubleClicked, hContactIconChangedEvent;
HIMAGELIST hCListImages;
BOOL(WINAPI * MySetProcessWorkingSetSize) (HANDLE, SIZE_T, SIZE_T);
extern BYTE nameOrder[];
static int statusModeList[] = { ID_STATUS_OFFLINE, ID_STATUS_ONLINE, ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED, ID_STATUS_DND, ID_STATUS_FREECHAT, ID_STATUS_INVISIBLE, ID_STATUS_ONTHEPHONE, ID_STATUS_OUTTOLUNCH };
static int skinIconStatusList[] = { SKINICON_STATUS_OFFLINE, SKINICON_STATUS_ONLINE, SKINICON_STATUS_AWAY, SKINICON_STATUS_NA, SKINICON_STATUS_OCCUPIED, SKINICON_STATUS_DND, SKINICON_STATUS_FREE4CHAT, SKINICON_STATUS_INVISIBLE, SKINICON_STATUS_ONTHEPHONE, SKINICON_STATUS_OUTTOLUNCH };
struct ProtoIconIndex
{
	char *szProto;
	int iIconBase;
}
static *protoIconIndex;
static int protoIconIndexCount;
static HANDLE hProtoAckHook;
static HANDLE hContactSettingChanged;

static int GetStatusModeDescription(WPARAM wParam, LPARAM lParam)
{
	static char szMode[64];
	char *descr;
	int noPrefixReqd = 0;
	switch (wParam) {
	case ID_STATUS_OFFLINE:
		descr = Translate("Offline");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_CONNECTING:
		descr = Translate("Connecting");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_ONLINE:
		descr = Translate("Online");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_AWAY:
		descr = Translate("Away");
		break;
	case ID_STATUS_DND:
		descr = Translate("DND");
		break;
	case ID_STATUS_NA:
		descr = Translate("NA");
		break;
	case ID_STATUS_OCCUPIED:
		descr = Translate("Occupied");
		break;
	case ID_STATUS_FREECHAT:
		descr = Translate("Free for chat");
		break;
	case ID_STATUS_INVISIBLE:
		descr = Translate("Invisible");
		break;
	case ID_STATUS_OUTTOLUNCH:
		descr = Translate("Out to lunch");
		break;
	case ID_STATUS_ONTHEPHONE:
		descr = Translate("On the phone");
		break;
	case ID_STATUS_IDLE:
		descr = Translate("Idle");
		break;
	default:
		if (wParam > ID_STATUS_CONNECTING && wParam < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES) {
			wsprintfA(szMode, Translate("Connecting (attempt %d)"), wParam - ID_STATUS_CONNECTING + 1);
			return (int) szMode;
		}
		return (int) (char *) NULL;
	}
	if (noPrefixReqd || !(lParam & GSMDF_PREFIXONLINE))
		return (int) descr;
	lstrcpyA(szMode, Translate("Online"));
	lstrcatA(szMode, ": ");
	lstrcatA(szMode, descr);
	return (int) szMode;
}

static int ProtocolAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *) lParam;

	if (ack->type != ACKTYPE_STATUS)
		return 0;
	CallService(MS_CLUI_PROTOCOLSTATUSCHANGED, ack->lParam, (LPARAM) ack->szModule);

	if ((int) ack->hProcess < ID_STATUS_ONLINE && ack->lParam >= ID_STATUS_ONLINE) {
		DWORD caps;
		caps = (DWORD) CallProtoService(ack->szModule, PS_GETCAPS, PFLAGNUM_1, 0);
		if (caps & PF1_SERVERCLIST) {
			HANDLE hContact;
			char *szProto;

			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
			while (hContact) {
				szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
				if (szProto != NULL && !strcmp(szProto, ack->szModule))
					if (DBGetContactSettingByte(hContact, "CList", "Delete", 0))
						CallService(MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);
				hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
			}
		}
	}

	cli.pfnTrayIconUpdateBase(ack->szModule);
	return 0;
}

int fnIconFromStatusMode(const char *szProto, int status)
{
	int index, i;

	for (index = 0; index < SIZEOF(statusModeList); index++)
		if (status == statusModeList[index])
			break;
	if (index == SIZEOF(statusModeList))
		index = 0;
	if (szProto == NULL)
		return index + 1;
	for (i = 0; i < protoIconIndexCount; i++) {
		if (strcmp(szProto, protoIconIndex[i].szProto))
			continue;
		return protoIconIndex[i].iIconBase + index;
	}
	return 1;
}

static int GetContactIcon(WPARAM wParam, LPARAM lParam)
{
	char *szProto;
	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	return cli.pfnIconFromStatusMode(szProto,
		szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord((HANDLE) wParam, szProto, "Status", ID_STATUS_OFFLINE));
}

static int ContactListShutdownProc(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact, hNext;

	//remove transitory contacts
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) {
		hNext = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
			CallService(MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);
		hContact = hNext;
	}
	ImageList_Destroy(hCListImages);
	UnhookEvent(hProtoAckHook);
	UninitCListEvents();
	free(protoIconIndex);
	DestroyHookableEvent(hContactDoubleClicked);
	UnhookEvent(hContactSettingChanged);
	return 0;
}

static int ContactListModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	int i, protoCount, j, iImg;
	PROTOCOLDESCRIPTOR **protoList;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM) & protoCount, (LPARAM) & protoList);
	protoIconIndexCount = 0;
	protoIconIndex = NULL;
	for (i = 0; i < protoCount; i++) {
		if (protoList[i]->type != PROTOTYPE_PROTOCOL)
			continue;
		protoIconIndex = (struct ProtoIconIndex *) realloc(protoIconIndex, sizeof(struct ProtoIconIndex) * (protoIconIndexCount + 1));
		protoIconIndex[protoIconIndexCount].szProto = protoList[i]->szName;
		for (j = 0; j < SIZEOF(statusModeList); j++) {
			iImg = ImageList_AddIcon(hCListImages, LoadSkinnedProtoIcon(protoList[i]->szName, statusModeList[j]));
			if (j == 0)
				protoIconIndex[protoIconIndexCount].iIconBase = iImg;
		}
		protoIconIndexCount++;
	}
	cli.pfnLoadContactTree();

	if ( !ServiceExists( MS_DB_CONTACT_GETSETTING_STR )) {
		MessageBox( NULL, TranslateT( "This plugin requires db3x plugin version 0.5.1.0 or later" ), _T("CList"), MB_OK );
		return 1;
	}

	LoadCLUIModule();
	return 0;
}

static int ContactDoubleClicked(WPARAM wParam, LPARAM lParam)
{
	// Check and an event from the CList queue for this hContact
	if (cli.pfnEventsProcessContactDoubleClick((HANDLE) wParam))
		NotifyEventHooks(hContactDoubleClicked, wParam, 0);

	return 0;
}

static BOOL CALLBACK AskForConfirmationDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hWnd);
		{
			LOGFONT lf;
			HFONT hFont;

			hFont = (HFONT) SendDlgItemMessage(hWnd, IDYES, WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			SendDlgItemMessage(hWnd, IDC_TOPLINE, WM_SETFONT, (WPARAM) CreateFontIndirect(&lf), 0);
		}
		{
			TCHAR szFormat[256];
			TCHAR szFinal[256];

			GetDlgItemText(hWnd, IDC_TOPLINE, szFormat, SIZEOF(szFormat));
			mir_sntprintf(szFinal, SIZEOF(szFinal), szFormat, cli.pfnGetContactDisplayName((HANDLE)lParam, 0));
			SetDlgItemText(hWnd, IDC_TOPLINE, szFinal);
		}
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return TRUE;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
		case IDYES:
			if (IsDlgButtonChecked(hWnd, IDC_HIDE)) {
				EndDialog(hWnd, IDC_HIDE);
				break;
			}
			//fall through
		case IDCANCEL:
		case IDNO:
			EndDialog(hWnd, LOWORD(wParam));
			break;
		}
		break;
	}

	case WM_CLOSE:
		SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDNO, BN_CLICKED), 0);
		break;

	case WM_DESTROY:
		DeleteObject((HFONT) SendDlgItemMessage(hWnd, IDC_TOPLINE, WM_GETFONT, 0, 0));
		break;
	}

	return FALSE;

}

static int MenuItem_DeleteContact(WPARAM wParam, LPARAM lParam)
{
	//see notes about deleting contacts on PF1_SERVERCLIST servers in m_protosvc.h
	int action;

	if (DBGetContactSettingByte(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT)) {
		// Ask user for confirmation, and if the contact should be archived (hidden, not deleted)
		action = DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DELETECONTACT), (HWND) lParam, AskForConfirmationDlgProc, wParam);
	}
	else {
		action = IDYES;
	}


	switch (action) {

	// Delete contact
	case IDYES:
		{
			char *szProto;

			szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
			if (szProto != NULL) {
				// Check if protocol uses server side lists
				DWORD caps;

				caps = (DWORD) CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				if (caps & PF1_SERVERCLIST) {
					int status;

					status = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
					if (status == ID_STATUS_OFFLINE || (status >= ID_STATUS_CONNECTING && status < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)) {
						// Set a flag so we remember to delete the contact when the protocol goes online the next time
						DBWriteContactSettingByte((HANDLE) wParam, "CList", "Delete", 1);
						MessageBox( NULL,
							TranslateT("This contact is on an instant messaging system which stores its contact list on a central server. The contact will be removed from the server and from your contact list when you next connect to that network."),
							TranslateT("Delete Contact"), MB_OK);
						return 0;
					}
				}
			}
			CallService(MS_DB_CONTACT_DELETE, wParam, 0);
			break;
		}

	// Archive contact
	case IDC_HIDE:
		DBWriteContactSettingByte((HANDLE) wParam, "CList", "Hidden", 1);
		break;
	}

	return 0;

}



static int MenuItem_AddContactToList(WPARAM wParam, LPARAM lParam)
{
	ADDCONTACTSTRUCT acs = { 0 };

	acs.handle = (HANDLE) wParam;
	acs.handleType = HANDLE_CONTACT;
	acs.szProto = "";

	CallService(MS_ADDCONTACT_SHOW, (WPARAM) NULL, (LPARAM) & acs);
	return 0;
}

static int GetIconsImageList(WPARAM wParam, LPARAM lParam)
{
	return (int) hCListImages;
}

static int ContactFilesDropped(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_FILE_SENDSPECIFICFILES, wParam, lParam);
	return 0;
}

static int CListIconsChanged(WPARAM wParam, LPARAM lParam)
{
	int i, j;

	for (i = 0; i < SIZEOF(statusModeList); i++)
		ImageList_ReplaceIcon(hCListImages, i + 1, LoadSkinnedIcon(skinIconStatusList[i]));
	ImageList_ReplaceIcon(hCListImages, IMAGE_GROUPOPEN, LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN));
	ImageList_ReplaceIcon(hCListImages, IMAGE_GROUPSHUT, LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT));
	for (i = 0; i < protoIconIndexCount; i++)
		for (j = 0; j < SIZEOF(statusModeList); j++)
			ImageList_ReplaceIcon(hCListImages, protoIconIndex[i].iIconBase + j, LoadSkinnedProtoIcon(protoIconIndex[i].szProto, statusModeList[j]));
	cli.pfnTrayIconIconsChanged();
	InvalidateRect((HWND) CallService(MS_CLUI_GETHWND, 0, 0), NULL, TRUE);
	return 0;
}

/*
Begin of Hrk's code for bug
*/
#define GWVS_HIDDEN 1
#define GWVS_VISIBLE 2
#define GWVS_COVERED 3
#define GWVS_PARTIALLY_COVERED 4

int GetWindowVisibleState(HWND hWnd, int iStepX, int iStepY)
{
	RECT rc = { 0 };
	POINT pt = { 0 };
	register int i = 0, j = 0, width = 0, height = 0, iCountedDots = 0, iNotCoveredDots = 0;
	BOOL bPartiallyCovered = FALSE;
	HWND hAux = 0;

	if (hWnd == NULL) {
		SetLastError(0x00000006);       //Wrong handle
		return -1;
	}
	//Some defaults now. The routine is designed for thin and tall windows.
	if (iStepX <= 0)
		iStepX = 4;
	if (iStepY <= 0)
		iStepY = 16;

	if (IsIconic(hWnd) || !IsWindowVisible(hWnd))
		return GWVS_HIDDEN;
	else {
		GetWindowRect(hWnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		for (i = rc.top; i < rc.bottom; i += (height / iStepY)) {
			pt.y = i;
			for (j = rc.left; j < rc.right; j += (width / iStepX)) {
				pt.x = j;
				hAux = WindowFromPoint(pt);
				while (GetParent(hAux) != NULL)
					hAux = GetParent(hAux);
				if (hAux != hWnd)       //There's another window!
					bPartiallyCovered = TRUE;
				else
					iNotCoveredDots++;  //Let's count the not covered dots.
				iCountedDots++; //Let's keep track of how many dots we checked.
			}
		}
		if (iNotCoveredDots == iCountedDots)    //Every dot was not covered: the window is visible.
			return GWVS_VISIBLE;
		else if (iNotCoveredDots == 0)  //They're all covered!
			return GWVS_COVERED;
		else                    //There are dots which are visible, but they are not as many as the ones we counted: it's partially covered.
			return GWVS_PARTIALLY_COVERED;
	}
}

int fnShowHide(WPARAM wParam, LPARAM lParam)
{
	HWND hwndContactList = (HWND) CallService(MS_CLUI_GETHWND, 0, 0);
	BOOL bShow = FALSE;

	int iVisibleState = GetWindowVisibleState(cli.hwndContactList, 0, 0);

	//bShow is FALSE when we enter the switch.
	switch (iVisibleState) {
	case GWVS_PARTIALLY_COVERED:
		//If we don't want to bring it to top, we can use a simple break. This goes against readability ;-) but the comment explains it.
		if (!DBGetContactSettingByte(NULL, "CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT))
			break;
	case GWVS_COVERED:     //Fall through (and we're already falling)
	case GWVS_HIDDEN:
		bShow = TRUE;
		break;
	case GWVS_VISIBLE:     //This is not needed, but goes for readability.
		bShow = FALSE;
		break;
	case -1:               //We can't get here, both cli.hwndContactList and iStepX and iStepY are right.
		return 0;
	}
	if (bShow == TRUE) {
		WINDOWPLACEMENT pl = { 0 };
		HMONITOR(WINAPI * MyMonitorFromWindow) (HWND, DWORD);
		RECT rcScreen, rcWindow;
		int offScreen = 0;

		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, FALSE);
		ShowWindow(cli.hwndContactList, SW_RESTORE);
		SetWindowPos(cli.hwndContactList, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		if (!DBGetContactSettingByte(NULL, "CList", "OnTop", SETTING_ONTOP_DEFAULT))
			SetWindowPos(cli.hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetForegroundWindow(cli.hwndContactList);
		DBWriteContactSettingByte(NULL, "CList", "State", SETTING_STATE_NORMAL);
		//this forces the window onto the visible screen
		MyMonitorFromWindow = (HMONITOR(WINAPI *) (HWND, DWORD)) GetProcAddress(GetModuleHandleA("USER32"), "MonitorFromWindow");
		GetWindowRect(cli.hwndContactList, &rcWindow);
		if (MyMonitorFromWindow) {
			if (MyMonitorFromWindow(cli.hwndContactList, 0) == NULL) {
				BOOL(WINAPI * MyGetMonitorInfoA) (HMONITOR, LPMONITORINFO);
				MONITORINFO mi = { 0 };
				HMONITOR hMonitor = MyMonitorFromWindow(cli.hwndContactList, 2);
				MyGetMonitorInfoA = (BOOL(WINAPI *) (HMONITOR, LPMONITORINFO)) GetProcAddress(GetModuleHandleA("USER32"), "GetMonitorInfoA");
				mi.cbSize = sizeof(mi);
				MyGetMonitorInfoA(hMonitor, &mi);
				rcScreen = mi.rcWork;
				offScreen = 1;
			}
		}
		else {
			RECT rcDest;
			if (IntersectRect(&rcDest, &rcScreen, &rcWindow) == 0)
				offScreen = 1;
		}
		if (offScreen) {
			if (rcWindow.top >= rcScreen.bottom)
				OffsetRect(&rcWindow, 0, rcScreen.bottom - rcWindow.bottom);
			else if (rcWindow.bottom <= rcScreen.top)
				OffsetRect(&rcWindow, 0, rcScreen.top - rcWindow.top);
			if (rcWindow.left >= rcScreen.right)
				OffsetRect(&rcWindow, rcScreen.right - rcWindow.right, 0);
			else if (rcWindow.right <= rcScreen.left)
				OffsetRect(&rcWindow, rcScreen.left - rcWindow.left, 0);
			SetWindowPos(cli.hwndContactList, 0, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top,
				SWP_NOZORDER);
		}
	}
	else {                      //It needs to be hidden
		ShowWindow(cli.hwndContactList, SW_HIDE);
		DBWriteContactSettingByte(NULL, "CList", "State", SETTING_STATE_HIDDEN);
		if (MySetProcessWorkingSetSize != NULL && DBGetContactSettingByte(NULL, "CList", "DisableWorkingSet", 1))
			MySetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
	}
	return 0;
}

/***************************************************************************************/

static int TrayIconProcessMessageStub( WPARAM wParam, LPARAM lParam ) {	return cli.pfnTrayIconProcessMessage( wParam, lParam ); }
static int TrayIconPauseAutoHideStub( WPARAM wParam, LPARAM lParam ) { return cli.pfnTrayIconPauseAutoHide( wParam, lParam ); }
static int CompareContactsStub( WPARAM wParam, LPARAM lParam ) { return cli.pfnCompareContacts( wParam, lParam ); }
static int ShowHideStub( WPARAM wParam, LPARAM lParam ) { return cli.pfnShowHide( wParam, lParam ); }
static int SetHideOfflineStub( WPARAM wParam, LPARAM lParam ) { return cli.pfnSetHideOffline( wParam, lParam ); }
static int Docking_ProcessWindowMessageStub( WPARAM wParam, LPARAM lParam ) { return cli.pfnDocking_ProcessWindowMessage( wParam, lParam ); }
static int HotkeysProcessMessageStub( WPARAM wParam, LPARAM lParam ) { return cli.pfnHotkeysProcessMessage( wParam, lParam ); }

int LoadContactListModule2(void)
{
	HookEvent(ME_SYSTEM_SHUTDOWN, ContactListShutdownProc);
	HookEvent(ME_SYSTEM_MODULESLOADED, ContactListModulesLoaded);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	hProtoAckHook = (HANDLE) HookEvent(ME_PROTO_ACK, ProtocolAck);
	hContactDoubleClicked = CreateHookableEvent(ME_CLIST_DOUBLECLICKED);
	hContactIconChangedEvent = CreateHookableEvent(ME_CLIST_CONTACTICONCHANGED);
	CreateServiceFunction(MS_CLIST_CONTACTDOUBLECLICKED, ContactDoubleClicked);
	CreateServiceFunction(MS_CLIST_CONTACTFILESDROPPED, ContactFilesDropped);
	CreateServiceFunction(MS_CLIST_GETSTATUSMODEDESCRIPTION, GetStatusModeDescription);
	CreateServiceFunction(MS_CLIST_GETCONTACTDISPLAYNAME, GetContactDisplayName);
	CreateServiceFunction(MS_CLIST_INVALIDATEDISPLAYNAME, InvalidateDisplayName);
	CreateServiceFunction(MS_CLIST_TRAYICONPROCESSMESSAGE, TrayIconProcessMessageStub );
	CreateServiceFunction(MS_CLIST_PAUSEAUTOHIDE, TrayIconPauseAutoHideStub);
	CreateServiceFunction(MS_CLIST_CONTACTSCOMPARE, CompareContactsStub);
	CreateServiceFunction(MS_CLIST_CONTACTCHANGEGROUP, ContactChangeGroup);
	CreateServiceFunction(MS_CLIST_SHOWHIDE, ShowHideStub);
	CreateServiceFunction(MS_CLIST_SETHIDEOFFLINE, SetHideOfflineStub);
	CreateServiceFunction(MS_CLIST_DOCKINGPROCESSMESSAGE, Docking_ProcessWindowMessageStub);
	CreateServiceFunction(MS_CLIST_DOCKINGISDOCKED, Docking_IsDocked);
	CreateServiceFunction(MS_CLIST_HOTKEYSPROCESSMESSAGE, HotkeysProcessMessageStub);
	CreateServiceFunction(MS_CLIST_GETCONTACTICON, GetContactIcon);
	MySetProcessWorkingSetSize = (BOOL(WINAPI *) (HANDLE, SIZE_T, SIZE_T)) GetProcAddress(GetModuleHandleA("kernel32"), "SetProcessWorkingSetSize");
	InitDisplayNameCache();
	InitCListEvents();
	InitGroupServices();
	InitTray();
	{
		CLISTMENUITEM mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		CreateServiceFunction("CList/DeleteContactCommand", MenuItem_DeleteContact);
		mi.position = 2000070000;
		mi.flags = 0;
		mi.hIcon = LoadIcon(cli.hInst, MAKEINTRESOURCE(IDI_DELETE));
		mi.pszContactOwner = NULL;      //on every contact
		mi.pszName = Translate("De&lete");
		mi.pszService = "CList/DeleteContactCommand";
		CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
		CreateServiceFunction("CList/AddToListContactCommand", MenuItem_AddContactToList);
		mi.position = -2050000000;
		mi.flags = CMIF_NOTONLIST;
		mi.hIcon = LoadIcon(cli.hInst, MAKEINTRESOURCE(IDI_ADDCONTACT));
		mi.pszName = Translate("&Add permanently to list");
		mi.pszService = "CList/AddToListContactCommand";
		CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
	}

	hCListImages = ImageList_Create(16, 16, ILC_MASK | (IsWinVerXPPlus()? ILC_COLOR32 : ILC_COLOR16), 13, 0);
	HookEvent(ME_SKIN_ICONSCHANGED, CListIconsChanged);
	CreateServiceFunction(MS_CLIST_GETICONSIMAGELIST, GetIconsImageList);
	ImageList_AddIcon(hCListImages, LoadIcon(cli.hInst, MAKEINTRESOURCE(IDI_BLANK)));
	{
		int i;
		for (i = 0; i < SIZEOF(statusModeList); i++)
			ImageList_AddIcon(hCListImages, LoadSkinnedIcon(skinIconStatusList[i]));
	}
	//see IMAGE_GROUP... in clist.h if you add more images above here
	ImageList_AddIcon(hCListImages, LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN));
	ImageList_AddIcon(hCListImages, LoadSkinnedIcon(SKINICON_OTHER_GROUPSHUT));
	return 0;
}

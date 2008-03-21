// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright � 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001-2002 Jon Keating, Richard Hughes
// Copyright � 2002-2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004-2008 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $URL$
// Revision       : $Revision$
// Last change on : $Date$
// Last change by : $Author$
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

#include <win2k.h>
#include <uxtheme.h>

extern BOOL bPopUpService;

static const char* szLogLevelDescr[] = {
	LPGEN("Display all problems"),
	LPGEN("Display problems causing possible loss of data"),
	LPGEN("Display explanations for disconnection"),
	LPGEN("Display problems requiring user intervention"),
	LPGEN("Do not display any problems (not recommended)")
};

static BOOL (WINAPI *pfnEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

static void LoadDBCheckState(CIcqProto* ppro, HWND hwndDlg, int idCtrl, const char* szSetting, BYTE bDef)
{
	CheckDlgButton(hwndDlg, idCtrl, ppro->getByte(NULL, szSetting, bDef));
}

static void StoreDBCheckState(CIcqProto* ppro, HWND hwndDlg, int idCtrl, const char* szSetting)
{
	ppro->setByte(NULL, szSetting, (BYTE)IsDlgButtonChecked(hwndDlg, idCtrl));
}

static void OptDlgChanged(HWND hwndDlg)
{
	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// standalone option pages

static BOOL CALLBACK DlgProcIcqOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLong( hwndDlg, GWL_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLong( hwndDlg, GWL_USERDATA, lParam );
		{
			DWORD dwUin = ppro->getUin(NULL);
			if (dwUin)
				SetDlgItemInt(hwndDlg, IDC_ICQNUM, dwUin, FALSE);
			else // keep it empty when no UIN entered
				SetDlgItemTextA(hwndDlg, IDC_ICQNUM, "");

			char pszPwd[16];
			if (!ppro->getStringStatic(NULL, "Password", pszPwd, sizeof(pszPwd)))
			{
				CallService(MS_DB_CRYPT_DECODESTRING, strlennull(pszPwd) + 1, (LPARAM)pszPwd);

				//bit of a security hole here, since it's easy to extract a password from an edit box
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, pszPwd);
			}

			char szServer[MAX_PATH];
			if (!ppro->getStringStatic(NULL, "OscarServer", szServer, MAX_PATH))
				SetDlgItemTextA(hwndDlg, IDC_ICQSERVER, szServer);
			else
				SetDlgItemTextA(hwndDlg, IDC_ICQSERVER, DEFAULT_SERVER_HOST);

			SetDlgItemInt(hwndDlg, IDC_ICQPORT, ppro->getWord(NULL, "OscarPort", DEFAULT_SERVER_PORT), FALSE);
			LoadDBCheckState(ppro, hwndDlg, IDC_KEEPALIVE, "KeepAlive", 1);
			LoadDBCheckState(ppro, hwndDlg, IDC_SECURE, "SecureLogin", DEFAULT_SECURE_LOGIN);
			SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_SETRANGE, FALSE, MAKELONG(0, 4));
			SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_SETPOS, TRUE, 4-ppro->getByte(NULL, "ShowLogLevel", LOG_WARNING));
			{
				char buf[MAX_PATH];
				SetDlgItemTextUtf(hwndDlg, IDC_LEVELDESCR, ICQTranslateUtfStatic(szLogLevelDescr[4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_GETPOS, 0, 0)], buf, MAX_PATH));
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_HIDE);
			LoadDBCheckState(ppro, hwndDlg, IDC_NOERRMULTI, "IgnoreMultiErrorBox", 0);
		}
		return TRUE;

	case WM_HSCROLL:
		{
			char str[MAX_PATH];

			SetDlgItemTextUtf(hwndDlg, IDC_LEVELDESCR, ICQTranslateUtfStatic(szLogLevelDescr[4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL,TBM_GETPOS, 0, 0)], str, MAX_PATH));
			OptDlgChanged(hwndDlg);
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_LOOKUPLINK:
				CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_FORGOT_PASSWORD);
				return TRUE;

			case IDC_NEWUINLINK:
				CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_REGISTER);
				return TRUE;

			case IDC_RESETSERVER:
				SetDlgItemTextA(hwndDlg, IDC_ICQSERVER, DEFAULT_SERVER_HOST);
				SetDlgItemInt(hwndDlg, IDC_ICQPORT, DEFAULT_SERVER_PORT, FALSE);
				OptDlgChanged(hwndDlg);
				return TRUE;
			}

			if (ppro->icqOnline() && LOWORD(wParam) != IDC_NOERRMULTI)
			{
				char szClass[80];
				GetClassNameA((HWND)lParam, szClass, sizeof(szClass));

				if (stricmp(szClass, "EDIT") || HIWORD(wParam) == EN_CHANGE)
					ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_SHOW);
			}

			if ((LOWORD(wParam)==IDC_ICQNUM || LOWORD(wParam)==IDC_PASSWORD || LOWORD(wParam)==IDC_ICQSERVER || LOWORD(wParam)==IDC_ICQPORT) &&
				(HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))
			{
				return 0;
			}

			OptDlgChanged(hwndDlg);
			break;
		}

	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{

			case PSN_APPLY:
				{
					char str[128];

					ppro->setDword(NULL, UNIQUEIDSETTING, GetDlgItemInt(hwndDlg, IDC_ICQNUM, NULL, FALSE));
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, sizeof(ppro->m_szPassword));
					if (strlennull(str))
					{
						strcpy(ppro->m_szPassword, str);
						ppro->m_bRememberPwd = TRUE;
					}
					else
						ppro->m_bRememberPwd = ppro->getByte(NULL, "RememberPass", 0);

					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(ppro->m_szPassword), (LPARAM)str);
					ppro->setString(NULL, "Password", str);
					GetDlgItemTextA(hwndDlg,IDC_ICQSERVER, str, sizeof(str));
					ppro->setString(NULL, "OscarServer", str);
					ppro->setWord(NULL, "OscarPort", (WORD)GetDlgItemInt(hwndDlg, IDC_ICQPORT, NULL, FALSE));
					StoreDBCheckState(ppro, hwndDlg, IDC_KEEPALIVE, "KeepAlive");
					StoreDBCheckState(ppro, hwndDlg, IDC_SECURE, "SecureLogin");
					StoreDBCheckState(ppro, hwndDlg, IDC_NOERRMULTI, "IgnoreMultiErrorBox");
					ppro->setByte(NULL, "ShowLogLevel", (BYTE)(4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_GETPOS, 0, 0)));

					return TRUE;
				}
			}
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const UINT icqPrivacyControls[] = {
	IDC_DCALLOW_ANY, IDC_DCALLOW_CLIST, IDC_DCALLOW_AUTH, IDC_ADD_ANY, IDC_ADD_AUTH, 
	IDC_WEBAWARE, IDC_PUBLISHPRIMARY, IDC_STATIC_DC1, IDC_STATIC_DC2, IDC_STATIC_CLIST
};

static BOOL CALLBACK DlgProcIcqPrivacyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLong( hwndDlg, GWL_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLong( hwndDlg, GWL_USERDATA, lParam );
		{
			int nDcType = ppro->getByte(NULL, "DCType", 0);
			int nAddAuth = ppro->getByte(NULL, "Auth", 1);

			if (!ppro->icqOnline())
			{
				icq_EnableMultipleControls(hwndDlg, icqPrivacyControls, sizeof(icqPrivacyControls)/sizeof(icqPrivacyControls[0]), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_NOTONLINE), SW_SHOW);
			}
			else 
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_NOTONLINE), SW_HIDE);
			}
			CheckDlgButton(hwndDlg, IDC_DCALLOW_ANY, (nDcType == 0));
			CheckDlgButton(hwndDlg, IDC_DCALLOW_CLIST, (nDcType == 1));
			CheckDlgButton(hwndDlg, IDC_DCALLOW_AUTH, (nDcType == 2));
			CheckDlgButton(hwndDlg, IDC_ADD_ANY, (nAddAuth == 0));
			CheckDlgButton(hwndDlg, IDC_ADD_AUTH, (nAddAuth == 1));
			LoadDBCheckState(ppro, hwndDlg, IDC_WEBAWARE, "WebAware", 0);
			LoadDBCheckState(ppro, hwndDlg, IDC_PUBLISHPRIMARY, "PublishPrimaryEmail", 0);
			LoadDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_CLIST, "StatusMsgReplyCList", 0);
			LoadDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible", 0);
			if (!ppro->getByte(NULL, "StatusMsgReplyCList", 0))
				EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DCALLOW_ANY:
		case IDC_DCALLOW_CLIST:
		case IDC_DCALLOW_AUTH:
		case IDC_ADD_ANY:
		case IDC_ADD_AUTH:
		case IDC_WEBAWARE:
		case IDC_PUBLISHPRIMARY:
		case IDC_STATUSMSG_VISIBLE:
			if ((HWND)lParam != GetFocus())  return 0;
			break;
		case IDC_STATUSMSG_CLIST:
			if (IsDlgButtonChecked(hwndDlg, IDC_STATUSMSG_CLIST)) 
			{
				EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, TRUE);
				LoadDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible", 0);
			}
			else 
			{
				EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
				CheckDlgButton(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
			}
			break;
		default:
			return 0;
		}
		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			StoreDBCheckState(ppro, hwndDlg, IDC_WEBAWARE, "WebAware");
			StoreDBCheckState(ppro, hwndDlg, IDC_PUBLISHPRIMARY, "PublishPrimaryEmail");
			StoreDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_CLIST, "StatusMsgReplyCList");
			StoreDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible");
			if (IsDlgButtonChecked(hwndDlg, IDC_DCALLOW_AUTH))
				ppro->setByte(NULL, "DCType", 2);
			else if (IsDlgButtonChecked(hwndDlg, IDC_DCALLOW_CLIST))
				ppro->setByte(NULL, "DCType", 1);
			else 
				ppro->setByte(NULL, "DCType", 0);
			StoreDBCheckState(ppro, hwndDlg, IDC_ADD_AUTH, "Auth");

			if (ppro->icqOnline())
			{
				PBYTE buf=NULL;
				int buflen=0;

				ppro->ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail", (BYTE)!ppro->getByte(NULL, "PublishPrimaryEmail", 0), TLV_EMAIL);
				ppro->ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail0", 0, TLV_EMAIL);
				ppro->ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail1", 0, TLV_EMAIL);

				ppackTLVByte(&buf, &buflen, (BYTE)!ppro->getByte(NULL, "Auth", 1), TLV_AUTH, 1);
				ppackTLVByte(&buf, &buflen, (BYTE)ppro->getByte(NULL, "WebAware", 0), TLV_WEBAWARE, 1);

				ppro->icq_changeUserDetailsServ(META_SET_FULLINFO_REQ, (char*)buf, (WORD)buflen);

				SAFE_FREE((void**)&buf);

				// Send a status packet to notify the server about the webaware setting
				{
					WORD wStatus = MirandaStatusToIcq(ppro->m_iStatus);

					if (ppro->m_iStatus == ID_STATUS_INVISIBLE)
					{
						if (ppro->m_bSsiEnabled)
							ppro->updateServVisibilityCode(3);
						ppro->icq_setstatus(wStatus, FALSE);
						// Tell who is on our visible list
						ppro->icq_sendEntireVisInvisList(0);
					}
					else
					{
						ppro->icq_setstatus(wStatus, FALSE);
						if (ppro->m_bSsiEnabled)
							ppro->updateServVisibilityCode(4);
						// Tell who is on our invisible list
						ppro->icq_sendEntireVisInvisList(1);
					}
				}
			}
			return TRUE;
		}
		break;
	}

	return FALSE;  
}

/////////////////////////////////////////////////////////////////////////////////////////

static HWND hCpCombo;

struct CPTABLE {
	WORD cpId;
	char *cpName;
};

struct CPTABLE cpTable[] = {
	{  874,  LPGEN("Thai") },
	{  932,  LPGEN("Japanese") },
	{  936,  LPGEN("Simplified Chinese") },
	{  949,  LPGEN("Korean") },
	{  950,  LPGEN("Traditional Chinese") },
	{  1250, LPGEN("Central European") },
	{  1251, LPGEN("Cyrillic") },
	{  1252, LPGEN("Latin I") },
	{  1253, LPGEN("Greek") },
	{  1254, LPGEN("Turkish") },
	{  1255, LPGEN("Hebrew") },
	{  1256, LPGEN("Arabic") },
	{  1257, LPGEN("Baltic") },
	{  1258, LPGEN("Vietnamese") },
	{  1361, LPGEN("Korean (Johab)") },
	{   -1,  NULL}
};

static BOOL CALLBACK FillCpCombo(LPSTR str)
{
	int i;
	UINT cp;

	cp = atoi(str);
	for (i=0; cpTable[i].cpName != NULL && cpTable[i].cpId!=cp; i++);
	if (cpTable[i].cpName) 
		ComboBoxAddStringUtf(hCpCombo, cpTable[i].cpName, cpTable[i].cpId);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const UINT icqUnicodeControls[] = {IDC_UTFALL,IDC_UTFSTATIC,IDC_UTFCODEPAGE};
static const UINT icqDCMsgControls[] = {IDC_DCPASSIVE};
static const UINT icqXStatusControls[] = {IDC_XSTATUSAUTO,IDC_XSTATUSRESET};
static const UINT icqAimControls[] = {IDC_AIMENABLE};

static BOOL CALLBACK DlgProcIcqFeaturesOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLong( hwndDlg, GWL_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLong( hwndDlg, GWL_USERDATA, lParam );
		{
			BYTE bData = ppro->getByte(NULL, "UtfEnabled", DEFAULT_UTF_ENABLED);
			CheckDlgButton(hwndDlg, IDC_UTFENABLE, bData?TRUE:FALSE);
			CheckDlgButton(hwndDlg, IDC_UTFALL, bData==2?TRUE:FALSE);
			icq_EnableMultipleControls(hwndDlg, icqUnicodeControls, SIZEOF(icqUnicodeControls), bData?TRUE:FALSE);
			LoadDBCheckState(ppro, hwndDlg, IDC_TEMPVISIBLE, "TempVisListEnabled",DEFAULT_TEMPVIS_ENABLED);
			LoadDBCheckState(ppro, hwndDlg, IDC_SLOWSEND, "SlowSend", DEFAULT_SLOWSEND);
			LoadDBCheckState(ppro, hwndDlg, IDC_ONLYSERVERACKS, "OnlyServerAcks", DEFAULT_ONLYSERVERACKS);
			bData = ppro->getByte(NULL, "DirectMessaging", DEFAULT_DCMSG_ENABLED);
			CheckDlgButton(hwndDlg, IDC_DCENABLE, bData?TRUE:FALSE);
			CheckDlgButton(hwndDlg, IDC_DCPASSIVE, bData==1?TRUE:FALSE);
			icq_EnableMultipleControls(hwndDlg, icqDCMsgControls, SIZEOF(icqDCMsgControls), bData?TRUE:FALSE);
			bData = ppro->getByte(NULL, "XStatusEnabled", DEFAULT_XSTATUS_ENABLED);
			CheckDlgButton(hwndDlg, IDC_XSTATUSENABLE, bData);
			icq_EnableMultipleControls(hwndDlg, icqXStatusControls, SIZEOF(icqXStatusControls), bData);
			LoadDBCheckState(ppro, hwndDlg, IDC_XSTATUSAUTO, "XStatusAuto", DEFAULT_XSTATUS_AUTO);
			LoadDBCheckState(ppro, hwndDlg, IDC_XSTATUSRESET, "XStatusReset", DEFAULT_XSTATUS_RESET);
			LoadDBCheckState(ppro, hwndDlg, IDC_KILLSPAMBOTS, "KillSpambots", DEFAULT_KILLSPAM_ENABLED);
			LoadDBCheckState(ppro, hwndDlg, IDC_AIMENABLE, "AimEnabled", DEFAULT_AIM_ENABLED);
			icq_EnableMultipleControls(hwndDlg, icqAimControls, SIZEOF(icqAimControls), ppro->icqOnline()?FALSE:TRUE);

			hCpCombo = GetDlgItem(hwndDlg, IDC_UTFCODEPAGE);
			int sCodePage = ppro->getWord(NULL, "AnsiCodePage", CP_ACP);
			ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_UTFCODEPAGE), LPGEN("System default codepage"), 0);
			EnumSystemCodePagesA(FillCpCombo, CP_INSTALLED);
			if(sCodePage == 0)
				SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_SETCURSEL, (WPARAM)0, 0);
			else 
			{
				for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETCOUNT, 0, 0); i++) 
				{
					if (SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETITEMDATA, (WPARAM)i, 0) == sCodePage)
					{
						SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_SETCURSEL, (WPARAM)i, 0);
						break;
					}
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_UTFENABLE:
			icq_EnableMultipleControls(hwndDlg, icqUnicodeControls, sizeof(icqUnicodeControls)/sizeof(icqUnicodeControls[0]), IsDlgButtonChecked(hwndDlg, IDC_UTFENABLE));
			break;
		case IDC_DCENABLE:
			icq_EnableMultipleControls(hwndDlg, icqDCMsgControls, sizeof(icqDCMsgControls)/sizeof(icqDCMsgControls[0]), IsDlgButtonChecked(hwndDlg, IDC_DCENABLE));
			break;
		case IDC_XSTATUSENABLE:
			icq_EnableMultipleControls(hwndDlg, icqXStatusControls, sizeof(icqXStatusControls)/sizeof(icqXStatusControls[0]), IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE));
			break;
		}
		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			if (IsDlgButtonChecked(hwndDlg, IDC_UTFENABLE))
				ppro->m_bUtfEnabled = IsDlgButtonChecked(hwndDlg, IDC_UTFALL)?2:1;
			else
				ppro->m_bUtfEnabled = 0;
			{
				int i = SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETCURSEL, 0, 0);
				ppro->m_wAnsiCodepage = (WORD)SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETITEMDATA, (WPARAM)i, 0);
				ppro->setWord(NULL, "AnsiCodePage", ppro->m_wAnsiCodepage);
			}
			ppro->setByte(NULL, "UtfEnabled", ppro->m_bUtfEnabled);
			ppro->m_bTempVisListEnabled = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TEMPVISIBLE);
			ppro->setByte(NULL, "TempVisListEnabled", ppro->m_bTempVisListEnabled);
			StoreDBCheckState(ppro, hwndDlg, IDC_SLOWSEND, "SlowSend");
			StoreDBCheckState(ppro, hwndDlg, IDC_ONLYSERVERACKS, "OnlyServerAcks");
			if (IsDlgButtonChecked(hwndDlg, IDC_DCENABLE))
				ppro->m_bDCMsgEnabled = IsDlgButtonChecked(hwndDlg, IDC_DCPASSIVE)?1:2;
			else
				ppro->m_bDCMsgEnabled = 0;
			ppro->setByte(NULL, "DirectMessaging", ppro->m_bDCMsgEnabled);
			ppro->m_bXStatusEnabled = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE);
			ppro->setByte(NULL, "XStatusEnabled", ppro->m_bXStatusEnabled);
			StoreDBCheckState(ppro, hwndDlg, IDC_XSTATUSAUTO, "XStatusAuto");
			StoreDBCheckState(ppro, hwndDlg, IDC_XSTATUSRESET, "XStatusReset");
			StoreDBCheckState(ppro, hwndDlg, IDC_KILLSPAMBOTS , "KillSpambots");
			StoreDBCheckState(ppro, hwndDlg, IDC_AIMENABLE, "AimEnabled");
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static const UINT icqContactsControls[] = {IDC_ADDSERVER,IDC_LOADFROMSERVER,IDC_SAVETOSERVER,IDC_UPLOADNOW};
static const UINT icqAvatarControls[] = {IDC_AUTOLOADAVATARS,IDC_BIGGERAVATARS,IDC_STRICTAVATARCHECK};

static BOOL CALLBACK DlgProcIcqContactsOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLong( hwndDlg, GWL_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLong( hwndDlg, GWL_USERDATA, lParam );

		LoadDBCheckState(ppro, hwndDlg, IDC_ENABLE, "UseServerCList", DEFAULT_SS_ENABLED);
		LoadDBCheckState(ppro, hwndDlg, IDC_ADDSERVER, "ServerAddRemove", DEFAULT_SS_ADDSERVER);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOADFROMSERVER, "LoadServerDetails", DEFAULT_SS_LOAD);
		LoadDBCheckState(ppro, hwndDlg, IDC_SAVETOSERVER, "StoreServerDetails", DEFAULT_SS_STORE);
		LoadDBCheckState(ppro, hwndDlg, IDC_ENABLEAVATARS, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);
		LoadDBCheckState(ppro, hwndDlg, IDC_AUTOLOADAVATARS, "AvatarsAutoLoad", DEFAULT_LOAD_AVATARS);
		LoadDBCheckState(ppro, hwndDlg, IDC_BIGGERAVATARS, "AvatarsAllowBigger", DEFAULT_BIGGER_AVATARS);
		LoadDBCheckState(ppro, hwndDlg, IDC_STRICTAVATARCHECK, "StrictAvatarCheck", DEFAULT_AVATARS_CHECK);

		icq_EnableMultipleControls(hwndDlg, icqContactsControls, sizeof(icqContactsControls)/sizeof(icqContactsControls[0]), 
			ppro->getByte(NULL, "UseServerCList", DEFAULT_SS_ENABLED)?TRUE:FALSE);
		icq_EnableMultipleControls(hwndDlg, icqAvatarControls, sizeof(icqAvatarControls)/sizeof(icqAvatarControls[0]), 
			ppro->getByte(NULL, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED)?TRUE:FALSE);

		if (ppro->icqOnline())
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_OFFLINETOENABLE), SW_SHOW);
			EnableDlgItem(hwndDlg, IDC_ENABLE, FALSE);
			EnableDlgItem(hwndDlg, IDC_ENABLEAVATARS, FALSE);
		}
		else
			EnableDlgItem(hwndDlg, IDC_UPLOADNOW, FALSE);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_UPLOADNOW:
			ppro->ShowUploadContactsDialog();
			return TRUE;
		case IDC_ENABLE:
			icq_EnableMultipleControls(hwndDlg, icqContactsControls, sizeof(icqContactsControls)/sizeof(icqContactsControls[0]), IsDlgButtonChecked(hwndDlg, IDC_ENABLE));
			if (ppro->icqOnline()) 
				ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_SHOW);
			else 
				EnableDlgItem(hwndDlg, IDC_UPLOADNOW, FALSE);
			break;
		case IDC_ENABLEAVATARS:
			icq_EnableMultipleControls(hwndDlg, icqAvatarControls, sizeof(icqAvatarControls)/sizeof(icqAvatarControls[0]), IsDlgButtonChecked(hwndDlg, IDC_ENABLEAVATARS));
			break;
		}
		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY )
		{
			StoreDBCheckState(ppro, hwndDlg, IDC_ENABLE, "UseServerCList");
			StoreDBCheckState(ppro, hwndDlg, IDC_ADDSERVER, "ServerAddRemove");
			StoreDBCheckState(ppro, hwndDlg, IDC_LOADFROMSERVER, "LoadServerDetails");
			StoreDBCheckState(ppro, hwndDlg, IDC_SAVETOSERVER, "StoreServerDetails");
			StoreDBCheckState(ppro, hwndDlg, IDC_ENABLEAVATARS, "AvatarsEnabled");
			StoreDBCheckState(ppro, hwndDlg, IDC_AUTOLOADAVATARS, "AvatarsAutoLoad");
			StoreDBCheckState(ppro, hwndDlg, IDC_BIGGERAVATARS, "AvatarsAllowBigger");
			StoreDBCheckState(ppro, hwndDlg, IDC_STRICTAVATARCHECK, "StrictAvatarCheck");
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DlgProcIcqPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int CIcqProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	HMODULE hUxTheme = 0;

	if (IsWinVerXPPlus())
	{
		hUxTheme = GetModuleHandleA("uxtheme.dll");
		if (hUxTheme) 
			pfnEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
	}

	odp.cbSize = sizeof(odp);
	odp.position = -800000000;
	odp.hInstance = hInst;
	odp.ptszGroup = LPGENT("Network");
	odp.dwInitParam = LPARAM(this);
	odp.ptszTitle = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQ);
	odp.pfnDlgProc = DlgProcIcqOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQCONTACTS);
	odp.pfnDlgProc = DlgProcIcqContactsOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Features");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQFEATURES);
	odp.pfnDlgProc = DlgProcIcqFeaturesOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Privacy");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQPRIVACY);
	odp.pfnDlgProc = DlgProcIcqPrivacyOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	if (bPopUpService)
	{
		odp.position = 100000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUPS);
		odp.groupPosition = 900000000;
		odp.pfnDlgProc = DlgProcIcqPopupOpts;
		odp.ptszGroup = LPGENT("Popups");
		odp.ptszTab = NULL;
		CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	}
	return 0;
}

// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright � 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001,2002 Jon Keating, Richard Hughes
// Copyright � 2002,2003,2004 Martin  berg, Sam Kothari, Robert Rainwater
// Copyright � 2004,2005 Joe Kucera
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
// File name      : $Source$
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



PLUGINLINK* pluginLink;
HINSTANCE hInst;
char gpszICQProtoName[MAX_PATH];
HANDLE hHookUserInfoInit = NULL;
HANDLE hHookOptionInit = NULL;
HANDLE hHookUserMenu = NULL;
HANDLE hHookIdleEvent = NULL;
static HANDLE hUserMenuAuth = NULL;
static HANDLE hUserMenuGrant = NULL;

extern HANDLE hServerConn;
extern int gnCurrentStatus;
extern icq_mode_messages modeMsgs;
extern CRITICAL_SECTION modeMsgsMutex;
CRITICAL_SECTION localSeqMutex;
CRITICAL_SECTION connectionHandleMutex;
HANDLE ghServerNetlibUser;
HANDLE hDirectNetlibUser;
HANDLE hsmsgrequest;

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"ICQ Oscar v8 / Joe",
	PLUGIN_MAKE_VERSION(0,3,4,6),
	"Support for ICQ network, slightly enhanced.",
	"Joe Kucera, Martin �berg, Richard Hughes, Jon Keating, etc",
	"jokusoftware@users.sourceforge.net",
	"(C) 2000-2005 M.�berg, R.Hughes, J.Keating, J.Kucera",
  "http://www.miranda-im.org/download/details.php?action=viewfile&id=1683",
	0,		//not transient
	0		//doesn't replace anything built-in
};

static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam);
static int icq_PrebuildContactMenu(WPARAM wParam, LPARAM lParam);



PLUGININFO __declspec(dllexport) *MirandaPluginInfo(DWORD mirandaVersion)
{

	// Only load for 0.3.3 or greater
	// Miranda IM v0.3.3 contained several netlib struct changes requiring a forced upgrade
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 3, 3, 0))
	{
		return NULL;
	}
	else
	{
		return &pluginInfo;
	}

}



BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{

	hInst = hinstDLL;

	return TRUE;

}



int __declspec(dllexport) Load(PLUGINLINK *link)
{

	PROTOCOLDESCRIPTOR pd;


	pluginLink = link;

	srand(time(NULL));
	_tzset();


	// Get module name from DLL file name
    {

		char* str1;
        char str2[MAX_PATH];


		GetModuleFileName(hInst, str2, MAX_PATH);
        str1 = strrchr(str2, '\\');
        if (str1 != NULL && (strlen(str1+1) > 4))
		{
			strncpy(gpszICQProtoName, str1+1, strlen(str1+1)-4);
			gpszICQProtoName[strlen(str1+1)-3] = 0;
        }
		CharUpper(gpszICQProtoName);
    }

	icq_FirstRunCheck();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);

	InitializeCriticalSection(&connectionHandleMutex);
	InitializeCriticalSection(&localSeqMutex);
	InitializeCriticalSection(&modeMsgsMutex);
	InitCookies();

	// Register the module
	ZeroMemory(&pd, sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = gpszICQProtoName;
	pd.type   = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// Initialize status message struct
	ZeroMemory(&modeMsgs, sizeof(icq_mode_messages));


	// Reset a bunch of session specific settings
	ResetSettingsOnLoad();


	// Setup services
	{

		char pszServiceName[MAX_PATH+30];


		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_GETCAPS);
		CreateServiceFunction(pszServiceName , IcqGetCaps);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_GETNAME);
		CreateServiceFunction(pszServiceName , IcqGetName);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_LOADICON);
		CreateServiceFunction(pszServiceName , IcqLoadIcon);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_SETSTATUS);
		CreateServiceFunction(pszServiceName , IcqSetStatus);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_GETSTATUS);
		CreateServiceFunction(pszServiceName , IcqGetStatus);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_SETAWAYMSG);
		CreateServiceFunction(pszServiceName , IcqSetAwayMsg);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_AUTHALLOW);
		CreateServiceFunction(pszServiceName , IcqAuthAllow);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_AUTHDENY);
		CreateServiceFunction(pszServiceName , IcqAuthDeny);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_BASICSEARCH);
		CreateServiceFunction(pszServiceName , IcqBasicSearch);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_SEARCHBYEMAIL);
		CreateServiceFunction(pszServiceName , IcqSearchByEmail);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, MS_ICQ_SEARCHBYDETAILS);
		CreateServiceFunction(pszServiceName, IcqSearchByDetails);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_SEARCHBYNAME);
		CreateServiceFunction(pszServiceName , IcqSearchByDetails);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_CREATEADVSEARCHUI);
		CreateServiceFunction(pszServiceName , IcqCreateAdvSearchUI);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_SEARCHBYADVANCED);
		CreateServiceFunction(pszServiceName , IcqSearchByAdvanced);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, MS_ICQ_SENDSMS);
		CreateServiceFunction(pszServiceName, IcqSendSms);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_ADDTOLIST);
		CreateServiceFunction(pszServiceName , IcqAddToList);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_ADDTOLISTBYEVENT);
		CreateServiceFunction(pszServiceName , IcqAddToListByEvent);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_CHANGEINFO);
		CreateServiceFunction(pszServiceName , IcqChangeInfo);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_FILERESUME);
		CreateServiceFunction(pszServiceName , IcqFileResume);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_GETINFO);
		CreateServiceFunction(pszServiceName , IcqGetInfo);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_MESSAGE);
		CreateServiceFunction(pszServiceName , IcqSendMessage);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_MESSAGE"W");
		CreateServiceFunction(pszServiceName , IcqSendMessageW);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_URL);
		CreateServiceFunction(pszServiceName , IcqSendUrl);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_CONTACTS);
		CreateServiceFunction(pszServiceName , IcqSendContacts);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_SETAPPARENTMODE);
		CreateServiceFunction(pszServiceName , IcqSetApparentMode);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_GETAWAYMSG);
		CreateServiceFunction(pszServiceName , IcqGetAwayMsg);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_FILEALLOW);
		CreateServiceFunction(pszServiceName , IcqFileAllow);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_FILEDENY);
		CreateServiceFunction(pszServiceName , IcqFileDeny);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_FILECANCEL);
		CreateServiceFunction(pszServiceName , IcqFileCancel);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_FILE);
		CreateServiceFunction(pszServiceName , IcqSendFile);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSR_AWAYMSG);
		CreateServiceFunction(pszServiceName , IcqRecvAwayMsg);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSR_FILE);
		CreateServiceFunction(pszServiceName ,IcqRecvFile);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSR_MESSAGE);
		CreateServiceFunction(pszServiceName ,IcqRecvMessage);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSR_URL);
		CreateServiceFunction(pszServiceName ,IcqRecvUrl);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSR_CONTACTS);
		CreateServiceFunction(pszServiceName ,IcqRecvContacts);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSR_AUTH);
		CreateServiceFunction(pszServiceName ,IcqRecvAuth);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_AUTHREQUEST);
		CreateServiceFunction(pszServiceName ,IcqSendAuthRequest);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_ADDED);
		CreateServiceFunction(pszServiceName, IcqSendYouWereAdded);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PSS_USERISTYPING);
		CreateServiceFunction(pszServiceName, IcqSendUserIsTyping);
    strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, PS_GETAVATARINFO);
    CreateServiceFunction(pszServiceName, IcqGetAvatarInfo);
		strcpy(pszServiceName, gpszICQProtoName); strcat(pszServiceName, ME_ICQ_STATUSMSGREQ);
		hsmsgrequest=CreateHookableEvent(pszServiceName);
	}


	InitDirectConns();
	InitServerLists();
	icq_InitInfoUpdate();

	// Initialize charset conversion routines
	InitI18N();

	UpdateGlobalSettings();

	gnCurrentStatus = ID_STATUS_OFFLINE;

	hHookUserMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, icq_PrebuildContactMenu);

	{
		CLISTMENUITEM mi;
		char pszServiceName[MAX_PATH+30];

		strcpy(pszServiceName, gpszICQProtoName);
		strcat(pszServiceName, MS_REQ_AUTH);
		CreateServiceFunction(pszServiceName, icq_RequestAuthorization);

		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 1000030000;
		mi.flags = 0;
		mi.hIcon = NULL; // TODO: add icon
		mi.pszContactOwner = gpszICQProtoName;
		mi.pszName = Translate("Request authorization");
		mi.pszService = pszServiceName;
    hUserMenuAuth = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

		strcpy(pszServiceName, gpszICQProtoName);
		strcat(pszServiceName, MS_GRANT_AUTH);
		CreateServiceFunction(pszServiceName, IcqGrantAuthorization);

		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 1000029999;
		mi.flags = 0;
		mi.hIcon = NULL; // TODO: add icon
		mi.pszContactOwner = gpszICQProtoName;
		mi.pszName = Translate("Grant authorization");
		mi.pszService = pszServiceName;
		hUserMenuGrant = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
	}

	return 0;

}



int __declspec(dllexport) Unload(void)
{

	if (hServerConn)
	{

		icq_packet packet;


		packet.wLen = 0;
		write_flap(&packet, ICQ_CLOSE_CHAN);
		sendServPacket(&packet);

		icq_serverDisconnect(1);

	}

	UninitServerLists();
	UninitDirectConns();
	icq_InfoUpdateCleanup();

	Netlib_CloseHandle(hDirectNetlibUser);
	Netlib_CloseHandle(ghServerNetlibUser);
	UninitCookies();
	DeleteCriticalSection(&modeMsgsMutex);
	DeleteCriticalSection(&localSeqMutex);
	DeleteCriticalSection(&connectionHandleMutex);
	SAFE_FREE(&modeMsgs.szAway);
	SAFE_FREE(&modeMsgs.szNa);
	SAFE_FREE(&modeMsgs.szOccupied);
	SAFE_FREE(&modeMsgs.szDnd);
	SAFE_FREE(&modeMsgs.szFfc);


	if (hHookUserInfoInit)
		UnhookEvent(hHookUserInfoInit);

	if (hHookOptionInit)
		UnhookEvent(hHookOptionInit);

	if (hsmsgrequest)
		DestroyHookableEvent(hsmsgrequest);

	if (hHookUserMenu)
		UnhookEvent(hHookUserMenu);

	if (hHookIdleEvent)
		UnhookEvent(hHookIdleEvent);

	return 0;

}



static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	NETLIBUSER nlu = {0};
	char pszP2PName[MAX_PATH+3];


	strcpy(pszP2PName, gpszICQProtoName);
	strcat(pszP2PName, "P2P");

	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_HTTPGATEWAY;
	nlu.szDescriptiveName = Translate("ICQ server connection");
	nlu.szSettingsModule = gpszICQProtoName;
	nlu.szHttpGatewayHello = "http://http.proxy.icq.com/hello";
	nlu.szHttpGatewayUserAgent = "Mozilla/4.08 [en] (WinNT; U ;Nav)";
	nlu.pfnHttpGatewayInit = icq_httpGatewayInit;
	nlu.pfnHttpGatewayBegin = icq_httpGatewayBegin;
	nlu.pfnHttpGatewayWrapSend = icq_httpGatewayWrapSend;
	nlu.pfnHttpGatewayUnwrapRecv = icq_httpGatewayUnwrapRecv;
	ghServerNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	nlu.flags = NUF_OUTGOING | NUF_INCOMING;
	nlu.szDescriptiveName = Translate("ICQ client-to-client connections");
	nlu.szSettingsModule = pszP2PName;
	nlu.minIncomingPorts = 1;
	hDirectNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	hHookOptionInit = HookEvent(ME_OPT_INITIALISE, IcqOptInit);
	hHookUserInfoInit = HookEvent(ME_USERINFO_INITIALISE, OnDetailsInit);

	hHookIdleEvent = HookEvent(ME_IDLE_CHANGED, IcqIdleChanged);

	return 0;
}



static int icq_PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
  CLISTMENUITEM mi;

  ZeroMemory(&mi, sizeof(mi));
  mi.cbSize = sizeof(mi);
  if (!DBGetContactSettingByte((HANDLE)wParam, gpszICQProtoName, "Auth", 0))
    mi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_HIDDEN;
  else
    mi.flags = CMIM_FLAGS | CMIM_NAME;
  mi.pszName = Translate("Request authorization");

  CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hUserMenuAuth, (LPARAM) & mi);

  if (!DBGetContactSettingByte((HANDLE)wParam, gpszICQProtoName, "Grant", 0))
    mi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_HIDDEN;
  else
    mi.flags = CMIM_FLAGS | CMIM_NAME;
  mi.pszName = Translate("Grant authorization");

  CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hUserMenuGrant, (LPARAM) & mi);

  return 0;
}



void UpdateGlobalSettings()
{

	gbAimEnabled = DBGetContactSettingByte(NULL, gpszICQProtoName, "AimEnabled", DEFAULT_AIM_ENABLED);
	gbSsiEnabled = DBGetContactSettingByte(NULL, gpszICQProtoName, "UseServerCList", DEFAULT_SS_ENABLED);
  gbAvatarsEnabled = DBGetContactSettingByte(NULL, gpszICQProtoName, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);

}

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
#include "../../core/commonheaders.h"
#include <ctype.h>

#define DDEMESSAGETIMEOUT  1000
#define WNDCLASS_DDEMSGWINDOW  "MirandaDdeMsgWindow"

struct DdeMsgWindowData {
	int fAcked,fData;
	HWND hwndDde;
};

static LRESULT CALLBACK DdeMessageWindow(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	struct DdeMsgWindowData *dat;
	ATOM hSzItem;
	HGLOBAL hDdeData;

	dat=(struct DdeMsgWindowData*)GetWindowLong(hwnd,0);
	switch(msg) {
		case WM_DDE_ACK:
			dat->fAcked=1;
			dat->hwndDde=(HWND)wParam;
			return 0;
		case WM_DDE_DATA:
			UnpackDDElParam(msg,lParam,(PUINT)&hDdeData,(PUINT)&hSzItem);
			dat->fData=1;
			if(hDdeData) {
				DDEDATA *data;
				int release;
				data=(DDEDATA*)GlobalLock(hDdeData);
				if(data->fAckReq) {
					DDEACK ack={0};
					PostMessage((HWND)wParam,WM_DDE_ACK,(WPARAM)hwnd,PackDDElParam(WM_DDE_ACK,*(PUINT)&ack,(UINT)hSzItem));
				}
				else GlobalDeleteAtom(hSzItem);
				release=data->fRelease;
				GlobalUnlock(hDdeData);
				if(release) GlobalFree(hDdeData);
			}
			else GlobalDeleteAtom(hSzItem);
			return 0;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static int DoDdeRequest(const char *szItemName,HWND hwndDdeMsg)
{
	ATOM hSzItemName;
	DWORD timeoutTick,thisTick;
	MSG msg;
	struct DdeMsgWindowData *dat=(struct DdeMsgWindowData*)GetWindowLong(hwndDdeMsg,0);

	hSzItemName=GlobalAddAtom(szItemName);
	if(!PostMessage(dat->hwndDde,WM_DDE_REQUEST,(WPARAM)hwndDdeMsg,MAKELPARAM(CF_TEXT,hSzItemName))) {
		GlobalDeleteAtom(hSzItemName);
		return 1;
	}
	timeoutTick=GetTickCount()+5000;
	dat->fData=0; dat->fAcked=0;
	do {
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if(dat->fData || dat->fAcked) break;
		thisTick=GetTickCount();
		if(thisTick>timeoutTick) break;
	} while(MsgWaitForMultipleObjects(0,NULL,FALSE,timeoutTick-thisTick,QS_ALLINPUT)==WAIT_OBJECT_0);

	if(!dat->fData) {
		GlobalDeleteAtom(hSzItemName);
		return 1;
	}
	return 0;
}

//see Q160957 and http://developer.netscape.com/docs/manuals/communicator/DDE/index.htm
static int DdeOpenUrl(const char *szBrowser,char *szUrl,int newWindow,HWND hwndDdeMsg)
{
	ATOM hSzBrowser,hSzTopic;
	DWORD dwResult;
	char *szItemName;
	struct DdeMsgWindowData *dat=(struct DdeMsgWindowData*)GetWindowLong(hwndDdeMsg,0);

	hSzBrowser=GlobalAddAtom(szBrowser);
	hSzTopic=GlobalAddAtom("WWW_OpenURL");
	dat->fAcked=0;
	if(!SendMessageTimeout(HWND_BROADCAST,WM_DDE_INITIATE,(WPARAM)hwndDdeMsg,MAKELPARAM(hSzBrowser,hSzTopic),SMTO_ABORTIFHUNG|SMTO_NORMAL,DDEMESSAGETIMEOUT,&dwResult)
	   || !dat->fAcked) {
		GlobalDeleteAtom(hSzTopic);
		GlobalDeleteAtom(hSzBrowser);
		return 1;
	}
	szItemName=(char*)malloc(lstrlen(szUrl)+7);
	wsprintf(szItemName,"\"%s\",,%d",szUrl,newWindow?0:-1);
	if(DoDdeRequest(szItemName,hwndDdeMsg)) {
		free(szItemName);
		GlobalDeleteAtom(hSzTopic);
		GlobalDeleteAtom(hSzBrowser);
		return 1;
	}
	PostMessage(dat->hwndDde,WM_DDE_TERMINATE,(WPARAM)hwndDdeMsg,0);
	GlobalDeleteAtom(hSzTopic);
	GlobalDeleteAtom(hSzBrowser);
	free(szItemName);
	return 0;
}

static int OpenURL(WPARAM wParam,LPARAM lParam)
{
	char *szUrl=(char*)lParam;
	char *szResult;
	HWND hwndDdeMsg;
	struct DdeMsgWindowData msgWndData={0};
	char *pszProtocol;
	HKEY hKey;
	char szSubkey[80];
	char szCommandName[MAX_PATH];
	DWORD dataLength;
	int success=0;

	hwndDdeMsg=CreateWindow(WNDCLASS_DDEMSGWINDOW,"",0,0,0,0,0,NULL,NULL,GetModuleHandle(NULL),NULL);
	SetWindowLong(hwndDdeMsg,0,(LONG)&msgWndData);

	if(!_strnicmp(szUrl,"ftp:",4) || !_strnicmp(szUrl,"ftp.",4)) pszProtocol="ftp";
	if(!_strnicmp(szUrl,"mailto:",7)) pszProtocol="mailto";
	if(!_strnicmp(szUrl,"news:",5)) pszProtocol="news";
	else pszProtocol="http";
	wsprintf(szSubkey,"%s\\shell\\open\\command",pszProtocol);
	if(RegOpenKeyEx(HKEY_CURRENT_USER,szSubkey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS
	   || RegOpenKeyEx(HKEY_CLASSES_ROOT,szSubkey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS) {
		dataLength=sizeof(szCommandName);
		if(RegQueryValueEx(hKey,NULL,NULL,NULL,(PBYTE)szCommandName,&dataLength)==ERROR_SUCCESS) {
			strlwr(szCommandName);
			if(strstr(szCommandName,"mozilla") || strstr(szCommandName,"netscape"))
				success=(DdeOpenUrl("mozilla",szUrl,wParam,hwndDdeMsg)==0 || DdeOpenUrl("netscape",szUrl,wParam,hwndDdeMsg)==0);
			else if(strstr(szCommandName,"iexplore") || strstr(szCommandName,"msimn"))
				success=0==DdeOpenUrl("iexplore",szUrl,wParam,hwndDdeMsg);
			else if(strstr(szCommandName,"opera"))
				success=0==DdeOpenUrl("opera",szUrl,wParam,hwndDdeMsg);
			//opera's the default anyway
		}
		RegCloseKey(hKey);
	}

	DestroyWindow(hwndDdeMsg);
	if(success) return 0;

	//wack a protocol on it
	if((isalpha(szUrl[0]) && szUrl[1]==':') || szUrl[0]=='\\') {
		szResult=(char*)malloc(lstrlen(szUrl)+9);
		wsprintf(szResult,"file:///%s",szUrl);
	}
	else {
		int i;
		for(i=0;isalpha(szUrl[i]);i++);
		if(szUrl[i]==':') szResult=_strdup(szUrl);
		else {
			if(!_strnicmp(szUrl,"ftp.",4)) {
				szResult=(char*)malloc(lstrlen(szUrl)+7);
				wsprintf(szResult,"ftp://%s",szUrl);
			}
			else {
				szResult=(char*)malloc(lstrlen(szUrl)+8);
				wsprintf(szResult,"http://%s",szUrl);
			}
		}
	}
	ShellExecute(NULL, "open",szResult, NULL, NULL, SW_SHOW);
	free(szResult);
	return 0;
}

int InitOpenUrl(void)
{
	WNDCLASS wcl;
	wcl.lpfnWndProc=DdeMessageWindow;
	wcl.cbClsExtra=0;
	wcl.cbWndExtra=sizeof(void*);
	wcl.hInstance=GetModuleHandle(NULL);
	wcl.hCursor=NULL;
	wcl.lpszClassName=WNDCLASS_DDEMSGWINDOW;
	wcl.hbrBackground=NULL;
	wcl.hIcon=NULL;
	wcl.lpszMenuName=NULL;
	wcl.style=0;
	RegisterClass(&wcl);
	CreateServiceFunction(MS_UTILS_OPENURL,OpenURL);
	return 0;
}
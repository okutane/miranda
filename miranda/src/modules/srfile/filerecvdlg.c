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
#include <shlobj.h>
#include "file.h"

#define MAX_MRU_DIRS    5


static BOOL CALLBACK ClipSiblingsChildEnumProc(HWND hwnd,LPARAM lParam)
{
	SetWindowLong(hwnd,GWL_STYLE,GetWindowLong(hwnd,GWL_STYLE)|WS_CLIPSIBLINGS);
	return TRUE;
}

static void GetLowestExistingDirName(const char *szTestDir,char *szExistingDir,int cchExistingDir)
{
	DWORD dwAttributes;
	char *pszLastBackslash;

	lstrcpyn(szExistingDir,szTestDir,cchExistingDir);
	while((dwAttributes=GetFileAttributes(szExistingDir))!=0xffffffff && !(dwAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
		pszLastBackslash=strrchr(szExistingDir,'\\');
		if(pszLastBackslash==NULL) {*szExistingDir='\0'; break;}
		*pszLastBackslash='\0';
	}
	if(szExistingDir[0]=='\0') GetCurrentDirectory(cchExistingDir,szExistingDir);
}

static const char validFilenameChars[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_!&{}-=#@~,. ";
static void RemoveInvalidFilenameChars(char *szString)
{
	int i;
	for(i=strspn(szString,validFilenameChars);szString[i];i+=strspn(szString+i+1,validFilenameChars)+1)
		if(szString[i]>=0) szString[i]='%';
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	char szDir[MAX_PATH];
	switch(uMsg) {
		case BFFM_INITIALIZED:
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
			break;
		case BFFM_SELCHANGED:
			if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
					SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			break;
	}
	return 0;
}

int BrowseForFolder(HWND hwnd,char *szPath)
{
	BROWSEINFO bi={0};
	LPMALLOC pMalloc;
	ITEMIDLIST *pidlResult;
	int result=0;

	if(SUCCEEDED(OleInitialize(NULL))) {
		if(SUCCEEDED(CoGetMalloc(1,&pMalloc))) {
			bi.hwndOwner=hwnd;
			bi.pszDisplayName=szPath;
			bi.lpszTitle=Translate("Select Folder");
			bi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
			bi.lpfn=BrowseCallbackProc;
			bi.lParam=(LPARAM)szPath;

			pidlResult=SHBrowseForFolder(&bi);
			if(pidlResult) {
				SHGetPathFromIDList(pidlResult,szPath);
				lstrcat(szPath,"\\");
				result=1;
			}
			pMalloc->lpVtbl->Free(pMalloc,pidlResult);
			pMalloc->lpVtbl->Release(pMalloc);
		}
		OleUninitialize();
	}
	return result;
}

static void ReplaceStr(char str[], int len, char *from, char *to) {
    char *tmp;

    if (tmp=strstr(str, from)) {
        int pos = tmp - str;
        int tlen = lstrlen(from);

        tmp = _strdup(str);
        if (lstrlen(to)>tlen)
            tmp = (char*)realloc(tmp, lstrlen(tmp)+1+lstrlen(to)-tlen);
    
        MoveMemory(tmp+pos+lstrlen(to), tmp+pos+tlen, lstrlen(tmp)+1-pos-tlen);
        CopyMemory(tmp+pos, to, lstrlen(to));
        mir_snprintf(str, len, "%s", tmp);
        free(tmp);
    }
}

void GetContactReceivedFilesDir(HANDLE hContact,char *szDir,int cchDir)
{
	DBVARIANT dbv;
	char *szRecvFilesDir, szTemp[MAX_PATH];
	int len;

	if(DBGetContactSetting(NULL,"SRFile","RecvFilesDirAdv",&dbv)||lstrlen(dbv.pszVal)==0) {
		char szDbPath[MAX_PATH];
		
		CallService(MS_DB_GETPROFILEPATH,(WPARAM)MAX_PATH,(LPARAM)szDbPath);
		lstrcat(szDbPath,"\\");
		lstrcat(szDbPath,Translate("Received Files"));
		lstrcat(szDbPath,"\\%userid%");
		szRecvFilesDir=_strdup(szDbPath);
	}
	else {
		szRecvFilesDir=_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
    lstrcpyn(szTemp,szRecvFilesDir,sizeof(szTemp));
    if (hContact) {
        CONTACTINFO ci;
        char szNick[64];
        char szUsername[64];
        char szProto[64];

        szNick[0] = '\0';
        szUsername[0] = '\0';
        szProto[0] = '\0';

        ZeroMemory(&ci, sizeof(ci));
        ci.cbSize = sizeof(ci);
        ci.hContact = hContact;
        ci.szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
        ci.dwFlag = CNF_UNIQUEID;
        mir_snprintf(szProto, sizeof(szProto), "%s", ci.szProto);
        if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
            switch (ci.type) {
                case CNFT_ASCIIZ:
                    mir_snprintf(szUsername, sizeof(szUsername), "%s", ci.pszVal);
                    miranda_sys_free(ci.pszVal);
                    break;
                case CNFT_DWORD:
                    mir_snprintf(szUsername, sizeof(szUsername), "%u", ci.dVal);
                    break;
            }
        }
        mir_snprintf(szNick, sizeof(szNick), "%s", (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
        if (lstrlen(szUsername)==0) {
            mir_snprintf(szUsername, sizeof(szUsername), "%s", (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
        }
        RemoveInvalidFilenameChars(szNick);
        RemoveInvalidFilenameChars(szUsername);
        RemoveInvalidFilenameChars(szProto);
        ReplaceStr(szTemp, sizeof(szTemp), "%nick%", szNick);
        ReplaceStr(szTemp, sizeof(szTemp), "%userid%", szUsername);
        ReplaceStr(szTemp, sizeof(szTemp), "%proto%", szProto);
    }
	lstrcpyn(szDir,szTemp,cchDir);
	free(szRecvFilesDir);
	len=lstrlen(szDir);
	if(len+1<cchDir && szDir[len-1]!='\\') lstrcpy(szDir+len,"\\");
}

BOOL CALLBACK DlgProcRecvFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct FileDlgData *dat;

	dat=(struct FileDlgData*)GetWindowLong(hwndDlg,GWL_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{	char *contactName;
			char szPath[450];

			TranslateDialogDefault(hwndDlg);

			dat=(struct FileDlgData*)calloc(1,sizeof(struct FileDlgData));
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)dat);
			dat->hContact=((CLISTEVENT*)lParam)->hContact;
			dat->hDbEvent=((CLISTEVENT*)lParam)->hDbEvent;
			dat->hPreshutdownEvent=HookEventMessage(ME_SYSTEM_PRESHUTDOWN,hwndDlg,M_PRESHUTDOWN);
			dat->dwTicks=GetTickCount();
			
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_FILE));
			EnumChildWindows(hwndDlg,ClipSiblingsChildEnumProc,0);
			dat->hUIIcons[0]=LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ADDCONTACT),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
			dat->hUIIcons[1]=LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_USERDETAILS),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
			dat->hUIIcons[2]=LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_HISTORY),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
			dat->hUIIcons[3]=LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_DOWNARROW),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
			SendDlgItemMessage(hwndDlg,IDC_ADD,BM_SETIMAGE,IMAGE_ICON,(LPARAM)dat->hUIIcons[0]);
			SendDlgItemMessage(hwndDlg,IDC_DETAILS,BM_SETIMAGE,IMAGE_ICON,(LPARAM)dat->hUIIcons[1]);
			SendDlgItemMessage(hwndDlg,IDC_HISTORY,BM_SETIMAGE,IMAGE_ICON,(LPARAM)dat->hUIIcons[2]);
			SendDlgItemMessage(hwndDlg,IDC_USERMENU,BM_SETIMAGE,IMAGE_ICON,(LPARAM)dat->hUIIcons[3]);
			SendDlgItemMessage(hwndDlg,IDC_ADD,BUTTONSETASFLATBTN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_DETAILS,BUTTONSETASFLATBTN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_HISTORY,BUTTONSETASFLATBTN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_USERMENU,BUTTONSETASFLATBTN,0,0);
			SendMessage(GetDlgItem(hwndDlg,IDC_ADD), BUTTONADDTOOLTIP, (WPARAM)Translate("Add Contact Permanently to List"), 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_USERMENU), BUTTONADDTOOLTIP, (WPARAM)Translate("User Menu"), 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_DETAILS), BUTTONADDTOOLTIP, (WPARAM)Translate("View User's Details"), 0);
			SendMessage(GetDlgItem(hwndDlg,IDC_HISTORY), BUTTONADDTOOLTIP, (WPARAM)Translate("View User's History"), 0);

			contactName=(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)dat->hContact,0);
			SetDlgItemText(hwndDlg,IDC_FROM,contactName);
			GetContactReceivedFilesDir(dat->hContact,szPath,sizeof(szPath));
			SetDlgItemText(hwndDlg,IDC_FILEDIR,szPath);
			{	int i;
				char idstr[32];
				DBVARIANT dbv;
				HRESULT (STDAPICALLTYPE *MySHAutoComplete)(HWND,DWORD);

				MySHAutoComplete=(HRESULT (STDAPICALLTYPE*)(HWND,DWORD))GetProcAddress(GetModuleHandle("shlwapi"),"SHAutoComplete");
				if(MySHAutoComplete) MySHAutoComplete(GetWindow(GetDlgItem(hwndDlg,IDC_FILEDIR),GW_CHILD),1);
				for(i=0;i<MAX_MRU_DIRS;i++) {
					wsprintf(idstr,"MruDir%d",i);
					if(DBGetContactSetting(NULL,"SRFile",idstr,&dbv)) break;
					SendDlgItemMessage(hwndDlg,IDC_FILEDIR,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
					DBFreeVariant(&dbv);
				}
			}

			CallService(MS_DB_EVENT_MARKREAD,(WPARAM)dat->hContact,(LPARAM)dat->hDbEvent);

			{	DBEVENTINFO dbei={0};
				DBTIMETOSTRING dbtts;

				dbei.cbSize=sizeof(dbei);
				dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)dat->hDbEvent,0);
				dbei.pBlob=(PBYTE)malloc(dbei.cbBlob);
				CallService(MS_DB_EVENT_GET,(WPARAM)dat->hDbEvent,(LPARAM)&dbei);
				dat->fs=(HANDLE)*(PDWORD)dbei.pBlob;
				lstrcpyn(szPath, dbei.pBlob+4, min(dbei.cbBlob+1,sizeof(szPath)));
				SetDlgItemText(hwndDlg,IDC_FILENAMES,szPath);
				lstrcpyn(szPath, dbei.pBlob+4+strlen(dbei.pBlob+4)+1, min(dbei.cbBlob-4-strlen(dbei.pBlob+4),sizeof(szPath)));
				SetDlgItemText(hwndDlg,IDC_MSG,szPath);
				free(dbei.pBlob);

				dbtts.szFormat="t d";
				dbtts.szDest=szPath;
				dbtts.cbDest=sizeof(szPath);
				CallService(MS_DB_TIME_TIMESTAMPTOSTRING,dbei.timestamp,(LPARAM)&dbtts);
				SetDlgItemText(hwndDlg,IDC_DATE,szPath);
			}
			
			{
				char *szProto;

				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)dat->hContact,0);
				if (szProto) {
					CONTACTINFO ci;
					int hasName = 0;
					char buf[128];
					ZeroMemory(&ci,sizeof(ci));

					ci.cbSize = sizeof(ci);
					ci.hContact = dat->hContact;
					ci.szProto = szProto;
					ci.dwFlag = CNF_UNIQUEID;
					if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) {
						switch(ci.type) {
							case CNFT_ASCIIZ:
								hasName = 1;
								mir_snprintf(buf, sizeof(buf), "%s", ci.pszVal);
								free(ci.pszVal);
								break;
							case CNFT_DWORD:
								hasName = 1;
								mir_snprintf(buf, sizeof(buf),"%u",ci.dVal);
								break;
						}
					}
					SetDlgItemText(hwndDlg,IDC_NAME,hasName?buf:contactName);
				}
			}

			if(DBGetContactSettingByte(dat->hContact,"CList","NotOnList",0)) {
				RECT rcBtn1,rcBtn2,rcDateCtrl;
				GetWindowRect(GetDlgItem(hwndDlg,IDC_ADD),&rcBtn1);
				GetWindowRect(GetDlgItem(hwndDlg,IDC_USERMENU),&rcBtn2);
				GetWindowRect(GetDlgItem(hwndDlg,IDC_DATE),&rcDateCtrl);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_DATE),0,0,0,rcDateCtrl.right-rcDateCtrl.left-(rcBtn2.left-rcBtn1.left),rcDateCtrl.bottom-rcDateCtrl.top,SWP_NOZORDER|SWP_NOMOVE);
			}
			else if(DBGetContactSettingByte(NULL,"SRFile","AutoAccept",0)) {
				//don't check auto-min here to fix BUG#647620
				PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(hwndDlg,IDOK));
			}
			if(!DBGetContactSettingByte(dat->hContact,"CList","NotOnList",0))
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD),SW_HIDE);
			return TRUE;
		}
		case M_FILEEXISTSDLGREPLY:
			return SendMessage(dat->hwndTransfer,msg,wParam,lParam);
		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);
		case WM_DRAWITEM:
		{	LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;
			if(dis->hwndItem==GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				char *szProto;

				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)dat->hContact,0);
				if (szProto) {
					HICON hIcon;

					hIcon=(HICON)CallProtoService(szProto,PS_LOADICON,PLI_PROTOCOL|PLIF_SMALL,0);
					if (hIcon) {
						DrawIconEx(dis->hDC,dis->rcItem.left,dis->rcItem.top,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
						DestroyIcon(hIcon);
					}
				}
			}
			return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
		}
		case WM_COMMAND:
			if(CallService(MS_CLIST_MENUPROCESSCOMMAND,MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU),(LPARAM)dat->hContact))
				break;
			switch (LOWORD(wParam))
			{
				case IDC_FILEDIRBROWSE:
				{
					char szDirName[MAX_PATH],szExistingDirName[MAX_PATH];

					GetDlgItemText(hwndDlg,IDC_FILEDIR,szDirName,sizeof(szDirName));
					GetLowestExistingDirName(szDirName,szExistingDirName,sizeof(szExistingDirName));
					if(BrowseForFolder(hwndDlg,szExistingDirName))
						SetDlgItemText(hwndDlg,IDC_FILEDIR,szExistingDirName);
					return TRUE;
				}
				case IDOK:
					if(dat->hwndTransfer) return SendMessage(dat->hwndTransfer,msg,wParam,lParam);
					{	//most recently used directories
						char szRecvDir[MAX_PATH],szDefaultRecvDir[MAX_PATH];
						GetDlgItemText(hwndDlg,IDC_FILEDIR,szRecvDir,sizeof(szRecvDir));
						GetContactReceivedFilesDir(NULL,szDefaultRecvDir,sizeof(szDefaultRecvDir));
						if(_strnicmp(szRecvDir,szDefaultRecvDir,lstrlen(szDefaultRecvDir))) {
							char idstr[32];
							int i;
							DBVARIANT dbv;
							for(i=MAX_MRU_DIRS-2;i>=0;i--) {
								wsprintf(idstr,"MruDir%d",i);
								if(DBGetContactSetting(NULL,"SRFile",idstr,&dbv)) continue;
								wsprintf(idstr,"MruDir%d",i+1);
								DBWriteContactSettingString(NULL,"SRFile",idstr,dbv.pszVal);
								DBFreeVariant(&dbv);
							}
							DBWriteContactSettingString(NULL,"SRFile",idstr,szRecvDir);
						}
					}
					EnableWindow(GetDlgItem(hwndDlg,IDC_FILENAMES),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_MSG),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_FILEDIR),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_FILEDIRBROWSE),FALSE);
					dat->hwndTransfer=CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_FILETRANSFERINFO),hwndDlg,DlgProcFileTransfer);
					//check for auto-minimize here to fix BUG#647620
					if(DBGetContactSettingByte(NULL,"SRFile","AutoAccept",0) && DBGetContactSettingByte(NULL,"SRFile","AutoMin",0))
						ShowWindow(hwndDlg,SW_SHOWMINIMIZED);
					return TRUE;
				case IDCANCEL:					
					if (dat->fs) CallContactService(dat->hContact,PSS_FILEDENY,(WPARAM)dat->fs,(LPARAM)Translate("Cancelled"));
					dat->fs=NULL; /* the protocol will free the handle */
					if(dat->hwndTransfer) return SendMessage(dat->hwndTransfer,msg,wParam,lParam);
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDC_ADD:
				{	ADDCONTACTSTRUCT acs={0};
					
					acs.handle=dat->hContact;
					acs.handleType=HANDLE_CONTACT;
					acs.szProto="";
					CallService(MS_ADDCONTACT_SHOW,(WPARAM)hwndDlg,(LPARAM)&acs);
					if(!DBGetContactSettingByte(dat->hContact,"CList","NotOnList",0))
						ShowWindow(GetDlgItem(hwndDlg,IDC_ADD), SW_HIDE);
					return TRUE;
				}
				case IDC_USERMENU:
				{	RECT rc;
					HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,(WPARAM)dat->hContact,0);
					GetWindowRect((HWND)lParam,&rc);
					TrackPopupMenu(hMenu,0,rc.left,rc.bottom,0,hwndDlg,NULL);
					DestroyMenu(hMenu);
					break;
				}
				case IDC_DETAILS:
					CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)dat->hContact,0);
					return TRUE;
				case IDC_HISTORY:
					CallService(MS_HISTORY_SHOWCONTACTHISTORY,(WPARAM)dat->hContact,0);
					return TRUE;
			}
			break;
		case M_PRESHUTDOWN:
		{
			if (IsWindow(dat->hwndTransfer)) PostMessage(dat->hwndTransfer,WM_CLOSE,0,0);
			break;
		}
		case WM_DESTROY:
			if(dat->hPreshutdownEvent) UnhookEvent(dat->hPreshutdownEvent);
			if(dat->hwndTransfer) DestroyWindow(dat->hwndTransfer);
			DestroyIcon(dat->hUIIcons[3]);
			DestroyIcon(dat->hUIIcons[2]);
			DestroyIcon(dat->hUIIcons[1]);
			DestroyIcon(dat->hUIIcons[0]);
			free(dat);
			return TRUE;
	}
	return FALSE;
}

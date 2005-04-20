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
#include "netlib.h"

#define MS_NETLIB_LOGWIN "Netlib/Log/Win"

extern HANDLE hConnectionHeaderMutex;

#define TIMEFORMAT_NONE         0
#define TIMEFORMAT_HHMMSS       1
#define TIMEFORMAT_MILLISECONDS 2
#define TIMEFORMAT_MICROSECONDS 3
struct {
	HWND hwndOpts;
	int toOutputDebugString;
	int toFile;
	char *szFile;
	int timeFormat;
	int showUser;
	int dumpSent,dumpRecv,dumpProxy;
	int textDumps,autoDetectText;
	CRITICAL_SECTION cs;
} logOptions;
static __int64 mirandaStartTime,perfCounterFreq;

static const char *szTimeFormats[]={"No times","Standard hh:mm:ss times","Times in milliseconds","Times in microseconds"};

static BOOL CALLBACK LogOptionsDlgProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
		case WM_INITDIALOG:
			logOptions.hwndOpts=hwndDlg;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg,IDC_DUMPRECV,logOptions.dumpRecv?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_DUMPSENT,logOptions.dumpSent?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_DUMPPROXY,logOptions.dumpProxy?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TEXTDUMPS,logOptions.textDumps?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_AUTODETECTTEXT,logOptions.autoDetectText?BST_CHECKED:BST_UNCHECKED);
			{	int i;
				for(i=0;i<sizeof(szTimeFormats)/sizeof(szTimeFormats[0]);i++)
					SendDlgItemMessage(hwndDlg,IDC_TIMEFORMAT,CB_ADDSTRING,0,(LPARAM)Translate(szTimeFormats[i]));
			}
			SendDlgItemMessage(hwndDlg,IDC_TIMEFORMAT,CB_SETCURSEL,logOptions.timeFormat,0);
			CheckDlgButton(hwndDlg,IDC_SHOWNAMES,logOptions.showUser?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TOOUTPUTDEBUGSTRING,logOptions.toOutputDebugString?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TOFILE,logOptions.toFile?BST_CHECKED:BST_UNCHECKED);
			SetDlgItemText(hwndDlg,IDC_FILENAME,logOptions.szFile);
			CheckDlgButton(hwndDlg,IDC_SHOWTHISDLGATSTART,DBGetContactSettingByte(NULL,"Netlib","ShowLogOptsAtStart",0)?BST_CHECKED:BST_UNCHECKED);
			{	DBVARIANT dbv;
				if(!DBGetContactSetting(NULL,"Netlib","RunAtStart",&dbv)) {
					SetDlgItemText(hwndDlg,IDC_RUNATSTART,dbv.pszVal);
					DBFreeVariant(&dbv);
				}
			}
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_DUMPRECV:
					logOptions.dumpRecv=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_DUMPSENT:
					logOptions.dumpSent=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_DUMPPROXY:
					logOptions.dumpProxy=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_TEXTDUMPS:
					logOptions.textDumps=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_AUTODETECTTEXT:
					logOptions.autoDetectText=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_TIMEFORMAT:
					logOptions.timeFormat=SendDlgItemMessage(hwndDlg,IDC_TIMEFORMAT,CB_GETCURSEL,0,0);
					break;
				case IDC_SHOWNAMES:
					logOptions.showUser=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_TOOUTPUTDEBUGSTRING:
					logOptions.toOutputDebugString=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_TOFILE:
					logOptions.toFile=IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
					break;
				case IDC_FILENAME:
					if(HIWORD(wParam)!=EN_CHANGE) break;
					if((HWND)lParam==GetFocus()) {
						CheckDlgButton(hwndDlg,IDC_TOFILE,BST_UNCHECKED);
						logOptions.toFile=0;
					}
					EnterCriticalSection(&logOptions.cs);
					if(logOptions.szFile) free(logOptions.szFile);
					{	int len;
						len=GetWindowTextLength((HWND)lParam);
						logOptions.szFile=(char*)malloc(len+1);
						GetWindowText((HWND)lParam,logOptions.szFile,len+1);
					}
					LeaveCriticalSection(&logOptions.cs);
					break;
				case IDC_FILENAMEBROWSE:
				case IDC_RUNATSTARTBROWSE:
				{	char str[MAX_PATH+2];
					OPENFILENAME ofn={0};
					char filter[512],*pfilter;

					GetWindowText(GetWindow((HWND)lParam,GW_HWNDPREV),str,sizeof(str));
					ofn.lStructSize=OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner=hwndDlg;
					ofn.Flags=OFN_HIDEREADONLY;
					if (LOWORD(wParam)==IDC_FILENAMEBROWSE) {
						ofn.lpstrTitle=Translate("Select where log file will be created");
					} else {
						ofn.Flags|=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
						ofn.lpstrTitle=Translate("Select program to be run");
					}					
					strcpy(filter,Translate("All Files"));
					strcat(filter," (*)");
					pfilter=filter+strlen(filter)+1;
					strcpy(pfilter,"*");
					pfilter=pfilter+strlen(pfilter)+1;
					*pfilter='\0';
					ofn.lpstrFilter=filter;
					ofn.lpstrFile=str;
					ofn.nMaxFile=sizeof(str)-2;
					ofn.nMaxFileTitle=MAX_PATH;
					if (LOWORD(wParam)==IDC_FILENAMEBROWSE) {
						if(!GetSaveFileName(&ofn)) return 1;
					} else {
						if(!GetOpenFileName(&ofn)) return 1;						
					}
					if(LOWORD(wParam)==IDC_RUNATSTARTBROWSE && strchr(str,' ')!=NULL) {
						MoveMemory(str+1,str,sizeof(str)-2);
						str[0]='"';
						lstrcat(str,"\"");
					}
					SetWindowText(GetWindow((HWND)lParam,GW_HWNDPREV),str);
					break;
				}
				case IDC_SHOWTHISDLGATSTART:
					DBWriteContactSettingByte(NULL,"Netlib","ShowLogOptsAtStart",(BYTE)IsDlgButtonChecked(hwndDlg,LOWORD(wParam)));
					break;
				case IDC_RUNATSTART:
					if(HIWORD(wParam)!=EN_CHANGE) break;
					{	int len;
						char *str;
						len=GetWindowTextLength((HWND)lParam);
						str=(char*)malloc(len+1);
						GetWindowText((HWND)lParam,str,len+1);
						DBWriteContactSettingString(NULL,"Netlib","RunAtStart",str);
						free(str);
					}
					break;
				case IDC_RUNNOW:
					{	int len;
						char *str;
						STARTUPINFO si={0};
						PROCESS_INFORMATION pi;
						len=GetWindowTextLength(GetDlgItem(hwndDlg,IDC_RUNATSTART));
						str=(char*)malloc(len+1);
						GetDlgItemText(hwndDlg,IDC_RUNATSTART,str,len+1);
						si.cb=sizeof(si);
						if(str[0]) CreateProcess(NULL,str,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
					}
					break;
				case IDC_SAVE:
					DBWriteContactSettingByte(NULL,"Netlib","DumpRecv",(BYTE)logOptions.dumpRecv);
					DBWriteContactSettingByte(NULL,"Netlib","DumpSent",(BYTE)logOptions.dumpSent);
					DBWriteContactSettingByte(NULL,"Netlib","DumpProxy",(BYTE)logOptions.dumpProxy);
					DBWriteContactSettingByte(NULL,"Netlib","TextDumps",(BYTE)logOptions.textDumps);
					DBWriteContactSettingByte(NULL,"Netlib","AutoDetectText",(BYTE)logOptions.autoDetectText);
					DBWriteContactSettingByte(NULL,"Netlib","TimeFormat",(BYTE)logOptions.timeFormat);
					DBWriteContactSettingByte(NULL,"Netlib","ShowUser",(BYTE)logOptions.showUser);
					DBWriteContactSettingByte(NULL,"Netlib","ToOutputDebugString",(BYTE)logOptions.toOutputDebugString);
					DBWriteContactSettingByte(NULL,"Netlib","ToFile",(BYTE)logOptions.toFile);
					DBWriteContactSettingString(NULL,"Netlib","File",logOptions.szFile?logOptions.szFile:"");
					break;
				case IDOK:
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
		case WM_DESTROY:
			logOptions.hwndOpts=NULL;
			break;
	}
	return FALSE;
}

void NetlibLogShowOptions(void)
{
	if(logOptions.hwndOpts==NULL)
		logOptions.hwndOpts=CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_NETLIBLOGOPTS),NULL,LogOptionsDlgProc);
	SetForegroundWindow(logOptions.hwndOpts);
}

static void CreateDirectoryTree(char *szDir)
{
	DWORD dwAttributes;
	char *pszLastBackslash,szTestDir[MAX_PATH];

	lstrcpyn(szTestDir,szDir,sizeof(szTestDir));
	if((dwAttributes=GetFileAttributes(szTestDir))!=0xffffffff && dwAttributes&FILE_ATTRIBUTE_DIRECTORY) return;
	pszLastBackslash=strrchr(szTestDir,'\\');
	if(pszLastBackslash==NULL) return;
	*pszLastBackslash='\0';
	CreateDirectoryTree(szTestDir);
	CreateDirectory(szTestDir,NULL);
}

static int NetlibLog(WPARAM wParam,LPARAM lParam)
{
	struct NetlibUser *nlu=(struct NetlibUser*)wParam;
	struct NetlibUser nludummy;
	const char *pszMsg=(const char*)lParam;
	char *szLine;
	char szTime[32];
	LARGE_INTEGER liTimeNow;
	DWORD dwOriginalLastError;

	if( (nlu != NULL && GetNetlibHandleType(nlu)!=NLH_USER) || pszMsg==NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if (nlu==NULL) { /* if the Netlib user handle is NULL, just pretend its not */
		nlu=&nludummy;
		nlu->user.szSettingsModule="(NULL)";
	}
	dwOriginalLastError=GetLastError();
	QueryPerformanceCounter(&liTimeNow);
	liTimeNow.QuadPart-=mirandaStartTime;
	switch(logOptions.timeFormat) {
		case TIMEFORMAT_HHMMSS:
			GetTimeFormat(LOCALE_USER_DEFAULT,TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER,NULL,NULL,szTime,sizeof(szTime)-1);
			break;
		case TIMEFORMAT_MILLISECONDS:
			mir_snprintf(szTime,sizeof(szTime)-1,"%I64u.%03I64u",liTimeNow.QuadPart/perfCounterFreq,1000*(liTimeNow.QuadPart%perfCounterFreq)/perfCounterFreq);
			break;
		case TIMEFORMAT_MICROSECONDS:
			mir_snprintf(szTime,sizeof(szTime)-1,"%I64u.%06I64u",liTimeNow.QuadPart/perfCounterFreq,1000000*(liTimeNow.QuadPart%perfCounterFreq)/perfCounterFreq);
			break;
		default:
			szTime[0]='\0';
			break;
	}
	EnterCriticalSection(&logOptions.cs);
	if(logOptions.showUser) lstrcat(szTime," ");
	szLine=(char*)alloca(lstrlen(pszMsg)+lstrlen(nlu->user.szSettingsModule)+5+lstrlen(szTime));
	if(logOptions.timeFormat || logOptions.showUser)
		sprintf(szLine,"[%s%s] %s\n",szTime,logOptions.showUser?nlu->user.szSettingsModule:"",pszMsg);
	else
		sprintf(szLine,"%s\n",pszMsg);
	if(logOptions.toOutputDebugString) OutputDebugString(szLine);
	if(logOptions.toFile && logOptions.szFile[0]) {
		FILE *fp;
		fp=fopen(logOptions.szFile,"at");
		if(!fp) {
			CreateDirectoryTree(logOptions.szFile);
			fp=fopen(logOptions.szFile,"at");
		}
		if(fp) {			
			fputs(szLine,fp);
			fclose(fp);
		}
	}
	LeaveCriticalSection(&logOptions.cs);
	SetLastError(dwOriginalLastError);
	return 1;
}



void NetlibDumpData(struct NetlibConnection *nlc,PBYTE buf,int len,int sent,int flags)
{
	int isText=1;
	char szTitleLine[128];
	char *szBuf;
	int titleLineLen;
	struct NetlibUser *nlu;

	// This section checks a number of conditions and aborts
	// the dump if the data should not be written to the log

	// Check packet flags
	if ((flags&MSG_PEEK) || (flags&MSG_NODUMP))
		return;

	// Check user's log settings
	if (!(logOptions.toOutputDebugString ||
		(logOptions.toFile && logOptions.szFile[0])))
		return;
	if ((sent && !logOptions.dumpSent) ||
		(!sent && !logOptions.dumpRecv))
		return;
	if ((flags&MSG_DUMPPROXY) && !logOptions.dumpProxy)
		return;


	if (!logOptions.textDumps)
		isText = 0;
	else if (!(flags&MSG_DUMPASTEXT)) {
		if (logOptions.autoDetectText) {
			int i;
			for(i = 0; i<len; i++)
				if ((buf[i]<' ' && buf[i]!='\t' && buf[i]!='\r' && buf[i]!='\n') || buf[i]>=0x80)
				{
					isText = 0;
					break;
				}
		}
		else
			isText = 0;
	}

	WaitForSingleObject(hConnectionHeaderMutex, INFINITE);
	nlu = nlc ? nlc->nlu : NULL;
	titleLineLen = mir_snprintf(szTitleLine,sizeof(szTitleLine), "(%p:%u) Data %s%s\n", nlc, nlc?nlc->s:0, sent?"sent":"received", flags & MSG_DUMPPROXY?" (proxy)":"");
	ReleaseMutex(hConnectionHeaderMutex);

	// Text data
	if (isText)
	{
		szBuf = (char*)alloca(titleLineLen + len + 1);
		CopyMemory(szBuf, szTitleLine, titleLineLen);
		CopyMemory(szBuf + titleLineLen, (const char*)buf, len);
		szBuf[titleLineLen + len] = '\0';
	}
	// Binary data
	else
	{
		int line, col, colsInLine;
		char *pszBuf;

		szBuf = (char*)alloca(titleLineLen + ((len+16)>>4) * 76 + 1);
		CopyMemory(szBuf, szTitleLine, titleLineLen);
		pszBuf = szBuf + titleLineLen;
		for (line = 0; ; line += 16)
		{
			colsInLine = min(16, len - line);
			pszBuf += wsprintf(pszBuf, "%08X: ", line);
			// Dump data as hex
			for (col = 0; col < colsInLine; col++)
				pszBuf += wsprintf(pszBuf, "%02X%c", buf[line + col], ((col&3)==3 && col != 15)?'-':' ');
			// Fill out last line with blanks
			for ( ; col<16; col++)
			{
				lstrcpy(pszBuf, "   ");
				pszBuf += 3;
			}
			*pszBuf++ = ' ';
			for (col = 0; col < colsInLine; col++)
				*pszBuf++ = buf[line+col]<' '?'.':(char)buf[line+col];
			if (len-line<=16)
				break;
			*pszBuf++ = '\n'; // End each line with a break
		}
		*pszBuf = '\0';
	}

	NetlibLog((WPARAM)nlu,(LPARAM)szBuf);

}

void NetlibLogInit(void)
{
	DBVARIANT dbv;
	LARGE_INTEGER li;

	CreateServiceFunction(MS_NETLIB_LOG,NetlibLog);
	QueryPerformanceFrequency(&li);
	perfCounterFreq=li.QuadPart;
	QueryPerformanceCounter(&li);
	mirandaStartTime=li.QuadPart;
	InitializeCriticalSection(&logOptions.cs);
	logOptions.dumpRecv=DBGetContactSettingByte(NULL,"Netlib","DumpRecv",1);
	logOptions.dumpSent=DBGetContactSettingByte(NULL,"Netlib","DumpSent",1);
	logOptions.dumpProxy=DBGetContactSettingByte(NULL,"Netlib","DumpProxy",0);
	logOptions.textDumps=DBGetContactSettingByte(NULL,"Netlib","TextDumps",1);
	logOptions.autoDetectText=DBGetContactSettingByte(NULL,"Netlib","AutoDetectText",1);
	logOptions.timeFormat=DBGetContactSettingByte(NULL,"Netlib","TimeFormat",TIMEFORMAT_HHMMSS);
	logOptions.showUser=DBGetContactSettingByte(NULL,"Netlib","ShowUser",1);
	logOptions.toOutputDebugString=DBGetContactSettingByte(NULL,"Netlib","ToOutputDebugString",0);
	logOptions.toFile=DBGetContactSettingByte(NULL,"Netlib","ToFile",0);
	if(!DBGetContactSetting(NULL,"Netlib","File",&dbv)) {
		logOptions.szFile=_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(logOptions.toFile && logOptions.szFile[0]) {
		FILE *fp;
		fp=fopen(logOptions.szFile,"wt");
		if(fp) fclose(fp);
	}
	if(DBGetContactSettingByte(NULL,"Netlib","ShowLogOptsAtStart",0))
		NetlibLogShowOptions();
	if(!DBGetContactSetting(NULL,"Netlib","RunAtStart",&dbv)) {
		STARTUPINFO si={0};
		PROCESS_INFORMATION pi;
		si.cb=sizeof(si);
		if(dbv.pszVal[0]) CreateProcess(NULL,dbv.pszVal,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
		DBFreeVariant(&dbv);
	}
}

void NetlibLogShutdown(void)
{
	if(IsWindow(logOptions.hwndOpts)) DestroyWindow(logOptions.hwndOpts);
	DeleteCriticalSection(&logOptions.cs);
	if(logOptions.szFile) free(logOptions.szFile);
}


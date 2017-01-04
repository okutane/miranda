/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2017 Miranda ICQ/IM project,
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
#include <sys/types.h>
#include <sys/stat.h>
#include "file.h"

static void SetFileListAndSizeControls(HWND hwndDlg,struct FileDlgData *dat)
{
	int fileCount=0,dirCount=0,totalSize=0,i;
	struct _stat statbuf;
	TCHAR str[64];

	for ( i=0; dat->files[i]; i++ ) {
		if ( _tstat( dat->files[i], &statbuf ) == 0 ) {
			if ( statbuf.st_mode & _S_IFDIR)
				dirCount++;
			else
				fileCount++;
			totalSize += statbuf.st_size;
	}	}

	GetSensiblyFormattedSize(totalSize,str,SIZEOF(str),0,1,NULL);
	SetDlgItemText(hwndDlg,IDC_TOTALSIZE,str);
	if(i>1) {
		TCHAR szFormat[32];
		if(fileCount && dirCount) {
			mir_sntprintf(szFormat,SIZEOF(szFormat),_T("%s, %s"),TranslateTS(fileCount==1?_T("%d file"):_T("%d files")),TranslateTS(dirCount==1?_T("%d directory"):_T("%d directories")));
			mir_sntprintf(str,SIZEOF(str),szFormat,fileCount,dirCount);
		}
		else if(fileCount) {
			lstrcpy(szFormat,TranslateT("%d files"));
			mir_sntprintf(str,SIZEOF(str),szFormat,fileCount);
		}
		else {
			lstrcpy(szFormat,TranslateT("%d directories"));
			mir_sntprintf(str,SIZEOF(str),szFormat,dirCount);
		}
		SetDlgItemText(hwndDlg,IDC_FILE,str);
	}
	else SetDlgItemText(hwndDlg,IDC_FILE,dat->files[0]);

	EnableWindow(GetDlgItem(hwndDlg, IDOK), fileCount || dirCount);
}

static void FilenameToFileList(HWND hwndDlg, struct FileDlgData* dat, const TCHAR* buf)
{
	DWORD dwFileAttributes;

	// Make sure that the file matrix is empty (the user may select files several times)
	FreeFilesMatrix(&dat->files);

	// Get the file attributes of selection
	dwFileAttributes = GetFileAttributes( buf );
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		return;

	// Check if the selection is a directory or a file
	if ( GetFileAttributes( buf ) & FILE_ATTRIBUTE_DIRECTORY ) {
		const TCHAR* pBuf;
		int nNumberOfFiles = 0;
		int nTemp;
		int fileOffset;

		// :NOTE: The first string in the buffer is the directory, followed by a
		// NULL separated list of all files

		// fileOffset is the offset to the first file.
		fileOffset = lstrlen(buf) + 1;

		// Count number of files
		pBuf = buf + fileOffset;
		while ( *pBuf ) {
			pBuf += lstrlen(pBuf) + 1;
			nNumberOfFiles++;
		}

		// Allocate memory for a pointer array
		if (( dat->files = ( TCHAR* *)mir_alloc((nNumberOfFiles + 1) * sizeof(TCHAR*))) == NULL )
			return;

		// Fill the array
		pBuf = buf + fileOffset;
		nTemp = 0;
		while(*pBuf)
		{
			// Allocate space for path+filename
			int cbFileNameLen = lstrlen( pBuf );
			dat->files[nTemp] = ( TCHAR* )mir_alloc( sizeof(TCHAR)*(fileOffset + cbFileNameLen + 1));

			// Add path to filename and copy into array
			#if defined( _UNICODE )
				CopyMemory(dat->files[nTemp], buf, (fileOffset-1)*sizeof( TCHAR ));
				dat->files[nTemp][fileOffset-1] = '\\';
				_tcscpy(dat->files[nTemp] + fileOffset - (buf[fileOffset-2]=='\\'?1:0), pBuf);
			#else
				CopyMemory(dat->files[nTemp], buf, fileOffset-1 );
				dat->files[nTemp][fileOffset-1] = '\\';
				strcpy(dat->files[nTemp] + fileOffset - (buf[fileOffset-2]=='\\'?1:0), pBuf);
			#endif
			// Move pointers to next file...
			pBuf += cbFileNameLen + 1;
			nTemp++;
		}
		// Terminate array
		dat->files[nNumberOfFiles] = NULL;
	}
	// ...the selection is a single file
	else
	{
		if (( dat->files = ( TCHAR **)mir_alloc(2 * sizeof( TCHAR*))) == NULL ) // Leaks when aborted
			return;

		dat->files[0] = mir_tstrdup(buf);
		dat->files[1] = NULL;
	}

	// Update dialog text with new file selection
	SetFileListAndSizeControls(hwndDlg, dat);
}

#define M_FILECHOOSEDONE  (WM_USER+100)
void __cdecl ChooseFilesThread(void* param)
{
	HWND hwndDlg = ( HWND )param;
	TCHAR filter[128], *pfilter;
	TCHAR* buf = ( TCHAR* )mir_alloc( sizeof(TCHAR)*32767 );
	if ( buf == NULL )
		PostMessage( hwndDlg, M_FILECHOOSEDONE, 0, ( LPARAM )( TCHAR* )NULL );
	else {
		OPENFILENAME ofn = {0};
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner   = hwndDlg;
		lstrcpy( filter, TranslateT( "All Files" ));
		lstrcat( filter, _T(" (*)" ));
		pfilter = filter + lstrlen( filter )+1;
		lstrcpy( pfilter, _T( "*" ));
		pfilter = filter + lstrlen( filter )+1;
		pfilter[ 0 ] = '\0';
		ofn.lpstrFilter = filter;
		ofn.lpstrFile = buf; *buf = 0;
		ofn.nMaxFile = 32767;
		ofn.Flags = OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
		if ( GetOpenFileName( &ofn ))
			PostMessage( hwndDlg, M_FILECHOOSEDONE, 0, ( LPARAM )buf );
		else {
			mir_free( buf );
			PostMessage( hwndDlg, M_FILECHOOSEDONE, 0, ( LPARAM )( TCHAR* )NULL );
}	}	}

static BOOL CALLBACK ClipSiblingsChildEnumProc(HWND hwnd,LPARAM)
{
	SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_CLIPSIBLINGS);
	return TRUE;
}

static WNDPROC OldSendEditProc;
static LRESULT CALLBACK SendEditSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
		case WM_CHAR:
			if(wParam=='\n' && GetKeyState(VK_CONTROL)&0x8000) {
				PostMessage(GetParent(hwnd),WM_COMMAND,IDOK,0);
				return 0;
			}
			break;
		case WM_SYSCHAR:
			if((wParam=='s' || wParam=='S') && GetKeyState(VK_MENU)&0x8000) {
				PostMessage(GetParent(hwnd),WM_COMMAND,IDOK,0);
				return 0;
			}
			break;
	}
	return CallWindowProc(OldSendEditProc,hwnd,msg,wParam,lParam);
}

INT_PTR CALLBACK DlgProcSendFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct FileDlgData *dat;

	dat=(struct FileDlgData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
	{
		struct FileSendData *fsd=(struct FileSendData*)lParam;

		dat=(struct FileDlgData*)mir_calloc(sizeof(struct FileDlgData));
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG_PTR)dat);
		dat->hContact=fsd->hContact;
		dat->send=1;
		dat->hPreshutdownEvent=HookEventMessage(ME_SYSTEM_PRESHUTDOWN,hwndDlg,M_PRESHUTDOWN);
		dat->fs=NULL;
		dat->dwTicks=GetTickCount();

		TranslateDialogDefault(hwndDlg);
		EnumChildWindows(hwndDlg,ClipSiblingsChildEnumProc,0);
		OldSendEditProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_MSG),GWLP_WNDPROC,(LONG_PTR)SendEditSubclassProc);

		Window_SetIcon_IcoLib(hwndDlg, SKINICON_EVENT_FILE);
		Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View User's Details"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View User's History"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User Menu"));

        EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

		if(fsd->ppFiles!=NULL && fsd->ppFiles[0]!=NULL) {
			int totalCount,i;
			for(totalCount=0;fsd->ppFiles[totalCount];totalCount++);
			dat->files = ( TCHAR** )mir_alloc( sizeof(TCHAR*)*(totalCount+1)); // Leaks
			for(i=0;i<totalCount;i++)
				dat->files[i] = mir_tstrdup( fsd->ppFiles[i] );
			dat->files[totalCount]=NULL;
			SetFileListAndSizeControls(hwndDlg,dat);
		}
		{
			char *szProto;
			TCHAR* contactName = cli.pfnGetContactDisplayName( dat->hContact, 0 );
			SetDlgItemText(hwndDlg,IDC_TO,contactName);

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
							mir_snprintf(buf, SIZEOF(buf), "%s", ci.pszVal);
							mir_free(ci.pszVal);
							break;
						case CNFT_DWORD:
							hasName = 1;
							mir_snprintf(buf, SIZEOF(buf),"%u",ci.dVal);
							break;
				}	}

				if ( hasName )
					SetDlgItemTextA(hwndDlg,IDC_NAME,buf);
				else
					SetDlgItemText(hwndDlg,IDC_NAME,contactName);
		}	}

		if ( fsd->ppFiles == NULL ) {
       		EnableWindow(hwndDlg, FALSE);
			dat->closeIfFileChooseCancelled=1;
			PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_CHOOSE,BN_CLICKED),(LPARAM)GetDlgItem(hwndDlg,IDC_CHOOSE));
		}
		return TRUE;
	}

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;
			if(dis->hwndItem==GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				char *szProto;

				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)dat->hContact,0);
				if (szProto) {
					HICON hIcon = (HICON)CallProtoService(szProto,PS_LOADICON,PLI_PROTOCOL|PLIF_SMALL,0);
					if (hIcon) {
						DrawIconEx(dis->hDC,dis->rcItem.left,dis->rcItem.top,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
						DestroyIcon(hIcon);
		}	}	}	}
		return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
		
	case M_FILECHOOSEDONE:
		if( lParam != 0 ) {
			FilenameToFileList( hwndDlg, dat, ( TCHAR* )lParam );
			mir_free(( TCHAR* )lParam );
			dat->closeIfFileChooseCancelled = 0;
		}
		else if(dat->closeIfFileChooseCancelled) DestroyWindow(hwndDlg);
		EnableWindow(hwndDlg,TRUE);
		break;

	case WM_COMMAND:
		if(CallService(MS_CLIST_MENUPROCESSCOMMAND,MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU),(LPARAM)dat->hContact))
			break;
		switch (LOWORD(wParam))
		{
			case IDC_CHOOSE:
				EnableWindow(hwndDlg,FALSE);
				//GetOpenFileName() creates its own message queue which prevents any incoming events being processed
				forkthread(ChooseFilesThread,0,hwndDlg);
				break;
			case IDOK:
				EnableWindow(GetDlgItem(hwndDlg,IDC_FILENAME),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MSG),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHOOSE),FALSE);

				GetDlgItemText(hwndDlg,IDC_FILEDIR,dat->szSavePath,SIZEOF(dat->szSavePath));
				GetDlgItemText(hwndDlg,IDC_FILE,dat->szFilenames,SIZEOF(dat->szFilenames));
				GetDlgItemText(hwndDlg,IDC_MSG,dat->szMsg,SIZEOF(dat->szMsg));
				dat->hwndTransfer = FtMgr_AddTransfer(dat);
				SetWindowLongPtr( hwndDlg, GWLP_USERDATA, 0);
				DestroyWindow(hwndDlg);
				return TRUE;

			case IDCANCEL:
				DestroyWindow(hwndDlg);
				return TRUE;

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

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Button_FreeIcon_IcoLib(hwndDlg,IDC_DETAILS);
		Button_FreeIcon_IcoLib(hwndDlg,IDC_HISTORY);
		Button_FreeIcon_IcoLib(hwndDlg,IDC_USERMENU);

		if ( dat )
			FreeFileDlgData( dat );
		
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_MSG),GWLP_WNDPROC,(LONG_PTR)OldSendEditProc);
		return TRUE;
	}
	return FALSE;
}

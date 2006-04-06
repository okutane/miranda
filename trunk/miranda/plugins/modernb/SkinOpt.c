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
#include "m_clui.h"
#include "clist.h"
#include "m_clc.h"
#include "SkinEngine.h"
#include "io.h"
#include "commonprototypes.h"

extern BOOL glOtherSkinWasLoaded;
extern BYTE glSkinWasModified;
//#include "shlwapi.h"
/*******************************/
// Main skin selection routine //
/*******************************/
#define MAX_NAME 100
typedef struct _SkinListData
{
	char Name[MAX_NAME];
	char File[MAX_PATH];
} SkinListData;
HBITMAP hPreviewBitmap=NULL;
extern HWND hCLUIwnd;
static int AddItemToTree(HWND hTree, char * folder, char * itemName, void * data);
extern int LoadSkinFromIniFile(char*);
extern int RedrawCompleteWindow();
extern int LoadSkinFromDB();
int AddSkinToListFullName(HWND hwndDlg,char * fullName);
int AddSkinToList(HWND hwndDlg,char * path, char* file);
int FillAvailableSkinList(HWND hwndDlg);
static BOOL CALLBACK DlgSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int SkinOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=-1000000000;
	odp.hInstance=g_hInst;
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.pszGroup=Translate("Skin");
	odp.pszTitle=Translate("Load Skin");
	odp.pfnDlgProc=DlgSkinOpts;
	odp.flags=ODPF_BOLDGROUPS;
	//	odp.nIDBottomSimpleControl=IDC_STCLISTGROUP;
	//	odp.expertOnlyControls=expertOnlyControls;
	//	odp.nExpertOnlyControls=sizeof(expertOnlyControls)/sizeof(expertOnlyControls[0]);
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}

static BOOL CALLBACK DlgSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY: 
		{
			if (hPreviewBitmap) DeleteObject(hPreviewBitmap);
			break;
		}

	case WM_INITDIALOG:
		{ 
			int it;
			TranslateDialogDefault(hwndDlg);
			it=FillAvailableSkinList(hwndDlg);
			{
				/* Text Colors */
				DWORD c1,c2,c3,c4;
				c1=DBGetContactSettingDword(NULL,"Menu", "TextColour", CLCDEFAULT_TEXTCOLOUR);
				c2=DBGetContactSettingDword(NULL,"Menu", "SelTextColour", CLCDEFAULT_SELTEXTCOLOUR);
				c3=DBGetContactSettingDword(NULL,"FrameTitleBar", "TextColour", CLCDEFAULT_TEXTCOLOUR);
				c4=DBGetContactSettingDword(NULL,"StatusBar", "TextColour", CLCDEFAULT_TEXTCOLOUR);
				SendDlgItemMessage(hwndDlg,IDC_COLOUR_MENUNORMAL,CPM_SETCOLOUR,0,c1);                  
				SendDlgItemMessage(hwndDlg,IDC_COLOUR_MENUSELECTED,CPM_SETCOLOUR,0,c2);
				SendDlgItemMessage(hwndDlg,IDC_COLOUR_FRAMES,CPM_SETCOLOUR,0,c3);
				SendDlgItemMessage(hwndDlg,IDC_COLOUR_STATUSBAR,CPM_SETCOLOUR,0,c4);

				SendDlgItemMessage(hwndDlg,IDC_COLOUR_MENUNORMAL,CPM_SETDEFAULTCOLOUR,0,c1);                  
				SendDlgItemMessage(hwndDlg,IDC_COLOUR_MENUSELECTED,CPM_SETDEFAULTCOLOUR,0,c2);
				SendDlgItemMessage(hwndDlg,IDC_COLOUR_FRAMES,CPM_SETDEFAULTCOLOUR,0,c3);
				SendDlgItemMessage(hwndDlg,IDC_COLOUR_STATUSBAR,CPM_SETDEFAULTCOLOUR,0,c4);                              
				/* End of Text colors */
			}

			TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_TREE1),(HTREEITEM)it);						
		}
		return TRUE;
	case WM_COMMAND:
		{
			int isLoad=0;
			switch (LOWORD(wParam)) 
			{
			case IDC_COLOUR_MENUNORMAL:
			case IDC_COLOUR_MENUSELECTED:
			case IDC_COLOUR_FRAMES:
			case IDC_COLOUR_STATUSBAR:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_BUTTON_INFO:
				{
					char Author[255];
					char URL[MAX_PATH];
					char Contact[255];
					char Description[400];
					char text[2000];
					SkinListData *sd=NULL;  
					HTREEITEM hti=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_TREE1));				
					if (hti==0) return 0;
					{
						TVITEMA tvi={0};
						tvi.hItem=hti;
						tvi.mask=TVIF_HANDLE|TVIF_PARAM;
						TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&tvi);
						sd=(SkinListData*)(tvi.lParam);
					}
					if (!sd) return 0;
					GetPrivateProfileStringA("Skin_Description_Section","Author","(unknown)",Author,sizeof(Author),sd->File);
					GetPrivateProfileStringA("Skin_Description_Section","URL","",URL,sizeof(URL),sd->File);
					GetPrivateProfileStringA("Skin_Description_Section","Contact","",Contact,sizeof(Contact),sd->File);
					GetPrivateProfileStringA("Skin_Description_Section","Description","",Description,sizeof(Description),sd->File);
					_snprintf(text,sizeof(text),Translate("%s\n\n%s\n\nAuthor(s):\t %s\nContact:\t %s\nWeb:\t %s\n\nFile:\t %s"),
						sd->Name,Description,Author,Contact,URL,sd->File);
					MessageBoxA(hwndDlg,text,"Skin Information",MB_OK|MB_ICONINFORMATION);
				}
				break;
			case IDC_BUTTON_APPLY_SKIN:
				if (HIWORD(wParam)==BN_CLICKED)
				{ 		
					SkinListData *sd=NULL;  
					HTREEITEM hti=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_TREE1));				
					if (hti==0) return 0;
					{
						TVITEM tvi={0};
						tvi.hItem=hti;
						tvi.mask=TVIF_HANDLE|TVIF_PARAM;
						TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&tvi);
						sd=(SkinListData*)(tvi.lParam);
					}
					if (!sd) return 0;
					if (glSkinWasModified>0)
					{
						int res=0;
						if (glSkinWasModified==1)
							res=MessageBoxA(hwndDlg,Translate("Skin editor contains not stored changes.\n\nAll changes will be lost.\n\n Continue to load new skin?"),Translate("Warning!"),MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2|MB_TOPMOST);
						else
							res=MessageBoxA(hwndDlg,Translate("Current skin was not saved to file.\n\nAll changes will be lost.\n\n Continue to load new skin?"),Translate("Warning!"),MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2|MB_TOPMOST);
						if (res!=IDOK) return 0;
					}
					LoadSkinFromIniFile(sd->File);
					LoadSkinFromDB();	
					glOtherSkinWasLoaded=TRUE;
					pcli->pfnClcBroadcast( INTM_RELOADOPTIONS,0,0);
					CLUIFramesOnClistResize2(0,0,0);
					RedrawCompleteWindow();        
					CLUIFramesOnClistResize2(0,0,0);
					{
						HWND hwnd=(HWND)CallService(MS_CLUI_GETHWND,0,0);
						RECT rc={0};
						GetWindowRect(hwnd, &rc);
						OnMoving(hwnd,&rc);
					}
					if (hCLUIwnd)
					{
						SendDlgItemMessage(hCLUIwnd,IDC_LEFTMARGINSPIN,UDM_SETPOS,0,DBGetContactSettingByte(NULL,"CLUI","LeftClientMargin",0));
						SendDlgItemMessage(hCLUIwnd,IDC_RIGHTMARGINSPIN,UDM_SETPOS,0,DBGetContactSettingByte(NULL,"CLUI","RightClientMargin",0));
						SendDlgItemMessage(hCLUIwnd,IDC_TOPMARGINSPIN,UDM_SETPOS,0,DBGetContactSettingByte(NULL,"CLUI","TopClientMargin",0));
						SendDlgItemMessage(hCLUIwnd,IDC_BOTTOMMARGINSPIN,UDM_SETPOS,0,DBGetContactSettingByte(NULL,"CLUI","BottomClientMargin",0));
					}
				}
				break;
			case IDC_BUTTON_LOAD:
				isLoad=1;
				if (HIWORD(wParam)==BN_CLICKED)
				{
					{   		
						char str[MAX_PATH]={0};
						OPENFILENAMEA ofn={0};
						char filter[512]={0};
						int res=0;
						ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
						ofn.hwndOwner = hwndDlg;
						ofn.hInstance = NULL;

						sprintf(filter,"%s",Translate("Miranda skin file"));
						memmove(filter+MyStrLen(filter)," (*.msf)\0*.MSF\0\0",sizeof(" (*.msf)\0*.MSF\0\0"));
						ofn.lpstrFilter = filter;
						ofn.lpstrFile = str;
						ofn.Flags = isLoad?(OFN_FILEMUSTEXIST | OFN_HIDEREADONLY) : (OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY);
						ofn.nMaxFile = sizeof(str);
						ofn.nMaxFileTitle = MAX_PATH;
						ofn.lpstrDefExt = "msf";

						{
							DWORD tick=GetTickCount();
							res=GetOpenFileNameA(&ofn);
							if(!res) 
								if (GetTickCount()-tick<100)
								{
									res=GetOpenFileNameA(&ofn);
									if(!res) break;
								}
								else break;
						}
						if (res)
						{
							int it=AddSkinToListFullName(hwndDlg,ofn.lpstrFile);
							TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_TREE1),(HTREEITEM)it);
							//SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_SETCURSEL,it,0); 
							//SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_SKINS_LIST,LBN_SELCHANGE),0);
						}
					}
				}
			//case IDC_SKINS_LIST:
			//	{
			//		switch (HIWORD(wParam))
			//		{
			//		case LBN_SELCHANGE:
			//			//TODO: Skin list selection was changed
			//			{
			//				/*
			//				int item;            
			//				if (hPreviewBitmap) 
			//				{
			//					DeleteObject(hPreviewBitmap);
			//					hPreviewBitmap=NULL;
			//				}
			//				item=SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_GETCURSEL,0,0); 
			//				if (item!=-1)
			//				{
			//					//selected
			//					SkinListData * sd;
			//					sd=(SkinListData*)SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_GETITEMDATA,(WPARAM)item,(LPARAM)0); 
			//					if (sd)
			//					{
			//						//enable 'Apply' button
			//						//enable 'Info' button
			//						//update preview
			//						SendDlgItemMessageA(hwndDlg,IDC_EDIT_SKIN_FILENAME,WM_SETTEXT,0,(LPARAM)sd->File); //TODO made filepath unicode
			//						{
			//							char prfn[MAX_PATH]={0};
			//							char imfn[MAX_PATH]={0};
			//							char skinfolder[MAX_PATH]={0};
			//							GetPrivateProfileStringA("Skin_Description_Section","Preview","",imfn,sizeof(imfn),sd->File);
			//							GetSkinFolder(sd->File,skinfolder);
			//							_snprintf(prfn,sizeof(prfn),"%s\\%s",skinfolder,imfn);
			//							CallService(MS_UTILS_PATHTOABSOLUTE,(WPARAM)prfn,(LPARAM) imfn);
			//							hPreviewBitmap=intLoadGlyphImage(imfn);
			//						}
			//						EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_APPLY_SKIN),TRUE);
			//						EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_INFO),TRUE);

			//					}
			//				}
			//				else
			//				{
			//					//no selected
			//					SendDlgItemMessage(hwndDlg,IDC_EDIT_SKIN_FILENAME,WM_SETTEXT,0,(LPARAM)TranslateT("Select skin from list"));
			//					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_APPLY_SKIN),FALSE);
			//					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_INFO),FALSE);
			//				} 
			//				ShowWindowNew(GetDlgItem(hwndDlg,IDC_PREVIEW),hPreviewBitmap?SW_SHOW:SW_HIDE);
			//				if (hPreviewBitmap) InvalidateRect(GetDlgItem(hwndDlg,IDC_PREVIEW),NULL,TRUE);
			//				else  //prepeare text

			//				{
			//					char Author[255];
			//					char URL[MAX_PATH];
			//					char Contact[255];
			//					char Description[400];
			//					char text[2000];
			//					int item;
			//					SkinListData *sd;           
			//					item=SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_GETCURSEL,0,0); 
			//					if (item==-1) return 0;
			//					sd=(SkinListData*)SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_GETITEMDATA,(WPARAM)item,(LPARAM)0);              
			//					if (!sd) return 0;
			//					GetPrivateProfileStringA("Skin_Description_Section","Author","(unknown)",Author,sizeof(Author),sd->File);
			//					GetPrivateProfileStringA("Skin_Description_Section","URL","",URL,sizeof(URL),sd->File);
			//					GetPrivateProfileStringA("Skin_Description_Section","Contact","",Contact,sizeof(Contact),sd->File);
			//					GetPrivateProfileStringA("Skin_Description_Section","Description","",Description,sizeof(Description),sd->File);
			//					_snprintf(text,sizeof(text),Translate("Preview is not available\n\n%s\n----------------------\n\n%s\n\nAUTHOR(S):\n%s\n\nCONTACT:\n%s\n\nHOMEPAGE:\n%s"),
			//						sd->Name,Description,Author,Contact,URL);
			//					SendDlgItemMessageA(hwndDlg,IDC_STATIC_INFO,WM_SETTEXT,0,(LPARAM)text);
			//				}
			//				*/
			//			}
			//			break;

			//		}
			//		break;
			//	}
			}
			break;
		}
	case WM_DRAWITEM:
		if (wParam==IDC_PREVIEW)
		{
			//TODO:Draw hPreviewBitmap here
			HDC memDC, imgDC;
			HBITMAP hbmp,holdbmp,imgOldbmp;
			int mWidth, mHeight;
			RECT workRect={0};
			HBRUSH hbr=CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			DRAWITEMSTRUCT *dis=(DRAWITEMSTRUCT *)lParam;
			mWidth=dis->rcItem.right-dis->rcItem.left;
			mHeight=dis->rcItem.bottom-dis->rcItem.top;
			memDC=CreateCompatibleDC(dis->hDC);
			hbmp=CreateBitmap32(mWidth,mHeight);
			holdbmp=SelectObject(memDC,hbmp);
			workRect=dis->rcItem;
			OffsetRect(&workRect,-workRect.left,-workRect.top);
			FillRect(memDC,&workRect,hbr);     
			DeleteObject(hbr);
			if (hPreviewBitmap)
			{
				//variables
				BITMAP bmp={0};
				POINT imgPos={0};
				int wWidth,wHeight;
				int dWidth,dHeight;
				float xScale=1, yScale=1;
				//GetSize
				GetObject(hPreviewBitmap,sizeof(BITMAP),&bmp);
				wWidth=workRect.right-workRect.left;
				wHeight=workRect.bottom-workRect.top;
				if (wWidth<bmp.bmWidth) xScale=(float)wWidth/bmp.bmWidth;
				if (wHeight<bmp.bmHeight) yScale=(float)wHeight/bmp.bmHeight;
				xScale=min(xScale,yScale);
				yScale=xScale;                    
				dWidth=(int)(xScale*bmp.bmWidth);
				dHeight=(int)(yScale*bmp.bmHeight);
				//CalcPosition
				imgPos.x=workRect.left+((wWidth-dWidth)>>1);
				imgPos.y=workRect.top+((wHeight-dHeight)>>1);     
				//DrawImage
				if (!gdiPlusFail) //Use gdi+ engine
				{
					DrawAvatarImageWithGDIp(memDC,imgPos.x,imgPos.y,dWidth,dHeight,hPreviewBitmap,0,0,bmp.bmWidth,bmp.bmHeight,8,255);
				}   
				else
				{
					BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					imgDC=CreateCompatibleDC(dis->hDC);
					imgOldbmp=SelectObject(imgDC,hPreviewBitmap);                 
					MyAlphaBlend(memDC,imgPos.x,imgPos.y,dWidth,dHeight,imgDC,0,0,bmp.bmWidth,bmp.bmHeight,bf);
					SelectObject(imgDC,imgOldbmp);
					ModernDeleteDC(imgDC);
				}
			}
			BitBlt(dis->hDC,dis->rcItem.left,dis->rcItem.top,mWidth,mHeight,memDC,0,0,SRCCOPY);
			SelectObject(memDC,holdbmp);
			DeleteObject(hbmp);
			ModernDeleteDC(memDC);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) 
		{
		case IDC_TREE1:
		{		
			NMTREEVIEWA * nmtv = (NMTREEVIEWA *) lParam;
			if (!nmtv) return 0;
			if (nmtv->hdr.code==TVN_SELCHANGEDA)
			{	
				SkinListData * sd=NULL;
				if (hPreviewBitmap) 
				{
					DeleteObject(hPreviewBitmap);
					hPreviewBitmap=NULL;
				}
				if (nmtv->itemNew.lParam)
				{
					//char sdFile[MAX_PATH]={0};
					sd=(SkinListData*)nmtv->itemNew.lParam;
				    //PathCompactPathExA(sdFile,sd->File,60,0);
					{
						char buf[MAX_PATH];
						CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)sd->File, (LPARAM)buf);
						SendDlgItemMessageA(hwndDlg,IDC_EDIT_SKIN_FILENAME,WM_SETTEXT,0,(LPARAM)buf); //TODO made filepath unicode
					}
					{
							char prfn[MAX_PATH]={0};
							char imfn[MAX_PATH]={0};
							char skinfolder[MAX_PATH]={0};
							GetPrivateProfileStringA("Skin_Description_Section","Preview","",imfn,sizeof(imfn),sd->File);
							GetSkinFolder(sd->File,skinfolder);
							_snprintf(prfn,sizeof(prfn),"%s\\%s",skinfolder,imfn);
							CallService(MS_UTILS_PATHTOABSOLUTE,(WPARAM)prfn,(LPARAM) imfn);
							hPreviewBitmap=intLoadGlyphImage(imfn);
						}
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_APPLY_SKIN),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_INFO),TRUE);
					if (hPreviewBitmap) 
						InvalidateRect(GetDlgItem(hwndDlg,IDC_PREVIEW),NULL,TRUE);
					else  //prepeare text
						{
							char Author[255];
							char URL[MAX_PATH];
							char Contact[255];
							char Description[400];
							char text[2000];
							SkinListData* sd=NULL;
							HTREEITEM hti=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_TREE1));				
							if (hti==0) return 0;
							{
								TVITEM tvi={0};
								tvi.hItem=hti;
								tvi.mask=TVIF_HANDLE|TVIF_PARAM;
								TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&tvi);
								sd=(SkinListData*)(tvi.lParam);
							}
							if (!sd) return 0;
							GetPrivateProfileStringA("Skin_Description_Section","Author","(unknown)",Author,sizeof(Author),sd->File);
							GetPrivateProfileStringA("Skin_Description_Section","URL","",URL,sizeof(URL),sd->File);
							GetPrivateProfileStringA("Skin_Description_Section","Contact","",Contact,sizeof(Contact),sd->File);
							GetPrivateProfileStringA("Skin_Description_Section","Description","",Description,sizeof(Description),sd->File);
							_snprintf(text,sizeof(text),Translate("Preview is not available\n\n%s\n----------------------\n\n%s\n\nAUTHOR(S):\n%s\n\nCONTACT:\n%s\n\nHOMEPAGE:\n%s"),
							sd->Name,Description,Author,Contact,URL);
							ShowWindow(GetDlgItem(hwndDlg,IDC_PREVIEW),SW_HIDE);
							ShowWindow(GetDlgItem(hwndDlg,IDC_STATIC_INFO),SW_SHOW);
							SendDlgItemMessageA(hwndDlg,IDC_STATIC_INFO,WM_SETTEXT,0,(LPARAM)text);
						}					
				}
				else
				{
					//no selected
					SendDlgItemMessage(hwndDlg,IDC_EDIT_SKIN_FILENAME,WM_SETTEXT,0,(LPARAM)TranslateT("Select skin from list"));
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_APPLY_SKIN),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_INFO),FALSE);
					SendDlgItemMessageA(hwndDlg,IDC_STATIC_INFO,WM_SETTEXT,0,(LPARAM)Translate("Please select skin to apply"));
					ShowWindow(GetDlgItem(hwndDlg,IDC_PREVIEW),SW_HIDE);
				}
				ShowWindow(GetDlgItem(hwndDlg,IDC_PREVIEW),hPreviewBitmap?SW_SHOW:SW_HIDE);
				return 0;
			}			
			else if (nmtv->hdr.code==TVN_SELCHANGEDA)
			{
				if (nmtv->itemOld.lParam)
					mir_free((void*)(nmtv->itemOld.lParam));
				return 0;
			}
			break;
		}
		case 0:
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_APPLY:
				{
					{
						DWORD tick=GetTickCount();
						{
							/* Text Colors */
							DWORD c1,c2,c3,c4;
							c1=SendDlgItemMessage(hwndDlg,IDC_COLOUR_MENUNORMAL,CPM_GETCOLOUR,0,0);
							c2=SendDlgItemMessage(hwndDlg,IDC_COLOUR_MENUSELECTED,CPM_GETCOLOUR,0,0);
							c3=SendDlgItemMessage(hwndDlg,IDC_COLOUR_FRAMES,CPM_GETCOLOUR,0,0);
							c4=SendDlgItemMessage(hwndDlg,IDC_COLOUR_STATUSBAR,CPM_GETCOLOUR,0,0);
							DBWriteContactSettingDword(NULL,"Menu", "TextColour", c1);
							DBWriteContactSettingDword(NULL,"Menu", "SelTextColour", c2);
							DBWriteContactSettingDword(NULL,"FrameTitleBar", "TextColour", c3);
							DBWriteContactSettingDword(NULL,"StatusBar", "TextColour", c4);
							/* End of Text colors */
						}
						pcli->pfnClcBroadcast( INTM_RELOADOPTIONS,0,0);
						NotifyEventHooks(ME_BACKGROUNDCONFIG_CHANGED,0,0);
						pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);	
						RedrawWindow(GetParent(pcli->hwndContactTree),NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN);
					}
					return TRUE;
				}
				break;
			}
			break;
		}
	}
	return FALSE;
}

int SearchSkinFiles(HWND hwndDlg, char * Folder)
{
	struct _finddata_t fd={0};
	char mask[MAX_PATH];
	long hFile; 
	_snprintf(mask,sizeof(mask),"%s\\*.msf",Folder); 
	//fd.attrib=_A_SUBDIR;
	hFile=_findfirst(mask,&fd);
	if (hFile!=-1)
	{
		do {     
			AddSkinToList(hwndDlg,Folder,fd.name);
		}while (!_findnext(hFile,&fd));
		_findclose(hFile);
	}
	_snprintf(mask,sizeof(mask),"%s\\*",Folder);
	hFile=_findfirst(mask,&fd);
	{
		do {
			if (fd.attrib&_A_SUBDIR && !(boolstrcmpi(fd.name,".")||boolstrcmpi(fd.name,"..")))
			{//Next level of subfolders
				char path[MAX_PATH];
				_snprintf(path,sizeof(path),"%s\\%s",Folder,fd.name);
				SearchSkinFiles(hwndDlg,path);
			}
		}while (!_findnext(hFile,&fd));
		_findclose(hFile);
	}
	return 0;
}
int FillAvailableSkinList(HWND hwndDlg)
{
	struct _finddata_t fd={0};
	//long hFile; 
	int res=-1;
	char path[MAX_PATH];//,mask[MAX_PATH];
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)"Skins", (LPARAM)path);
	SearchSkinFiles(hwndDlg,path);
	/*_snprintf(mask,sizeof(mask),"%s\\*.msf",path); 
	//fd.attrib=_A_SUBDIR;
	hFile=_findfirst(mask,&fd);
	if (hFile!=-1)
	{
		do {     
			AddSkinToList(hwndDlg,path,fd.name);
		}while (!_findnext(hFile,&fd));
		_findclose(hFile);
	}
	_snprintf(mask,sizeof(mask),"%s\\*",path);
	hFile=_findfirst(mask,&fd);
	if (hFile!=-1)
	{
		do {
			if (fd.attrib&_A_SUBDIR && !(boolstrcmpi(fd.name,".")||boolstrcmpi(fd.name,"..")))
			{//second level of subfolders
				struct _finddata_t fd2={0};
				long hFile2; 
				int res2=-1;
				char path2[MAX_PATH],mask2[MAX_PATH];
				_snprintf(path2,sizeof(path2),"%s\\%s",path,fd.name); 
				_snprintf(mask2,sizeof(mask2),"%s\\*.msf",path2); 
				hFile2=_findfirst(mask2,&fd2);
				if (hFile2!=-1)
				{
					do {     
						AddSkinToList(hwndDlg,path2,fd2.name);
					}while (!_findnext(hFile2,&fd2));
					_findclose(hFile2);
				}//end second level
			}
		}while (!_findnext(hFile,&fd));
		_findclose(hFile);
	}
	*/
	{
		char * skinfile;
		char skinfull[MAX_PATH];
		skinfile=DBGetStringA(NULL,SKIN,"SkinFile");
		if (skinfile)
		{
			CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)skinfile, (LPARAM)skinfull);
			res=AddSkinToListFullName(hwndDlg,skinfull);

			mir_free(skinfile);
		}
	}
	return res;
}
int AddSkinToListFullName(HWND hwndDlg,char * fullName)
{
	char path[MAX_PATH]={0};
	char file[MAX_PATH]={0};
	char *buf;
	strcpy(path,fullName);
	buf=path+MyStrLen(path);  
	while (buf>path)
	{
		if (*buf=='\\')
		{
			*buf='\0';
			break;
		}
		buf--;
	}
	buf++;
	strcpy(file,buf);
	return AddSkinToList(hwndDlg,path,file);
}


int AddSkinToList(HWND hwndDlg,char * path, char* file)
{
	{
		char buf[MAX_PATH];
		_snprintf(buf,sizeof(buf),"%s\\%s",path,file);
		
	}
	{
		char fullName[MAX_PATH]={0};     
		char defskinname[MAX_PATH]={0};
		SkinListData * sd=NULL;
		sd=(SkinListData *)mir_alloc(sizeof(SkinListData));
		if (!sd) return 0;
		_snprintf(fullName,sizeof(fullName),"%s\\%s",path,file);
		memmove(defskinname,file,MyStrLen(file)-4);
		defskinname[MyStrLen(file)+1]='\0';
		GetPrivateProfileStringA("Skin_Description_Section","Name",defskinname,sd->Name,sizeof(sd->Name),fullName);
		strcpy(sd->File,fullName);
		return AddItemToTree(GetDlgItem(hwndDlg,IDC_TREE1),fullName,sd->Name,sd);
		/*{
			int i,c;
			SkinListData * sd2;
			c=SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_GETCOUNT,0,0);
			for (i=0; i<c;i++)
			{
				sd2=(SkinListData *)SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_GETITEMDATA,(WPARAM)i,0); 
				if (sd2 && boolstrcmpi(sd2->File,sd->File)) return i;
			}
		}
		{
			int item;
			item=SendDlgItemMessageA(hwndDlg,IDC_SKINS_LIST,LB_ADDSTRING,0,(LPARAM)sd->Name);   //TODO UNICODE SKIN NAMES
			if (item!=-1)
			{
				SendDlgItemMessage(hwndDlg,IDC_SKINS_LIST,LB_SETITEMDATA,(WPARAM)item,(LPARAM)sd); 
			}
			return item;
		}
		*/
	}
	return -1;
}

#define TreeView_InsertItemA(hwnd, lpis) \
	(HTREEITEM)SendMessageA((hwnd), TVM_INSERTITEMA, 0, (LPARAM)(LPTV_INSERTSTRUCTA)(lpis))

#define TreeView_GetItemA(hwnd, pitem) \
	(BOOL)SendMessageA((hwnd), TVM_GETITEMA, 0, (LPARAM)(TV_ITEM *)(pitem))

static HTREEITEM FindChild(HWND hTree, HTREEITEM Parent, char * Caption, void * data)
{
  HTREEITEM res=NULL, tmp=NULL;
  if (Parent) 
    tmp=TreeView_GetChild(hTree,Parent);
  else 
	tmp=TreeView_GetRoot(hTree);
  while (tmp)
  {
    TVITEMA tvi;
    char buf[255];
    tvi.hItem=tmp;
    tvi.mask=TVIF_TEXT|TVIF_HANDLE;
    tvi.pszText=(LPSTR)&buf;
    tvi.cchTextMax=254;
    TreeView_GetItemA(hTree,&tvi);
    if (boolstrcmpi(Caption,tvi.pszText))
	{
		if (data)
		{
			SkinListData * sd=NULL;
			TVITEM tvi={0};
			tvi.hItem=tmp;
			tvi.mask=TVIF_HANDLE|TVIF_PARAM;
			TreeView_GetItem(hTree,&tvi);
			sd=(SkinListData*)(tvi.lParam);
			if (sd)
				if (!strcmpi(sd->File,((SkinListData*)data)->File))
					return tmp;
		}
		else
			return tmp;
	}
    tmp=TreeView_GetNextSibling(hTree,tmp);
  }
  return tmp;
}


static int AddItemToTree(HWND hTree, char * folder, char * itemName, void * data)
{
	HTREEITEM rootItem=NULL;
	HTREEITEM cItem=NULL;
	char path[MAX_PATH];//,mask[MAX_PATH];
	char * ptr;
	char * ptrE;
	BOOL ext=FALSE;
	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)folder, (LPARAM)path);
	ptrE=path;
	while (*ptrE!='\\' && *ptrE!='\0' && *ptrE!=':') ptrE++;
	if (*ptrE=='\\')
	{
			*ptrE='\0';
			ptrE++;
	}
	else ptrE=path;
	ptr=ptrE;
	do 
	{
		
		while (*ptrE!='\\' && *ptrE!='\0') ptrE++;
		if (*ptrE=='\\')
		{
			*ptrE='\0';
			ptrE++;
			// find item if not - create;
			{
				cItem=FindChild(hTree,rootItem,ptr,NULL);
				if (!cItem) // not found - create node
				{
					TVINSERTSTRUCTA tvis;
					tvis.hParent=rootItem;
					tvis.hInsertAfter=TVI_ROOT;
					tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
					tvis.item.pszText=ptr;
					{
						tvis.item.lParam=(LPARAM)NULL;
					}
					cItem=TreeView_InsertItemA(hTree,&tvis);
					
				}	
				rootItem=cItem;
			}
			ptr=ptrE;
		}
		else ext=TRUE;
	}while (!ext);
	//Insert item node
	cItem=FindChild(hTree,rootItem,itemName,data);
	if (!cItem)
	{
		TVINSERTSTRUCTA tvis;
		tvis.hParent=rootItem;
		tvis.hInsertAfter=TVI_SORT;
		tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
		tvis.item.pszText=itemName;
		tvis.item.lParam=(LPARAM)data;
		return (int)TreeView_InsertItemA(hTree,&tvis);
	}
	else
		return (int)cItem;
	return 0;
}

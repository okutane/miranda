/*

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
*/
#include "commonheaders.h"

static HFONT hEmailFont=NULL;
static HCURSOR hHandCursor=NULL;

static INT_PTR CALLBACK EditUserEmailDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
			if(*(char*)lParam) SetWindowText(hwndDlg,TranslateT("Edit E-Mail Address"));
			TranslateDialogDefault(hwndDlg);
			SetDlgItemTextA(hwndDlg,IDC_EMAIL,(char*)lParam);
			EnableWindow(GetDlgItem(hwndDlg,IDOK),*(char*)lParam);
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					GetDlgItemTextA(hwndDlg,IDC_EMAIL,(char*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA),256);
					//fall through
				case IDCANCEL:
					EndDialog(hwndDlg,wParam);
				case IDC_EMAIL:
					if(HIWORD(wParam)==EN_CHANGE)
						EnableWindow(GetDlgItem(hwndDlg,IDOK),GetWindowTextLength(GetDlgItem(hwndDlg,IDC_EMAIL)));
					break;
			}
			break;
	}
	return FALSE;
}

static INT_PTR CALLBACK EditUserPhoneDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int noRecursion=0;

	switch(msg) {
		case WM_INITDIALOG:
		{	char *szText=(char*)lParam;
			int i,item,countryCount;
			struct CountryListEntry *countries;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
			if(szText[0]) SetWindowTextA(hwndDlg,"Edit Phone Number");
			TranslateDialogDefault(hwndDlg);
			if(lstrlenA(szText)>4 && !lstrcmpA(szText+lstrlenA(szText)-4," SMS")) {
				CheckDlgButton(hwndDlg,IDC_SMS,BST_CHECKED);
				szText[lstrlenA(szText)-4]='\0';
			}
			EnableWindow(GetDlgItem(hwndDlg,IDOK),szText[0]);
			SendDlgItemMessage(hwndDlg,IDC_AREA,EM_LIMITTEXT,31,0);
			SendDlgItemMessage(hwndDlg,IDC_NUMBER,EM_LIMITTEXT,63,0);
			CallService(MS_UTILS_GETCOUNTRYLIST,(WPARAM)&countryCount,(LPARAM)&countries);
			for(i=0;i<countryCount;i++) {
				if(countries[i].id==0 || countries[i].id==0xFFFF) continue;
				item=SendDlgItemMessageA(hwndDlg,IDC_COUNTRY,CB_ADDSTRING,0,(LPARAM)countries[i].szName);
				SendDlgItemMessage(hwndDlg,IDC_COUNTRY,CB_SETITEMDATA,item,countries[i].id);
			}
			SetDlgItemTextA(hwndDlg,IDC_PHONE,szText);
			return TRUE;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					{	char *szText=(char*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
						int isValid=1;
						GetDlgItemTextA(hwndDlg,IDC_PHONE,szText,252);
						if(lstrlenA(szText)<7 || szText[0]!='+') isValid=0;
						if(isValid) isValid=(lstrlenA(szText+1)==(int)strspn(szText+1,"0123456789 ()-"));
						if(!isValid) {
							MessageBox(hwndDlg,TranslateT("The phone number should start with a + and consist of numbers, spaces, brackets and hyphens only."),TranslateT("Invalid Phone Number"),MB_OK);
							break;
						}
						if(IsDlgButtonChecked(hwndDlg,IDC_SMS)) lstrcatA(szText," SMS");
					}
					//fall through
				case IDCANCEL:
					EndDialog(hwndDlg,wParam);
				case IDC_COUNTRY:
					if(HIWORD(wParam)!=CBN_SELCHANGE) break;
				case IDC_AREA:
				case IDC_NUMBER:
					if(LOWORD(wParam)!=IDC_COUNTRY && HIWORD(wParam)!=EN_CHANGE) break;
					if(noRecursion) break;
					EnableWindow(GetDlgItem(hwndDlg,IDOK),TRUE);
					{	char szPhone[96],szArea[32],szNumber[64];
						GetDlgItemTextA(hwndDlg,IDC_AREA,szArea,SIZEOF(szArea));
						GetDlgItemTextA(hwndDlg,IDC_NUMBER,szNumber,SIZEOF(szNumber));
						mir_snprintf(szPhone,SIZEOF(szPhone),"+%u (%s) %s",SendDlgItemMessage(hwndDlg,IDC_COUNTRY,CB_GETITEMDATA,SendDlgItemMessage(hwndDlg,IDC_COUNTRY,CB_GETCURSEL,0,0),0),szArea,szNumber);
						noRecursion=1;
						SetDlgItemTextA(hwndDlg,IDC_PHONE,szPhone);
						noRecursion=0;
					}
					break;
				case IDC_PHONE:
					if(HIWORD(wParam)!=EN_UPDATE) break;
					if(noRecursion) break;
					noRecursion=1;
					{	
						char szText[256],*pText=NULL,*pArea,*pNumber;
						int isValid=1;
						GetDlgItemTextA(hwndDlg,IDC_PHONE,szText,SIZEOF(szText));
						if (szText[0] != '+')
							isValid=0;

						if ( isValid ) {
							int i, country = strtol( szText+1, &pText, 10 );
							if ( pText - szText > 4 )
								isValid = 0;
							else {
								for ( i = SendDlgItemMessage( hwndDlg, IDC_COUNTRY, CB_GETCOUNT, 0, 0 )-1; i >= 0; i-- )
									if ( country == SendDlgItemMessage(hwndDlg,IDC_COUNTRY,CB_GETITEMDATA,i,0)) {
										SendDlgItemMessage(hwndDlg,IDC_COUNTRY,CB_SETCURSEL,i,0);
										break;
									}
								if ( i < 0 )
									isValid = 0;
							}
						}
						if ( isValid ) {
							pArea = pText+strcspn(pText,"0123456789");
							pText = pArea+strspn(pArea,"0123456789");
							if(*pText) {
								*pText='\0';
								pNumber = pText+1+strcspn(pText+1,"0123456789");
								SetDlgItemTextA(hwndDlg,IDC_NUMBER,pNumber);
							}
							SetDlgItemTextA(hwndDlg,IDC_AREA,pArea);
						}
						if ( !isValid ) {
							SendDlgItemMessage(hwndDlg,IDC_COUNTRY,CB_SETCURSEL,-1,0);
							SetDlgItemTextA(hwndDlg,IDC_AREA,"");
							SetDlgItemTextA(hwndDlg,IDC_NUMBER,"");
						}
					}
					noRecursion=0;
					EnableWindow(GetDlgItem(hwndDlg,IDOK),GetWindowTextLength(GetDlgItem(hwndDlg,IDC_PHONE)));
					break;
			}
			break;
	}
	return FALSE;
}

static int IsOverEmail(HWND hwndDlg,TCHAR* szEmail,int cchEmail)
{
	RECT rc;
	HWND hwndEmails;
	TCHAR szText[256];
	HDC hdc;
	SIZE textSize;
	LVHITTESTINFO hti;

	hwndEmails=GetDlgItem(hwndDlg,IDC_EMAILS);
	GetCursorPos(&hti.pt);
	ScreenToClient(hwndEmails,&hti.pt);
	GetClientRect(hwndEmails,&rc);
	if(!PtInRect(&rc,hti.pt)) return 0;
	if(ListView_SubItemHitTest(hwndEmails,&hti)==-1) return 0;
	if(hti.iSubItem!=1) return 0;
	if(!(hti.flags&LVHT_ONITEMLABEL)) return 0;
	ListView_GetSubItemRect(hwndEmails,hti.iItem,1,LVIR_LABEL,&rc);
	ListView_GetItemText(hwndEmails,hti.iItem,1,szText,SIZEOF(szText));
	hdc=GetDC(hwndEmails);
	SelectObject(hdc,hEmailFont);
	GetTextExtentPoint32(hdc,szText,lstrlen(szText),&textSize);
	ReleaseDC(hwndEmails,hdc);
	if(hti.pt.x<rc.left+textSize.cx) {
		if(szEmail && cchEmail) lstrcpyn(szEmail,szText,cchEmail);
		return 1;
	}
	return 0;
}

#define M_REMAKELISTS  (WM_USER+1)
INT_PTR CALLBACK ContactDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
			if(hEmailFont) DeleteObject(hEmailFont);
			{	LOGFONT lf;
				hEmailFont=(HFONT)SendDlgItemMessage(hwndDlg,IDC_EMAILS,WM_GETFONT,0,0);
				GetObject(hEmailFont,sizeof(lf),&lf);
				lf.lfUnderline=1;
				hEmailFont=CreateFontIndirect(&lf);
			}
			if(hHandCursor==NULL) {
				if(IsWinVer2000Plus()) hHandCursor=LoadCursor(NULL,IDC_HAND);
				else hHandCursor=LoadCursor(hMirandaInst,MAKEINTRESOURCE(IDC_HYPERLINKHAND));
			}
			TranslateDialogDefault(hwndDlg);
			{	LVCOLUMN lvc;
				RECT rc;
				GetClientRect(GetDlgItem(hwndDlg,IDC_EMAILS),&rc);
				rc.right-=GetSystemMetrics(SM_CXVSCROLL);
				lvc.mask=LVCF_WIDTH;
				lvc.cx=rc.right/4;
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_EMAILS),0,&lvc);
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_PHONES),0,&lvc);
				lvc.cx=rc.right-rc.right/4-40;
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_EMAILS),1,&lvc);
				lvc.cx=rc.right-rc.right/4-90;
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_PHONES),1,&lvc);
				lvc.cx=50;
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_PHONES),2,&lvc);
				lvc.cx=20;
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_EMAILS),2,&lvc);
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_EMAILS),3,&lvc);
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_PHONES),3,&lvc);
				ListView_InsertColumn(GetDlgItem(hwndDlg,IDC_PHONES),4,&lvc);
			}
			return TRUE;
		case M_REMAKELISTS:
			{	char *szProto;
			LVITEM lvi;
			int i;
			char idstr[33];
			TCHAR idstr2[33];
			DBVARIANT dbv;
			HANDLE hContact=(HANDLE)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);

			if (hContact != NULL) {
				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
				if (szProto==NULL) break;
				//e-mails
				ListView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_EMAILS));
				lvi.mask=LVIF_TEXT|LVIF_PARAM;
				lvi.lParam=(LPARAM)(-1);
				lvi.iSubItem=0;
				lvi.iItem=0;
				for(i=-1;;i++) {
					if(i==-1) {
						if(DBGetContactSettingTString(hContact,szProto,"e-mail",&dbv))
							continue;
						lvi.pszText=TranslateT("Primary");
					}
					else {
						wsprintfA(idstr, "e-mail%d", i );
						if(DBGetContactSettingTString(hContact,szProto,idstr,&dbv))
							break;
						lvi.pszText=idstr2;
						wsprintf(idstr2,_T("%d"),i+2);
					}
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_EMAILS),&lvi);
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_EMAILS),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				lvi.iSubItem=0;
				for(i=0;;i++) {
					lvi.lParam=i;
					wsprintfA(idstr,"Mye-mail%d",i);
					if(DBGetContactSettingTString(hContact,"UserInfo",idstr,&dbv))
						break;
					lvi.pszText=idstr2;
					wsprintf(idstr2,TranslateT("Custom %d"),i+1);
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_EMAILS),&lvi);
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_EMAILS),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				lvi.mask=LVIF_PARAM;
				lvi.lParam=(LPARAM)(-2);
				ListView_InsertItem(GetDlgItem(hwndDlg,IDC_EMAILS),&lvi);
				//phones
				ListView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_PHONES));
				lvi.mask=LVIF_TEXT|LVIF_PARAM;
				lvi.lParam=(LPARAM)(-1);
				lvi.iSubItem=0;
				lvi.iItem=0;
				if(!DBGetContactSettingTString(hContact,szProto,"Phone",&dbv)) {
					lvi.pszText=TranslateT("Primary");
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_PHONES),&lvi);
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				if(!DBGetContactSettingTString(hContact,szProto,"Fax",&dbv)) {
					lvi.pszText=TranslateT("Fax");
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_PHONES),&lvi);
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				if(!DBGetContactSettingTString(hContact,szProto,"Cellular",&dbv)) {
					lvi.pszText=TranslateT("Mobile");
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_PHONES),&lvi);
					if(lstrlenA(dbv.pszVal)>4 && !lstrcmpA(dbv.pszVal+lstrlenA(dbv.pszVal)-4," SMS")) {
						ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,2,_T("y"));
						dbv.ptszVal[lstrlen(dbv.ptszVal)-4]='\0';
					}
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				if(!DBGetContactSettingTString(hContact,szProto,"CompanyPhone",&dbv)) {
					lvi.pszText=TranslateT("Work Phone");
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_PHONES),&lvi);
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				if(!DBGetContactSettingTString(hContact,szProto,"CompanyFax",&dbv)) {
					lvi.pszText=TranslateT("Work Fax");
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_PHONES),&lvi);
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				lvi.iSubItem=0;
				for(i=0;;i++) {
					lvi.lParam=i;
					wsprintfA(idstr,"MyPhone%d",i);
					if(DBGetContactSettingTString(hContact,"UserInfo",idstr,&dbv))
						break;
					lvi.pszText=idstr2;
					wsprintf(idstr2,TranslateT("Custom %d"),i+1);
					ListView_InsertItem(GetDlgItem(hwndDlg,IDC_PHONES),&lvi);
					if(lstrlen(dbv.ptszVal)>4 && !lstrcmp(dbv.ptszVal+lstrlen(dbv.ptszVal)-4,_T(" SMS"))) {
						ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,2,_T("y"));
						dbv.ptszVal[lstrlen(dbv.ptszVal)-4]='\0';
					}
					ListView_SetItemText(GetDlgItem(hwndDlg,IDC_PHONES),lvi.iItem,1,dbv.ptszVal);
					DBFreeVariant(&dbv);
					lvi.iItem++;
				}
				lvi.mask=LVIF_PARAM;
				lvi.lParam=(LPARAM)(-2);
				ListView_InsertItem(GetDlgItem(hwndDlg,IDC_PHONES),&lvi);
			}
			break;
		}
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
						case PSN_INFOCHANGED:
							SendMessage(hwndDlg,M_REMAKELISTS,0,0);
							break;
					}
					break;
				case IDC_EMAILS:
				case IDC_PHONES:
					switch (((LPNMHDR)lParam)->code) {
						case NM_CUSTOMDRAW:
						{	NMLVCUSTOMDRAW *nm=(NMLVCUSTOMDRAW*)lParam;
							switch(nm->nmcd.dwDrawStage) {
								case CDDS_PREPAINT:
								case CDDS_ITEMPREPAINT:
									SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,CDRF_NOTIFYSUBITEMDRAW);
									return TRUE;
								case CDDS_SUBITEM|CDDS_ITEMPREPAINT:
								{
									RECT rc;
									ListView_GetSubItemRect(nm->nmcd.hdr.hwndFrom,nm->nmcd.dwItemSpec,nm->iSubItem,LVIR_LABEL,&rc);
									if(nm->iSubItem==1 && nm->nmcd.hdr.idFrom==IDC_EMAILS) {
										HFONT hoFont;
										TCHAR szText[256];
										ListView_GetItemText(nm->nmcd.hdr.hwndFrom,nm->nmcd.dwItemSpec,nm->iSubItem,szText,SIZEOF(szText));
										hoFont=(HFONT)SelectObject(nm->nmcd.hdc,hEmailFont);
										SetTextColor(nm->nmcd.hdc,RGB(0,0,255));
										DrawText(nm->nmcd.hdc,szText,-1,&rc,DT_END_ELLIPSIS|DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_TOP);
										SelectObject(nm->nmcd.hdc,hoFont);
										SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,CDRF_SKIPDEFAULT);
										return TRUE;
									}

									HICON hIcon = NULL;
									if(nm->nmcd.lItemlParam==(LPARAM)(-2) && nm->iSubItem-3==(nm->nmcd.hdr.idFrom==IDC_PHONES))
										hIcon = LoadSkinIcon( SKINICON_OTHER_ADDCONTACT );
									else if(nm->iSubItem>1 && nm->nmcd.lItemlParam!=(LPARAM)(-1) && nm->nmcd.lItemlParam!=(LPARAM)(-2)) {
										static int iconResources[3]={SKINICON_OTHER_RENAME,SKINICON_OTHER_DELETE};
										if(nm->iSubItem==2 && nm->nmcd.hdr.idFrom==IDC_PHONES) {
											TCHAR szText[2];
											ListView_GetItemText(nm->nmcd.hdr.hwndFrom,nm->nmcd.dwItemSpec,nm->iSubItem,szText,SIZEOF(szText));
											if(szText[0]) hIcon = LoadSkinIcon( SKINICON_OTHER_SMS );
										}
										else hIcon = LoadSkinIcon( iconResources[nm->iSubItem-3+(nm->nmcd.hdr.idFrom==IDC_EMAILS)] );
									}
									else break;
									DrawIconEx(nm->nmcd.hdc,(rc.left+rc.right-GetSystemMetrics(SM_CXSMICON))/2,(rc.top+rc.bottom-GetSystemMetrics(SM_CYSMICON))/2,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
									IconLib_ReleaseIcon(hIcon, 0);
									SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,CDRF_SKIPDEFAULT);
									return TRUE;
								}
							}
							break;
						}
						case NM_CLICK:
						{	NMLISTVIEW *nm=(NMLISTVIEW*)lParam;
							LVITEM lvi;
							TCHAR szEmail[256];
							HANDLE hContact=(HANDLE)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
							char *szIdTemplate=nm->hdr.idFrom==IDC_PHONES?"MyPhone%d":"Mye-mail%d";
							LVHITTESTINFO hti;

							if(IsOverEmail(hwndDlg,szEmail,SIZEOF(szEmail))) {
								TCHAR szExec[264];
								lstrcpy(szExec, _T("mailto:"));
								lstrcat(szExec,szEmail);
								ShellExecute(hwndDlg,_T("open"),szExec,NULL,NULL,SW_SHOW);
								break;
							}
							if(nm->iSubItem<2) break;
							hti.pt.x=(short)LOWORD(GetMessagePos());
							hti.pt.y=(short)HIWORD(GetMessagePos());
							ScreenToClient(nm->hdr.hwndFrom,&hti.pt);
							if(ListView_SubItemHitTest(nm->hdr.hwndFrom,&hti)==-1) break;
							lvi.mask=LVIF_PARAM;
							lvi.iItem=hti.iItem;
							lvi.iSubItem=0;
							ListView_GetItem(nm->hdr.hwndFrom,&lvi);
							if(lvi.lParam==(LPARAM)(-1)) break;
							if(lvi.lParam==(LPARAM)(-2)) {
								if(hti.iSubItem-3==(nm->hdr.idFrom==IDC_PHONES)) {
									//add
									char szNewData[256]="",idstr[33];
									int i;
									DBVARIANT dbv;
									if(IDOK!=DialogBoxParam(hMirandaInst,MAKEINTRESOURCE(nm->hdr.idFrom==IDC_PHONES?IDD_ADDPHONE:IDD_ADDEMAIL),hwndDlg,nm->hdr.idFrom==IDC_PHONES?EditUserPhoneDlgProc:EditUserEmailDlgProc,(LPARAM)szNewData))
										break;
									for(i=0;;i++) {
										wsprintfA(idstr,szIdTemplate,i);
										if(DBGetContactSettingString(hContact,"UserInfo",idstr,&dbv)) break;
										DBFreeVariant(&dbv);
									}
									DBWriteContactSettingString(hContact,"UserInfo",idstr,szNewData);
									SendMessage(hwndDlg,M_REMAKELISTS,0,0);
								}
							}
							else {
								if(hti.iSubItem-3==(nm->hdr.idFrom==IDC_PHONES)) {
									//delete
									int i;
									char idstr[33];
									DBVARIANT dbv;
									for(i=lvi.lParam;;i++) {
										wsprintfA(idstr,szIdTemplate,i+1);
										if(DBGetContactSettingString(hContact,"UserInfo",idstr,&dbv)) break;
										wsprintfA(idstr,szIdTemplate,i);
										DBWriteContactSettingString(hContact,"UserInfo",idstr,dbv.pszVal);
										DBFreeVariant(&dbv);
									}
									wsprintfA(idstr,szIdTemplate,i);
									DBDeleteContactSetting(hContact,"UserInfo",idstr);
									SendMessage(hwndDlg,M_REMAKELISTS,0,0);
								}
								else if(hti.iSubItem-2==(nm->hdr.idFrom==IDC_PHONES)) {
									//edit
									char szText[256],idstr[33];
									DBVARIANT dbv;
									wsprintfA(idstr,szIdTemplate,lvi.lParam);
									if(DBGetContactSettingString(hContact,"UserInfo",idstr,&dbv)) break;
									lstrcpynA(szText,dbv.pszVal,SIZEOF(szText));
									DBFreeVariant(&dbv);
									if(IDOK!=DialogBoxParam(hMirandaInst,MAKEINTRESOURCE(nm->hdr.idFrom==IDC_PHONES?IDD_ADDPHONE:IDD_ADDEMAIL),hwndDlg,nm->hdr.idFrom==IDC_PHONES?EditUserPhoneDlgProc:EditUserEmailDlgProc,(LPARAM)szText))
										break;
									DBWriteContactSettingString(hContact,"UserInfo",idstr,szText);
									SendMessage(hwndDlg,M_REMAKELISTS,0,0);
								}
							}
							break;
						}
					}
					break;
			}
			break;
		case WM_SETCURSOR:
			if(LOWORD(lParam)!=HTCLIENT) break;
			if(GetForegroundWindow()==GetParent(hwndDlg)) {
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(hwndDlg,&pt);
				SetFocus(ChildWindowFromPoint(hwndDlg,pt));	  //ugly hack because listviews ignore their first click
			}
			if(IsOverEmail(hwndDlg,NULL,0)) {
				SetCursor(hHandCursor);
				SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,TRUE);
				return TRUE;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDCANCEL:
					SendMessage(GetParent(hwndDlg),msg,wParam,lParam);
					break;
			}
			break;
	}
	return FALSE;
}

/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2016 Miranda ICQ/IM project,
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
 * Simple editor for the message log templates
 *
 */

#include "commonheaders.h"
#pragma hdrstop

/*
* hardcoded default set of templates for both LTR and RTL.
* cannot be changed and may be used at any time to "revert" to a working layout
*/

char *TemplateNames[] = {
	"Message In",
	"Message Out",
	"Group In (Start)",
	"Group Out (Start)",
	"Group In (Inner)",
	"Group Out (Inner)",
	"Status change",
	"Error message"
};

TTemplateSet LTR_Default = { TRUE,
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%S %T%|%M"),
							_T("%S %T%|%M"),
							_T("%I %S %&D, %&T, %N %M%! "),
							_T("%I%S %D, %T, %e%l%M"),
							"Default LTR"
						  };

TTemplateSet RTL_Default = { TRUE,
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%S %T%|%M"),
							_T("%S %T%|%M"),
							_T("%I%S %D, %T, %N %M%! "),
							_T("%I%S %D, %T, %e%l%M"),
							"Default RTL"
						  };

TTemplateSet LTR_Active, RTL_Active;
static int                      helpActive = 0;


/*
* loads template set overrides from hContact into the given set of already existing
* templates
*/

static void LoadTemplatesFrom(TTemplateSet *tSet, HANDLE hContact, int rtl)
{
	DBVARIANT dbv = {0};
	int i;

	for (i = 0; i <= TMPL_ERRMSG; i++) {
		if (M->GetTString(hContact, rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[i], &dbv))
			continue;
		if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR)
			mir_sntprintf(tSet->szTemplates[i], TEMPLATE_LENGTH, _T("%s"), dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
}

void LoadDefaultTemplates()
{
	int i;

	LTR_Active = LTR_Default;
	RTL_Active = RTL_Default;

	if (M->GetByte(RTLTEMPLATES_MODULE, "setup", 0) < 2) {
		for (i = 0; i <= TMPL_ERRMSG; i++)
			M->WriteTString(NULL, RTLTEMPLATES_MODULE, TemplateNames[i], RTL_Default.szTemplates[i]);
		M->WriteByte(RTLTEMPLATES_MODULE, "setup", 2);
	}
	if (M->GetByte(TEMPLATES_MODULE, "setup", 0) < 2) {
		for (i = 0; i <= TMPL_ERRMSG; i++)
			M->WriteTString(NULL, TEMPLATES_MODULE, TemplateNames[i], LTR_Default.szTemplates[i]);
		M->WriteByte(TEMPLATES_MODULE, "setup", 2);
	}
	LoadTemplatesFrom(&LTR_Active, (HANDLE)0, 0);
	LoadTemplatesFrom(&RTL_Active, (HANDLE)0, 1);
}

INT_PTR CALLBACK DlgProcTemplateEditor(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct TWindowData *dat = 0;
	TemplateEditorInfo *teInfo = 0;
	TTemplateSet *tSet;
	int i;
	dat = (struct TWindowData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	/*
	* since this dialog needs a struct MessageWindowData * but has no container, we can store
	* the extended info struct in pContainer *)
	*/
	if (dat) {
		teInfo = (TemplateEditorInfo *)dat->pContainer;
		tSet = teInfo->rtl ? dat->pContainer->rtl_templates : dat->pContainer->ltr_templates;
	}

	switch (msg) {
		case WM_INITDIALOG: {
			TemplateEditorNew *teNew = (TemplateEditorNew *)lParam;
			COLORREF url_visited = RGB(128, 0, 128);
			COLORREF url_unvisited = RGB(0, 0, 255);
			dat = (struct TWindowData *) malloc(sizeof(struct TWindowData));

			TranslateDialogDefault(hwndDlg);

			ZeroMemory((void *) dat, sizeof(struct TWindowData));
			dat->pContainer = (struct TContainerData *)malloc(sizeof(struct TContainerData));
			ZeroMemory((void *)dat->pContainer, sizeof(struct TContainerData));
			teInfo = (TemplateEditorInfo *)dat->pContainer;
			ZeroMemory((void *)teInfo, sizeof(TemplateEditorInfo));
			teInfo->hContact = teNew->hContact;
			teInfo->rtl = teNew->rtl;
			teInfo->hwndParent = teNew->hwndParent;

			LoadOverrideTheme(dat->pContainer);
			/*
			* set hContact to the first found contact so that we can use the Preview window properly
			* also, set other parameters needed by the streaming function to display events
			*/

			SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK);
			SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
			SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_EXLIMITTEXT, 0, 0x80000000);

			dat->hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
			dat->szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)dat->hContact, 0);
			while(dat->szProto == 0 && dat->hContact != 0) {
				dat->hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)dat->hContact, 0);
				dat->szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)dat->hContact, 0);
			}
			dat->dwFlags = dat->pContainer->theme.dwFlags;

			dat->cache = CContactCache::getContactCache(dat->hContact);
			dat->cache->updateState();
			dat->cache->updateUIN();
			dat->cache->updateStats(TSessionStats::INIT_TIMER);
			GetMYUIN(dat);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dat);
			ShowWindow(hwndDlg, SW_SHOW);
			SendDlgItemMessage(hwndDlg, IDC_EDITTEMPLATE, EM_LIMITTEXT, (WPARAM)TEMPLATE_LENGTH - 1, 0);
			SetWindowText(hwndDlg, CTranslator::getOpt(CTranslator::OPT_TEMP_TITLE));
			Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
			for (i = 0; i <= TMPL_ERRMSG; i++) {
				SendDlgItemMessageA(hwndDlg, IDC_TEMPLATELIST, LB_ADDSTRING, 0, (LPARAM)Translate(TemplateNames[i]));
				SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_SETITEMDATA, i, (LPARAM)i);
			}
			Utils::enableDlgControl(teInfo->hwndParent, IDC_MODIFY, FALSE);
			Utils::enableDlgControl(teInfo->hwndParent, IDC_RTLMODIFY, FALSE);

			SendDlgItemMessage(hwndDlg, IDC_COLOR1, CPM_SETCOLOUR, 0, M->GetDword("cc1", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR2, CPM_SETCOLOUR, 0, M->GetDword("cc2", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR3, CPM_SETCOLOUR, 0, M->GetDword("cc3", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR4, CPM_SETCOLOUR, 0, M->GetDword("cc4", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR5, CPM_SETCOLOUR, 0, M->GetDword("cc5", SRMSGDEFSET_BKGCOLOUR));
			SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
			return(TRUE);
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
				case IDC_RESETALLTEMPLATES:
					if (MessageBox(0, CTranslator::getOpt(CTranslator::OPT_TEMP_RESET),
								   CTranslator::getOpt(CTranslator::OPT_TEMP_TITLE), MB_YESNO | MB_ICONQUESTION) == IDYES) {
						M->WriteByte(teInfo->rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, "setup", 0);
						LoadDefaultTemplates();
						MessageBox(0, CTranslator::getOpt(CTranslator::OPT_TEMP_WASRESET),
								   CTranslator::getOpt(CTranslator::OPT_TEMP_TITLE), MB_OK);
						DestroyWindow(hwndDlg);
					}
					break;
				case IDC_TEMPLATELIST:
					switch (HIWORD(wParam)) {
						case LBN_DBLCLK: {
							LRESULT iIndex = SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
							if (iIndex != LB_ERR) {
								SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[iIndex]);
								teInfo->inEdit = iIndex;
								teInfo->changed = FALSE;
								teInfo->selchanging = FALSE;
								SetFocus(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE));
								SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, FALSE, 0);
							}
							break;
						}
						case LBN_SELCHANGE: {
							LRESULT iIndex = SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
							teInfo->selchanging = TRUE;
							if (iIndex != LB_ERR) {
								SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[iIndex]);
								teInfo->inEdit = iIndex;
								teInfo->changed = FALSE;
							}
							SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
							break;
						}
					}
					break;
				case IDC_VARIABLESHELP:
					CallService(MS_UTILS_OPENURL, 0, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/Templates");
					break;
				case IDC_EDITTEMPLATE:
					if (HIWORD(wParam) == EN_CHANGE) {
						if (!teInfo->selchanging) {
							teInfo->changed = TRUE;
							teInfo->updateInfo[teInfo->inEdit] = TRUE;
							Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, TRUE);
							Utils::enableDlgControl(hwndDlg, IDC_FORGET, TRUE);
							Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, FALSE);
							Utils::enableDlgControl(hwndDlg, IDC_REVERT, TRUE);
						}
						InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
					}
					break;
				case IDC_SAVETEMPLATE: {
					TCHAR newTemplate[TEMPLATE_LENGTH + 2];

					GetWindowText(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), newTemplate, TEMPLATE_LENGTH);
					CopyMemory(tSet->szTemplates[teInfo->inEdit], newTemplate, sizeof(TCHAR) * TEMPLATE_LENGTH);
					teInfo->changed = FALSE;
					teInfo->updateInfo[teInfo->inEdit] = FALSE;
					Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, TRUE);
					Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
					InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
					M->WriteTString(teInfo->hContact, teInfo->rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[teInfo->inEdit], newTemplate);
					SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
					break;
				}
				case IDC_FORGET: {
					teInfo->changed = FALSE;
					teInfo->updateInfo[teInfo->inEdit] = FALSE;
					teInfo->selchanging = TRUE;
					SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[teInfo->inEdit]);
					SetFocus(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE));
					InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, TRUE);
					Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
					teInfo->selchanging = FALSE;
					SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
					break;
				}
				case IDC_REVERT: {
					teInfo->changed = FALSE;
					teInfo->updateInfo[teInfo->inEdit] = FALSE;
					teInfo->selchanging = TRUE;
					CopyMemory(tSet->szTemplates[teInfo->inEdit], LTR_Default.szTemplates[teInfo->inEdit], sizeof(TCHAR) * TEMPLATE_LENGTH);
					SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[teInfo->inEdit]);
					DBDeleteContactSetting(teInfo->hContact, teInfo->rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[teInfo->inEdit]);
					SetFocus(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE));
					InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
					teInfo->selchanging = FALSE;
					Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, TRUE);
					SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
					break;
				}
				case IDC_UPDATEPREVIEW:
					SendMessage(hwndDlg, DM_UPDATETEMPLATEPREVIEW, 0, 0);
					break;
			}
			break;
		case WM_DRAWITEM: {
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			int iItem = dis->itemData;
			HBRUSH bkg, oldBkg;
			SetBkMode(dis->hDC, TRANSPARENT);
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
			if (dis->itemState & ODS_SELECTED) {
				if (teInfo->updateInfo[iItem] == TRUE) {
					bkg = CreateSolidBrush(RGB(255, 0, 0));
					oldBkg = (HBRUSH)SelectObject(dis->hDC, bkg);
					FillRect(dis->hDC, &dis->rcItem, bkg);
					SelectObject(dis->hDC, oldBkg);
					DeleteObject(bkg);
				} else
					FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));

				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			} else {
				if (teInfo->updateInfo[iItem] == TRUE)
					SetTextColor(dis->hDC, RGB(255, 0, 0));
				else
					SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
			}
			char *pszName = Translate(TemplateNames[iItem]);
			TextOutA(dis->hDC, dis->rcItem.left, dis->rcItem.top, pszName, lstrlenA(pszName));
			return(TRUE);
		}
		case DM_UPDATETEMPLATEPREVIEW: {
			DBEVENTINFO dbei = {0};
			int iIndex = SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
			TCHAR szTemp[TEMPLATE_LENGTH + 2];

			if (teInfo->changed) {
				CopyMemory(szTemp, tSet->szTemplates[teInfo->inEdit], TEMPLATE_LENGTH * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[teInfo->inEdit], TEMPLATE_LENGTH);
			}
			dbei.szModule = dat->szProto;
			dbei.timestamp = time(NULL);
			dbei.eventType = (iIndex == 6) ? EVENTTYPE_STATUSCHANGE : EVENTTYPE_MESSAGE;
			dbei.eventType = (iIndex == 7) ? EVENTTYPE_ERRMSG : dbei.eventType;
			if (dbei.eventType == EVENTTYPE_ERRMSG)
				dbei.szModule = "Sample error message";
			dbei.cbSize = sizeof(dbei);
			dbei.pBlob = (iIndex == 6) ? (BYTE *)"is now offline (was online)" : (BYTE *)"The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";
			dbei.cbBlob = lstrlenA((char *)dbei.pBlob) + 1;
			dbei.flags = (iIndex == 1 || iIndex == 3 || iIndex == 5) ? DBEF_SENT : 0;
			dbei.flags |= (teInfo->rtl ? DBEF_RTL : 0);
			dat->lastEventTime = (iIndex == 4 || iIndex == 5) ? time(NULL) - 1 : 0;
			dat->iLastEventType = MAKELONG(dbei.flags, dbei.eventType);
			SetWindowText(GetDlgItem(hwndDlg, IDC_PREVIEW), _T(""));
			dat->dwFlags = MWF_LOG_ALL;
			dat->dwFlags = (teInfo->rtl ? dat->dwFlags | MWF_LOG_RTL : dat->dwFlags & ~MWF_LOG_RTL);
			dat->dwFlags = (iIndex == 0 || iIndex == 1) ? dat->dwFlags & ~MWF_LOG_GROUPMODE : dat->dwFlags | MWF_LOG_GROUPMODE;
			mir_sntprintf(dat->szMyNickname, safe_sizeof(dat->szMyNickname), _T("My Nickname"));
			StreamInEvents(hwndDlg, 0, 1, 1, &dbei);
			SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_SETSEL, -1, -1);
			if (teInfo->changed)
				CopyMemory(tSet->szTemplates[teInfo->inEdit], szTemp, TEMPLATE_LENGTH * sizeof(TCHAR));
			break;
		}
		case WM_DESTROY:
			Utils::enableDlgControl(teInfo->hwndParent, IDC_MODIFY, TRUE);
			Utils::enableDlgControl(teInfo->hwndParent, IDC_RTLMODIFY, TRUE);
			if (dat->pContainer)
				free(dat->pContainer);
			if (dat)
				free(dat);

			M->WriteDword(SRMSGMOD_T, "cc1", SendDlgItemMessage(hwndDlg, IDC_COLOR1, CPM_GETCOLOUR, 0, 0));
			M->WriteDword(SRMSGMOD_T, "cc2", SendDlgItemMessage(hwndDlg, IDC_COLOR2, CPM_GETCOLOUR, 0, 0));
			M->WriteDword(SRMSGMOD_T, "cc3", SendDlgItemMessage(hwndDlg, IDC_COLOR3, CPM_GETCOLOUR, 0, 0));
			M->WriteDword(SRMSGMOD_T, "cc4", SendDlgItemMessage(hwndDlg, IDC_COLOR4, CPM_GETCOLOUR, 0, 0));
			M->WriteDword(SRMSGMOD_T, "cc5", SendDlgItemMessage(hwndDlg, IDC_COLOR5, CPM_GETCOLOUR, 0, 0));

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			break;
	}
	return(FALSE);
}

/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
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
 * The dialog to customize per container options
 *
 */

#include "commonheaders.h"
#pragma hdrstop

static void MY_CheckDlgButton(HWND hWnd, UINT id, int iCheck)
{
	CheckDlgButton(hWnd, id, iCheck ? BST_CHECKED : BST_UNCHECKED);
}

static void ReloadGlobalContainerSettings(bool fForceReconfig)
{
	struct TContainerData *pC = pFirstContainer;

	while (pC) {
		if (!pC->settings->fPrivate) {
			Utils::SettingsToContainer(pC);
			if(fForceReconfig)
				SendMessage(pC->hwnd, DM_CONFIGURECONTAINER, 0, 0);
			else
				SendMessage(pC->hwnd, WM_SIZE, 0, 1);
			BroadCastContainer(pC, DM_SETINFOPANEL, 0, 0);
		}
		pC = pC->pNextContainer;
	}
}

/**
 * Apply a container setting
 *
 * @param pContainer ContainerWindowData *: the container
 * @param flags      DWORD: the flag values to set or clear
 * @param mode       int: bit #0 set/clear, any bit from 16-31 indicates that dwFlagsEx should be affected
 * @param fForceResize
 */
void TSAPI ApplyContainerSetting(TContainerData *pContainer, DWORD flags, UINT mode, bool fForceResize)
{
	DWORD dwOld = pContainer->dwFlags;
	bool  isEx = (mode & 0xffff0000) ? true : false;
	bool  set = (mode & 0x01) ? true : false;

	if (!pContainer->settings->fPrivate) {
		if(!isEx)
			pContainer->dwFlags = (set ? pContainer->dwFlags | flags : pContainer->dwFlags & ~flags);
		else
			pContainer->dwFlagsEx = (set ? pContainer->dwFlagsEx | flags : pContainer->dwFlagsEx & ~flags);

		Utils::ContainerToSettings(pContainer);
		if (flags & CNT_INFOPANEL)
			BroadCastContainer(pContainer, DM_SETINFOPANEL, 0, 0);
		if (flags & CNT_SIDEBAR) {
			struct TContainerData *pC = pFirstContainer;
			while (pC) {
				if (!pC->settings->fPrivate) {
					SendMessage(pC->hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
				}
				pC = pC->pNextContainer;
			}
		}
		else
			ReloadGlobalContainerSettings(fForceResize);
	}
	else {
		if(!isEx)
			pContainer->dwFlags = (set ? pContainer->dwFlags | flags : pContainer->dwFlags & ~flags);
		else
			pContainer->dwFlagsEx = (set ? pContainer->dwFlagsEx | flags : pContainer->dwFlagsEx & ~flags);
		Utils::ContainerToSettings(pContainer);
		if (flags & CNT_SIDEBAR)
			SendMessage(pContainer->hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
		else
			SendMessage(pContainer->hwnd, DM_CONFIGURECONTAINER, 0, 0);
		if (flags & CNT_INFOPANEL)
			BroadCastContainer(pContainer, DM_SETINFOPANEL, 0, 0);
	}

	if(fForceResize)
		SendMessage(pContainer->hwnd, WM_SIZE, 0, 1);

	BroadCastContainer(pContainer, DM_BBNEEDUPDATE, 0, 0);
}

#define NR_O_PAGES 10
#define NR_O_OPTIONSPERPAGE 10

static struct _tagPages {
	UINT idTitle;
	UINT idDesc;
	UINT uIds[10];
} o_pages[] = {
	{ CTranslator::CNT_OPT_TITLE_GEN, CTranslator::STR_LAST, IDC_O_NOTABS, IDC_O_STICKY, IDC_VERTICALMAX, 0, 0, 0, 0, 0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_LAYOUT, CTranslator::STR_LAST, IDC_CNTNOSTATUSBAR, IDC_HIDEMENUBAR, IDC_UIDSTATUSBAR, IDC_HIDETOOLBAR, IDC_INFOPANEL, IDC_BOTTOMTOOLBAR, 0, 0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_TABS, CTranslator::CNT_OPT_DESC_TABS, IDC_TABMODE, IDC_O_TABMODE, IDC_O_SBARLAYOUT, IDC_SBARLAYOUT, IDC_FLASHICON, IDC_FLASHLABEL, IDC_SINGLEROWTAB, IDC_BUTTONTABS, IDC_STYLEDTABS, IDC_CLOSEBUTTONONTABS},
	{ CTranslator::CNT_OPT_TITLE_NOTIFY, CTranslator::CNT_OPT_DESC_NOTIFY, IDC_O_DONTREPORT, IDC_DONTREPORTUNFOCUSED2, IDC_ALWAYSPOPUPSINACTIVE, IDC_O_EXPLAINGLOBALNOTIFY, 0, 0, 0, 0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_FLASHING, CTranslator::STR_LAST, IDC_O_FLASHDEFAULT, IDC_O_FLASHALWAYS, IDC_O_FLASHNEVER, 0, 0, 0, 0, 0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_TITLEBAR, CTranslator::STR_LAST, IDC_O_HIDETITLE, IDC_STATICICON, IDC_TITLEFORMAT, IDC_O_TITLEBARFORMAT, 0, 0, 0, 0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_THEME, CTranslator::CNT_OPT_DESC_THEME, IDC_THEME, IDC_SELECTTHEME, IDC_USEGLOBALSIZE, IDC_SAVESIZEASGLOBAL, IDC_LABEL_PRIVATETHEME, 0, 0, 0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_TRANS, CTranslator::CNT_OPT_DESC_TRANS, IDC_TRANSPARENCY, IDC_TRANSPARENCY_ACTIVE, IDC_TRANSPARENCY_INACTIVE, IDC_TLABEL_ACTIVE, IDC_TLABEL_INACTIVE, IDC_TSLABEL_ACTIVE, IDC_TSLABEL_INACTIVE,0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_AVATARS, CTranslator::STR_LAST, IDC_O_STATIC_AVATAR, IDC_O_STATIC_OWNAVATAR, IDC_AVATARMODE, IDC_OWNAVATARMODE, 0, 0, 0, 0, 0, 0},
	{ CTranslator::CNT_OPT_TITLE_SOUNDS, CTranslator::STR_LAST, IDC_O_ENABLESOUNDS, IDC_O_SOUNDSMINIMIZED, IDC_O_SOUNDSUNFOCUSED, IDC_O_SOUNDSINACTIVE, IDC_O_SOUNDSFOCUSED, 0, 0, 0, 0, 0},
};

static void ShowPage(HWND hwndDlg, int iPage, BOOL fShow)
{
	if (iPage >= 0 && iPage < NR_O_PAGES) {
		for (int i = 0; i < NR_O_OPTIONSPERPAGE && o_pages[iPage].uIds[i] != 0; i++)
			Utils::showDlgControl(hwndDlg, o_pages[iPage].uIds[i], fShow ? SW_SHOW : SW_HIDE);
	}
	if (fShow) {
		SetDlgItemText(hwndDlg, IDC_TITLEBOX, CTranslator::get(o_pages[iPage].idTitle));
		if (o_pages[iPage].idDesc != CTranslator::STR_LAST)
			SetDlgItemText(hwndDlg, IDC_DESC, CTranslator::get(o_pages[iPage].idDesc));
		else
			SetDlgItemText(hwndDlg, IDC_DESC, _T(""));
	}
	Utils::showDlgControl(hwndDlg, IDC_O_EXPLAINGLOBALNOTIFY, (iPage == 3 && nen_options.bWindowCheck) ? SW_SHOW : SW_HIDE);
}

INT_PTR CALLBACK DlgProcContainerOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct TContainerData *pContainer = 0;
	HWND   hwndTree = GetDlgItem(hwndDlg, IDC_SECTIONTREE);
	pContainer = (struct TContainerData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG: {
			TCHAR 			szNewTitle[128];
			TContainerData *pContainer = 0;
			int   			i, j;
			TVINSERTSTRUCT 	tvis = {0};
			HTREEITEM 		hItem;
			int				nr_layouts = 0;
			const 			TSideBarLayout* sblayouts = CSideBar::getLayouts(nr_layouts);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) lParam);
			pContainer = (TContainerData *) lParam;
			pContainer->hWndOptions = hwndDlg;
			TranslateDialogDefault(hwndDlg);
			mir_sntprintf(szNewTitle, SIZEOF(szNewTitle), _T("\t%s"), pContainer->szName);
			SetWindowText(hwndDlg, CTranslator::get(CTranslator::CNT_OPT_TITLE));
			mir_sntprintf(szNewTitle, SIZEOF(szNewTitle), CTranslator::get(CTranslator::CNT_OPT_HEADERBAR), pContainer->szName);
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, szNewTitle);
			Utils::enableDlgControl(hwndDlg, IDC_O_HIDETITLE, CSkin::m_frameSkins ? FALSE : TRUE);
			CheckDlgButton(hwndDlg, IDC_CNTPRIVATE, pContainer->settings->fPrivate ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::get(CTranslator::CNT_OPT_TABSTOP));
			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::get(CTranslator::CNT_OPT_TABSBOTTOM));
			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::get(CTranslator::CNT_OPT_TABSLEFT));
			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)CTranslator::get(CTranslator::CNT_OPT_TABSRIGHT));

			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1,
							   (LPARAM)CTranslator::getOpt(CTranslator::OPT_GEN_GLOBALLY_ON));
			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1,
							   (LPARAM)CTranslator::getOpt(CTranslator::OPT_GEN_ON_IF_PRESENT));
			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1,
							   (LPARAM)CTranslator::getOpt(CTranslator::OPT_GEN_GLOBALLY_OFF));
			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1,
							   (LPARAM)CTranslator::getOpt(CTranslator::OPT_GEN_ON_ALWAYS_BOTTOM));

			SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_INSERTSTRING, -1,
							   (LPARAM)CTranslator::getOpt(CTranslator::OPT_GEN_ON_IF_PRESENT));
			SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_INSERTSTRING, -1,
							   (LPARAM)CTranslator::getOpt(CTranslator::OPT_GEN_DONT_SHOW));

			for(i = 0; i < nr_layouts; i++)
				SendDlgItemMessage(hwndDlg, IDC_SBARLAYOUT, CB_INSERTSTRING, -1, (LPARAM)TranslateTS(sblayouts[i].szName));

			/* bits 24 - 31 of dwFlagsEx hold the side bar layout id */
			SendDlgItemMessage(hwndDlg, IDC_SBARLAYOUT, CB_SETCURSEL, (WPARAM)((pContainer->settings->dwFlagsEx & 0xff000000) >> 24), 0);


			SendMessage(hwndDlg, DM_SC_INITDIALOG, (WPARAM)0, (LPARAM)pContainer->settings);
			SendDlgItemMessage(hwndDlg, IDC_TITLEFORMAT, EM_LIMITTEXT, TITLE_FORMATLEN - 1, 0);
			SetDlgItemText(hwndDlg, IDC_TITLEFORMAT, pContainer->settings->szTitleFormat);
			SetDlgItemText(hwndDlg, IDC_THEME, pContainer->szRelThemeFile);
			for (i = 0; i < NR_O_PAGES; i++) {
				tvis.hParent = NULL;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
				tvis.item.pszText = const_cast<TCHAR *>(CTranslator::get(o_pages[i].idTitle));
				tvis.item.lParam = i;
				hItem = TreeView_InsertItem(hwndTree, &tvis);
				if (i == 0)
					SendMessage(hwndTree, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
				for (j = 0; j < NR_O_OPTIONSPERPAGE && o_pages[i].uIds[j] != 0; j++)
					Utils::showDlgControl(hwndDlg, o_pages[i].uIds[j], SW_HIDE);
				ShowPage(hwndDlg, i, FALSE);
			}
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)PluginConfig.g_iconContainer);
			ShowPage(hwndDlg, 0, TRUE);
			SetFocus(hwndTree);
			Utils::enableDlgControl(hwndDlg, IDC_APPLY, FALSE);

			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_DESC, WM_GETFONT, 0, 0);
			LOGFONT lf = {0};

			GetObject(hFont, sizeof(lf), &lf);
			//lf.lfWeight = FW_BOLD;
			lf.lfHeight = (int)(lf.lfHeight * 1.2);
			hFont = CreateFontIndirect(&lf);

			SendDlgItemMessage(hwndDlg, IDC_TITLEBOX, WM_SETFONT, (WPARAM)hFont, TRUE);

			if(pContainer->isCloned && pContainer->hContactFrom != 0) {
				Utils::showDlgControl(hwndDlg, IDC_CNTPRIVATE, SW_HIDE);
				Utils::showDlgControl(hwndDlg, IDC_O_CNTPRIVATE, SW_HIDE);
			}
			return FALSE;
		}

		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
		{
			HWND hwndChild = (HWND)lParam;
			UINT id = GetDlgCtrlID(hwndChild);

			if(hwndChild == GetDlgItem(hwndDlg, IDC_TITLEBOX)) {
				::SetTextColor((HDC)wParam, RGB(60, 60, 150));
			} else if(hwndChild == GetDlgItem(hwndDlg, IDC_DESC))
				::SetTextColor((HDC)wParam, RGB(160, 50, 50));
			SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
			return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
		}

		case WM_HSCROLL:
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_TRANSPARENCY_ACTIVE) || (HWND)lParam == GetDlgItem(hwndDlg, IDC_TRANSPARENCY_INACTIVE)) {
				char szBuf[20];
				DWORD dwPos = SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);
				_snprintf(szBuf, 10, "%d", dwPos);
				if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_TRANSPARENCY_ACTIVE))
					SetWindowTextA(GetDlgItem(hwndDlg, IDC_TLABEL_ACTIVE), szBuf);
				if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_TRANSPARENCY_INACTIVE))
					SetWindowTextA(GetDlgItem(hwndDlg, IDC_TLABEL_INACTIVE), szBuf);
				Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
			}
			break;
		case WM_NOTIFY:
			if (wParam == IDC_SECTIONTREE && ((LPNMHDR)lParam)->code == TVN_SELCHANGED) {
				NMTREEVIEW *pmtv = (NMTREEVIEW *)lParam;

				ShowPage(hwndDlg, pmtv->itemOld.lParam, 0);
				ShowPage(hwndDlg, pmtv->itemNew.lParam, 1);
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CNTPRIVATE: {

					if(IsDlgButtonChecked(hwndDlg, IDC_CNTPRIVATE)) {
						Utils::ReadPrivateContainerSettings(pContainer, true);
						pContainer->settings->fPrivate = true;
					}
					else {
						if(pContainer->settings != &PluginConfig.globalContainerSettings) {
							char szCname[40];
							mir_snprintf(szCname, 40, "%s%d_Blob", CNT_BASEKEYNAME, pContainer->iContainerIndex);
							pContainer->settings->fPrivate = false;
							DBWriteContactSettingBlob(0, SRMSGMOD_T, szCname, pContainer->settings, sizeof(TContainerSettings));
							free(pContainer->settings);
						}
						pContainer->settings = &PluginConfig.globalContainerSettings;
						pContainer->settings->fPrivate = false;
					}
					SendMessage(hwndDlg, DM_SC_INITDIALOG, 0, (LPARAM)pContainer->settings);
					goto do_apply;
				}
				case IDC_TRANSPARENCY: {
					int isTrans = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY);

					Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_ACTIVE, isTrans ? TRUE : FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_INACTIVE, isTrans ? TRUE : FALSE);
					goto do_apply;
				}
				case IDC_SECTIONTREE:
				case IDC_DESC:
					return 0;
				case IDC_SAVESIZEASGLOBAL: {
					WINDOWPLACEMENT wp = {0};

					wp.length = sizeof(wp);
					if (GetWindowPlacement(pContainer->hwnd, &wp)) {
						M->WriteDword(SRMSGMOD_T, "splitx", wp.rcNormalPosition.left);
						M->WriteDword(SRMSGMOD_T, "splity", wp.rcNormalPosition.top);
						M->WriteDword(SRMSGMOD_T, "splitwidth", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
						M->WriteDword(SRMSGMOD_T, "splitheight", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
					}
					break;
				}
				case IDC_O_ENABLESOUNDS:
					SendMessage(hwndDlg, DM_SC_CONFIG, 0, 0);
					break;
				case IDC_TITLEFORMAT:
					if (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus())
						return TRUE;
					goto do_apply;
				case IDC_SELECTTHEME: {
					const TCHAR *szFileName = GetThemeFileName(0);

					if (PathFileExists(szFileName)) {
						SetDlgItemText(hwndDlg, IDC_THEME, szFileName);
						goto do_apply;
					}
					break;
				}
				case IDOK:
				case IDC_APPLY: {

					SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, (LPARAM)pContainer->settings);

					pContainer->settings->dwTransparency = MAKELONG((WORD)SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_ACTIVE, TBM_GETPOS, 0, 0),
							(WORD)SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_INACTIVE, TBM_GETPOS, 0, 0));

					pContainer->settings->avatarMode = (WORD)SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_GETCURSEL, 0, 0);
					pContainer->settings->ownAvatarMode = (WORD)SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_GETCURSEL, 0, 0);

					GetDlgItemText(hwndDlg, IDC_TITLEFORMAT, pContainer->settings->szTitleFormat, TITLE_FORMATLEN);
					pContainer->settings->szTitleFormat[TITLE_FORMATLEN - 1] = 0;

					pContainer->szRelThemeFile[0] = pContainer->szAbsThemeFile[0] = 0;

					if (GetWindowTextLengthA(GetDlgItem(hwndDlg, IDC_THEME)) > 0) {
						TCHAR	szFinalThemeFile[MAX_PATH], szFilename[MAX_PATH];

						GetDlgItemText(hwndDlg, IDC_THEME, szFilename, MAX_PATH);
						szFilename[MAX_PATH - 1] = 0;
						M->pathToAbsolute(szFilename, szFinalThemeFile);

						if(_tcscmp(szFilename, pContainer->szRelThemeFile))
						   pContainer->fPrivateThemeChanged = TRUE;

						if (PathFileExists(szFinalThemeFile))
							mir_sntprintf(pContainer->szRelThemeFile, MAX_PATH, _T("%s"), szFilename);
						else
							pContainer->szRelThemeFile[0] = 0;
					}
					else {
						pContainer->szRelThemeFile[0] = 0;
						pContainer->fPrivateThemeChanged = TRUE;
					}

					Utils::SettingsToContainer(pContainer);

					if (!IsDlgButtonChecked(hwndDlg, IDC_CNTPRIVATE)) {
						ReloadGlobalContainerSettings(true);
						::DBWriteContactSettingBlob(0, SRMSGMOD_T, CNT_KEYNAME, &PluginConfig.globalContainerSettings, sizeof(TContainerSettings));
					}
					else {
#if defined (_UNICODE)
						char *szSetting = "CNTW_";
#else
						char *szSetting = "CNT_";
#endif
						Utils::SaveContainerSettings(pContainer, szSetting);
					}

					SendMessage(pContainer->hwnd, DM_CONFIGURECONTAINER, 0, 0);
					BroadCastContainer(pContainer, DM_SETINFOPANEL, 0, 0);

					ShowWindow(pContainer->hwnd, SW_HIDE);
					{
						RECT	rc;

						GetWindowRect(pContainer->hwnd, &rc);
						SetWindowPos(pContainer->hwnd, 0, rc.left, rc.top, (rc.right - rc.left) - 1, (rc.bottom - rc.top) - 1, SWP_NOZORDER | SWP_DRAWFRAME | SWP_FRAMECHANGED);
						SetWindowPos(pContainer->hwnd, 0, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOZORDER | SWP_DRAWFRAME | SWP_SHOWWINDOW);
					}

					if (LOWORD(wParam) == IDOK)
						DestroyWindow(hwndDlg);
					else
						Utils::enableDlgControl(hwndDlg, IDC_APPLY, FALSE);

					break;
				}
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;
				default:
do_apply:
					Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
					break;
			}
			break;
		case DM_SC_INITDIALOG: {
			TContainerSettings* cs = (TContainerSettings *)lParam;

			DWORD dwFlags = cs->dwFlags;
			DWORD dwTransparency = cs->dwTransparency;
			DWORD dwFlagsEx = cs->dwFlagsEx;
			char szBuf[20];
			int  isTrans;
			BOOL fAllowTrans = FALSE;

			if(PluginConfig.m_WinVerMajor >= 6)
				fAllowTrans = TRUE;
			else
				fAllowTrans = (!CSkin::m_skinEnabled);

			MY_CheckDlgButton(hwndDlg, IDC_O_HIDETITLE, dwFlags & CNT_NOTITLE);
			MY_CheckDlgButton(hwndDlg, IDC_O_DONTREPORT, dwFlags & CNT_DONTREPORT);
			MY_CheckDlgButton(hwndDlg, IDC_O_NOTABS, dwFlags & CNT_HIDETABS);
			MY_CheckDlgButton(hwndDlg, IDC_O_STICKY, dwFlags & CNT_STICKY);
			MY_CheckDlgButton(hwndDlg, IDC_O_FLASHNEVER, dwFlags & CNT_NOFLASH);
			MY_CheckDlgButton(hwndDlg, IDC_O_FLASHALWAYS, dwFlags & CNT_FLASHALWAYS);
			MY_CheckDlgButton(hwndDlg, IDC_O_FLASHDEFAULT, !((dwFlags & CNT_NOFLASH) || (dwFlags & CNT_FLASHALWAYS)));
			MY_CheckDlgButton(hwndDlg, IDC_TRANSPARENCY, dwFlags & CNT_TRANSPARENCY);
			MY_CheckDlgButton(hwndDlg, IDC_DONTREPORTUNFOCUSED2, dwFlags & CNT_DONTREPORTUNFOCUSED);
			MY_CheckDlgButton(hwndDlg, IDC_ALWAYSPOPUPSINACTIVE, dwFlags & CNT_ALWAYSREPORTINACTIVE);
			MY_CheckDlgButton(hwndDlg, IDC_CNTNOSTATUSBAR, dwFlags & CNT_NOSTATUSBAR);
			MY_CheckDlgButton(hwndDlg, IDC_HIDEMENUBAR, dwFlags & CNT_NOMENUBAR);
			MY_CheckDlgButton(hwndDlg, IDC_STATICICON, dwFlags & CNT_STATICICON);
			MY_CheckDlgButton(hwndDlg, IDC_HIDETOOLBAR, dwFlags & CNT_HIDETOOLBAR);
			MY_CheckDlgButton(hwndDlg, IDC_BOTTOMTOOLBAR, dwFlags & CNT_BOTTOMTOOLBAR);
			MY_CheckDlgButton(hwndDlg, IDC_UIDSTATUSBAR, dwFlags & CNT_UINSTATUSBAR);
			MY_CheckDlgButton(hwndDlg, IDC_VERTICALMAX, dwFlags & CNT_VERTICALMAX);
			MY_CheckDlgButton(hwndDlg, IDC_INFOPANEL, dwFlags & CNT_INFOPANEL);
			MY_CheckDlgButton(hwndDlg, IDC_USEGLOBALSIZE, dwFlags & CNT_GLOBALSIZE);

			MY_CheckDlgButton(hwndDlg, IDC_FLASHICON, dwFlagsEx & TCF_FLASHICON);
			MY_CheckDlgButton(hwndDlg, IDC_FLASHLABEL, dwFlagsEx & TCF_FLASHLABEL);
			MY_CheckDlgButton(hwndDlg, IDC_STYLEDTABS, dwFlagsEx & TCF_STYLED);
			MY_CheckDlgButton(hwndDlg, IDC_CLOSEBUTTONONTABS, dwFlagsEx & TCF_CLOSEBUTTON);

			MY_CheckDlgButton(hwndDlg, IDC_SINGLEROWTAB, dwFlagsEx & TCF_SINGLEROWTABCONTROL);
			MY_CheckDlgButton(hwndDlg, IDC_BUTTONTABS, dwFlagsEx & TCF_FLAT);

			MY_CheckDlgButton(hwndDlg, IDC_O_ENABLESOUNDS, !(dwFlags & CNT_NOSOUND));
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSMINIMIZED, dwFlagsEx & CNT_EX_SOUNDS_MINIMIZED);
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSUNFOCUSED, dwFlagsEx & CNT_EX_SOUNDS_UNFOCUSED);
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSINACTIVE, dwFlagsEx & CNT_EX_SOUNDS_INACTIVETABS);
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSFOCUSED, dwFlagsEx & CNT_EX_SOUNDS_FOCUSED);

			SendMessage(hwndDlg, DM_SC_CONFIG, 0, 0);

			if(!(dwFlagsEx & (TCF_SBARLEFT | TCF_SBARRIGHT)))
				SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_SETCURSEL, dwFlags & CNT_TABSBOTTOM ? 1 : 0, 0);
			else
				SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_SETCURSEL, dwFlagsEx & TCF_SBARLEFT ? 2 : 3, 0);

			if (LOBYTE(LOWORD(GetVersion())) >= 5 && fAllowTrans)
				CheckDlgButton(hwndDlg, IDC_TRANSPARENCY, dwFlags & CNT_TRANSPARENCY);
			else
				CheckDlgButton(hwndDlg, IDC_TRANSPARENCY, FALSE);

			Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY, PluginConfig.m_WinVerMajor >= 5 && fAllowTrans ? TRUE : FALSE);

			isTrans = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY);
			Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_ACTIVE, isTrans ? TRUE : FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_INACTIVE, isTrans ? TRUE : FALSE);

			_snprintf(szBuf, 10, "%d", LOWORD(dwTransparency));
			SetWindowTextA(GetDlgItem(hwndDlg, IDC_TLABEL_ACTIVE), szBuf);
			_snprintf(szBuf, 10, "%d", HIWORD(dwTransparency));
			SetWindowTextA(GetDlgItem(hwndDlg, IDC_TLABEL_INACTIVE), szBuf);

			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_ACTIVE, TBM_SETRANGE, 0, (LPARAM)MAKELONG(50, 255));
			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_INACTIVE, TBM_SETRANGE, 0, (LPARAM)MAKELONG(50, 255));

			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_ACTIVE, TBM_SETPOS, TRUE, (LPARAM) LOWORD(dwTransparency));
			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_INACTIVE, TBM_SETPOS, TRUE, (LPARAM) HIWORD(dwTransparency));

			Utils::enableDlgControl(hwndDlg, IDC_O_DONTREPORT, nen_options.bWindowCheck == 0);
			Utils::enableDlgControl(hwndDlg, IDC_DONTREPORTUNFOCUSED2, nen_options.bWindowCheck == 0);
			Utils::enableDlgControl(hwndDlg, IDC_ALWAYSPOPUPSINACTIVE, nen_options.bWindowCheck == 0);

			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_SETCURSEL, (WPARAM)cs->avatarMode, 0);
			SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_SETCURSEL, (WPARAM)cs->ownAvatarMode, 0);

			Utils::showDlgControl(hwndDlg, IDC_O_EXPLAINGLOBALNOTIFY, nen_options.bWindowCheck ? SW_SHOW : SW_HIDE);
			break;
		}

		case DM_SC_CONFIG: {
			LRESULT enable = (IsDlgButtonChecked(hwndDlg, IDC_O_ENABLESOUNDS) ? BST_CHECKED : BST_UNCHECKED);
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSINACTIVE, enable);
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSUNFOCUSED, enable);
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSMINIMIZED, enable);
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSFOCUSED, enable);
			return(0);
		}

		case DM_SC_BUILDLIST: {
			DWORD dwNewFlags = 0, dwNewFlagsEx = 0;
			TContainerSettings* cs = (TContainerSettings *)lParam;

			dwNewFlags = (IsDlgButtonChecked(hwndDlg, IDC_O_HIDETITLE) ? CNT_NOTITLE : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_O_DONTREPORT) ? CNT_DONTREPORT : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_O_NOTABS) ? CNT_HIDETABS : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_O_STICKY) ? CNT_STICKY : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_O_FLASHALWAYS) ? CNT_FLASHALWAYS : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_O_FLASHNEVER) ? CNT_NOFLASH : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY) ? CNT_TRANSPARENCY : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_DONTREPORTUNFOCUSED2) ? CNT_DONTREPORTUNFOCUSED : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_ALWAYSPOPUPSINACTIVE) ? CNT_ALWAYSREPORTINACTIVE : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_CNTNOSTATUSBAR) ? CNT_NOSTATUSBAR : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_HIDEMENUBAR) ? CNT_NOMENUBAR : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_STATICICON) ? CNT_STATICICON : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_HIDETOOLBAR) ? CNT_HIDETOOLBAR : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_BOTTOMTOOLBAR) ? CNT_BOTTOMTOOLBAR : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_UIDSTATUSBAR) ? CNT_UINSTATUSBAR : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_USEGLOBALSIZE) ? CNT_GLOBALSIZE : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_INFOPANEL) ? CNT_INFOPANEL : 0) |
						 (IsDlgButtonChecked(hwndDlg, IDC_O_ENABLESOUNDS) ? 0 : CNT_NOSOUND) |
						 (IsDlgButtonChecked(hwndDlg, IDC_VERTICALMAX) ? CNT_VERTICALMAX : 0) |
						 (CNT_NEWCONTAINERFLAGS);

			LRESULT iTabMode = SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_GETCURSEL, 0, 0);
			LRESULT iTabLayout = SendDlgItemMessage(hwndDlg, IDC_SBARLAYOUT, CB_GETCURSEL, 0, 0);

			dwNewFlagsEx = 0;

			if(iTabMode < 2)
				dwNewFlags = ((iTabMode == 1) ? (dwNewFlags | CNT_TABSBOTTOM) : (dwNewFlags & ~CNT_TABSBOTTOM));
			else {
				dwNewFlags &= ~CNT_TABSBOTTOM;
				dwNewFlagsEx = iTabMode == 2 ? TCF_SBARLEFT : TCF_SBARRIGHT;
			}

			dwNewFlagsEx |= ((IsDlgButtonChecked(hwndDlg, IDC_FLASHICON) ? TCF_FLASHICON : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_FLASHLABEL) ? TCF_FLASHLABEL : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_BUTTONTABS) ? TCF_FLAT : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_STYLEDTABS) ? TCF_STYLED : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_CLOSEBUTTONONTABS) ? TCF_CLOSEBUTTON : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_SINGLEROWTAB) ? TCF_SINGLEROWTABCONTROL : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSMINIMIZED) ? CNT_EX_SOUNDS_MINIMIZED : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSUNFOCUSED) ? CNT_EX_SOUNDS_UNFOCUSED : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSFOCUSED) ? CNT_EX_SOUNDS_FOCUSED : 0) |
						   (IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSINACTIVE) ? CNT_EX_SOUNDS_INACTIVETABS : 0)
						  );

			/* bits 24 - 31 of dwFlagsEx hold the sidebar layout id */
			dwNewFlagsEx |= ((int)((iTabLayout << 24) & 0xff000000));

			if (IsDlgButtonChecked(hwndDlg, IDC_O_FLASHDEFAULT))
				dwNewFlags &= ~(CNT_FLASHALWAYS | CNT_NOFLASH);

			cs->dwFlags = dwNewFlags;
			cs->dwFlagsEx = dwNewFlagsEx;
			break;
		}
		case WM_DESTROY: {
			pContainer->hWndOptions = 0;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TITLEBOX, WM_GETFONT, 0, 0);
			DeleteObject(hFont);
			break;
		}
	}
	return FALSE;
}

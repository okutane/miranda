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

UNICODE done

*/
#include <commonheaders.h>
#include <richedit.h>
#include "../coolsb/coolscroll.h"

#ifndef SPI_GETDESKWALLPAPER
#define SPI_GETDESKWALLPAPER 115
#endif
//loads of stuff that didn't really fit anywhere else

extern int /*g_isConnecting,*/ during_sizing;

extern void ( *saveRecalcScrollBar )(HWND hwnd, struct ClcData *dat);

static int MY_pathIsAbsolute(const TCHAR *path)
{
	if (!path || !(lstrlen(path) > 2))
		return 0;
	if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
        return 1;
	return 0;
}

size_t MY_pathToRelative(const TCHAR *pSrc, TCHAR *pOut)
{
	if (!pSrc || !lstrlen(pSrc) || lstrlen(pSrc) > MAX_PATH)
        return 0;
	if (!MY_pathIsAbsolute(pSrc)) {
		mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
		return lstrlen(pOut);
	} else {
		TCHAR szTmp[MAX_PATH];
		mir_sntprintf(szTmp, SIZEOF(szTmp), _T("%s"), pSrc);
		_tcslwr(szTmp);
		if (_tcsstr(szTmp, cfg::dat.tszProfilePath)) {
			mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc + lstrlen(cfg::dat.tszProfilePath) - 1);
			pOut[0]='.';
			return lstrlen(pOut);
		} else {
			mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
			return lstrlen(pOut);
		}
	}
}

size_t MY_pathToAbsolute(const TCHAR *pSrc, TCHAR *pOut)
{
	if (!pSrc || !lstrlen(pSrc) || lstrlen(pSrc) > MAX_PATH)
        return 0;
	if (MY_pathIsAbsolute(pSrc)&&pSrc[0]!='.')
		mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
	else if (pSrc[0]=='.')
		mir_sntprintf(pOut, MAX_PATH, _T("%s\\%s"), cfg::dat.tszProfilePath, pSrc);

	return lstrlen(pOut);
}

/*
 * performs hit-testing for reversed (mirrored) contact rows when using RTL
 * shares all the init stuff with HitTest()
 */

int RTL_HitTest(HWND hwnd, struct ClcData *dat, int testx, int testy, struct ClcContact *hitcontact, DWORD *flags, int indent, int hit)
{
    RECT clRect;
    int right, checkboxWidth, cxSmIcon, i, width;
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    SIZE textSize;
    HDC hdc;
    HFONT hFont;

    GetClientRect(hwnd, &clRect);
    right = clRect.right;

    // avatar check
    if(hitcontact->type == CLCIT_CONTACT && cfg::dat.dwFlags & CLUI_FRAME_AVATARS && hitcontact->ace != NULL && hitcontact->avatarLeft != -1) {
        if(testx < right - hitcontact->avatarLeft && testx > right - hitcontact->avatarLeft - cfg::dat.avatarSize) {
            if(flags)
                *flags |= CLCHT_ONAVATAR;
        }
    }
    if (testx > right - (dat->leftMargin + indent * dat->groupIndent)) {
        if (flags)
            *flags |= CLCHT_ONITEMINDENT;
        return hit;
    }
    checkboxWidth = 0;
    if (style & CLS_CHECKBOXES && hitcontact->type == CLCIT_CONTACT)
        checkboxWidth = dat->checkboxSize + 2;
    if (style & CLS_GROUPCHECKBOXES && hitcontact->type == CLCIT_GROUP)
        checkboxWidth = dat->checkboxSize + 2;
    if (hitcontact->type == CLCIT_INFO && hitcontact->flags & CLCIIF_CHECKBOX)
        checkboxWidth = dat->checkboxSize + 2;
    if (testx > right - (dat->leftMargin + indent * dat->groupIndent + checkboxWidth)) {
        if (flags)
            *flags |= CLCHT_ONITEMCHECK;
        return hit;
    }
    if (testx > right - (dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace)) {
        if (flags)
            *flags |= CLCHT_ONITEMICON;
        return hit;
    }
    cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    for (i = 0; i< dat->extraColumnsCount; i++) {
        if (hitcontact->iExtraImage[i] == 0xFF)
            continue;
        if (testx >= dat->extraColumnSpacing * (dat->extraColumnsCount - i) && testx < dat->extraColumnSpacing * (dat->extraColumnsCount - i) + cxSmIcon) {
            if (flags)
                *flags |= CLCHT_ONITEMEXTRA | (i << 24);
            return hit;
        }
    }
    if(hitcontact->extraCacheEntry >= 0 && hitcontact->extraCacheEntry < cfg::nextCacheEntry && cfg::eCache[hitcontact->extraCacheEntry].iExtraValid) {
        int rightOffset = hitcontact->extraIconRightBegin;
        int images_present = 0;

        for (i = 5; i >= 0; i--) {
            if (cfg::eCache[hitcontact->extraCacheEntry].iExtraImage[i] == 0xFF)
                continue;
            if(!((1 << i) & cfg::eCache[hitcontact->extraCacheEntry].dwXMask))
                continue;
            images_present++;
            if (testx < right - (rightOffset - (cfg::dat.exIconScale + 2) * images_present) && testx > right - (rightOffset - (cfg::dat.exIconScale + 2) * images_present + (cfg::dat.exIconScale))) {
                if (flags)
                    *flags |= (CLCHT_ONITEMEXTRAEX | ((i + 1) << 24));
                return hit;
            }
        }
    }

    hdc = GetDC(hwnd);
    if (hitcontact->type == CLCIT_GROUP)
        hFont = reinterpret_cast<HFONT>(SelectObject(hdc, dat->fontInfo[FONTID_GROUPS].hFont));
    else
        hFont = reinterpret_cast<HFONT>(SelectObject(hdc, dat->fontInfo[FONTID_CONTACTS].hFont));
    GetTextExtentPoint32(hdc, hitcontact->szText, lstrlen(hitcontact->szText), &textSize);
    width = textSize.cx;
    if (hitcontact->type == CLCIT_GROUP) {
        char *szCounts;
        szCounts = pcli->pfnGetGroupCountsText(dat, hitcontact);
        if (szCounts[0]) {
            GetTextExtentPoint32A(hdc, " ", 1, &textSize);
            width += textSize.cx;
            SelectObject(hdc, dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
            GetTextExtentPoint32A(hdc, szCounts, lstrlenA(szCounts), &textSize);
            width += textSize.cx;
        }
    }
    SelectObject(hdc, hFont);
    ReleaseDC(hwnd, hdc);
    if (testx > right - (dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace + width + 4 + (cfg::dat.dwFlags & CLUI_FRAME_AVATARS ? cfg::dat.avatarSize : 0))) {
        if (flags)
            *flags |= CLCHT_ONITEMLABEL;
        return hit;
    }
    if (cfg::dat.dwFlags & CLUI_FULLROWSELECT && !(GetKeyState(VK_SHIFT) & 0x8000) && testx < right - (dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace + width + 4 + (cfg::dat.dwFlags & CLUI_FRAME_AVATARS ? cfg::dat.avatarSize : 0))) {
        if (flags)
            *flags |= CLCHT_ONITEMSPACE;
        return hit;
    }
    if (flags)
        *flags |= CLCHT_NOWHERE;
    return -1;
}

int HitTest(HWND hwnd, struct ClcData *dat, int testx, int testy, struct ClcContact **contact, struct ClcGroup **group, DWORD *flags)
{
    struct ClcContact *hitcontact;
    struct ClcGroup *hitgroup;
    int hit, indent, width, i, cxSmIcon;
    int checkboxWidth;
    SIZE textSize;
    HDC hdc;
    RECT clRect;
    HFONT hFont;
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
	BYTE mirror_mode = cfg::dat.bUseDCMirroring;

    if (flags)
        *flags = 0;
    GetClientRect(hwnd, &clRect);
    if (testx < 0 || testy < 0 || testy >= clRect.bottom || testx >= clRect.right) {
        if (flags) {
            if (testx < 0)
                *flags |= CLCHT_TOLEFT;
            else if (testx >= clRect.right)
                *flags |= CLCHT_TORIGHT;
            if (testy < 0)
                *flags |= CLCHT_ABOVE;
            else if (testy >= clRect.bottom)
                *flags |= CLCHT_BELOW;
        }
        return -1;
    }
    if (testx< dat->leftMargin) {
        if (flags)
            *flags |= CLCHT_INLEFTMARGIN | CLCHT_NOWHERE;
        return -1;
    }
   	hit = RowHeight::hitTest(dat, dat->yScroll + testy);
	if (hit != -1)
		hit = pcli->pfnGetRowByIndex(dat, hit, &hitcontact, &hitgroup);

    if (hit == -1) {
        if (flags)
            *flags |= CLCHT_NOWHERE | CLCHT_BELOWITEMS;
        return -1;
    }
    if (contact)
        *contact = hitcontact;
    if (group)
        *group = hitgroup;

    for (indent = 0; hitgroup->parent; indent++,hitgroup = hitgroup->parent) {
        ;
    }

    if(!dat->bisEmbedded) {
        if(hitcontact->type == CLCIT_CONTACT) {
            if(mirror_mode == 1 || (mirror_mode == 2 && cfg::eCache[hitcontact->extraCacheEntry].dwCFlags & ECF_RTLNICK))
                return RTL_HitTest(hwnd, dat, testx, testy, hitcontact, flags, indent, hit);
        }
        else if(hitcontact->type == CLCIT_GROUP) {
            if(cfg::dat.bGroupAlign == CLC_GROUPALIGN_RIGHT || (hitcontact->isRtl && cfg::dat.bGroupAlign == CLC_GROUPALIGN_AUTO))
                return RTL_HitTest(hwnd, dat, testx, testy, hitcontact, flags, indent, hit);
        }
    }

    // avatar check
    if(hitcontact->type == CLCIT_CONTACT && cfg::dat.dwFlags & CLUI_FRAME_AVATARS && hitcontact->ace != NULL && hitcontact->avatarLeft != -1) {
        if(testx >hitcontact->avatarLeft && testx < hitcontact->avatarLeft + cfg::dat.avatarSize) {
            if(flags)
                *flags |= CLCHT_ONAVATAR;
        }
    }
    if (testx< dat->leftMargin + indent * dat->groupIndent) {
        if (flags)
            *flags |= CLCHT_ONITEMINDENT;
        return hit;
    }
    checkboxWidth = 0;
    if (style & CLS_CHECKBOXES && hitcontact->type == CLCIT_CONTACT)
        checkboxWidth = dat->checkboxSize + 2;
    if (style & CLS_GROUPCHECKBOXES && hitcontact->type == CLCIT_GROUP)
        checkboxWidth = dat->checkboxSize + 2;
    if (hitcontact->type == CLCIT_INFO && hitcontact->flags & CLCIIF_CHECKBOX)
        checkboxWidth = dat->checkboxSize + 2;
    if (testx< dat->leftMargin + indent * dat->groupIndent + checkboxWidth) {
        if (flags)
            *flags |= CLCHT_ONITEMCHECK;
        return hit;
    }
    if (testx< dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace) {
        if (flags)
            *flags |= CLCHT_ONITEMICON;
        return hit;
    }
    cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    for (i = 0; i< dat->extraColumnsCount; i++) {
        if (hitcontact->iExtraImage[i] == 0xFF)
            continue;
        if (testx >= clRect.right - dat->extraColumnSpacing * (dat->extraColumnsCount - i) && testx< clRect.right - dat->extraColumnSpacing * (dat->extraColumnsCount - i) + cxSmIcon) {
            if (flags)
                *flags |= CLCHT_ONITEMEXTRA | (i << 24);
            return hit;
        }
    }
    if(hitcontact->extraCacheEntry >= 0 && hitcontact->extraCacheEntry < cfg::nextCacheEntry && cfg::eCache[hitcontact->extraCacheEntry].iExtraValid) {
        //int rightOffset = clRect.right;
        int rightOffset = hitcontact->extraIconRightBegin;
        int images_present = 0;

        for (i = 5; i >= 0; i--) {
            if (cfg::eCache[hitcontact->extraCacheEntry].iExtraImage[i] == 0xFF)
                continue;
            if(!((1 << i) & cfg::eCache[hitcontact->extraCacheEntry].dwXMask))
                continue;
            images_present++;
            if (testx > (rightOffset - (cfg::dat.exIconScale + 2) * images_present) && testx < (rightOffset - (cfg::dat.exIconScale + 2) * images_present + (cfg::dat.exIconScale))) {
                if (flags)
                    *flags |= (CLCHT_ONITEMEXTRAEX | ((i + 1) << 24));
                return hit;
            }
        }
    }
    hdc = GetDC(hwnd);
    if (hitcontact->type == CLCIT_GROUP)
        hFont = reinterpret_cast<HFONT>(SelectObject(hdc, dat->fontInfo[FONTID_GROUPS].hFont));
    else
        hFont = reinterpret_cast<HFONT>(SelectObject(hdc, dat->fontInfo[FONTID_CONTACTS].hFont));
    GetTextExtentPoint32(hdc, hitcontact->szText, lstrlen(hitcontact->szText), &textSize);
    width = textSize.cx;
    if (hitcontact->type == CLCIT_GROUP) {
        char *szCounts;
        szCounts = pcli->pfnGetGroupCountsText(dat, hitcontact);
        if (szCounts[0]) {
            GetTextExtentPoint32A(hdc, " ", 1, &textSize);
            width += textSize.cx;
            SelectObject(hdc, dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
            GetTextExtentPoint32A(hdc, szCounts, lstrlenA(szCounts), &textSize);
            width += textSize.cx;
        }
    }
    SelectObject(hdc, hFont);
    ReleaseDC(hwnd, hdc);
    if (cfg::dat.dwFlags & CLUI_FULLROWSELECT && !(GetKeyState(VK_SHIFT) & 0x8000) && testx > dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace + width + 4 + (cfg::dat.dwFlags & CLUI_FRAME_AVATARS ? cfg::dat.avatarSize : 0)) {
        if (flags)
            *flags |= CLCHT_ONITEMSPACE;
        return hit;
    }
    if (testx< dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace + width + 4 + (cfg::dat.dwFlags & CLUI_FRAME_AVATARS ? cfg::dat.avatarSize : 0)) {
        if (flags)
            *flags |= CLCHT_ONITEMLABEL;
        return hit;
    }
    if (flags)
        *flags |= CLCHT_NOWHERE;
    return -1;
}

void ScrollTo(HWND hwnd, struct ClcData *dat, int desty, int noSmooth)
{
	DWORD startTick, nowTick;
	int oldy = dat->yScroll;
	RECT clRect, rcInvalidate;
	int maxy, previousy;

	if (dat->iHotTrack != -1 && dat->yScroll != desty) {
		pcli->pfnInvalidateItem(hwnd, dat, dat->iHotTrack);
		dat->iHotTrack = -1;
		ReleaseCapture();
	}
	GetClientRect(hwnd, &clRect);
	rcInvalidate = clRect;

	maxy = RowHeight::getTotalHeight(dat)-clRect.bottom;
	if (desty > maxy)
		desty = maxy;
	if (desty < 0)
		desty = 0;
	if (abs(desty - dat->yScroll) < 4)
		noSmooth = 1;
	if (!noSmooth && dat->exStyle & CLS_EX_NOSMOOTHSCROLLING)
		noSmooth = 1;
	previousy = dat->yScroll;
	if (!noSmooth) {
		startTick = GetTickCount();
		for (; ;) {
			nowTick = GetTickCount();
			if (nowTick >= startTick + dat->scrollTime)
				break;
			dat->yScroll = oldy + (desty - oldy) * (int) (nowTick - startTick) / dat->scrollTime;
			if (dat->backgroundBmpUse & CLBF_SCROLL || dat->hBmpBackground == NULL)
				ScrollWindowEx(hwnd, 0, previousy - dat->yScroll, NULL, NULL, NULL, NULL, SW_INVALIDATE);
			else
				InvalidateRect(hwnd, NULL, FALSE);
			previousy = dat->yScroll;
            if(cfg::dat.bSkinnedScrollbar && !dat->bisEmbedded)
                CoolSB_SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
            else
                SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
			UpdateWindow(hwnd);
		}
	}
	dat->yScroll = desty;
	if (dat->backgroundBmpUse & CLBF_SCROLL || dat->hBmpBackground == NULL) {
		if(!noSmooth)
			ScrollWindowEx(hwnd, 0, previousy - dat->yScroll, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		else
			InvalidateRect(hwnd, NULL, FALSE);
	}
	else
		InvalidateRect(hwnd, NULL, FALSE);

    if(cfg::dat.bSkinnedScrollbar && !dat->bisEmbedded)
        CoolSB_SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
    else
        SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
	dat->forceScroll = 0;
}

void RecalcScrollBar(HWND hwnd, struct ClcData *dat)
{
    SCROLLINFO si = { 0 };
    RECT clRect;
    NMCLISTCONTROL nm;

    RowHeight::calcRowHeights(dat, hwnd);

    GetClientRect(hwnd, &clRect);
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nMax = pcli->pfnGetRowTotalHeight(dat)-1;
    si.nPage = clRect.bottom;
    si.nPos = dat->yScroll;

    if (GetWindowLong(hwnd, GWL_STYLE) & CLS_CONTACTLIST) {
        if (dat->noVScrollbar == 0) {
            if(cfg::dat.bSkinnedScrollbar && !dat->bisEmbedded)
                CoolSB_SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            else
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        }
    }
    else {
        if(cfg::dat.bSkinnedScrollbar && !dat->bisEmbedded)
            CoolSB_SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        else
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    }
    ScrollTo(hwnd, dat, dat->yScroll, 1);
    nm.hdr.code = CLN_LISTSIZECHANGE;
    nm.hdr.hwndFrom = hwnd;
    nm.hdr.idFrom = GetDlgCtrlID(hwnd);
    nm.pt.y = si.nMax;
    SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
	//saveRecalcScrollBar(hwnd, dat);
}

void SetGroupExpand(HWND hwnd,struct ClcData *dat,struct ClcGroup *group,int newState)
{
	int contentCount;
	int groupy;
	int newy;
	int posy;
	RECT clRect;
	NMCLISTCONTROL nm;

    if (newState == -1)
        group->expanded ^= 1;
    else {
        if (group->expanded == (newState != 0))
            return;
        group->expanded = newState != 0;
    }
	InvalidateRect(hwnd,NULL,FALSE);
    contentCount = pcli->pfnGetGroupContentsCount(group,1);

	groupy=pcli->pfnGetRowsPriorTo(&dat->list,group,-1);
	if(dat->selection>groupy && dat->selection<groupy+contentCount) dat->selection=groupy;
	pcli->pfnRecalcScrollBar(hwnd,dat);

	GetClientRect(hwnd,&clRect);
	newy=dat->yScroll;
	posy = RowHeight::getItemBottomY(dat, groupy+contentCount);
	if(posy>=newy+clRect.bottom)
		newy=posy-clRect.bottom;
	posy = RowHeight::getItemTopY(dat, groupy);
	if(newy>posy) newy=posy;
	ScrollTo(hwnd,dat,newy,0);

	nm.hdr.code=CLN_EXPANDED;
	nm.hdr.hwndFrom=hwnd;
	nm.hdr.idFrom=GetDlgCtrlID(hwnd);
	nm.hItem=(HANDLE)group->groupId;
	nm.action = (group->expanded);
	SendMessage(GetParent(hwnd),WM_NOTIFY,0,(LPARAM)&nm);
}

static WNDPROC OldRenameEditWndProc;
static LRESULT CALLBACK RenameEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
	case VK_RETURN:
		pcli->pfnEndRename(GetParent(hwnd), (struct ClcData *) GetWindowLongPtr(GetParent(hwnd), 0), 1);
		return 0;
	case VK_ESCAPE:
		pcli->pfnEndRename(GetParent(hwnd), (struct ClcData *) GetWindowLongPtr(GetParent(hwnd), 0), 0);
		return 0;
		}
		break;
	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg = (MSG *) lParam;
			if (msg->message == WM_KEYDOWN && msg->wParam == VK_TAB)
				return 0;
			if (msg->message == WM_CHAR && msg->wParam == '\t')
				return 0;
		}
		return DLGC_WANTMESSAGE;
	case WM_KILLFOCUS:
		pcli->pfnEndRename(GetParent(hwnd), (struct ClcData *) GetWindowLongPtr(GetParent(hwnd), 0), 1);
		return 0;
	}
	return CallWindowProc(OldRenameEditWndProc, hwnd, msg, wParam, lParam);
}

void BeginRenameSelection(HWND hwnd, struct ClcData *dat)
{
    struct ClcContact *contact;
    struct ClcGroup *group;
    int indent, x, y, h;
    RECT clRect;

    KillTimer(hwnd, TIMERID_RENAME);
    ReleaseCapture();
    dat->iHotTrack = -1;
    dat->selection = pcli->pfnGetRowByIndex(dat, dat->selection, &contact, &group);
    if (dat->selection == -1)
        return;
    if (contact->type != CLCIT_CONTACT && contact->type != CLCIT_GROUP)
        return;
    for (indent = 0; group->parent; indent++,group = group->parent) {
        ;
    }
    GetClientRect(hwnd, &clRect);
    x = indent * dat->groupIndent + dat->iconXSpace - 2;
    //y = dat->selection * dat->rowHeight - dat->yScroll;
   	y = RowHeight::getItemTopY(dat, dat->selection)-dat->yScroll;

    h=dat->row_heights[dat->selection];
    {
        int i;
        for (i=0; i <= FONTID_LAST; i++)
           if (h<dat->fontInfo[i].fontHeight+2) h=dat->fontInfo[i].fontHeight+2;
    }
#if defined(_UNICODE)
	dat->hwndRenameEdit = CreateWindowEx(0, _T("RichEdit20W"),contact->szText,WS_CHILD|WS_BORDER|ES_MULTILINE|ES_AUTOHSCROLL,x,y,clRect.right-x,h,hwnd,NULL,g_hInst,NULL);
    {
        if((contact->type == CLCIT_CONTACT && cfg::eCache[contact->extraCacheEntry].dwCFlags & ECF_RTLNICK) || (contact->type == CLCIT_GROUP && contact->isRtl)) {
            PARAFORMAT2 pf2;
            ZeroMemory((void *)&pf2, sizeof(pf2));
            pf2.cbSize = sizeof(pf2);
            pf2.dwMask = PFM_RTLPARA;
            pf2.wEffects = PFE_RTLPARA;
            SetWindowText(dat->hwndRenameEdit, _T(""));
            SendMessage(dat->hwndRenameEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
            SetWindowText(dat->hwndRenameEdit, contact->szText);
        }
    }
#else
    dat->hwndRenameEdit = CreateWindow(_T("EDIT"),contact->szText,WS_CHILD|WS_BORDER|ES_MULTILINE|ES_AUTOHSCROLL,x,y,clRect.right-x,h,hwnd,NULL,g_hInst,NULL);
#endif
    //dat->hwndRenameEdit = CreateWindow(_T("EDIT"), contact->szText, WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, x, y, clRect.right - x, dat->rowHeight, hwnd, NULL, g_hInst, NULL);
    OldRenameEditWndProc = (WNDPROC) SetWindowLongPtr(dat->hwndRenameEdit, GWLP_WNDPROC, (LONG_PTR) RenameEditSubclassProc);
    SendMessage(dat->hwndRenameEdit, WM_SETFONT, (WPARAM) (contact->type == CLCIT_GROUP ? dat->fontInfo[FONTID_GROUPS].hFont : dat->fontInfo[FONTID_CONTACTS].hFont), 0);
    SendMessage(dat->hwndRenameEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN | EC_USEFONTINFO, 0);
    SendMessage(dat->hwndRenameEdit, EM_SETSEL, 0, (LPARAM) (-1));
    ShowWindow(dat->hwndRenameEdit, SW_SHOW);
    SetFocus(dat->hwndRenameEdit);
}

extern void ( *saveLoadClcOptions )(HWND hwnd,struct ClcData *dat);

void LoadClcOptions(HWND hwnd, struct ClcData *dat)
{
	dat->bkChanged = 0;

	saveLoadClcOptions(hwnd, dat);

	dat->min_row_heigh = (int)cfg::getByte("CLC","RowHeight",CLCDEFAULT_ROWHEIGHT);
	dat->group_row_height = (int)cfg::getByte("CLC","GRowHeight",CLCDEFAULT_ROWHEIGHT);
	dat->row_border = 0;
	dat->rightMargin = cfg::getByte("CLC", "RightMargin", CLCDEFAULT_LEFTMARGIN);
	dat->bkColour =  cfg::getByte("CLC", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS) ?
                     GetSysColor(COLOR_3DFACE) : cfg::getDword("CLC", "BkColour", CLCDEFAULT_BKCOLOUR);
	if (!dat->bkChanged) {
		if(cfg::dat.hBrushCLCBk)
			DeleteObject(cfg::dat.hBrushCLCBk);
		cfg::dat.hBrushCLCBk = CreateSolidBrush(dat->bkColour);
		if (dat->hBmpBackground) {
			if(cfg::dat.hdcPic) {
				SelectObject(cfg::dat.hdcPic, cfg::dat.hbmPicOld);
				DeleteDC(cfg::dat.hdcPic);
				cfg::dat.hdcPic = 0;
				cfg::dat.hbmPicOld = 0;
		}	}

		cfg::dat.bmpBackground = dat->hBmpBackground;
		if(cfg::dat.bmpBackground) {
			HDC hdcThis = GetDC(pcli->hwndContactList);
			GetObject(cfg::dat.bmpBackground, sizeof(cfg::dat.bminfoBg), &(cfg::dat.bminfoBg));
			cfg::dat.hdcPic = CreateCompatibleDC(hdcThis);
			cfg::dat.hbmPicOld = reinterpret_cast<HBITMAP>(SelectObject(cfg::dat.hdcPic, cfg::dat.bmpBackground));
			ReleaseDC(pcli->hwndContactList, hdcThis);
		}
	}
	if (cfg::getByte("CLCExt", "EXBK_FillWallpaper", 0)) {
		char wpbuf[MAX_PATH];
		if (dat->hBmpBackground)
			DeleteObject(dat->hBmpBackground); dat->hBmpBackground = NULL;

		SystemParametersInfoA(SPI_GETDESKWALLPAPER, MAX_PATH, wpbuf, 0);

		// we have a wallpaper string
		if (strlen(wpbuf) > 0) {
			dat->hBmpBackground = reinterpret_cast<HBITMAP>(LoadImageA(NULL, wpbuf, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		}
		cfg::dat.bmpBackground = dat->hBmpBackground;
		if(cfg::dat.bmpBackground) {
			HDC hdcThis = GetDC(pcli->hwndContactList);
			GetObject(cfg::dat.bmpBackground, sizeof(cfg::dat.bminfoBg), &(cfg::dat.bminfoBg));
			cfg::dat.hdcPic = CreateCompatibleDC(hdcThis);
			cfg::dat.hbmPicOld = reinterpret_cast<HBITMAP>(SelectObject(cfg::dat.hdcPic, cfg::dat.bmpBackground));
			ReleaseDC(pcli->hwndContactList, hdcThis);
		}
	}
}

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
---------------------------------------------------------------------------

custom tab control for tabSRMM. Allows for configuartion of colors and backgrounds
for different tab states (active,  unread etc..)

no visual style support yet - will be added later.

*/

#include "commonheaders.h"
#pragma hdrstop
#include "msgs.h"
#include <uxtheme.h>

extern MYGLOBALS myGlobals;
extern WNDPROC OldTabControlProc;
extern struct ContainerWindowData *pFirstContainer;
/*
 * visual styles support (XP+)
 * returns 0 on failure
 */

HMODULE hUxTheme = 0;

// function pointers

BOOL (PASCAL* pfnIsThemeActive)() = 0;
HANDLE (PASCAL* pfnOpenThemeData)(HWND hwnd, LPCWSTR pszClassList) = 0;
UINT (PASCAL* pfnDrawThemeBackground)(HANDLE htheme, HDC hdc, int iPartID, int iStateID, RECT* prcBx, RECT* prcClip) = 0;
UINT (PASCAL* pfnCloseThemeData)(HANDLE hTheme) = 0;
HRESULT (PASCAL* pfnGetThemeBackgroundRegion)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, HRGN *pRegion) = 0;
BOOL (PASCAL* pfnIsThemeBackgroundPartiallyTransparent)(HANDLE hTheme, int iPartId, int iStateId) = 0;
HRESULT (PASCAL* pfnDrawThemeBackgroundEx)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const DTBGOPTS *pOptions) = 0;

void FreeTabConfig(), ReloadTabConfig();

int InitVSApi()
{
    if((hUxTheme = LoadLibraryA("uxtheme.dll")) == 0)
        return FALSE;

    pfnIsThemeActive = GetProcAddress(hUxTheme, "IsThemeActive");
    pfnOpenThemeData = GetProcAddress(hUxTheme, "OpenThemeData");
    pfnDrawThemeBackground = GetProcAddress(hUxTheme, "DrawThemeBackground");
    pfnCloseThemeData = GetProcAddress(hUxTheme, "CloseThemeData");
    pfnGetThemeBackgroundRegion = GetProcAddress(hUxTheme, "GetThemeBackgroundRegion");
    pfnIsThemeBackgroundPartiallyTransparent = GetProcAddress(hUxTheme, "IsThemeBackgroundPartiallyTransparent");
    pfnDrawThemeBackgroundEx = GetProcAddress(hUxTheme, "DrawThemeBackgroundEx");
    if(pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0) {
        return 1;
    }
    return 0;
}

int FreeVSApi()
{
    if(hUxTheme != 0)
        FreeLibrary(hUxTheme);
    return 0;
}
void RectScreenToClient(HWND hwnd, RECT *rc)
{
    POINT p1, p2;

    p1.x = rc->left;
    p1.y = rc->top;
    p2.x = rc->right;
    p2.y = rc->bottom;
    ScreenToClient(hwnd, &p1);
    ScreenToClient(hwnd, &p2);
    rc->left = p1.x;
    rc->top = p1.y;
    rc->right = p2.x;
    rc->bottom = p2.y;
}

	/*
     * tabctrl helper function
	 * Finds leftmost down item.
     */

UINT FindLeftDownItem(HWND hwnd)
{
	RECT rctLeft = {100000,0,0,0}, rctCur;
	int nCount = TabCtrl_GetItemCount(hwnd) - 1;
	UINT nItem=0;
    int i;
    
	for(i = 0;i < nCount;i++) {
        TabCtrl_GetItemRect(hwnd, i, &rctCur);
		if(rctCur.left > 0 && rctCur.left <= rctLeft.left) {
			if(rctCur.bottom > rctLeft.bottom) {
				rctLeft = rctCur;
				nItem = i;	
			}
		}
	}
	return nItem;
}

static struct colOptions {UINT id; UINT defclr; char *szKey; } tabcolors[] = {
    IDC_TXTCLRNORMAL, COLOR_BTNTEXT, "tab_txt_normal",
    IDC_TXTCLRACTIVE, COLOR_BTNTEXT, "tab_txt_active",
    IDC_TXTCLRUNREAD, COLOR_HOTLIGHT, "tab_txt_unread",
    IDC_TXTCLRHOTTRACK, COLOR_HOTLIGHT, "tab_txt_hottrack",
    IDC_BKGCLRNORMAL, COLOR_3DFACE, "tab_bg_normal",
    IDC_BKGCLRACTIVE, COLOR_3DFACE, "tab_bg_active",
    IDC_BKGCLRUNREAD, COLOR_3DFACE, "tab_bg_unread",
    IDC_BKGCOLORHOTTRACK, COLOR_3DFACE, "tab_bg_hottrack",
    0, 0, NULL
};

#define HINT_ACTIVATE_RIGHT_SIDE 1
#define HINT_ACTIVE_ITEM 2
#define FLOAT_ITEM_HEIGHT_SHIFT 2
#define ACTIVE_ITEM_HEIGHT_SHIFT 2
#define SHIFT_FROM_CUT_TO_SPIN 4
#define HINT_TRANSPARENT 16
#define HINT_HOTTRACK 32

/*
 * draws the item contents (icon and label)
 * it obtains the label and icon handle directly from the message window data
 */
 
void DrawItem(struct myTabCtrl *tabdat, HDC dc, RECT *rcItem, int nHint, int nItem)
{
    TCITEM item = {0};
    struct MessageWindowData *dat = 0;
    DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER;
    item.mask = TCIF_PARAM;
    TabCtrl_GetItem(tabdat->hwnd, nItem, &item);

    /*
     * get the message window data for the session to which this tab item belongs
     */
    
    dat = (struct MessageWindowData *)GetWindowLong((HWND)item.lParam, GWL_USERDATA);
    
    if(dat) {
        HICON hIcon;
        HBRUSH bg;
        HFONT oldFont;
        DWORD dwStyle = tabdat->dwStyle;
        BOOL bFill = (dwStyle & TCS_BUTTONS);
        int oldMode = 0;
        InflateRect(rcItem, -1, -1);
        
        if(nHint & HINT_ACTIVE_ITEM)
            SetTextColor(dc, myGlobals.tabConfig.colors[1]);
        else if(dat->mayFlashTab == TRUE)
            SetTextColor(dc, myGlobals.tabConfig.colors[2]);
        else if(nHint & HINT_HOTTRACK)
            SetTextColor(dc, myGlobals.tabConfig.colors[3]);
        else
            SetTextColor(dc, myGlobals.tabConfig.colors[0]);

        oldMode = SetBkMode(dc, TRANSPARENT);

        if(bFill) {
            if(dat->mayFlashTab == TRUE)
                bg = myGlobals.tabConfig.m_hBrushUnread;
            else if(nHint & HINT_ACTIVE_ITEM)
                bg = myGlobals.tabConfig.m_hBrushActive;
            else if(nHint & HINT_HOTTRACK)
                bg = myGlobals.tabConfig.m_hBrushHottrack;
            else
                bg = myGlobals.tabConfig.m_hBrushDefault;
            FillRect(dc, rcItem, bg);
        }
        rcItem->left++;
        rcItem->right--;
        rcItem->top++;
        rcItem->bottom--;
        
        if(dat->dwFlags & MWF_ERRORSTATE)
            hIcon = myGlobals.g_iconErr;
        else if(dat->mayFlashTab)
            hIcon = dat->iFlashIcon;
        else
            hIcon = dat->hTabIcon;

        if(dat->mayFlashTab == FALSE || (dat->mayFlashTab == TRUE && dat->bTabFlash != 0) || !(myGlobals.m_TabAppearance & TCF_FLASHICON)) {
            DrawIconEx (dc, rcItem->left + tabdat->m_xpad - 1, (rcItem->bottom + rcItem->top - tabdat->cy) / 2, hIcon, tabdat->cx, tabdat->cy, 0, NULL, DI_NORMAL | DI_COMPAT); 
        }
        rcItem->left += (tabdat->cx + 2 + tabdat->m_xpad);
        
        if(dat->mayFlashTab == FALSE || (dat->mayFlashTab == TRUE && dat->bTabFlash != 0) || !(myGlobals.m_TabAppearance & TCF_FLASHLABEL)) {
            //oldFont = SelectObject(dc, myGlobals.tabConfig.m_hMenuFont);
            oldFont = SelectObject(dc, (HFONT)SendMessage(tabdat->hwnd, WM_GETFONT, 0, 0));
            if(!(tabdat->dwStyle & TCS_MULTILINE)) {
                rcItem->right -= tabdat->m_xpad;
                dwTextFlags |= DT_WORD_ELLIPSIS;
            }
            DrawText(dc, dat->newtitle, _tcslen(dat->newtitle), rcItem, dwTextFlags);
            SelectObject(dc, oldFont);
        }
        if(oldMode)
            SetBkMode(dc, oldMode);
    }
}

/*
 * draws the item rect in *classic* style (no visual themes
 */

void DrawItemRect(struct myTabCtrl *tabdat, HDC dc, RECT *rcItem, int nHint)
{
    POINT pt;
    DWORD dwStyle = tabdat->dwStyle;
    int rows = TabCtrl_GetRowCount(tabdat->hwnd);
    
    rcItem->bottom -= 1;
	if(rcItem->left >= 0) {

        /*
         * draw "button style" tabs... raised edge for hottracked, sunken edge for active (pushed)
         * otherwise, they get a normal border
         */
        
        if(dwStyle & TCS_BUTTONS) {
            // draw frame controls for button or bottom tabs
            if(dwStyle & TCS_BOTTOM) {
                rcItem->top++;
            }
            else
                rcItem->bottom--;
            
            rcItem->right += (rows == 1 ? 3 : 0);
            if(nHint & HINT_ACTIVE_ITEM)
                DrawEdge(dc, rcItem, EDGE_ETCHED, BF_RECT|BF_SOFT);
            else if(nHint & HINT_HOTTRACK)
                DrawEdge(dc, rcItem, EDGE_RAISED, BF_RECT|BF_SOFT);
            else
                DrawEdge(dc, rcItem, EDGE_BUMP, BF_RECT | BF_MONO | BF_SOFT);
            return;
        }
        SelectObject(dc, myGlobals.tabConfig.m_hPenLight);
		
		if(nHint & HINT_ACTIVE_ITEM) {
            if(dwStyle & TCS_BOTTOM) {
                FillRect(dc, rcItem, GetSysColorBrush(COLOR_3DFACE));
                rcItem->bottom +=2;
            }
            else {
                rcItem->bottom += 2;
                FillRect(dc, rcItem, GetSysColorBrush(COLOR_3DFACE));
                rcItem->bottom--;
                rcItem->top -=2;
            }
        }
        if(dwStyle & TCS_BOTTOM) {
            MoveToEx(dc, rcItem->left, rcItem->top - (nHint & HINT_ACTIVE_ITEM ? 1 : 0), &pt);
            LineTo(dc, rcItem->left, rcItem->bottom - 2);
            LineTo(dc, rcItem->left + 2, rcItem->bottom);
            SelectObject(dc, myGlobals.tabConfig.m_hPenShadow);
            LineTo(dc, rcItem->right - 3, rcItem->bottom);

            LineTo(dc, rcItem->right - 1, rcItem->bottom - 2);
            LineTo(dc, rcItem->right - 1, rcItem->top - 1);
            MoveToEx(dc, rcItem->right - 2, rcItem->top, &pt);
            SelectObject(dc, myGlobals.tabConfig.m_hPenItemShadow);
            LineTo(dc, rcItem->right - 2, rcItem->bottom - 1);
            MoveToEx(dc, rcItem->right - 3, rcItem->bottom - 1, &pt);
            LineTo(dc, rcItem->left + 2, rcItem->bottom - 1);
        }
        else {
            MoveToEx(dc, rcItem->left, rcItem->bottom, &pt);
            LineTo(dc, rcItem->left, rcItem->top + 2);
            LineTo(dc, rcItem->left + 2, rcItem->top);
            LineTo(dc, rcItem->right - 2, rcItem->top);
            SelectObject(dc, myGlobals.tabConfig.m_hPenItemShadow);

            MoveToEx(dc, rcItem->right - 2, rcItem->top + 1, &pt);
            LineTo(dc, rcItem->right - 2, rcItem->bottom + 1);
            SelectObject(dc, myGlobals.tabConfig.m_hPenShadow);
            MoveToEx(dc, rcItem->right - 1, rcItem->top + 2, &pt);
            LineTo(dc, rcItem->right - 1, rcItem->bottom + 1);
        }
	}
}

int DWordAlign(int n)
{ 
    int rem = n % 4; 
    if(rem) 
        n += (4 - rem); 
    return n; 
}

/*
 * draws a theme part (identifier in uiPartNameID) using the given clipping rectangle
 */

HRESULT DrawThemesPart(struct myTabCtrl *tabdat, HDC hDC, int iPartId, int iStateId, LPRECT prcBox)
{
    HRESULT hResult;
    
    if(pfnDrawThemeBackground == NULL)
        return 0;
    
    if(tabdat->hTheme != 0)
        hResult = pfnDrawThemeBackground(tabdat->hTheme, hDC, iPartId, iStateId, prcBox, NULL);
	return hResult;
}

/*
 * draw a themed tab item. mirrors the bitmap for bottom-aligned tabs
 */

void DrawThemesXpTabItem(HDC pDC, int ixItem, RECT *rcItem, UINT uiFlag, struct myTabCtrl *tabdat) 
{
	BOOL bBody  = (uiFlag & 1) ? TRUE : FALSE;
	BOOL bSel   = (uiFlag & 2) ? TRUE : FALSE;
	BOOL bHot   = (uiFlag & 4) ? TRUE : FALSE;
	BOOL bBottom = (uiFlag & 8) ? TRUE : FALSE;	// mirror
    SIZE szBmp;
    HDC  dcMem;
    HBITMAP bmpMem, pBmpOld;
    RECT rcMem;
    BITMAPINFO biOut; 
    BITMAPINFOHEADER *bihOut;
    int nBmpWdtPS;
    int nSzBuffPS;
    LPBYTE pcImg = NULL, pcImg1 = NULL;
    int nStart = 0, nLenSub = 0;
    szBmp.cx = rcItem->right - rcItem->left;
    szBmp.cy = rcItem->bottom - rcItem->top;
    
    dcMem = CreateCompatibleDC(pDC);
	bmpMem = CreateCompatibleBitmap(pDC, szBmp.cx, szBmp.cy);

    pBmpOld = SelectObject(dcMem, bmpMem);

    rcMem.left = rcMem.top = 0;
    rcMem.right = szBmp.cx;
    rcMem.bottom = szBmp.cy;
    
    //if(bSel)
//        rcMem.bottom++;

    /*
     * blit the background to the memory dc, so that transparent tabs will draw properly
     * for bottom tabs, it's more complex, because the background part must not be mirrored
     * the body part does not need that (filling with the background color is much faster
     * and sufficient for the tab "page" part.
     */
    
    if(bBody)
        FillRect(dcMem, rcItem, GetSysColorBrush(COLOR_3DFACE));
    else {
        if(!bBottom)
            BitBlt(dcMem, 0, 0, szBmp.cx, szBmp.cy, pDC, rcItem->left, rcItem->top, SRCCOPY);
        else {
            /*
             * mirror the background horizontally for bottom tabs.
             */
            BitBlt(dcMem, 0, 0, szBmp.cx, szBmp.cy, pDC, rcItem->left, rcItem->top, SRCCOPY);

            ZeroMemory(&biOut,sizeof(BITMAPINFO));	// Fill local pixel arrays
            bihOut = &biOut.bmiHeader;

            bihOut->biSize = sizeof (BITMAPINFOHEADER);
            bihOut->biCompression = BI_RGB;
            bihOut->biPlanes = 1;		  
            bihOut->biBitCount = 24;	// force as RGB: 3 bytes, 24 bits
            bihOut->biWidth =szBmp.cx; 
            bihOut->biHeight=szBmp.cy;

            nBmpWdtPS = DWordAlign(szBmp.cx*3);
            nSzBuffPS = ((nBmpWdtPS * szBmp.cy) / 8 + 2) * 8;

            pcImg1 = malloc(nSzBuffPS);

            GetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg1, &biOut, DIB_RGB_COLORS);
            bihOut->biHeight = -szBmp.cy; 				// to mirror bitmap is eough to use negative height between Get/SetDIBits
            SetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg1, &biOut, DIB_RGB_COLORS);

            free(pcImg1);
        }
    }
    
    if(bBody) 
        DrawThemesPart(tabdat, dcMem, 9, 0, &rcMem);	// TABP_PANE=9,  0, 'TAB'
    else { 
        int iStateId = bSel ? 3 : (bHot ? 2 : 1);
        DrawThemesPart(tabdat, dcMem, rcItem->left < 20 ? 2 : 1, iStateId, &rcMem);
    }
																// TABP_TABITEM=1, TIS_SELECTED=3:TIS_HOT=2:TIS_NORMAL=1, 'TAB'
    ZeroMemory(&biOut,sizeof(BITMAPINFO));	// Fill local pixel arrays
    bihOut = &biOut.bmiHeader;
    
	bihOut->biSize = sizeof (BITMAPINFOHEADER);
	bihOut->biCompression = BI_RGB;
	bihOut->biPlanes = 1;		  
    bihOut->biBitCount = 24;	// force as RGB: 3 bytes, 24 bits
	bihOut->biWidth =szBmp.cx; 
    bihOut->biHeight=szBmp.cy;

	nBmpWdtPS = DWordAlign(szBmp.cx*3);
	nSzBuffPS = ((nBmpWdtPS * szBmp.cy) / 8 + 2) * 8;
    
	if(bBottom)
        pcImg = malloc(nSzBuffPS);
        
	if(bBody && bBottom) {
        nStart = 3;
        nLenSub = 4;	// if bottom oriented flip the body contest only (no shadows were flipped)
    }
    // flip (mirror) the bitmap horizontally
    
    if(!bBody && bBottom)	{									// get bits: 
		GetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
        bihOut->biHeight = -szBmp.cy; 				// to mirror bitmap is eough to use negative height between Get/SetDIBits
        SetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
    }
    
	if(pcImg)
        free(pcImg);

    /*
     * finally, blit the result to the destination dc
     */
    
    BitBlt(pDC, rcItem->left, rcItem->top, szBmp.cx, szBmp.cy, dcMem, 0, 0, SRCCOPY);
	SelectObject(dcMem, pBmpOld);
    DeleteObject(bmpMem);
    DeleteDC(dcMem);
}

#define FIXED_TAB_SIZE 100

BOOL CALLBACK TabControlSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    struct myTabCtrl *tabdat = 0;
    tabdat = (struct myTabCtrl *)GetWindowLong(hwnd, GWL_USERDATA);
    
    if(tabdat)
        tabdat->dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    
    switch(msg) {
        case EM_SUBCLASSED:
        {
            tabdat = (struct myTabCtrl *)malloc(sizeof(struct myTabCtrl));
            SetWindowLong(hwnd, GWL_USERDATA, (LONG)tabdat);
            ZeroMemory((void *)tabdat, sizeof(struct myTabCtrl));
            tabdat->hwnd = hwnd;
            tabdat->cx = GetSystemMetrics(SM_CXSMICON);
            tabdat->cy = GetSystemMetrics(SM_CYSMICON);

            tabdat->m_skinning = FALSE;
            if(IsWinVerXPPlus() && myGlobals.m_VSApiEnabled != 0) {
                if(pfnIsThemeActive != 0)
                    if(pfnIsThemeActive()) {
                        tabdat->m_skinning = TRUE;
                        if(pfnOpenThemeData != 0) {
                            if((tabdat->hTheme = pfnOpenThemeData(NULL, L"TAB")) == 0)
                                tabdat->m_skinning = FALSE;
                        }
                    }
            }
            tabdat->m_xpad = DBGetContactSettingByte(NULL, SRMSGMOD_T, "x-pad", 4);
            tabdat->m_fixedwidth = DBGetContactSettingDword(NULL, SRMSGMOD_T, "fixedtabs", FIXED_TAB_SIZE);
            return 0;
        }
        case EM_THEMECHANGED:
            tabdat->m_skinning = FALSE;
            if(IsWinVerXPPlus() && myGlobals.m_VSApiEnabled != 0) {
                if(pfnIsThemeActive != 0)
                    if(pfnIsThemeActive()) {
                        tabdat->m_skinning = TRUE;
                        if(tabdat->hTheme != 0 && pfnCloseThemeData != 0)
                            pfnCloseThemeData(tabdat->hTheme);
                        if(pfnOpenThemeData != 0) {
                            if((tabdat->hTheme = pfnOpenThemeData(NULL, L"TAB")) == 0)
                                tabdat->m_skinning = FALSE;
                        }
                    }
            }
            break;
        case EM_SEARCHSCROLLER:
        {
            HWND hwndChild;
            /*
             * search the updown control (scroll arrows) to subclass it...
             * the control is dynamically created and may not exist as long as it is
             * not needed. So we have to search it everytime we need to paint. However,
             * it is sufficient to search it once. So this message is called, whenever
             * a new tab is inserted
             */

            if((hwndChild = FindWindowEx(hwnd, 0, _T("msctls_updown32"), NULL)) != 0)
                DestroyWindow(hwndChild);
            return 0;
        }
        case EM_VALIDATEBOTTOM:
            {
                BOOL bClassicDraw = (tabdat->m_skinning == FALSE) || (myGlobals.m_TabAppearance & TCF_NOSKINNING) || (tabdat->dwStyle & TCS_BUTTONS) || (tabdat->dwStyle & TCS_BOTTOM && (myGlobals.m_TabAppearance & TCF_FLAT));
                if((tabdat->dwStyle & TCS_BOTTOM) && !bClassicDraw && myGlobals.tabConfig.m_bottomAdjust != 0) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                break;
            }
        case WM_HSCROLL:
        {
            BOOL bClassicDraw = (tabdat->m_skinning == FALSE) || (myGlobals.m_TabAppearance & TCF_NOSKINNING) || (tabdat->dwStyle & TCS_BUTTONS) || (tabdat->dwStyle & TCS_BOTTOM && (myGlobals.m_TabAppearance & TCF_FLAT));
            if((tabdat->dwStyle & TCS_BOTTOM) && !bClassicDraw && myGlobals.tabConfig.m_bottomAdjust != 0) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case TCM_INSERTITEM:
        case TCM_DELETEITEM:
            if(!(tabdat->dwStyle & TCS_MULTILINE)) {
                LRESULT result;
                RECT rc;
                if(TabCtrl_GetItemCount(hwnd) >= 1 && msg == TCM_INSERTITEM) {
                    TabCtrl_GetItemRect(hwnd, 0, &rc);
                    TabCtrl_SetItemSize(hwnd, 10, rc.bottom - rc.top);
                }
                result = CallWindowProc(OldTabControlProc, hwnd, msg, wParam, lParam);
                TabCtrl_GetItemRect(hwnd, 0, &rc);
                SendMessage(hwnd, EM_SEARCHSCROLLER, 0, 0);
                SendMessage(hwnd, WM_SIZE, 0, 0);
                return result;
            }
            break;
        case WM_DESTROY:
        case EM_UNSUBCLASSED:
            if(tabdat) {
                if(tabdat->hTheme != 0 && pfnCloseThemeData != 0)
                    pfnCloseThemeData(tabdat->hTheme);
                // clean up pre-created gdi objects
                free(tabdat);
                SetWindowLong(hwnd, GWL_USERDATA, 0L);
            }
            break;
        case WM_MBUTTONDOWN: {
            POINT pt;
            GetCursorPos(&pt);
            SendMessage(GetParent(hwnd), DM_CLOSETABATMOUSE, 0, (LPARAM)&pt);
            return 1;
        }
        case WM_SIZE:
        {
            if(!(tabdat->dwStyle & TCS_MULTILINE)) {
                RECT rcClient, rc;
                DWORD newItemSize;
                int iTabs = TabCtrl_GetItemCount(hwnd);
                if(iTabs > 1) {
                    GetClientRect(hwnd, &rcClient);
                    TabCtrl_GetItemRect(hwnd, iTabs - 1, &rc);
                    newItemSize = (rcClient.right - rcClient.left) - 6 - (tabdat->dwStyle & TCS_BUTTONS ? (iTabs - 1) * 12 : 0);
                    //_DebugPopup(0, "width: %d, is: %d", rcClient.right - rcClient.left, newItemSize);
                    newItemSize = newItemSize / iTabs;
                    if(newItemSize < tabdat->m_fixedwidth) {
                        TabCtrl_SetItemSize(hwnd, newItemSize, rc.bottom - rc.top);
                    }
                    else {
                        TabCtrl_SetItemSize(hwnd, tabdat->m_fixedwidth, rc.bottom - rc.top);
                    }
                }
            }
            break;
        }
        case WM_ERASEBKGND:
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdcreal, hdc;
            RECT rectTemp, rctPage, rctActive, rcItem;
            RECT rectUpDn = {0,0,0,0};
            int nCount = TabCtrl_GetItemCount(hwnd), i;
            TCITEM item = {0};
            int iActive, hotItem;
            POINT pt;
            DWORD dwStyle = tabdat->dwStyle;
            HPEN hPenOld = 0;
            UINT uiFlags = 1;
            UINT uiBottom = 0;
            TCHITTESTINFO hti;
            BOOL bClassicDraw = (tabdat->m_skinning == FALSE) || (myGlobals.m_TabAppearance & TCF_NOSKINNING) || (dwStyle & TCS_BUTTONS) || (dwStyle & TCS_BOTTOM && (myGlobals.m_TabAppearance & TCF_FLAT));
            HBITMAP bmpMem, bmpOld;
            DWORD cx, cy;
            
            hdcreal = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rctPage);
            iActive = TabCtrl_GetCurSel(hwnd);
            TabCtrl_GetItemRect(hwnd, iActive, &rctActive);
            cx = rctPage.right - rctPage.left;
            cy = rctPage.bottom - rctPage.top;
            
            /*
             * draw everything to a memory dc to avoid flickering
             */
            
            hdc = CreateCompatibleDC(hdcreal);
            bmpMem = CreateCompatibleBitmap(hdcreal, cx, cy);

            bmpOld = SelectObject(hdc, bmpMem);

            FillRect(hdc, &ps.rcPaint, GetSysColorBrush(COLOR_3DFACE));
            if(dwStyle & TCS_BUTTONS) {
                RECT rc1;
                TabCtrl_GetItemRect(hwnd, nCount - 1, &rc1);
                if(dwStyle & TCS_BOTTOM) {
                    rctPage.bottom = rc1.top;
                    uiBottom = 8;
                }
                else {
                    rctPage.top = rc1.bottom;
                    uiBottom = 0;
                }
            }
            else {
                if(dwStyle & TCS_BOTTOM) {
                    rctPage.bottom = rctActive.top;
                    uiBottom = 8;
                }
                else {
                    rctPage.top = rctActive.bottom;
                    uiBottom = 0;
                }
            }
            
            hPenOld = SelectObject(hdc, myGlobals.tabConfig.m_hPenLight);
            /*
             * visual style support
             */
            
            if(!bClassicDraw && IntersectRect(&rectTemp, &rctPage, &ps.rcPaint)) {
                RECT rcClip, rcClient;
                
                GetClipBox(hdc, &rcClip);
                rcClient = rctPage;
                if(dwStyle & TCS_BOTTOM) {
                    rcClient.bottom = rctPage.bottom;
                    uiFlags |= uiBottom;
                }
                else
                    rcClient.top = rctPage.top;
                DrawThemesXpTabItem(hdc, -1, &rcClient, uiFlags, tabdat);	// TABP_PANE=9,0,'TAB'
            }
            else {
                if(IntersectRect(&rectTemp, &rctPage, &ps.rcPaint)) {
                    if(dwStyle & TCS_BUTTONS) {
                        rectTemp = rctPage;
                        if(dwStyle & TCS_BOTTOM) {
                            rectTemp.top--;
                            rectTemp.bottom--;
                        }
                        else {
                            rectTemp.bottom--;
                            rectTemp.top++;
                        }
                        MoveToEx(hdc, rectTemp.left, rectTemp.bottom, &pt);
                        LineTo(hdc, rectTemp.left, rectTemp.top + 1);
                        LineTo(hdc, rectTemp.right - 1, rectTemp.top + 1);
                        SelectObject(hdc, myGlobals.tabConfig.m_hPenShadow);
                        LineTo(hdc, rectTemp.right - 1, rectTemp.bottom);
                        LineTo(hdc, rectTemp.left, rectTemp.bottom);
                    }
                    else {
                        rectTemp = rctPage;

                        MoveToEx(hdc, rectTemp.left, rectTemp.bottom - 1, &pt);
                        LineTo(hdc, rectTemp.left, rectTemp.top);

                        if(dwStyle & TCS_BOTTOM) {
                            LineTo(hdc, rectTemp.right - 1, rectTemp.top);
                            SelectObject(hdc, myGlobals.tabConfig.m_hPenShadow);
                            LineTo(hdc, rectTemp.right - 1, rectTemp.bottom - 1);
                            LineTo(hdc, rctActive.right, rectTemp.bottom - 1);
                            MoveToEx(hdc, rctActive.left - 2, rectTemp.bottom -1, &pt);
                            LineTo(hdc, rectTemp.left - 1, rectTemp.bottom - 1);
                            SelectObject(hdc, myGlobals.tabConfig.m_hPenItemShadow);
                            MoveToEx(hdc, rectTemp.right - 2, rectTemp.top + 1, &pt);
                            LineTo(hdc, rectTemp.right - 2, rectTemp.bottom - 2);
                            LineTo(hdc, rctActive.right, rectTemp.bottom - 2);
                            MoveToEx(hdc, rctActive.left - 2, rectTemp.bottom - 2, &pt);
                            LineTo(hdc, rectTemp.left, rectTemp.bottom - 2);
                        }
                        else {
                            if(rctActive.left >= 0) {
                                LineTo(hdc, rctActive.left, rctActive.bottom);
                                if(IsRectEmpty(&rectUpDn))
                                    MoveToEx(hdc, rctActive.right, rctActive.bottom, &pt);
                                else {
                                    if(rctActive.right >= rectUpDn.left)
                                        MoveToEx(hdc, rectUpDn.left - SHIFT_FROM_CUT_TO_SPIN + 2, rctActive.bottom + 1, &pt);
                                    else
                                        MoveToEx(hdc, rctActive.right, rctActive.bottom, &pt);
                                }
                                LineTo(hdc, rectTemp.right - 2, rctActive.bottom);
                            }
                            else {
                                RECT rectItemLeftmost;
                                UINT nItemLeftmost = FindLeftDownItem(hwnd);
                                TabCtrl_GetItemRect(hwnd, nItemLeftmost, &rectItemLeftmost);
                                LineTo(hdc, rectTemp.right - 2, rctActive.bottom);
                            }
                            SelectObject(hdc, myGlobals.tabConfig.m_hPenItemShadow);
                            LineTo(hdc, rectTemp.right - 2, rectTemp.bottom - 2);
                            LineTo(hdc, rectTemp.left, rectTemp.bottom - 2);

                            SelectObject(hdc, myGlobals.tabConfig.m_hPenShadow);
                            MoveToEx(hdc, rectTemp.right - 1, rctActive.bottom, &pt);
                            LineTo(hdc, rectTemp.right - 1, rectTemp.bottom - 1);
                            LineTo(hdc, rectTemp.left - 2, rectTemp.bottom - 1);
                        }
                    }
                }
            }
            uiFlags = 0;
            /*
             * figure out hottracked item (if any)
             */
            GetCursorPos(&hti.pt);
            ScreenToClient(hwnd, &hti.pt);
            hti.flags = 0;
            hotItem = TabCtrl_HitTest(hwnd, &hti);
            for(i = 0; i < nCount; i++) {
                if(i != iActive) {
                    TabCtrl_GetItemRect(hwnd, i, &rcItem);
                    if(!bClassicDraw && uiBottom) {
                        rcItem.top -= myGlobals.tabConfig.m_bottomAdjust;
                        rcItem.bottom -= myGlobals.tabConfig.m_bottomAdjust;
                    }
                    if (IntersectRect(&rectTemp, &rcItem, &ps.rcPaint)) {
                        int nHint = 0;
                        if(!bClassicDraw) {
                            InflateRect(&rcItem, 0, 0);
                            DrawThemesXpTabItem(hdc, i, &rcItem, uiFlags | uiBottom | (i == hotItem ? 4 : 0), tabdat);
                            DrawItem(tabdat, hdc, &rcItem, nHint | (i == hotItem ? HINT_HOTTRACK : 0), i);
                        }
                        else {
                            DrawItemRect(tabdat, hdc, &rcItem, nHint | (i == hotItem ? HINT_HOTTRACK : 0));
                            DrawItem(tabdat, hdc, &rcItem, nHint | (i == hotItem ? HINT_HOTTRACK : 0), i);
                        }
                    }
                }
            }
            /*
             * draw the active item
             */
            if(!bClassicDraw && uiBottom) {
                rctActive.top -= myGlobals.tabConfig.m_bottomAdjust;
                rctActive.bottom -= myGlobals.tabConfig.m_bottomAdjust;
            }
            if (IntersectRect(&rectTemp, &rctActive, &ps.rcPaint) && rctActive.left >= 0) {
                int nHint = 0;
                rcItem = rctActive;
                if(!bClassicDraw) {
                    //if(!uiBottom)
                        //rcItem.bottom--;
                    InflateRect(&rcItem, 2, 2);
                    DrawThemesXpTabItem(hdc, iActive, &rcItem, 2 | uiBottom, tabdat);
                    DrawItem(tabdat, hdc, &rcItem, nHint | HINT_ACTIVE_ITEM, iActive);
                }
                else {
                    if(!(dwStyle & TCS_BUTTONS)) {
                        if(iActive == 0) {
                            rcItem.right+=2;
                            rcItem.left--;
                        }
                        else
                            InflateRect(&rcItem, 2, 0);
                    }
                    DrawItemRect(tabdat, hdc, &rcItem, HINT_ACTIVATE_RIGHT_SIDE|HINT_ACTIVE_ITEM | nHint);
                    DrawItem(tabdat, hdc, &rcItem, HINT_ACTIVE_ITEM | nHint, iActive);
                }
            }
            if(hPenOld)
                SelectObject(hdc, hPenOld);

            /*
             * finally, bitblt the contents of the memory dc to the real dc
             */
            BitBlt(hdcreal, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);
            SelectObject(hdc, bmpOld);
            DeleteObject(bmpMem);
            DeleteDC(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            short amount = (short)(HIWORD(wParam));
            if(lParam != -1)
                break;
            if(amount > 0)
                SendMessage(GetParent(hwnd), DM_SELECTTAB, DM_SELECT_PREV, 0);
            else if(amount < 0)
                SendMessage(GetParent(hwnd), DM_SELECTTAB, DM_SELECT_NEXT, 0);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
    }
    return CallWindowProc(OldTabControlProc, hwnd, msg, wParam, lParam); 
}

void ReloadTabConfig()
{
    NONCLIENTMETRICS nclim;
    int i = 0;
    BOOL iLabelDefault = myGlobals.m_TabAppearance & TCF_LABELUSEWINCOLORS;
    BOOL iBkgDefault = myGlobals.m_TabAppearance & TCF_BKGUSEWINCOLORS;
    
    myGlobals.tabConfig.m_hPenLight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
    myGlobals.tabConfig.m_hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
    myGlobals.tabConfig.m_hPenItemShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

    nclim.cbSize = sizeof(nclim);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &nclim, 0);
    myGlobals.tabConfig.m_hMenuFont = CreateFontIndirect(&nclim.lfMessageFont);

    while(tabcolors[i].szKey != NULL) {
        if(i < 4)
            myGlobals.tabConfig.colors[i] = iLabelDefault ? GetSysColor(tabcolors[i].defclr) : DBGetContactSettingDword(NULL, SRMSGMOD_T, tabcolors[i].szKey, GetSysColor(tabcolors[i].defclr));
        else
            myGlobals.tabConfig.colors[i] = iBkgDefault ? GetSysColor(tabcolors[i].defclr) :  DBGetContactSettingDword(NULL, SRMSGMOD_T, tabcolors[i].szKey, GetSysColor(tabcolors[i].defclr));
        i++;
    }

    myGlobals.tabConfig.m_hBrushDefault = CreateSolidBrush(myGlobals.tabConfig.colors[4]);
    myGlobals.tabConfig.m_hBrushActive = CreateSolidBrush(myGlobals.tabConfig.colors[5]);
    myGlobals.tabConfig.m_hBrushUnread = CreateSolidBrush(myGlobals.tabConfig.colors[6]);
    myGlobals.tabConfig.m_hBrushHottrack = CreateSolidBrush(myGlobals.tabConfig.colors[7]);
    
    myGlobals.tabConfig.m_bottomAdjust = (int)DBGetContactSettingDword(NULL, SRMSGMOD_T, "bottomadjust", 0);
}

void FreeTabConfig()
{
    DeleteObject(myGlobals.tabConfig.m_hPenItemShadow);
    DeleteObject(myGlobals.tabConfig.m_hPenLight);
    DeleteObject(myGlobals.tabConfig.m_hPenShadow);
    DeleteObject(myGlobals.tabConfig.m_hMenuFont);
    DeleteObject(myGlobals.tabConfig.m_hBrushActive);
    DeleteObject(myGlobals.tabConfig.m_hBrushDefault);
    DeleteObject(myGlobals.tabConfig.m_hBrushUnread);
    DeleteObject(myGlobals.tabConfig.m_hBrushHottrack);
}

/*
 * options dialog for setting up tab options
 */

BOOL CALLBACK DlgProcTabConfig(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
        {
            DWORD dwFlags = DBGetContactSettingDword(NULL, SRMSGMOD_T, "tabconfig", TCF_DEFAULT);
            int i = 0;
            COLORREF clr;

            TranslateDialogDefault(hwndDlg);
            
            SetWindowTextA(hwndDlg, Translate("Configure tab appearance"));
            CheckDlgButton(hwndDlg, IDC_FLATTABS2, dwFlags & TCF_FLAT);
            CheckDlgButton(hwndDlg, IDC_FLASHICON, dwFlags & TCF_FLASHICON);
            CheckDlgButton(hwndDlg, IDC_FLASHLABEL, dwFlags & TCF_FLASHLABEL);
            CheckDlgButton(hwndDlg, IDC_NOSKINNING, dwFlags & TCF_NOSKINNING);
            CheckDlgButton(hwndDlg, IDC_SINGLEROWTAB, dwFlags & TCF_SINGLEROWTABCONTROL);
            CheckDlgButton(hwndDlg, IDC_LABELUSEWINCOLORS, dwFlags & TCF_LABELUSEWINCOLORS);
            CheckDlgButton(hwndDlg, IDC_BKGUSEWINCOLORS2, dwFlags & TCF_BKGUSEWINCOLORS);
            CheckDlgButton(hwndDlg, IDC_ALWAYSFIXED, dwFlags & TCF_ALWAYSFIXEDWIDTH);
            
            SendMessage(hwndDlg, WM_COMMAND, IDC_LABELUSEWINCOLORS, 0);
            
            if(myGlobals.m_VSApiEnabled == 0) {
                CheckDlgButton(hwndDlg, IDC_NOSKINNING, TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_NOSKINNING), FALSE);
            }
            while(tabcolors[i].szKey != NULL) {
                clr = (COLORREF)DBGetContactSettingDword(NULL, SRMSGMOD_T, tabcolors[i].szKey, GetSysColor(tabcolors[i].defclr));
                SendDlgItemMessage(hwndDlg, tabcolors[i].id, CPM_SETCOLOUR, 0, (LPARAM)clr);
                i++;
            }

            SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
            SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPIN, UDM_SETPOS, 0, (int)DBGetContactSettingByte(NULL, SRMSGMOD_T, "tborder", 0));
            SetDlgItemInt(hwndDlg, IDC_TABBORDER, (int)DBGetContactSettingByte(NULL, SRMSGMOD_T, "tborder", 0), FALSE);;

            SendDlgItemMessage(hwndDlg, IDC_BOTTOMTABADJUSTSPIN, UDM_SETRANGE, 0, MAKELONG(3, -3));
            SendDlgItemMessage(hwndDlg, IDC_BOTTOMTABADJUSTSPIN, UDM_SETPOS, 0, myGlobals.tabConfig.m_bottomAdjust);
            SetDlgItemInt(hwndDlg, IDC_BOTTOMTABADJUST, myGlobals.tabConfig.m_bottomAdjust, TRUE);

            SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTER, UDM_SETRANGE, 0, MAKELONG(10, 0));
            SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTER, UDM_SETPOS, 0, (int)DBGetContactSettingByte(NULL, SRMSGMOD_T, "tborder_outer", 2));
            SetDlgItemInt(hwndDlg, IDC_TABBORDEROUTER, (int)DBGetContactSettingByte(NULL, SRMSGMOD_T, "tborder_outer", 2), FALSE);;

            SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(10, 1));
            SendDlgItemMessage(hwndDlg, IDC_SPIN3, UDM_SETRANGE, 0, MAKELONG(10, 1));
            SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETPOS, 0, (LPARAM)DBGetContactSettingByte(NULL, SRMSGMOD_T, "y-pad", 3));
            SendDlgItemMessage(hwndDlg, IDC_SPIN3, UDM_SETPOS, 0, (LPARAM)DBGetContactSettingByte(NULL, SRMSGMOD_T, "x-pad", 4));
            SetDlgItemInt(hwndDlg, IDC_TABPADDING, (int)DBGetContactSettingByte(NULL, SRMSGMOD_T, "y-pad", 3), FALSE);;
            SetDlgItemInt(hwndDlg, IDC_HTABPADDING, (int)DBGetContactSettingByte(NULL, SRMSGMOD_T, "x-pad", 4), FALSE);;
            ShowWindow(hwndDlg, SW_SHOWNORMAL);
            return TRUE;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_LABELUSEWINCOLORS:
                case IDC_BKGUSEWINCOLORS2:
                {
                    int i = 0;
                    BOOL iLabel = IsDlgButtonChecked(hwndDlg, IDC_LABELUSEWINCOLORS);
                    BOOL iBkg = IsDlgButtonChecked(hwndDlg, IDC_BKGUSEWINCOLORS2);
                    while(tabcolors[i].szKey != NULL) {
                        if(i < 4)
                            EnableWindow(GetDlgItem(hwndDlg, tabcolors[i].id), iLabel ? FALSE : TRUE);
                        else
                            EnableWindow(GetDlgItem(hwndDlg, tabcolors[i].id), iBkg ? FALSE : TRUE);
                        i++;
                    }
                    break;
                }
                case IDOK:
                {
                    int i = 0;
                    COLORREF clr;
                    BOOL translated;
                    
                    struct ContainerWindowData *pContainer = pFirstContainer;
                    
                    DWORD dwFlags = (IsDlgButtonChecked(hwndDlg, IDC_FLATTABS2) ? TCF_FLAT : 0) |
                                    (IsDlgButtonChecked(hwndDlg, IDC_FLASHICON) ? TCF_FLASHICON : 0) |
                                    (IsDlgButtonChecked(hwndDlg, IDC_FLASHLABEL) ? TCF_FLASHLABEL : 0) |
                                    (IsDlgButtonChecked(hwndDlg, IDC_SINGLEROWTAB) ? TCF_SINGLEROWTABCONTROL : 0) |
                                    (IsDlgButtonChecked(hwndDlg, IDC_LABELUSEWINCOLORS) ? TCF_LABELUSEWINCOLORS : 0) |
                                    (IsDlgButtonChecked(hwndDlg, IDC_BKGUSEWINCOLORS2) ? TCF_BKGUSEWINCOLORS : 0) |
                                    (IsDlgButtonChecked(hwndDlg, IDC_ALWAYSFIXED) ? TCF_ALWAYSFIXEDWIDTH : 0) |
                                    (IsDlgButtonChecked(hwndDlg, IDC_NOSKINNING) ? TCF_NOSKINNING : 0);
                    
                    DBWriteContactSettingDword(NULL, SRMSGMOD_T, "tabconfig", dwFlags);
                    myGlobals.m_TabAppearance = dwFlags;
                    while(tabcolors[i].szKey != NULL) {
                        DBGetContactSettingDword(NULL, SRMSGMOD_T, tabcolors[i].szKey, GetSysColor(tabcolors[i].defclr));
                        clr = SendDlgItemMessage(hwndDlg, tabcolors[i].id, CPM_GETCOLOUR, 0, 0);
                        DBWriteContactSettingDword(NULL, SRMSGMOD_T, tabcolors[i].szKey, clr);
                        i++;
                    }
                    DBWriteContactSettingByte(NULL, SRMSGMOD_T, "y-pad", GetDlgItemInt(hwndDlg, IDC_TABPADDING, NULL, FALSE));
                    DBWriteContactSettingByte(NULL, SRMSGMOD_T, "x-pad", GetDlgItemInt(hwndDlg, IDC_HTABPADDING, NULL, FALSE));
                    DBWriteContactSettingByte(NULL, SRMSGMOD_T, "tborder", (BYTE) GetDlgItemInt(hwndDlg, IDC_TABBORDER, &translated, FALSE));
                    DBWriteContactSettingByte(NULL, SRMSGMOD_T, "tborder_outer", (BYTE) GetDlgItemInt(hwndDlg, IDC_TABBORDEROUTER, &translated, FALSE));
                    DBWriteContactSettingDword(NULL, SRMSGMOD_T, "bottomadjust", GetDlgItemInt(hwndDlg, IDC_BOTTOMTABADJUST, &translated, TRUE));
                    
                    FreeTabConfig();
                    ReloadTabConfig();
                    while(pContainer) {
                        TabCtrl_SetPadding(GetDlgItem(pContainer->hwnd, IDC_MSGTABS), GetDlgItemInt(hwndDlg, IDC_HTABPADDING, NULL, FALSE), GetDlgItemInt(hwndDlg, IDC_TABPADDING, NULL, FALSE));
                        RedrawWindow(GetDlgItem(pContainer->hwnd, IDC_MSGTABS), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
                        pContainer = pContainer->pNextContainer;
                    }
                    DestroyWindow(hwndDlg);
                    break;
                }
                case IDCANCEL:
                    DestroyWindow(hwndDlg);
                    break;
            }
    }
    return FALSE;
}



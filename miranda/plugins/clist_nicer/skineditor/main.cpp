/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2016 Miranda ICQ/IM project, 
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
#include "skineditres.h"

#define ID_EXTBKSEPARATOR           40200

/*
PLUGININFO pluginInfo = {
    sizeof(PLUGININFO), 
	"Skin editor",
	PLUGIN_MAKE_VERSION(0, 0, 0, 4), 
	"Skin editor for clist_nicer+", 
	"Nightwish", 
	"", 
	"Copyright 2000-2006 Miranda-IM project", 
	"http://www.miranda-im.org", 
	0, 
	0
};
*/

PLUGININFOEX pluginInfo = {
#if defined(_UNICODE)
		sizeof(PLUGININFOEX), "Skin editor for clist_nicer+ (unicode)", PLUGIN_MAKE_VERSION(0, 0, 0, 4),
#else
		sizeof(PLUGININFOEX), "Skin editor for clist_nicer+", PLUGIN_MAKE_VERSION(0, 0, 0, 4),
#endif		
		"Allow inline skin item editing for clist nicer+",
		"Nightwish, Pixel", "", "Copyright 2000-2006 Miranda-IM project", "http://www.miranda-im.org",
		UNICODE_AWARE,
        0,
#if defined(_UNICODE)
        {0x21948c89, 0xb549, 0x4c9d, { 0x8b, 0x4f, 0x3f, 0x37, 0x26, 0xec, 0x6b, 0x4b }}
#else
        {0xa0c06bfe, 0x64cf, 0x487e, { 0x82, 0x87, 0x8c, 0x9b, 0x1, 0x97, 0x7d, 0xff }}
#endif
};

HINSTANCE g_hInst = 0;
PLUGINLINK *pluginLink;
struct MM_INTERFACE memoryManagerInterface;

StatusItems_t *StatusItems;
ChangedSItems_t ChangedSItems = {0};

static int LastModifiedItem = -1;
static int last_selcount = 0;
static int last_indizes[64];
static int ID_EXTBK_LAST = 0, ID_EXTBK_FIRST = 0;

/*                                                              
 * prototypes                                                                
 */

static void ChangeControlItems(HWND hwndDlg, int status, int except);
static BOOL CheckItem(int item, HWND hwndDlg);

static void ReActiveCombo(HWND hwndDlg)
{
    if (IsDlgButtonChecked(hwndDlg, IDC_IGNORE)) {
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));

        EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR2), !IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2LABLE), !IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));

        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));      
        ChangeControlItems(hwndDlg, !IsDlgButtonChecked(hwndDlg, IDC_IGNORE), IDC_IGNORE);
    } else {
        ChangeControlItems(hwndDlg, !IsDlgButtonChecked(hwndDlg, IDC_IGNORE), IDC_IGNORE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));

        EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR2), !IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));
        EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2LABLE), !IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));

        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
    }
}

// enabled or disabled the whole status controlitems group (with exceptional control)
static void ChangeControlItems(HWND hwndDlg, int status, int except)
{
    if (except != IDC_GRADIENT)
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT), status);
    if (except != IDC_GRADIENT_LR)
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), status);
    if (except != IDC_GRADIENT_RL)
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), status);
    if (except != IDC_GRADIENT_TB)
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), status);
    if (except != IDC_GRADIENT_BT)
        EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), status);
    if (except != IDC_CORNER)
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER), status);
    if (except != IDC_CORNER_TL)
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), status);
    if (except != IDC_CORNER_TR)
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), status);
    if (except != IDC_CORNER_BR)
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), status);
    if (except != IDC_CORNER_BL)
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), status);
    if (except != IDC_CORNER_TL)
        EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), status);
    if (except != IDC_MARGINLABLE)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MARGINLABLE), status);
    if (except != IDC_MRGN_TOP)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_TOP), status);
    if (except != IDC_MRGN_RIGHT)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_RIGHT), status);
    if (except != IDC_MRGN_BOTTOM)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_BOTTOM), status);
    if (except != IDC_MRGN_LEFT)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_LEFT), status);
    if (except != IDC_MRGN_TOP_SPIN)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_TOP_SPIN), status);
    if (except != IDC_MRGN_RIGHT_SPIN)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_RIGHT_SPIN), status);
    if (except != IDC_MRGN_BOTTOM_SPIN)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_BOTTOM_SPIN), status);
    if (except != IDC_MRGN_LEFT_SPIN)
        EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_LEFT_SPIN), status);
    if (except != IDC_BASECOLOUR)
        EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR), status);
    if (except != IDC_COLORLABLE)
        EnableWindow(GetDlgItem(hwndDlg, IDC_COLORLABLE), status);
    if (except != IDC_BASECOLOUR2)
        EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR2), status);
    if (except != IDC_COLOR2LABLE)
        EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2LABLE), status);
    if (except != IDC_COLOR2_TRANSPARENT)
        EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2_TRANSPARENT), status);
    if (except != IDC_TEXTCOLOUR)
        EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOUR), status);
    if (except != IDC_TEXTCOLOURLABLE)
        EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOURLABLE), status);

    if (except != IDC_ALPHA)
        EnableWindow(GetDlgItem(hwndDlg, IDC_ALPHA), status);
    if (except != IDC_ALPHASPIN)
        EnableWindow(GetDlgItem(hwndDlg, IDC_ALPHASPIN), status);
    if (except != IDC_ALPHALABLE)
        EnableWindow(GetDlgItem(hwndDlg, IDC_ALPHALABLE), status);
    if (except != IDC_IGNORE)
        EnableWindow(GetDlgItem(hwndDlg, IDC_IGNORE), status);

    if (except != IDC_BORDERTYPE)
        EnableWindow(GetDlgItem(hwndDlg, IDC_BORDERTYPE), status);
    
}

static void FillOptionDialogByStatusItem(HWND hwndDlg, StatusItems_t *item)
{
    char itoabuf[15];
    DWORD ret;
    int index;
    
    CheckDlgButton(hwndDlg, IDC_IGNORE, (item->IGNORED) ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hwndDlg, IDC_GRADIENT, (item->GRADIENT & GRADIENT_ACTIVE) ? BST_CHECKED : BST_UNCHECKED);
    EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), item->GRADIENT & GRADIENT_ACTIVE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), item->GRADIENT & GRADIENT_ACTIVE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), item->GRADIENT & GRADIENT_ACTIVE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), item->GRADIENT & GRADIENT_ACTIVE);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_LR, (item->GRADIENT & GRADIENT_LR) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_RL, (item->GRADIENT & GRADIENT_RL) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_TB, (item->GRADIENT & GRADIENT_TB) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_BT, (item->GRADIENT & GRADIENT_BT) ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hwndDlg, IDC_CORNER, (item->CORNER & CORNER_ACTIVE) ? BST_CHECKED : BST_UNCHECKED);
    EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), item->CORNER & CORNER_ACTIVE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), item->CORNER & CORNER_ACTIVE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), item->CORNER & CORNER_ACTIVE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), item->CORNER & CORNER_ACTIVE);

    CheckDlgButton(hwndDlg, IDC_CORNER_TL, (item->CORNER & CORNER_TL) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_CORNER_TR, (item->CORNER & CORNER_TR) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_CORNER_BR, (item->CORNER & CORNER_BR) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_CORNER_BL, (item->CORNER & CORNER_BL) ? BST_CHECKED : BST_UNCHECKED);

    ret = item->COLOR;
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_COLOR);
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_SETCOLOUR, 0, ret);

    ret = item->COLOR2;
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_COLOR2);
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_SETCOLOUR, 0, ret);

    CheckDlgButton(hwndDlg, IDC_COLOR2_TRANSPARENT, (item->COLOR2_TRANSPARENT) ? BST_CHECKED : BST_UNCHECKED);

    ret = item->TEXTCOLOR;
    SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOUR, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_TEXTCOLOR);
    SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOUR, CPM_SETCOLOUR, 0, ret);

    if (item->ALPHA == -1) {
        SetDlgItemTextA(hwndDlg, IDC_ALPHA, "");
    } else {
        ret = item->ALPHA;
        _itoa(ret, itoabuf, 10);    
        SetDlgItemTextA(hwndDlg, IDC_ALPHA, itoabuf);
    }

    if (item->MARGIN_LEFT == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_LEFT, "");
    else {
        ret = item->MARGIN_LEFT;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_LEFT, itoabuf);
    }

    if (item->MARGIN_TOP == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_TOP, "");
    else {
        ret = item->MARGIN_TOP;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_TOP, itoabuf);
    }

    if (item->MARGIN_RIGHT == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_RIGHT, "");
    else {
        ret = item->MARGIN_RIGHT;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_RIGHT, itoabuf);
    }

    if (item->MARGIN_BOTTOM == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_BOTTOM, "");
    else {
        ret = item->MARGIN_BOTTOM;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_BOTTOM, itoabuf);
    }
    if(item->BORDERSTYLE == -1)
        SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_SETCURSEL, 0, 0);
    else {
        index = 0;
        switch(item->BORDERSTYLE) {
            case 0:
            case -1:
                index = 0;
                break;
            case BDR_RAISEDOUTER:
                index = 1;
                break;
            case BDR_SUNKENINNER:
                index = 2;
                break;
            case EDGE_BUMP:
                index = 3;
                break;
            case EDGE_ETCHED:
                index = 4;
                break;
        }
        SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_SETCURSEL, (WPARAM)index, 0);
    }
    ReActiveCombo(hwndDlg);
}
// update dlg with selected item
static void FillOptionDialogByCurrentSel(HWND hwndDlg)
{
    int index = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETCURSEL, 0, 0);
    int itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, index, 0);
    if(itemData != ID_EXTBKSEPARATOR) {
        LastModifiedItem = itemData - ID_EXTBK_FIRST;

        if (CheckItem(itemData - ID_EXTBK_FIRST, hwndDlg)) {
            FillOptionDialogByStatusItem(hwndDlg, &StatusItems[itemData - ID_EXTBK_FIRST]);
        }
    }
}


// enabled all status controls if the selected item is a separator
static BOOL CheckItem(int item, HWND hwndDlg)
{
    if (StatusItems[item].statusID == ID_EXTBKSEPARATOR) {
        ChangeControlItems(hwndDlg, 0, 0);
        return FALSE;
    } else {
        ChangeControlItems(hwndDlg, 1, 0);
        return TRUE;
    }
}

static void SetChangedStatusItemFlag(WPARAM wParam, HWND hwndDlg)
{
    if (LOWORD(wParam) != IDC_ITEMS
       && (GetDlgItem(hwndDlg, LOWORD(wParam)) == GetFocus() || HIWORD(wParam) == CPN_COLOURCHANGED)
       && (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == CPN_COLOURCHANGED)) {
        switch (LOWORD(wParam)) {
            case IDC_IGNORE:
                ChangedSItems.bIGNORED = TRUE; break;
            case IDC_GRADIENT:
                ChangedSItems.bGRADIENT = TRUE; break;
            case IDC_GRADIENT_LR:
                ChangedSItems.bGRADIENT = TRUE;break;
            case IDC_GRADIENT_RL:
                ChangedSItems.bGRADIENT = TRUE; break;
            case IDC_GRADIENT_BT:
                ChangedSItems.bGRADIENT = TRUE; break;
            case IDC_GRADIENT_TB:
                ChangedSItems.bGRADIENT = TRUE; break;

            case IDC_CORNER:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_TL:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_TR:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_BR:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_BL:
                ChangedSItems.bCORNER = TRUE; break;

            case IDC_BASECOLOUR:
                ChangedSItems.bCOLOR = TRUE; break;     
            case IDC_BASECOLOUR2:
                ChangedSItems.bCOLOR2 = TRUE; break;
            case IDC_COLOR2_TRANSPARENT:
                ChangedSItems.bCOLOR2_TRANSPARENT = TRUE; break;
            case IDC_TEXTCOLOUR:
                ChangedSItems.bTEXTCOLOR = TRUE; break;

            case IDC_ALPHA:
                ChangedSItems.bALPHA = TRUE; break;
            case IDC_ALPHASPIN:
                ChangedSItems.bALPHA = TRUE; break;

            case IDC_MRGN_LEFT:
                ChangedSItems.bMARGIN_LEFT = TRUE; break;
            case IDC_MRGN_LEFT_SPIN:
                ChangedSItems.bMARGIN_LEFT = TRUE; break;

            case IDC_MRGN_TOP:
                ChangedSItems.bMARGIN_TOP = TRUE; break;
            case IDC_MRGN_TOP_SPIN:
                ChangedSItems.bMARGIN_TOP = TRUE; break;

            case IDC_MRGN_RIGHT:
                ChangedSItems.bMARGIN_RIGHT = TRUE; break;
            case IDC_MRGN_RIGHT_SPIN:
                ChangedSItems.bMARGIN_RIGHT = TRUE; break;

            case IDC_MRGN_BOTTOM:
                ChangedSItems.bMARGIN_BOTTOM = TRUE; break;
            case IDC_MRGN_BOTTOM_SPIN:
                ChangedSItems.bMARGIN_BOTTOM = TRUE; break;

            case IDC_BORDERTYPE:
                ChangedSItems.bBORDERSTYLE = TRUE; break;
        }
    }
}

static BOOL isValidItem(void)
{
    if (StatusItems[LastModifiedItem].statusID == ID_EXTBKSEPARATOR)
        return FALSE;

    return TRUE;
}

// updates the struct with the changed dlg item
static void UpdateStatusStructSettingsFromOptDlg(HWND hwndDlg, int index)
{
    char buf[15];
    ULONG bdrtype;
    
    if (ChangedSItems.bIGNORED)
        StatusItems[index]. IGNORED = IsDlgButtonChecked(hwndDlg, IDC_IGNORE);

    if (ChangedSItems.bGRADIENT) {
        StatusItems[index]. GRADIENT = GRADIENT_NONE;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT))
            StatusItems[index].GRADIENT |= GRADIENT_ACTIVE;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_LR))
            StatusItems[index].GRADIENT |= GRADIENT_LR;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_RL))
            StatusItems[index].GRADIENT |= GRADIENT_RL;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_TB))
            StatusItems[index].GRADIENT |= GRADIENT_TB;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_BT))
            StatusItems[index].GRADIENT |= GRADIENT_BT;
    }
    if (ChangedSItems.bCORNER) {
        StatusItems[index]. CORNER = CORNER_NONE;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER))
            StatusItems[index].CORNER |= CORNER_ACTIVE ;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_TL))
            StatusItems[index].CORNER |= CORNER_TL ;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_TR))
            StatusItems[index].CORNER |= CORNER_TR;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_BR))
            StatusItems[index].CORNER |= CORNER_BR;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_BL))
            StatusItems[index].CORNER |= CORNER_BL;
    }

    if (ChangedSItems.bCOLOR)
        StatusItems[index]. COLOR = SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_GETCOLOUR, 0, 0);

    if (ChangedSItems.bCOLOR2)
        StatusItems[index]. COLOR2 = SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_GETCOLOUR, 0, 0);

    if (ChangedSItems.bCOLOR2_TRANSPARENT)
        StatusItems[index]. COLOR2_TRANSPARENT = IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT);

    if (ChangedSItems.bTEXTCOLOR)
        StatusItems[index]. TEXTCOLOR = SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0);

    if (ChangedSItems.bALPHA) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_ALPHA), buf, 10);        // can be removed now
        if (lstrlenA(buf) > 0)
            StatusItems[index]. ALPHA = (BYTE) SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_LEFT) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_LEFT), buf, 10);        
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_LEFT = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_LEFT_SPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_TOP) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_TOP), buf, 10);     
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_TOP = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_TOP_SPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_RIGHT) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_RIGHT), buf, 10);       
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_RIGHT = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_RIGHT_SPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_BOTTOM) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_BOTTOM), buf, 10);      
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_BOTTOM = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_BOTTOM_SPIN, UDM_GETPOS, 0, 0);
    }
    if (ChangedSItems.bBORDERSTYLE) {
        bdrtype = SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_GETCURSEL, 0, 0);
        if(bdrtype == CB_ERR)
            StatusItems[index].BORDERSTYLE = 0;
        else {
            switch(bdrtype) {
                case 0:
                    StatusItems[index].BORDERSTYLE = 0;
                    break;
                case 1:
                    StatusItems[index].BORDERSTYLE = BDR_RAISEDOUTER;
                    break;
                case 2:
                    StatusItems[index].BORDERSTYLE = BDR_SUNKENINNER;
                    break;
                case 3:
                    StatusItems[index].BORDERSTYLE = EDGE_BUMP;
                    break;
                case 4:
                    StatusItems[index].BORDERSTYLE = EDGE_ETCHED;
                    break;
                default:
                    StatusItems[index].BORDERSTYLE = 0;
                    break;
            }
        }
    }
}

static void SaveLatestChanges(HWND hwndDlg)
{
    int n, itemData;
    // process old selection
    if (last_selcount > 0) {
        for (n = 0; n < last_selcount; n++) {
            itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[n], 0);
            if (itemData != ID_EXTBKSEPARATOR) {
                UpdateStatusStructSettingsFromOptDlg(hwndDlg, itemData - ID_EXTBK_FIRST);
            }
        }
    }

    // reset bChange
    ChangedSItems.bALPHA = FALSE;
    ChangedSItems.bGRADIENT = FALSE;
    ChangedSItems.bCORNER = FALSE;
    ChangedSItems.bCOLOR = FALSE;
    ChangedSItems.bCOLOR2 = FALSE;
    ChangedSItems.bCOLOR2_TRANSPARENT = FALSE;
    ChangedSItems.bTEXTCOLOR = FALSE;
    ChangedSItems.bALPHA = FALSE;
    ChangedSItems.bMARGIN_LEFT = FALSE;
    ChangedSItems.bMARGIN_TOP = FALSE;
    ChangedSItems.bMARGIN_RIGHT = FALSE;
    ChangedSItems.bMARGIN_BOTTOM = FALSE;
    ChangedSItems.bIGNORED = FALSE;
    ChangedSItems.bBORDERSTYLE = FALSE;
}

static UINT _controls_to_refresh[] = {
     IDC_BORDERTYPE,
     IDC_3DDARKCOLOR,       
     IDC_3DLIGHTCOLOR,      
     IDC_MRGN_BOTTOM,
     IDC_MRGN_LEFT,
     IDC_ALPHASPIN,        
     IDC_CORNER,            
     IDC_MRGN_TOP_SPIN,
     IDC_MRGN_RIGHT_SPIN,
     IDC_MRGN_BOTTOM_SPIN,  
     IDC_MRGN_LEFT_SPIN,
     IDC_GRADIENT,   
     IDC_GRADIENT_LR,
     IDC_GRADIENT_RL,
     IDC_GRADIENT_TB,      
     IDC_BASECOLOUR,
     IDC_ALPHA,
     IDC_MRGN_TOP,
     IDC_MRGN_RIGHT,
     IDC_GRADIENT_BT,       
     IDC_BASECOLOUR2,       
     IDC_TEXTCOLOUR,       
     IDC_CORNER_TL,         
     IDC_CORNER_TR,         
     IDC_CORNER_BR,         
     IDC_CORNER_BL,
     IDC_IGNORE,
     IDC_ALPHALABLE,
     IDC_COLOR2LABLE,
     IDC_COLORLABLE,
     IDC_TEXTCOLOURLABLE,
     IDC_COLOR2_TRANSPARENT,
     0
};

static void RefreshControls(HWND hwnd)
{
    for(int i = 0; _controls_to_refresh[i]; i++)
        InvalidateRect(GetDlgItem(hwnd, _controls_to_refresh[i]), NULL, FALSE);
}

// wenn die listbox ge�ndert wurde
static void OnListItemsChange(HWND hwndDlg)
{
    SendMessage(hwndDlg, WM_SETREDRAW, FALSE, 0);
    SaveLatestChanges(hwndDlg);

    // set new selection
    last_selcount = SendMessage(GetDlgItem(hwndDlg, IDC_ITEMS), LB_GETSELCOUNT, 0, 0);  
    if (last_selcount > 0) {
        int n, real_index, itemData, first_item;
        StatusItems_t DialogSettingForMultiSel;

    // get selected indizes
        SendMessage(GetDlgItem(hwndDlg, IDC_ITEMS), LB_GETSELITEMS, 64, (LPARAM) last_indizes);

    // initialize with first items value

        first_item = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[0], 0) - ID_EXTBK_FIRST;
        DialogSettingForMultiSel = StatusItems[first_item];
        for (n = 0; n < last_selcount; n++) {
            itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[n], 0);
            if (itemData != ID_EXTBKSEPARATOR) {
                real_index = itemData - ID_EXTBK_FIRST;
                if (StatusItems[real_index].ALPHA != StatusItems[first_item].ALPHA)
                    DialogSettingForMultiSel.ALPHA = -1;
                if (StatusItems[real_index].COLOR != StatusItems[first_item].COLOR)
                    DialogSettingForMultiSel.COLOR = CLCDEFAULT_COLOR;
                if (StatusItems[real_index].COLOR2 != StatusItems[first_item].COLOR2)
                    DialogSettingForMultiSel.COLOR2 = CLCDEFAULT_COLOR2;
                if (StatusItems[real_index].COLOR2_TRANSPARENT != StatusItems[first_item].COLOR2_TRANSPARENT)
                    DialogSettingForMultiSel.COLOR2_TRANSPARENT = CLCDEFAULT_COLOR2_TRANSPARENT;
                if (StatusItems[real_index].TEXTCOLOR != StatusItems[first_item].TEXTCOLOR)
                    DialogSettingForMultiSel.TEXTCOLOR = CLCDEFAULT_TEXTCOLOR;
                if (StatusItems[real_index].CORNER != StatusItems[first_item].CORNER)
                    DialogSettingForMultiSel.CORNER = CLCDEFAULT_CORNER;
                if (StatusItems[real_index].GRADIENT != StatusItems[first_item].GRADIENT)
                    DialogSettingForMultiSel.GRADIENT = CLCDEFAULT_GRADIENT;
                if (StatusItems[real_index].IGNORED != StatusItems[first_item].IGNORED)
                    DialogSettingForMultiSel.IGNORED = CLCDEFAULT_IGNORE;
                if (StatusItems[real_index].MARGIN_BOTTOM != StatusItems[first_item].MARGIN_BOTTOM)
                    DialogSettingForMultiSel.MARGIN_BOTTOM = -1;
                if (StatusItems[real_index].MARGIN_LEFT != StatusItems[first_item].MARGIN_LEFT)
                    DialogSettingForMultiSel.MARGIN_LEFT = -1;
                if (StatusItems[real_index].MARGIN_RIGHT != StatusItems[first_item].MARGIN_RIGHT)
                    DialogSettingForMultiSel.MARGIN_RIGHT = -1;
                if (StatusItems[real_index].MARGIN_TOP != StatusItems[first_item].MARGIN_TOP)
                    DialogSettingForMultiSel.MARGIN_TOP = -1;
                if (StatusItems[real_index].BORDERSTYLE != StatusItems[first_item].BORDERSTYLE)
                    DialogSettingForMultiSel.BORDERSTYLE = -1;
            }
        }

        if (last_selcount == 1 && StatusItems[first_item].statusID == ID_EXTBKSEPARATOR) {
            ChangeControlItems(hwndDlg, 0, 0);
            last_selcount = 0;
        } else
            ChangeControlItems(hwndDlg, 1, 0);
        FillOptionDialogByStatusItem(hwndDlg, &DialogSettingForMultiSel);
        InvalidateRect(GetDlgItem(hwndDlg, IDC_ITEMS), NULL, FALSE);
    }
    SendMessage(hwndDlg, WM_SETREDRAW, TRUE, 0);
    RefreshControls(hwndDlg);
}

// fills the combobox of the options dlg for the first time
static void FillItemList(HWND hwndDlg)
{
	int n, iOff;
	UINT item;

	for (n = 0; n <= ID_EXTBK_LAST - ID_EXTBK_FIRST; n++) {
		iOff = 0;
		if(strstr(StatusItems[n].szName, "{-}")) {
			item = SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_ADDSTRING, 0, (LPARAM)"------------------------");
			SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_SETITEMDATA, item, ID_EXTBKSEPARATOR);
			iOff = 3;
		}
		item = SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_ADDSTRING, 0, (LPARAM)&StatusItems[n].szName[iOff]);
		SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_SETITEMDATA, item, ID_EXTBK_FIRST + n);
	}
}

static BOOL CALLBACK SkinEdit_ExtBkDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SKINDESCRIPTION *psd = (SKINDESCRIPTION *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    if(psd) {
        ID_EXTBK_FIRST = psd->firstItem;
        ID_EXTBK_LAST = psd->lastItem;
        StatusItems = psd->StatusItems;
    }
    switch (msg) {
        case WM_INITDIALOG:
            psd = (SKINDESCRIPTION *)malloc(sizeof(SKINDESCRIPTION));
            ZeroMemory(psd, sizeof(SKINDESCRIPTION));
            CopyMemory(psd, (void *)lParam, sizeof(SKINDESCRIPTION));
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)psd);

            if(psd) {
                ID_EXTBK_FIRST = psd->firstItem;
                ID_EXTBK_LAST = psd->lastItem;
                StatusItems = psd->StatusItems;
            }

            TranslateDialogDefault(hwndDlg);
            FillItemList(hwndDlg);
            SendMessage(hwndDlg, WM_USER + 101, 0, 0);

            psd->hMenuItems = CreatePopupMenu();
            AppendMenu(psd->hMenuItems, MF_STRING | MF_DISABLED, (UINT_PTR)0, _T("Copy from"));
            AppendMenuA(psd->hMenuItems, MF_SEPARATOR, (UINT_PTR)0, NULL);

            {
				int i;
				
				for(i = ID_EXTBK_FIRST; i <= ID_EXTBK_LAST; i++) {
					int iOff = StatusItems[i - ID_EXTBK_FIRST].szName[0] == '{' ? 3 : 0;
					if(iOff)
						AppendMenuA(psd->hMenuItems, MF_SEPARATOR, (UINT_PTR)0, NULL);
					AppendMenuA(psd->hMenuItems, MF_STRING, (UINT_PTR)i, &StatusItems[i - ID_EXTBK_FIRST].szName[iOff]);
				}
			}
            return TRUE;
        case WM_USER + 101:
            {
                DBVARIANT dbv = {0};
                
                SendDlgItemMessage(hwndDlg, IDC_MRGN_LEFT_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_MRGN_TOP_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_MRGN_RIGHT_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_MRGN_BOTTOM_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));

                SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("<None>"));
                SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Raised"));
                SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Sunken"));
                SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Bumped"));
                SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Etched"));

                SendDlgItemMessage(hwndDlg, IDC_3DDARKCOLOR, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, "CLCExt", "3ddark", RGB(224,224,224)));
                SendDlgItemMessage(hwndDlg, IDC_3DLIGHTCOLOR, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, "CLCExt", "3dbright", RGB(224,224,224)));
                return 0;
            }

        case WM_DRAWITEM:
            {
                DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
                int iItem = dis->itemData;
                StatusItems_t *item = 0;

                SetBkMode(dis->hDC, TRANSPARENT);
                FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));

                if(iItem >= ID_EXTBK_FIRST && iItem <= ID_EXTBK_LAST)
                    item = &StatusItems[iItem - ID_EXTBK_FIRST];

                if (dis->itemState & ODS_SELECTED && iItem != ID_EXTBKSEPARATOR) {
                    FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
                    SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                }
                else {
                    FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
                    if(item && item->IGNORED)
                        SetTextColor(dis->hDC, RGB(255, 0, 0));
                    else
                        SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
                }
                if(iItem == ID_EXTBKSEPARATOR) {
                    HPEN    hPen, hPenOld;
                    POINT   pt;

                    hPen = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_WINDOWTEXT));
                    hPenOld = (HPEN)SelectObject(dis->hDC, hPen);

                    MoveToEx(dis->hDC, dis->rcItem.left, (dis->rcItem.top + dis->rcItem.bottom) / 2, &pt);
                    LineTo(dis->hDC, dis->rcItem.right, (dis->rcItem.top + dis->rcItem.bottom) / 2);
                    SelectObject(dis->hDC, hPenOld);
                    DeleteObject((HGDIOBJ)hPen);
                }
                else if(dis->itemID >= 0 && item) {
                    char   *szName = item->szName[0] == '{' ? &item->szName[3] : item->szName;

                    TextOutA(dis->hDC, dis->rcItem.left, dis->rcItem.top, szName, lstrlenA(szName));
                }
                return TRUE;
            }

        case WM_CONTEXTMENU:
            {
                POINT pt;
                RECT  rc;
                HWND hwndList = GetDlgItem(hwndDlg, IDC_ITEMS);

                GetCursorPos(&pt);
                GetWindowRect(hwndList, &rc);
                if(PtInRect(&rc, pt)) {
                    int iSelection = (int)TrackPopupMenu(psd->hMenuItems, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);

                    if(iSelection >= ID_EXTBK_FIRST && iSelection <= ID_EXTBK_LAST) {
                        iSelection -= ID_EXTBK_FIRST;

                        for(int i = ID_EXTBK_FIRST; i <= ID_EXTBK_LAST; i++) {
                            if(SendMessage(hwndList, LB_GETSEL, i - ID_EXTBK_FIRST, 0) > 0) {
                                int iIndex = SendMessage(hwndList, LB_GETITEMDATA, i - ID_EXTBK_FIRST, 0);
                                iIndex -= ID_EXTBK_FIRST;

                                if(iIndex >= 0) {
                                    StatusItems[iIndex].ALPHA = StatusItems[iSelection].ALPHA;
                                    StatusItems[iIndex].BORDERSTYLE = StatusItems[iSelection].BORDERSTYLE;
                                    StatusItems[iIndex].COLOR = StatusItems[iSelection].COLOR;
                                    StatusItems[iIndex].COLOR2 = StatusItems[iSelection].COLOR2;
                                    StatusItems[iIndex].COLOR2_TRANSPARENT = StatusItems[iSelection].COLOR2_TRANSPARENT;
                                    StatusItems[iIndex].CORNER = StatusItems[iSelection].CORNER;
                                    StatusItems[iIndex].GRADIENT = StatusItems[iSelection].GRADIENT;
                                    StatusItems[iIndex].IGNORED = StatusItems[iSelection].IGNORED;
                                    StatusItems[iIndex].imageItem = StatusItems[iSelection].imageItem;
                                    StatusItems[iIndex].MARGIN_BOTTOM = StatusItems[iSelection].MARGIN_BOTTOM;
                                    StatusItems[iIndex].MARGIN_LEFT = StatusItems[iSelection].MARGIN_LEFT;
                                    StatusItems[iIndex].MARGIN_RIGHT = StatusItems[iSelection].MARGIN_RIGHT;
                                    StatusItems[iIndex].MARGIN_TOP = StatusItems[iSelection].MARGIN_TOP;
                                    StatusItems[iIndex].TEXTCOLOR = StatusItems[iSelection].TEXTCOLOR;
                                }
                            }
                        }
                        OnListItemsChange(hwndDlg);
                    }
                }
                break;
            }
        case WM_COMMAND:
    // this will check if the user changed some actual statusitems values
    // if yes the flag bChanged will be set to TRUE
            SetChangedStatusItemFlag(wParam, hwndDlg);
            switch(LOWORD(wParam)) {
                case IDC_ITEMS:
                    if (HIWORD(wParam) != LBN_SELCHANGE)
                        return FALSE;
                    {
                        int iItem = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETCURSEL, 0, 0), 0);
                        if(iItem == ID_EXTBKSEPARATOR)
                            return FALSE;
                    }
                    OnListItemsChange(hwndDlg);
					if(psd->pfnClcOptionsChanged)
						psd->pfnClcOptionsChanged();
                    break;          
                case IDC_GRADIENT:
                    ReActiveCombo(hwndDlg);
                    break;
                case IDC_CORNER:
                    ReActiveCombo(hwndDlg);
                    break;
                case IDC_IGNORE:
                    ReActiveCombo(hwndDlg);
                    break;
                case IDC_COLOR2_TRANSPARENT:
                    ReActiveCombo(hwndDlg);
                    break;
                case IDC_BORDERTYPE:
                    break;
            }
            if ((LOWORD(wParam) == IDC_ALPHA || LOWORD(wParam) == IDC_MRGN_LEFT || LOWORD(wParam) == IDC_MRGN_BOTTOM || LOWORD(wParam) == IDC_MRGN_TOP || LOWORD(wParam) == IDC_MRGN_RIGHT) && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
                return 0;
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
            break;

        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->idFrom) {
                case 0:
                    switch (((LPNMHDR) lParam)->code) {
                        case PSN_APPLY:
                // save user made changes
                            SaveLatestChanges(hwndDlg);
                // save struct to DB
							if(psd->pfnSaveCompleteStruct)
								psd->pfnSaveCompleteStruct();
                            DBWriteContactSettingDword(NULL, "CLCExt", "3dbright", SendDlgItemMessage(hwndDlg, IDC_3DLIGHTCOLOR, CPM_GETCOLOUR, 0, 0));
                            DBWriteContactSettingDword(NULL, "CLCExt", "3ddark", SendDlgItemMessage(hwndDlg, IDC_3DDARKCOLOR, CPM_GETCOLOUR, 0, 0));

                            if(psd->pfnClcOptionsChanged)
								psd->pfnClcOptionsChanged();
							if(psd->hwndCLUI) {
								SendMessage(psd->hwndCLUI, WM_SIZE, 0, 0);
								PostMessage(psd->hwndCLUI, WM_USER+100, 0, 0);          // CLUIINTM_REDRAW
							}
                            break;
                    }
            }
            break;
        case WM_DESTROY:
            DestroyMenu(psd->hMenuItems);
            break;
        case WM_NCDESTROY:
            free(psd);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)0);
            break;
    }
    return FALSE;
}

/*                                                              
 * unimplemented                                                                
*/

static BOOL CALLBACK SkinEdit_ImageItemEditProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

static INT_PTR SkinEdit_FillByCurrentSel(WPARAM wParam, LPARAM lParam)
{
    if(wParam)
        FillOptionDialogByCurrentSel((HWND)wParam);
    return 0;
}

/*                                                              
 * service function                                                                
 * creates additional tab pages under the given parent window handle
 * expects a SKINDESCRIPTON * in lParam
*/

static INT_PTR SkinEdit_Invoke(WPARAM wParam, LPARAM lParam)
{
    SKINDESCRIPTION *psd = (SKINDESCRIPTION *)lParam;
    TCITEM  tci = {0};
    RECT    rcClient;
    int     iTabs;

    if(psd->cbSize != sizeof(SKINDESCRIPTION))
        return 0;

    iTabs = TabCtrl_GetItemCount(psd->hWndTab);
    GetClientRect(psd->hWndParent, &rcClient);

    tci.mask = TCIF_PARAM|TCIF_TEXT;
    tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SKINITEMEDIT), psd->hWndParent, (DLGPROC)SkinEdit_ExtBkDlgProc, (LPARAM)psd);

    tci.pszText = TranslateT("Skin items");
    TabCtrl_InsertItem(psd->hWndTab, iTabs++, &tci);
    MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 60, 1);
    psd->hwndSkinEdit = (HWND)tci.lParam;

    /*
    tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_IMAGEITEMEDIT), psd->hWndParent, (DLGPROC)SkinEdit_ImageItemEditProc, (LPARAM)psd);
    tci.pszText = TranslateT("Image items");
    TabCtrl_InsertItem(psd->hWndTab, iTabs++, &tci);
    MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 60, 1);
    psd->hwndImageEdit = (HWND)tci.lParam;
    */
    
    return (INT_PTR)psd->hwndSkinEdit;
}

static HANDLE hSvc_invoke = 0, hSvc_fillby = 0;

static int LoadModule()
{
    memset(&memoryManagerInterface, 0, sizeof(memoryManagerInterface));
    memoryManagerInterface.cbSize = sizeof(memoryManagerInterface);
    CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM) &memoryManagerInterface);

    hSvc_invoke = CreateServiceFunction(MS_CLNSE_INVOKE, (MIRANDASERVICE)SkinEdit_Invoke);
    hSvc_fillby = CreateServiceFunction(MS_CLNSE_FILLBYCURRENTSEL, (MIRANDASERVICE)SkinEdit_FillByCurrentSel);
    return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
#if defined(_UNICODE)
	pluginInfo.flags |= UNICODE_AWARE;
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 7, 0, 0))
#else
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 7, 0, 0))
#endif
		return NULL;
	return &pluginInfo;
}

/*
 * define our own MUUID, since this is a special plugin...
 */
extern "C" static const MUUID interfaces[] = {MIID_TESTPLUGIN, { 0x70ff4eef, 0xcb7b, 0x4d88, { 0x85, 0x60, 0x7d, 0xe3, 0xa6, 0x68, 0x5c, 0xe3 }}, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

/*
extern "C" __declspec(dllexport) PLUGININFO * MirandaPluginInfo(DWORD mirandaVersion)
{
    if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 4, 0, 0))
        return NULL;
    return &pluginInfo;
}
*/

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

static int systemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
    ModulesLoaded(wParam, lParam);
    return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK * link)
{
    pluginLink = link;
    return(LoadModule());
}

static int ShutdownProc(WPARAM wParam, LPARAM lParam)
{
    if(hSvc_invoke)
        DestroyServiceFunction(hSvc_invoke);
    if(hSvc_fillby)
        DestroyServiceFunction(hSvc_fillby);
    return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
    return ShutdownProc(0, 0);
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}


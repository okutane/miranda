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

$Id$
*/

/*
 * utility functions for the message dialog window
 */

#include "commonheaders.h"
#pragma hdrstop

#include "../../include/m_clc.h"
#include "../../include/m_clui.h"
#include "../../include/m_userinfo.h"
#include "../../include/m_history.h"
#include "../../include/m_addcontact.h"

#include "msgs.h"
#include "m_message.h"
#include "m_popup.h"
#include "m_smileyadd.h"
#include "m_metacontacts.h"
#include "msgdlgutils.h"

extern int g_SmileyAddAvail, g_MetaContactsAvail;
extern HMODULE g_hInst;
extern HBITMAP g_hbmUnknown;
extern HANDLE hMessageWindowList;
extern int g_IconEmpty, g_IconMsgEvent, g_IconTypingEvent, g_IconFileEvent, g_IconUrlEvent, g_IconError, g_IconSend;
extern HICON g_buttonBarIcons[];

void CalcDynamicAvatarSize(HWND hwndDlg, struct MessageWindowData *dat, BITMAP *bminfo)
{
    RECT rc;
    double aspect = 0, newWidth = 0, picAspect = 0;
    double picProjectedWidth = 0;
    int infospace = 0;
    
    GetClientRect(hwndDlg, &rc);
    if(dat->showUIElements & MWF_UI_SHOWINFO) {
        RECT rcname;
        GetClientRect(GetDlgItem(hwndDlg, IDC_NAME), &rcname);
        infospace = (rcname.right - rcname.left) + 4;
    }
    picAspect = (double)(bminfo->bmWidth / (double)bminfo->bmHeight);
    picProjectedWidth = (double)((dat->dynaSplitter + ((dat->showUIElements != 0) ? 28 : 2))) * picAspect;

    if(((rc.right - rc.left) - (int)picProjectedWidth) > (dat->iButtonBarNeeds + infospace)) {
        dat->iRealAvatarHeight = dat->dynaSplitter + ((dat->showUIElements != 0) ? 28 : 2);
        dat->bottomOffset = dat->dynaSplitter + 100;
    }
    else {
        dat->iRealAvatarHeight = dat->dynaSplitter;
        dat->bottomOffset = -33;
    }

    aspect = (double)dat->iRealAvatarHeight / (double)bminfo->bmHeight;
    newWidth = (double)bminfo->bmWidth * aspect;
    if(newWidth > (double)(rc.right - rc.left) * 0.8)
        newWidth = (double)(rc.right - rc.left) * 0.8;
    dat->pic.cy = dat->iRealAvatarHeight + 2*1;
    dat->pic.cx = (int)newWidth + 2*1;
}

int IsMetaContact(HWND hwndDlg, struct MessageWindowData *dat) 
{
    if(dat->hContact == 0 || dat->szProto == NULL)
       return 0;
    
    return (g_MetaContactsAvail && !strcmp(dat->szProto, "MetaContacts"));
}
char *GetCurrentMetaContactProto(HWND hwndDlg, struct MessageWindowData *dat)
{
    HANDLE hSubContact = 0;
    
    hSubContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)dat->hContact, 0);
    if(hSubContact)
        return (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubContact, 0);
    else
        return dat->szProto;
}

void WriteStatsOnClose(HWND hwndDlg, struct MessageWindowData *dat)
{
    DBEVENTINFO dbei;
    char buffer[450];
    HANDLE hNewEvent;
    time_t now = time(NULL);
    now = now - dat->stats.started;

    return;
    
    if(dat->hContact != 0 && DBGetContactSettingByte(NULL, SRMSGMOD_T, "logstatus", 0) != 0 && DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "logstatus", -1) != 0) {
        _snprintf(buffer, sizeof(buffer), "Session close - active for: %d:%02d:%02d, Sent: %d (%d), Rcvd: %d (%d)", now / 3600, now / 60, now % 60, dat->stats.iSent, dat->stats.iSentBytes, dat->stats.iReceived, dat->stats.iReceivedBytes);
        dbei.cbSize = sizeof(dbei);
        dbei.pBlob = (PBYTE) buffer;
        dbei.cbBlob = strlen(buffer) + 1;
        dbei.eventType = EVENTTYPE_STATUSCHANGE;
        dbei.flags = DBEF_READ;
        dbei.timestamp = time(NULL);
        dbei.szModule = dat->szProto;
        hNewEvent = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) dat->hContact, (LPARAM) & dbei);
        if (dat->hDbEventFirst == NULL) {
            dat->hDbEventFirst = hNewEvent;
            SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
        }
    }
}

int MsgWindowUpdateMenu(HWND hwndDlg, struct MessageWindowData *dat, HMENU submenu, int menuID)
{
    if(menuID == MENU_LOGMENU) {
        int iLocalTime = DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "uselocaltime", 0);
        int iRtl = DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "RTL", 0);
        int iLogStatus = (DBGetContactSettingByte(NULL, SRMSGMOD_T, "logstatus", 0) != 0) && (DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "logstatus", -1) != 0);

        CheckMenuItem(submenu, ID_LOGMENU_SHOWTIMESTAMP, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_SHOWTIME ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_MESSAGEBODYINANEWLINE, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_NEWLINE ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_USECONTACTSLOCALTIME, MF_BYCOMMAND | iLocalTime ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_SHOWDATE, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_SHOWDATES ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_SHOWSECONDS, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_SHOWSECONDS ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_INDENTMESSAGEBODY, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_INDENT ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_SHOWMESSAGEICONS, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_SHOWICONS ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_SHOWNICKNAME, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_SHOWNICK ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_UNDERLINETIMESTAMP, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_UNDERLINE ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_ACTIVATERTL, MF_BYCOMMAND | iRtl ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGMENU_DISPLAYTIMESTAMPAFTERNICKNAME, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_SWAPNICK ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_LOGITEMSTOSHOW_LOGSTATUSCHANGES, MF_BYCOMMAND | iLogStatus ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_MESSAGELOGFORMATTING_SHOWGRID, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_GRID ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_MESSAGELOGFORMATTING_USEINDIVIDUALBACKGROUNDCOLORS, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_INDIVIDUALBKG ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_MESSAGELOGFORMATTING_GROUPMESSAGES, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_GROUPMODE ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_TIMESTAMPSETTINGS_USELONGDATEFORMAT, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_LONGDATES ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_TIMESTAMPSETTINGS_USERELATIVETIMESTAMPS, MF_BYCOMMAND | dat->dwFlags & MWF_LOG_USERELATIVEDATES ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(submenu, ID_MESSAGELOG_MESSAGELOGSETTINGSAREGLOBAL, MF_BYCOMMAND | (DBGetContactSettingByte(NULL, SRMSGMOD_T, "ignorecontactsettings", 0) ? MF_CHECKED : MF_UNCHECKED));

        EnableMenuItem(submenu, ID_LOGMENU_SHOWDATE, dat->dwFlags & MWF_LOG_SHOWTIME ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(submenu, ID_LOGMENU_SHOWSECONDS, dat->dwFlags & MWF_LOG_SHOWTIME ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(submenu, ID_MESSAGELOG_APPLYMESSAGELOGSETTINGSTOALLCONTACTS, DBGetContactSettingByte(NULL, SRMSGMOD_T, "ignorecontactsettings", 0) ? MF_GRAYED : MF_ENABLED);
    }
    else if(menuID == MENU_PICMENU) {
        EnableMenuItem(submenu, ID_PICMENU_ALIGNFORFULL, MF_BYCOMMAND | ((dat->showPic && !(dat->dwFlags & MWF_LOG_DYNAMICAVATAR)) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(submenu, ID_PICMENU_ALIGNFORMAXIMUMLOGSIZE, MF_BYCOMMAND | ((dat->showPic && !(dat->dwFlags & MWF_LOG_DYNAMICAVATAR)) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(submenu, ID_PICMENU_RESETTHEAVATAR, MF_BYCOMMAND | ( dat->showPic ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(submenu, ID_PICMENU_DISABLEAUTOMATICAVATARUPDATES, MF_BYCOMMAND | ( dat->showPic ? MF_ENABLED : MF_GRAYED));
        CheckMenuItem(submenu, ID_PICMENU_DISABLEAUTOMATICAVATARUPDATES, MF_BYCOMMAND | (DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "noremoteavatar", 0) == 1) ? MF_CHECKED : MF_UNCHECKED);
    }
    return 0;
}

int MsgWindowMenuHandler(HWND hwndDlg, struct MessageWindowData *dat, int selection, int menuId)
{
    if(menuId == MENU_PICMENU || menuId == MENU_TABCONTEXT) {
        switch(selection) {
            case ID_TABMENU_SWITCHTONEXTTAB:
                SendMessage(dat->pContainer->hwnd, DM_SELECTTAB, (WPARAM) DM_SELECT_NEXT, 0);
                return 1;
            case ID_TABMENU_SWITCHTOPREVIOUSTAB:
                SendMessage(dat->pContainer->hwnd, DM_SELECTTAB, (WPARAM) DM_SELECT_PREV, 0);
                return 1;
            case ID_TABMENU_ATTACHTOCONTAINER:
                CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SELECTCONTAINER), hwndDlg, SelectContainerDlgProc, (LPARAM) hwndDlg);
                return 1;
            case ID_TABMENU_CONTAINEROPTIONS:
                CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), hwndDlg, DlgProcContainerOptions, (LPARAM) dat->pContainer);
                return 1;
            case ID_TABMENU_CLOSECONTAINER:
                SendMessage(dat->pContainer->hwnd, WM_CLOSE, 0, 0);
                return 1;
            case ID_TABMENU_CLOSETAB:
                SendMessage(hwndDlg, WM_CLOSE, 1, 0);
                return 1;
            case ID_PICMENU_TOGGLEAVATARDISPLAY:
                DBWriteContactSettingByte(dat->hContact, SRMSGMOD_T, "MOD_ShowPic", dat->showPic ? 0 : 1);
                ShowPicture(hwndDlg,dat,FALSE,FALSE, FALSE);
                SendMessage(hwndDlg, DM_UPDATEPICLAYOUT, 0, 0);
                SendMessage(hwndDlg, WM_SIZE, 0, 0);
                SendMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 1);
                return 1;
            case ID_PICMENU_ALIGNFORMAXIMUMLOGSIZE:
                SendMessage(hwndDlg, DM_ALIGNSPLITTERMAXLOG, 0, 0);
                return 1;
            case ID_PICMENU_ALIGNFORFULL:
                SendMessage(hwndDlg, DM_ALIGNSPLITTERFULL, 0, 0);
                return 1;
            case ID_PICMENU_RESETTHEAVATAR:
                DBDeleteContactSetting(dat->hContact, SRMSGMOD_T, "MOD_Pic");
                if(dat->hContactPic && dat->hContactPic != g_hbmUnknown)
                    DeleteObject(dat->hContactPic);
                dat->hContactPic = g_hbmUnknown;
                DBDeleteContactSetting(dat->hContact, "ContactPhoto", "File");
                SendMessage(hwndDlg, DM_RETRIEVEAVATAR, 0, 0);
                InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), NULL, TRUE);
                if(!(dat->dwFlags & MWF_LOG_DYNAMICAVATAR))
                    SendMessage(hwndDlg, DM_ALIGNSPLITTERFULL, 0, 0);
                else {
                    SendMessage(hwndDlg, DM_UPDATEPICLAYOUT, 0, 0);
                    SendMessage(hwndDlg, DM_RECALCPICTURESIZE, 0, 0);
                }
                return 1;
            case ID_PICMENU_DISABLEAUTOMATICAVATARUPDATES:
                {
                    int iState = DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "noremoteavatar", 0);
                    DBWriteContactSettingByte(dat->hContact, SRMSGMOD_T, "noremoteavatar", !iState);
                    return 1;
                }
            case ID_PICMENU_LOADALOCALPICTUREASAVATAR:
                {
                    char FileName[MAX_PATH];
                    char Filters[512];
                    OPENFILENAMEA ofn={0};

                    CallService(MS_UTILS_GETBITMAPFILTERSTRINGS,sizeof(Filters),(LPARAM)(char*)Filters);
                    ofn.lStructSize=sizeof(ofn);
                    ofn.hwndOwner=hwndDlg;
                    ofn.lpstrFile = FileName;
                    ofn.lpstrFilter = Filters;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.Flags=OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
                    *FileName = '\0';
                    if (GetOpenFileNameA(&ofn))
                        DBWriteContactSettingString(dat->hContact, "ContactPhoto", "File",FileName);
                    else
                        return 1;
                }
                return 1;
        }
    }
    else if(menuId == MENU_LOGMENU) {
        int iLocalTime = DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "uselocaltime", 0);
        int iRtl = DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "RTL", 0);
        int iLogStatus = (DBGetContactSettingByte(NULL, SRMSGMOD_T, "logstatus", 0) != 0) && (DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "logstatus", -1) != 0);
        int iIgnorePerContact = DBGetContactSettingByte(NULL, SRMSGMOD_T, "ignorecontactsettings", 0);

        DWORD dwOldFlags = dat->dwFlags;

        switch(selection) {

            case ID_LOGMENU_SHOWTIMESTAMP:
                dat->dwFlags ^= MWF_LOG_SHOWTIME;
                return 1;
            case ID_LOGMENU_MESSAGEBODYINANEWLINE:
                dat->dwFlags  ^= MWF_LOG_NEWLINE;
                return 1;
            case ID_LOGMENU_USECONTACTSLOCALTIME:
                iLocalTime ^=1;
                if(dat->hContact) {
                    DBWriteContactSettingByte(dat->hContact, SRMSGMOD_T, "uselocaltime", (BYTE) iLocalTime);
                    SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
                }
                return 1;
            case ID_LOGMENU_INDENTMESSAGEBODY:
                dat->dwFlags ^= MWF_LOG_INDENT;
                return 1;
            case ID_LOGMENU_ACTIVATERTL:
                iRtl ^= 1;
                dat->dwFlags = iRtl ? dat->dwFlags | MWF_LOG_RTL : dat->dwFlags & ~MWF_LOG_RTL;
                if(dat->hContact) {
                    DBWriteContactSettingByte(dat->hContact, SRMSGMOD_T, "RTL", (BYTE) iRtl);
                    SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0);
                }
                return 1;
            case ID_LOGMENU_SHOWDATE:
                dat->dwFlags ^= MWF_LOG_SHOWDATES;
                return 1;
            case ID_LOGMENU_SHOWSECONDS:
                dat->dwFlags ^= MWF_LOG_SHOWSECONDS;
                return 1;
            case ID_LOGMENU_SHOWMESSAGEICONS:
                dat->dwFlags ^= MWF_LOG_SHOWICONS;
                return 1;
            case ID_LOGMENU_SHOWNICKNAME:
                dat->dwFlags ^= MWF_LOG_SHOWNICK;
                return 1;
            case ID_LOGMENU_UNDERLINETIMESTAMP:
                dat->dwFlags ^= MWF_LOG_UNDERLINE;
                return 1;
            case ID_LOGMENU_DISPLAYTIMESTAMPAFTERNICKNAME:
                dat->dwFlags ^= MWF_LOG_SWAPNICK;
                return 1;
            case ID_LOGITEMSTOSHOW_LOGSTATUSCHANGES:
                DBWriteContactSettingByte(dat->hContact, SRMSGMOD_T, "logstatus", iLogStatus ? 0 : -1);
                return 1;
            case ID_LOGMENU_SAVETHESESETTINGSASDEFAULTVALUES:
                DBWriteContactSettingDword(NULL, SRMSGMOD_T, "mwflags", dat->dwFlags & MWF_LOG_ALL);
                WindowList_Broadcast(hMessageWindowList, DM_DEFERREDREMAKELOG, (WPARAM)hwndDlg, (LPARAM)(dat->dwFlags & MWF_LOG_ALL));
                return 1;
            case ID_MESSAGELOGFORMATTING_SHOWGRID:
                dat->dwFlags ^= MWF_LOG_GRID;
                if(dat->dwFlags & MWF_LOG_GRID)
                    SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(1,1));     // XXX margins in the log (looks slightly better)
                else
                    SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0,0));     // XXX margins in the log (looks slightly better)
                return 1;
            case ID_MESSAGELOGFORMATTING_USEINDIVIDUALBACKGROUNDCOLORS:
                dat->dwFlags ^= MWF_LOG_INDIVIDUALBKG;
                return 1;
            case ID_MESSAGELOGFORMATTING_GROUPMESSAGES:
                dat->dwFlags ^= MWF_LOG_GROUPMODE;
                return 1;
            case ID_TIMESTAMPSETTINGS_USELONGDATEFORMAT:
                dat->dwFlags ^= MWF_LOG_LONGDATES;
                return 1;
            case ID_TIMESTAMPSETTINGS_USERELATIVETIMESTAMPS:
                dat->dwFlags ^= MWF_LOG_USERELATIVEDATES;
                return 1;
            case ID_MESSAGELOG_MESSAGELOGSETTINGSAREGLOBAL:
                iIgnorePerContact = !iIgnorePerContact;
                DBWriteContactSettingByte(NULL, SRMSGMOD_T, "ignorecontactsettings", iIgnorePerContact);
                return 1;
            case ID_MESSAGELOG_APPLYMESSAGELOGSETTINGSTOALLCONTACTS:
                {
                    HANDLE hContact;
                    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
                    while(hContact) {
                        DBWriteContactSettingDword(hContact, SRMSGMOD_T, "mwflags", dat->dwFlags & MWF_LOG_ALL);
                        DBWriteContactSettingDword(hContact, SRMSGMOD, "splitsplity", dat->splitterY);
                        hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
                    }
                    WindowList_Broadcast(hMessageWindowList, DM_FORCEDREMAKELOG, (WPARAM)hwndDlg, (LPARAM)(dat->dwFlags & MWF_LOG_ALL));
                    return 1;
                }
        }
    }
    return 0;
}

void UpdateStatusBarTooltips(HWND hwndDlg, struct MessageWindowData *dat)
{
    if(dat->pContainer->hwndStatus && dat->pContainer->hwndActive == hwndDlg) {
        char szTipText[256], *szProto = NULL;
        CONTACTINFO ci;

        if(IsMetaContact(hwndDlg, dat))
            szProto = GetCurrentMetaContactProto(hwndDlg, dat);
        else
            szProto = dat->szProto;
        
        ZeroMemory(&ci, sizeof(ci));
        ci.cbSize = sizeof(ci);
        ci.hContact = NULL;
        ci.szProto = szProto;
        ci.dwFlag = CNF_DISPLAY;
        if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
            if(IsMetaContact(hwndDlg, dat))
                _snprintf(szTipText, sizeof(szTipText), Translate("You are %s on %s (MetaContact)"), ci.pszVal, szProto);
            else
                _snprintf(szTipText, sizeof(szTipText), Translate("You are %s on %s"), ci.pszVal, szProto); 
            SendMessage(dat->pContainer->hwndStatus, SB_SETTIPTEXTA, 2, (LPARAM)szTipText);
        }
        if(ci.pszVal)
            miranda_sys_free(ci.pszVal);
    }
}

void UpdateStatusBar(HWND hwndDlg, struct MessageWindowData *dat)
{
    if(dat->pContainer->hwndStatus && dat->pContainer->hwndActive == hwndDlg) {

        SetSelftypingIcon(hwndDlg, dat, DBGetContactSettingByte(dat->hContact, SRMSGMOD, SRMSGSET_TYPING, DBGetContactSettingByte(NULL, SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)));
        SendMessage(hwndDlg, DM_UPDATELASTMESSAGE, 0, 0);
        UpdateReadChars(hwndDlg, dat);
        if(dat->hProtoIcon)
            SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 2, (LPARAM)dat->hProtoIcon);
        UpdateStatusBarTooltips(hwndDlg, dat);
    }
}

void HandleIconFeedback(HWND hwndDlg, struct MessageWindowData *dat, int iIcon)
{
    TCITEM item = {0};
    int iOldIcon;
    
    item.mask = TCIF_IMAGE;
    TabCtrl_GetItem(GetDlgItem(dat->pContainer->hwnd, IDC_MSGTABS), dat->iTabID, &item);
    iOldIcon = item.iImage;

    if(iIcon == -1) {    // restore status image
        if(dat->dwFlags & MWF_ERRORSTATE) {
            iIcon = g_IconError;
        }
        else {
            iIcon = dat->iTabImage;
        }
    }
        
    item.iImage = iIcon;
    if (dat->pContainer->iChilds > 1 || !(dat->pContainer->dwFlags & CNT_HIDETABS)) {       // we have tabs
    }
    TabCtrl_SetItem(GetDlgItem(dat->pContainer->hwnd, IDC_MSGTABS), dat->iTabID, &item);
}

/*
 * retrieve the visiblity of the avatar window, depending on the global setting
 * and local mode
 */

int GetAvatarVisibility(HWND hwndDlg, struct MessageWindowData *dat)
{
    BYTE bAvatarMode = DBGetContactSettingByte(NULL, SRMSGMOD_T, "avatarmode", 0);

    dat->showPic = 0;
    switch(bAvatarMode) {
        case 0:             // globally on
            dat->showPic = 1;
            break;
        case 4:             // globally OFF
            dat->showPic = 0;
            break;
        case 3:             // on, if present
            if(dat->hContactPic && dat->hContactPic != g_hbmUnknown)
                dat->showPic = 1;
            else
                dat->showPic = 0;
            break;
        case 1:             // on for protocols with avatar support
            {
                int pCaps;

                if(dat->szProto) {
                    pCaps = CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_4, 0);
                    if((pCaps & PF4_AVATARS) && dat->hContactPic && dat->hContactPic != g_hbmUnknown) {
                        dat->showPic = 1;
                    }
                }
                break;
            }
        case 2:             // default (per contact, as it always was
            dat->showPic = DBGetContactSettingByte(dat->hContact, SRMSGMOD_T, "MOD_ShowPic", 0);
            break;
    }
    return dat->showPic;
}

void SetSelftypingIcon(HWND dlg, struct MessageWindowData *dat, int iMode)
{
    if(dat->pContainer->hwndStatus && dat->pContainer->hwndActive == dlg) {
        int nParts = SendMessage(dat->pContainer->hwndStatus, SB_GETPARTS, 0, 0);

        if(iMode)
            SendMessage(dat->pContainer->hwndStatus, SB_SETICON, nParts - 1, (LPARAM)g_buttonBarIcons[12]);
        else
            SendMessage(dat->pContainer->hwndStatus, SB_SETICON, nParts - 1, (LPARAM)g_buttonBarIcons[13]);

        InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
    }
}

/*
 * checks, if there is a valid smileypack installed for the given protocol
 */

int CheckValidSmileyPack(char *szProto, HICON *hButtonIcon)
{
    SMADD_INFO smainfo;

    if(g_SmileyAddAvail) {
        smainfo.cbSize = sizeof(smainfo);
        smainfo.Protocolname = szProto;
        CallService(MS_SMILEYADD_GETINFO, 0, (LPARAM)&smainfo);
        *hButtonIcon = smainfo.ButtonIcon;
        return smainfo.NumberOfVisibleSmileys;
    }
    else
        return 0;
}

/*
 * resize the default :) smiley to fit on the button (smileys may be bigger than 16x16.. so it needs to be done)
 */

void CreateSmileyIcon(struct MessageWindowData *dat, HICON hIcon)
{
    HBITMAP hBmp, hoBmp;
    HDC hdc, hdcMem;
    BITMAPINFOHEADER bih = {0};
    int widthBytes;
    RECT rc;
    HBRUSH hBkgBrush;
    ICONINFO ii;
    BITMAP bm;

    int sizex = 0;
    int sizey = 0;
    
    GetIconInfo(hIcon, &ii);
    GetObject(ii.hbmColor, sizeof(bm), &bm);
    sizex = GetSystemMetrics(SM_CXSMICON);
    sizey = GetSystemMetrics(SM_CYSMICON);
    DeleteObject(ii.hbmMask);
    DeleteObject(ii.hbmColor);

    hBkgBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    bih.biSize = sizeof(bih);
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    
    bih.biHeight = sizex;
    bih.biPlanes = 1;
    bih.biWidth = sizey;
    widthBytes = ((bih.biWidth*bih.biBitCount + 31) >> 5) * 4;
    rc.top = rc.left = 0;
    rc.right = bih.biWidth;
    rc.bottom = bih.biHeight;
    hdc = GetDC(NULL);
    hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
    hdcMem = CreateCompatibleDC(hdc);
    hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
    FillRect(hdcMem, &rc, hBkgBrush);
    DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
    SelectObject(hdcMem, hoBmp);

    DeleteDC(hdcMem);
    DeleteObject(hoBmp);
    ReleaseDC(NULL, hdc);
    DeleteObject(hBkgBrush);
    dat->hSmileyIcon = hBmp;
}

/*
 * return value MUST be free()'d by caller.
 */

TCHAR *QuoteText(TCHAR *text,int charsPerLine,int removeExistingQuotes)
{
    int inChar,outChar,lineChar;
    int justDoneLineBreak,bufSize;
    TCHAR *strout;

#ifdef _UNICODE
    bufSize = lstrlenW(text) + 23;
#else
    bufSize=strlen(text)+23;
#endif
    strout=(TCHAR*)malloc(bufSize * sizeof(TCHAR));
    inChar=0;
    justDoneLineBreak=1;
    for (outChar=0,lineChar=0;text[inChar];) {
        if (outChar>=bufSize-8) {
            bufSize+=20;
            strout=(TCHAR *)realloc(strout, bufSize * sizeof(TCHAR));
        }
        if (justDoneLineBreak && text[inChar]!='\r' && text[inChar]!='\n') {
            if (removeExistingQuotes)
                if (text[inChar]=='>') {
                    while (text[++inChar]!='\n');
                    inChar++;
                    continue;
                }
            strout[outChar++]='>'; strout[outChar++]=' ';
            lineChar=2;
        }
        if (lineChar==charsPerLine && text[inChar]!='\r' && text[inChar]!='\n') {
            int decreasedBy;
            for (decreasedBy=0;lineChar>10;lineChar--,inChar--,outChar--,decreasedBy++)
                if (strout[outChar]==' ' || strout[outChar]=='\t' || strout[outChar]=='-') break;
            if (lineChar<=10) {
                lineChar+=decreasedBy; inChar+=decreasedBy; outChar+=decreasedBy;
            } else inChar++;
            strout[outChar++]='\r'; strout[outChar++]='\n';
            justDoneLineBreak=1;
            continue;
        }
        strout[outChar++]=text[inChar];
        lineChar++;
        if (text[inChar]=='\n' || text[inChar]=='\r') {
            if (text[inChar]=='\r' && text[inChar+1]!='\n')
                strout[outChar++]='\n';
            justDoneLineBreak=1;
            lineChar=0;
        } else justDoneLineBreak=0;
        inChar++;
    }
    strout[outChar++]='\r';
    strout[outChar++]='\n';
    strout[outChar]=0;
    return strout;
}


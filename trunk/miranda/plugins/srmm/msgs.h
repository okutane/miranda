/*
SRMM

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
#ifndef SRMM_MSGS_H
#define SRMM_MSGS_H

#include <richedit.h>
#include <richole.h>
#define MSGERROR_CANCEL	0
#define MSGERROR_RETRY	    1

struct NewMessageWindowLParam
{
    HANDLE hContact;
    const char *szInitialText;
};

struct MessageSendInfo
{
    HANDLE hSendId;
};

struct MessageWindowData
{
    HANDLE hContact;
    HANDLE hDbEventFirst, hDbEventLast;
    struct MessageSendInfo *sendInfo;
    int sendCount;
    HANDLE hAckEvent;
    HANDLE hNewEvent;
    int showTime;
    HBRUSH hBkgBrush;
    int splitterY, originalSplitterY;
    char *sendBuffer;
    HICON hIcons[6];
    SIZE minEditBoxSize;
    int showInfo;
    int showButton;
    int lineHeight;
    int windowWasCascaded;
    int nFlash;
    int nFlashMax;
    int nLabelRight;
    int nTypeSecs;
    int nTypeMode;
    int showSend;
    DWORD nLastTyping;
    int showTyping;
    int showTypingWin;
    HWND hwndStatus;
    DWORD lastMessage;
    int showIcons;
    int showDate;
    int hideNames;
    char *szProto;
    WORD wStatus;
    WORD wOldStatus;
    TCmdList *cmdList;
    TCmdList *cmdListCurrent;
};

#define HM_EVENTSENT         (WM_USER+10)
#define DM_REMAKELOG         (WM_USER+11)
#define HM_DBEVENTADDED      (WM_USER+12)
#define DM_CASCADENEWWINDOW  (WM_USER+13)
#define DM_OPTIONSAPPLIED    (WM_USER+14)
#define DM_SPLITTERMOVED     (WM_USER+15)
#define DM_UPDATETITLE       (WM_USER+16)
#define DM_APPENDTOLOG       (WM_USER+17)
#define DM_ERRORDECIDED      (WM_USER+18)
#define DM_SCROLLLOGTOBOTTOM (WM_USER+19)
#define DM_TYPING            (WM_USER+20)
#define DM_UPDATEWINICON     (WM_USER+21)
#define DM_UPDATELASTMESSAGE (WM_USER+22)

#define EVENTTYPE_STATUSCHANGE 25368

struct CREOleCallback
{
    IRichEditOleCallbackVtbl *lpVtbl;
    unsigned refCount;
    IStorage *pictStg;
    int nextStgId;
};

BOOL CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int InitOptions(void);
BOOL CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int DbEventIsShown(DBEVENTINFO * dbei, struct MessageWindowData *dat);
void StreamInEvents(HWND hwndDlg, HANDLE hDbEventFirst, int count, int fAppend);
void LoadMsgLogIcons(void);
void FreeMsgLogIcons(void);

#define MSGFONTID_MYMSG		  0
#define MSGFONTID_YOURMSG	  1
#define MSGFONTID_MYNAME	  2
#define MSGFONTID_MYTIME	  3
#define MSGFONTID_MYCOLON	  4
#define MSGFONTID_YOURNAME	  5
#define MSGFONTID_YOURTIME	  6
#define MSGFONTID_YOURCOLON	  7
#define MSGFONTID_MESSAGEAREA 8
#define MSGFONTID_NOTICE      9

void LoadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour);
extern const int msgDlgFontCount;

#define LOADHISTORY_UNREAD    0
#define LOADHISTORY_COUNT     1
#define LOADHISTORY_TIME      2

#define SRMMMOD                    "SRMM"

#define SRMSGSET_SHOWBUTTONLINE    "ShowButtonLine"
#define SRMSGDEFSET_SHOWBUTTONLINE 1
#define SRMSGSET_SHOWINFOLINE      "ShowInfoLine"
#define SRMSGDEFSET_SHOWINFOLINE   1
#define SRMSGSET_AUTOPOPUP         "AutoPopup"
#define SRMSGDEFSET_AUTOPOPUP      0
#define SRMSGSET_AUTOMIN           "AutoMin"
#define SRMSGDEFSET_AUTOMIN        0
#define SRMSGSET_AUTOCLOSE         "AutoClose"
#define SRMSGDEFSET_AUTOCLOSE      0
#define SRMSGSET_SAVEPERCONTACT    "SavePerContact"
#define SRMSGDEFSET_SAVEPERCONTACT 0
#define SRMSGSET_CASCADE           "Cascade"
#define SRMSGDEFSET_CASCADE        1
#define SRMSGSET_SENDONENTER       "SendOnEnter"
#define SRMSGDEFSET_SENDONENTER    1
#define SRMSGSET_SENDONDBLENTER    "SendOnDblEnter"
#define SRMSGDEFSET_SENDONDBLENTER 0
#define SRMSGSET_STATUSICON        "UseStatusWinIcon"
#define SRMSGDEFSET_STATUSICON     0
#define SRMSGSET_SENDBUTTON        "UseSendButton"
#define SRMSGDEFSET_SENDBUTTON     0
#define SRMSGSET_CHARCOUNT         "ShowCharCount"
#define SRMSGDEFSET_CHARCOUNT      0
#define SRMSGSET_CTRLSUPPORT       "SupportCtrlUpDn"
#define SRMSGDEFSET_CTRLSUPPORT    1
#define SRMSGSET_MSGTIMEOUT        "MessageTimeout"
#define SRMSGDEFSET_MSGTIMEOUT     10000
#define SRMSGSET_MSGTIMEOUT_MIN    4000 // minimum value (4 seconds)

#define SRMSGSET_LOADHISTORY       "LoadHistory"
#define SRMSGDEFSET_LOADHISTORY    LOADHISTORY_UNREAD
#define SRMSGSET_LOADCOUNT         "LoadCount"
#define SRMSGDEFSET_LOADCOUNT      10
#define SRMSGSET_LOADTIME          "LoadTime"
#define SRMSGDEFSET_LOADTIME       10

#define SRMSGSET_SHOWLOGICONS      "ShowLogIcon"
#define SRMSGDEFSET_SHOWLOGICONS   1
#define SRMSGSET_HIDENAMES         "HideNames"
#define SRMSGDEFSET_HIDENAMES      1
#define SRMSGSET_SHOWTIME          "ShowTime"
#define SRMSGDEFSET_SHOWTIME       1
#define SRMSGSET_SHOWDATE          "ShowDate"
#define SRMSGDEFSET_SHOWDATE       0
#define SRMSGSET_SHOWSTATUSCH      "ShowStatusChanges"
#define SRMSGDEFSET_SHOWSTATUSCH   1
#define SRMSGSET_BKGCOLOUR         "BkgColour"
#define SRMSGDEFSET_BKGCOLOUR      GetSysColor(COLOR_WINDOW)

#define SRMSGSET_TYPING             "SupportTyping"
#define SRMSGSET_TYPINGNEW          "DefaultTyping"
#define SRMSGDEFSET_TYPINGNEW       1
#define SRMSGSET_TYPINGUNKNOWN      "UnknownTyping"
#define SRMSGDEFSET_TYPINGUNKNOWN   0
#define SRMSGSET_SHOWTYPING         "ShowTyping"
#define SRMSGDEFSET_SHOWTYPING      1
#define SRMSGSET_SHOWTYPINGWIN      "ShowTypingWin"
#define SRMSGDEFSET_SHOWTYPINGWIN   1
#define SRMSGSET_SHOWTYPINGNOWIN    "ShowTypingTray"
#define SRMSGDEFSET_SHOWTYPINGNOWIN 0
#define SRMSGSET_SHOWTYPINGCLIST    "ShowTypingClist"
#define SRMSGDEFSET_SHOWTYPINGCLIST 1

#endif

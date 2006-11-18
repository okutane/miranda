/*
Scriver

Copyright 2000-2003 Miranda ICQ/IM project,
Copyright 2005 Piotr Piastucki

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

typedef DWORD (WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);
extern PSLWA pSetLayeredWindowAttributes;
extern BOOL (WINAPI *pfnEnableThemeDialogTexture)(HANDLE, DWORD);
extern BOOL (WINAPI *pfnIsAppThemed)(VOID);

typedef struct ErrorWindowDataStruct
{
	TCHAR*	szName;
	TCHAR*	szDescription;
	char*	szText;
	int		textSize;
	int		flags;
	HWND	hwndParent;
} ErrorWindowData;

typedef struct TabCtrlDataStruct
{
	int		lastClickTime;
	WPARAM  clickWParam;
	LPARAM  clickLParam;
	POINT	mouseLBDownPos;
	int		lastClickTab;
	HIMAGELIST hDragImageList;
	int		bDragging;
	int		bDragged;
	int		destTab;
	int		srcTab;
} TabCtrlData;

typedef struct ParentWindowDataStruct
{
	HWND	hwnd;
	HANDLE	hContact;
	struct ParentWindowDataStruct 	*prev;
	struct ParentWindowDataStruct 	*next;
	int	    childrenCount;
	HWND	hwndActive;
	HWND	hwndStatus;
	HWND	hwndTabs;
	HWND	foregroundWindow;
	DWORD	flags2;
	RECT	childRect;
	POINT	mouseLBDownPos;
	int		mouseLBDown;
	int		nFlash;
	int		nFlashMax;
	int		bMinimized;
	int		bVMaximized;
	int		bTopmost;
	int		windowWasCascaded;
	TabCtrlData *tabCtrlDat;
	BOOL	isChat;
}ParentWindowData;

typedef struct MessageWindowTabDataStruct
{
	HWND	hwnd;
	HANDLE	hContact;
	char *szProto;
	ParentWindowData *parent;
}MessageWindowTabData;

#define NMWLP_INCOMING 1

typedef struct NewMessageWindowLParamStruct
{
	HANDLE	hContact;
	BOOL	isChat;
	int		isWchar;
	const char *szInitialText;
	int		flags;
} NewMessageWindowLParam;

struct MessageSendInfo
{
	HANDLE hSendId;
	int		timeout;
	char *	sendBuffer;
	int		sendBufferSize;
	int		flags;
};


struct MessageWindowData
{
	HWND hwnd;
	int	tabId;
	HANDLE hContact;
	ParentWindowData *parent;
	HWND hwndParent;
	HWND hwndLog;
	HANDLE hDbEventFirst, hDbEventLast;
	struct MessageSendInfo *sendInfo;
	int sendCount;
	int splitterPos, originalSplitterPos;
//	char *sendBuffer;
	SIZE minEditBoxSize;
	SIZE minTopSize;
	RECT minEditInit;
	int	minLogBoxHeight;
	int	minEditBoxHeight;
	SIZE toolbarSize;
	int windowWasCascaded;
	int nTypeSecs;
	int nTypeMode;
	int avatarWidth;
	int avatarHeight;
	HBITMAP avatarPic;
	DWORD nLastTyping;
	int showTyping;
	int showUnread;
	DWORD lastMessage;
	char *szProto;
	WORD wStatus;
	WORD wOldStatus;
	TCmdList *cmdList;
	TCmdList *cmdListCurrent;
	TCmdList *cmdListNew;
	time_t	startTime;
	time_t 	lastEventTime;
	int    	lastEventType;
	HANDLE  lastEventContact;
	DWORD	flags;
	int		messagesInProgress;
	int		codePage;
	struct avatarCacheEntry *ace;
	int		isMixed;
	int		sendAllConfirm;
};

#define HM_EVENTSENT         (WM_USER+10)
#define DM_REMAKELOG         (WM_USER+11)
#define HM_DBEVENTADDED      (WM_USER+12)
#define DM_CASCADENEWWINDOW  (WM_USER+13)
#define DM_OPTIONSAPPLIED    (WM_USER+14)
#define DM_SPLITTERMOVED     (WM_USER+15)
#define DM_APPENDTOLOG       (WM_USER+17)
#define DM_ERRORDECIDED      (WM_USER+18)
#define DM_SCROLLLOGTOBOTTOM (WM_USER+19)
#define DM_TYPING            (WM_USER+20)
#define DM_UPDATELASTMESSAGE (WM_USER+22)
#define DM_USERNAMETOCLIP    (WM_USER+23)
#define DM_CHANGEICONS		 (WM_USER+24)

#define DM_AVATARCALCSIZE    (WM_USER+25)
#define DM_GETAVATAR         (WM_USER+26)
#define DM_SENDMESSAGE		 (WM_USER+27)

#define HM_AVATARACK         (WM_USER+28)
#define HM_ACKEVENT          (WM_USER+29)

#define DM_UPDATEICON		 (WM_USER+31)

#define DM_CLEARLOG			 (WM_USER+46)
#define DM_SWITCHSTATUSBAR	 (WM_USER+47)
#define DM_SWITCHTOOLBAR	 (WM_USER+48)
#define DM_SWITCHTITLEBAR	 (WM_USER+49)
#define DM_SWITCHRTL		 (WM_USER+50)
#define DM_SWITCHUNICODE	 (WM_USER+51)
#define DM_GETCODEPAGE		 (WM_USER+52)
#define DM_SETCODEPAGE		 (WM_USER+53)
#define DM_MESSAGESENDING	 (WM_USER+54)
#define DM_GETWINDOWSTATE	 (WM_USER+55)
#define DM_STATUSICONCHANGE  (WM_USER+56)

#define DM_DEACTIVATE		 (WM_USER+61)
#define DM_MYAVATARCHANGED	 (WM_USER+62)
#define DM_PROTOAVATARCHANGED (WM_USER+63)
#define DM_AVATARCHANGED	 (WM_USER+64)
#define DM_SETFOCUS			  (WM_USER+65)

#define EM_SUBCLASSED        (WM_USER+0x101)
#define EM_UNSUBCLASSED      (WM_USER+0x102)

#define EVENTTYPE_STATUSCHANGE 25368

struct CREOleCallback
{
	IRichEditOleCallbackVtbl *lpVtbl;
	unsigned refCount;
	IStorage *pictStg;
	int nextStgId;
};

BOOL CALLBACK DlgProcParentWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int DbEventIsShown(DBEVENTINFO * dbei, struct MessageWindowData *dat);
int safe_wcslen(wchar_t *msg, int maxLen);
void StreamInEvents(HWND hwndDlg, HANDLE hDbEventFirst, int count, int fAppend);
void LoadMsgLogIcons(void);
void FreeMsgLogIcons(void);
TCHAR *GetNickname(HANDLE hContact, const char* szProto);

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

#define SRMSGSET_USETABS		   "UseTabs"
#define SRMSGDEFSET_USETABS		   1
#define SRMSGSET_TABSATBOTTOM	   "TabsPosition"
#define SRMSGDEFSET_TABSATBOTTOM   0
#define SRMSGSET_TABCLOSEBUTTON	   "TabCloseButton"
#define SRMSGDEFSET_TABCLOSEBUTTON 0
#define SRMSGSET_LIMITNAMES		   "LimitNamesOnTabs"
#define SRMSGDEFSET_LIMITNAMES	   1
#define SRMSGSET_LIMITNAMESLEN	   "LimitNamesLength"
#define SRMSGDEFSET_LIMITNAMESLEN  20
#define SRMSGSET_LIMITNAMESLEN_MIN 0
#define SRMSGSET_HIDEONETAB		   "HideOneTab"
#define SRMSGDEFSET_HIDEONETAB	   1
#define SRMSGSET_SEPARATECHATSCONTAINERS "SeparateChatsContainers"
#define SRMSGDEFSET_SEPARATECHATSCONTAINERS 0
#define SRMSGSET_LIMITTABS		   "LimitTabs"
#define SRMSGDEFSET_LIMITTABS	   0
#define SRMSGSET_LIMITTABSNUM      "LimitTabsNum"
#define SRMSGDEFSET_LIMITTABSNUM   10
#define SRMSGSET_LIMITCHATSTABS		  "LimitChatsTabs"
#define SRMSGDEFSET_LIMITCHATSTABS	   0
#define SRMSGSET_LIMITCHATSTABSNUM	   "LimitChatsTabsNum"
#define SRMSGDEFSET_LIMITCHATSTABSNUM  10

#define SRMSGSET_CASCADE           "Cascade"
#define SRMSGDEFSET_CASCADE        1
#define SRMSGSET_SAVEPERCONTACT    "SavePerContact"
#define SRMSGDEFSET_SAVEPERCONTACT 0

#define SRMSGSET_SHOWTITLEBAR	   "ShowTitleBar"
#define SRMSGDEFSET_SHOWTITLEBAR   1
#define SRMSGSET_SHOWSTATUSBAR	   "ShowStatusBar"
#define SRMSGDEFSET_SHOWSTATUSBAR  1

#define SRMSGSET_TOPMOST		   "Topmost"
#define SRMSGDEFSET_TOPMOST		   0
#define SRMSGSET_POPFLAGS          "PopupFlags"
#define SRMSGDEFSET_POPFLAGS       0
#define SRMSGSET_SHOWBUTTONLINE    "ShowButtonLine"
#define SRMSGDEFSET_SHOWBUTTONLINE 1
#define SRMSGSET_SHOWINFOLINE      "ShowInfoLine"
#define SRMSGDEFSET_SHOWINFOLINE   1
#define SRMSGSET_SHOWPROGRESS	   "ShowProgress"
#define SRMSGDEFSET_SHOWPROGRESS   0
#define SRMSGSET_AUTOPOPUP         "AutoPopupMsg"
#define SRMSGDEFSET_AUTOPOPUP      0
#define SRMSGSET_STAYMINIMIZED     "StayMinimized"
#define SRMSGDEFSET_SWITCHTOACTIVE 0
#define SRMSGSET_SWITCHTOACTIVE    "SwitchToActiveTab"
#define SRMSGDEFSET_STAYMINIMIZED  0
#define SRMSGSET_AUTOMIN           "AutoMin"
#define SRMSGDEFSET_AUTOMIN        0
#define SRMSGSET_AUTOCLOSE         "AutoClose"
#define SRMSGDEFSET_AUTOCLOSE      0
#define SRMSGSET_SAVESPLITTERPERCONTACT "SaveSplitterPerContact"
#define SRMSGDEFSET_SAVESPLITTERPERCONTACT 0
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
#define SRMSGSET_DELTEMP           "DeleteTempCont"
#define SRMSGDEFSET_DELTEMP        0
#define SRMSGSET_MSGTIMEOUT        "MessageTimeout"
#define SRMSGDEFSET_MSGTIMEOUT     10000
#define SRMSGSET_MSGTIMEOUT_MIN    4000 // minimum value (4 seconds)
#define SRMSGSET_FLASHCOUNT        "FlashMax"
#define SRMSGDEFSET_FLASHCOUNT     5

#define SRMSGSET_LOADHISTORY       "LoadHistory"
#define SRMSGDEFSET_LOADHISTORY    LOADHISTORY_UNREAD
#define SRMSGSET_LOADCOUNT         "LoadCount"
#define SRMSGDEFSET_LOADCOUNT      10
#define SRMSGSET_LOADTIME          "LoadTime"
#define SRMSGDEFSET_LOADTIME       10

#define SRMSGSET_USELONGDATE       "UseLongDate"
#define SRMSGDEFSET_USELONGDATE    0
#define SRMSGSET_SHOWSECONDS       "ShowSeconds"
#define SRMSGDEFSET_SHOWSECONDS    1
#define SRMSGSET_USERELATIVEDATE   "UseRelativeDate"
#define SRMSGDEFSET_USERELATIVEDATE 0

#define SRMSGSET_GROUPMESSAGES     "GroupMessages"
#define SRMSGDEFSET_GROUPMESSAGES	0
#define SRMSGSET_MARKFOLLOWUPS		"MarkFollowUps"
#define SRMSGDEFSET_MARKFOLLOWUPS	0
#define SRMSGSET_MESSAGEONNEWLINE   "MessageOnNewLine"
#define SRMSGDEFSET_MESSAGEONNEWLINE 0
#define SRMSGSET_DRAWLINES			"DrawLines"
#define SRMSGDEFSET_DRAWLINES		0
#define SRMSGSET_LINECOLOUR			"LineColour"
#define SRMSGDEFSET_LINECOLOUR		GetSysColor(COLOR_WINDOW)

#define SRMSGSET_SHOWLOGICONS		"ShowLogIcon"
#define SRMSGDEFSET_SHOWLOGICONS	1
#define SRMSGSET_HIDENAMES			"HideNames"
#define SRMSGDEFSET_HIDENAMES		1
#define SRMSGSET_SHOWTIME			"ShowTime"
#define SRMSGDEFSET_SHOWTIME		1
#define SRMSGSET_SHOWDATE			"ShowDate"
#define SRMSGDEFSET_SHOWDATE		0
#define SRMSGSET_SHOWSTATUSCH		"ShowStatusChanges"
#define SRMSGDEFSET_SHOWSTATUSCH	1
#define SRMSGSET_BKGCOLOUR				"BkgColour"
#define SRMSGDEFSET_BKGCOLOUR			GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INPUTBKGCOLOUR			"InputBkgColour"
#define SRMSGDEFSET_INPUTBKGCOLOUR		GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INCOMINGBKGCOLOUR		"IncomingBkgColour"
#define SRMSGDEFSET_INCOMINGBKGCOLOUR	GetSysColor(COLOR_WINDOW)
#define SRMSGSET_OUTGOINGBKGCOLOUR		"OutgoingBkgColour"
#define SRMSGDEFSET_OUTGOINGBKGCOLOUR	GetSysColor(COLOR_WINDOW)
#define SRMSGSET_USEIEVIEW				"UseIEView"
#define SRMSGDEFSET_USEIEVIEW			1


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

#define SRMSGSET_AVATARENABLE       "AvatarEnable"
#define SRMSGDEFSET_AVATARENABLE    1
#define SRMSGSET_LIMITAVHEIGHT      "AvatarLimitHeight"
#define SRMSGDEFSET_LIMITAVHEIGHT   0
#define SRMSGSET_AVHEIGHT           "AvatarHeight"
#define SRMSGDEFSET_AVHEIGHT        60
#define SRMSGSET_AVHEIGHTMIN        "AvatarHeightMin"
#define SRMSGDEFSET_AVHEIGHTMIN     20
#define SRMSGSET_AVATAR             "Avatar"

#define SRMSGSET_USETRANSPARENCY	"UseTransparency"
#define SRMSGDEFSET_USETRANSPARENCY 0
#define SRMSGSET_ACTIVEALPHA		"ActiveAlpha"
#define SRMSGDEFSET_ACTIVEALPHA		0
#define SRMSGSET_INACTIVEALPHA		"InactiveAlpha"
#define SRMSGDEFSET_INACTIVEALPHA	0
#define SRMSGSET_WINDOWTITLE		"WindowTitle"
#define SRMSGSET_SAVEDRAFTS			"SaveDrafts"
#define SRMSGDEFSET_SAVEDRAFTS		0
#define SRMSGSET_BUTTONVISIBILITY	"ButtonVisibility"
#define SRMSGDEFSET_BUTTONVISIBILITY 0xFFFF

#endif

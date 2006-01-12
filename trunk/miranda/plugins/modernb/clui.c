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

#include "m_clc.h"
#include "m_clui.h"
#include "m_skin.h"
#include "genmenu.h"
#include "wingdi.h"
#include <Winuser.h>
#include "skinengine.h"

HANDLE hSkinLoaded;
BOOL ON_SIZING_CYCLE=0;
extern BYTE CALLED_FROM_SHOWHIDE;

#define TM_AUTOALPHA  1
#define TM_DELAYEDSIZING 2
#define TM_BRINGOUTTIMEOUT 3
#define TM_BRINGINTIMEOUT 4
#define TM_UPDATEBRINGTIMER 5
#define TM_SMOTHALPHATRANSITION   20
#define TM_WINDOWUPDATE 100
#define TM_STATUSBARUPDATE  200
#define MENU_MIRANDAMENU         0xFFFF1234
#define MENU_STATUSMENU         0xFFFF1235

extern BOOL TransparentFlag;
extern void Docking_GetMonitorRectFromWindow(HWND hWnd,RECT *rc);
extern sCurrentWindowImageData * cachedWindow;
int BehindEdge_State;
int BehindEdgeSettings;
WORD BehindEdgeShowDelay;
WORD BehindEdgeHideDelay;
WORD BehindEdgeBorderSize;

extern BOOL LOCK_IMAGE_UPDATING;
extern BOOL LOCK_UPDATING;
#define UM_ALPHASUPPORT WM_USER+100


static HMODULE hUserDll;
static HIMAGELIST himlMirandaIcon;
HMENU hMenuMain;
BOOL SmoothAnimation;
float alphaStep;
BYTE alphaStart,alphaEnd;
extern BYTE CURRENT_ALPHA;
extern BOOL LayeredFlag;
extern int docked;
BOOL DestMode;
BYTE ANIMATION_IS_IN_PROGRESS=0;
BOOL SHOWHIDE_CALLED_FROM_ANIMATION=0;
BOOL firstTimeCallFlag=FALSE;
int SmoothAlphaTransition(HWND hwnd, BYTE GoalAlpha, BOOL wParam);
extern tPaintCallbackProc ClcPaintCallbackProc(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData);
extern int LoadModernButtonModule(void);
extern int CreateModernStatusBar(HWND parent);
int SmoothAlphaThreadTransition(HWND hwnd);
BOOL IsInChangingMode;
extern void (*savedLoadCluiGlobalOpts)(void);
/*
*	Function ShowWindowNew overloads API ShowWindow in case of
*  dropShaddow is enabled: it force to minimize window before hiding
*  to remove shadow.
*/
int ShowWindowNew(HWND hwnd, int cmd) 
{
	if (hwnd==pcli->hwndContactList 
		&& !IsInChangingMode
		&& cmd==SW_HIDE 
		&& !LayeredFlag 
		&& IsWinVerXPPlus()
		&& DBGetContactSettingByte(NULL,"CList","WindowShadow",0))
	{
		ShowWindow(hwnd,SW_MINIMIZE); //removing of shadow
		return ShowWindow(hwnd,cmd);
	}
	if (hwnd==pcli->hwndContactList
		&& !IsInChangingMode
		&& cmd==SW_RESTORE
		&& !LayeredFlag 
		&& IsWinVerXPPlus()		
		&& DBGetContactSettingByte(NULL,"CList","WindowShadow",0)
		&& SmoothAnimation
		&& !TransparentFlag 
		)
	{
		SmoothAlphaTransition(hwnd, 255, 1);		
	}
	return ShowWindow(hwnd,cmd);
}
struct sUpdatingWindow
{
	HDC screenDC;
	HDC offscreenDC;
	HBITMAP oldBitmap;
	HBITMAP currentBitmap;
	DWORD sizeX, sizeY;
};

struct sUpdatingWindow Update;
extern int HideWindow(HWND hwndContactList, int mode);
extern HFONT ChangeToFont(HDC hdc,struct ClcData *dat,int id,int *fontHeight);
static HANDLE hContactDraggingEvent,hContactDroppedEvent,hContactDragStopEvent;
extern int SetParentForContainers(HWND parent);
extern int OnShowHide(HWND hwnd, int mode);
extern int RedrawCompleteWindow();
extern int UpdateWindowImage();
extern int OnMoving(HWND hwnd,RECT *lParam);
extern int RepaintSubContainers();
extern int ActivateSubContainers(BOOL active);
extern int ReposButtons(HWND parent, BOOL draw,RECT *r);
extern int ValidateFrameImageProc(RECT * r);
extern int CLUIFramesApplyNewSizes(int mode);
extern int CLUIFramesOnClistResize2(WPARAM wParam,LPARAM lParam, int mode);
BOOL IS_WM_MOUSE_DOWN_IN_TRAY=0;

//int SkinUpdateWindow (HWND hwnd, HWND Caller);
UINT hMsgGetProfile=0;
extern void TrayIconDestroy(HWND hwnd);
extern HBITMAP CreateBitmap32(int cx, int cy);
extern HBITMAP CreateBitmap32Point(int cx, int cy, void ** bits);
int IsInMainWindow(HWND hwnd);
UINT uMsgProcessProfile;

//extern boolean canloadstatusbar;
boolean OnModulesLoadedCalled=FALSE;
IgnoreActivation=0;

HANDLE hSettingChangedHook=0;

static int transparentFocus=1;
static byte oldhideoffline;
static int lastreqh=0,requr=0,disableautoupd=1;
static int CurrAlpha=0;
int UpdatePendent=9999;
HANDLE hFrameContactTree;
int SkinMinX=0,SkinMinY=20;
BYTE showOpts;//for statusbar
BOOL ON_EDGE_SIZING=0;
RECT ON_EDGE_SIZING_POS={0};

typedef struct{
	int IconsCount;
	int CycleStartTick;
	char *szProto;
	int n;
	int TimerCreated;
} ProtoTicks,*pProtoTicks;
//int SkinUpdateWindowProc(HWND hwnd1);

ProtoTicks CycleStartTick[64];//max 64 protocols 

int CycleTimeInterval=2000;
int CycleIconCount=8;
int DefaultStep=100;
//extern struct wndFrame *Frames;
extern int nFramescount;
extern BOOL FillRect255Alpha(HDC memdc,RECT *fr);
extern BOOL (WINAPI *MyUpdateLayeredWindow)(HWND,HDC,POINT*,SIZE*,HDC,POINT*,COLORREF,BLENDFUNCTION*,DWORD);
BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
BOOL (WINAPI *MySetLayeredWindowAttributesNew)(HWND,COLORREF,BYTE,DWORD);
BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags);
int CreateTimerForConnectingIcon(WPARAM wParam,LPARAM lParam);

int CluiOptInit(WPARAM wParam,LPARAM lParam);
int SortList(WPARAM wParam,LPARAM lParam);
int LoadCluiServices(void);
int CluiProtocolStatusChanged(WPARAM wParam,LPARAM lParam);
extern int CheckProtocolOrder();

extern void SetAllExtraIcons(HWND hwndList,HANDLE hContact);
extern void ReloadExtraIcons();
extern void LoadExtraImageFunc();
//extern int CreateStatusBarhWnd(HWND parent);
extern int CreateStatusBarFrame();
extern int LoadProtocolOrderModule(void);
extern int CLUIFramesUpdateFrame(WPARAM wParam,LPARAM lParam);
extern int ExtraToColumnNum(int extra);
extern void TrayIconUpdateBase(const char *szChangedProto);
extern void DrawDataForStatusBar(LPDRAWITEMSTRUCT dis);
extern void InitGroupMenus();
extern EnterDragToScroll(HWND hwnd, int Y);
extern int UseOwnerDrawStatusBar;
//int SkinUpdateWindow(HWND hwnd,HWND hwnd2);

#define M_CREATECLC  (WM_USER+1)
#define M_SETALLEXTRAICONS (WM_USER+2)

#define MS_CLUI_SHOWMAINMENU    "CList/ShowMainMenu"
#define MS_CLUI_SHOWSTATUSMENU  "CList/ShowStatusMenu"

int TestCursorOnBorders();
int SizingOnBorder(POINT pt, int size);
#include "skinEngine.h"

extern BOOL UPDATE_ALLREADY_QUEUED;
extern BOOL POST_WAS_CANCELED;
extern BYTE CURRENT_ALPHA;
BOOL IsOnDesktop=0;
extern BOOL NeedToBeFullRepaint;

extern int dock_prevent_moving;
int show_event_started=0;
extern int JustUpdateWindowImage();
BOOL TransparentFlag=FALSE;// TransparentFlag
void ChangeWindowMode()
{
	BOOL storedVisMode=FALSE;
	LONG style,styleEx;
	LONG styleMask=WS_CLIPCHILDREN|WS_BORDER|WS_CAPTION|WS_MINIMIZEBOX|WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME|WS_SYSMENU;
	LONG styleMaskEx=WS_EX_TOOLWINDOW|WS_EX_LAYERED;
	LONG curStyle,curStyleEx;
	if (!pcli->hwndContactList) return;

	IsInChangingMode=TRUE;
	TransparentFlag=IsWinVer2000Plus()&&DBGetContactSettingByte( NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT);
	SmoothAnimation=IsWinVer2000Plus()&&DBGetContactSettingByte(NULL, "CLUI", "FadeInOut", 1);
	//1- If visible store it and hide
	if (IsWindowVisible(pcli->hwndContactList)) 
	{
		storedVisMode=TRUE;
		ShowWindowNew(pcli->hwndContactList,SW_HIDE);
		OnShowHide(pcli->hwndContactList,0);
	}
	//2- Calculate STYLES and STYLESEX
	if (!LayeredFlag)
	{
		style=0;
		styleEx=0;
		if (DBGetContactSettingByte(NULL,"CList","ThinBorder",0) || (DBGetContactSettingByte(NULL,"CList","NoBorder",0)))
		{
			style=WS_CLIPCHILDREN| (DBGetContactSettingByte(NULL,"CList","ThinBorder",0)?WS_BORDER:0);
			styleEx=WS_EX_TOOLWINDOW;
		} 
		else if (DBGetContactSettingByte(NULL,"CLUI","ShowCaption",SETTING_SHOWCAPTION_DEFAULT) && DBGetContactSettingByte(NULL,"CList","ToolWindow",SETTING_TOOLWINDOW_DEFAULT))
		{
			styleEx=WS_EX_TOOLWINDOW/*|WS_EX_WINDOWEDGE*/;
			style=WS_CAPTION|WS_MINIMIZEBOX|WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME;
		}
		else if (DBGetContactSettingByte(NULL,"CLUI","ShowCaption",SETTING_SHOWCAPTION_DEFAULT))
		{
			style=WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME;
		}
		else 
		{
			style=WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME;
			styleEx=WS_EX_TOOLWINDOW/*|WS_EX_WINDOWEDGE*/;
		}
	}
	else 
	{
		style=WS_CLIPCHILDREN;
		styleEx=WS_EX_TOOLWINDOW;
	}
	//3- TODO Update Layered mode
	if(TransparentFlag&&LayeredFlag)
		styleEx|=WS_EX_LAYERED;

	//4- Set Title
	{
		TCHAR titleText[255]={0};
		DBVARIANT dbv;
		if(DBGetContactSettingTString(NULL,"CList","TitleText",&dbv))
			lstrcpyn(titleText,TEXT(MIRANDANAME),sizeof(titleText));
		else 
		{
			lstrcpyn(titleText,dbv.ptszVal,sizeof(titleText));
			DBFreeVariant(&dbv);
		}
		SetWindowText(pcli->hwndContactList,titleText);
	}
	if (IsOnDesktop || (DBGetContactSettingByte(NULL,"CList","OnDesktop", 0) && !firstTimeCallFlag))
	{
			SetParent(pcli->hwndContactList,NULL);
			SetParentForContainers(NULL);
			IsOnDesktop=0;
	}
	//5- TODO Apply Style
	curStyleEx=GetWindowLong(pcli->hwndContactList,GWL_EXSTYLE);
	curStyle=GetWindowLong(pcli->hwndContactList,GWL_STYLE);	
	curStyleEx=(curStyleEx&(~styleMaskEx))|styleEx;
	curStyle=(curStyle&(~styleMask))|style;
	SetWindowLong(pcli->hwndContactList,GWL_EXSTYLE,curStyleEx);
	SetWindowLong(pcli->hwndContactList,GWL_STYLE,curStyle);

	if(LayeredFlag || !DBGetContactSettingByte(NULL,"CLUI","ShowMainMenu",SETTING_SHOWMAINMENU_DEFAULT)) 
		SetMenu(pcli->hwndContactList,NULL);
	else
		SetMenu(pcli->hwndContactList,hMenuMain);

	//6- Pin to desktop mode
	if (DBGetContactSettingByte(NULL,"CList","OnDesktop", 0)) 
	{
		HWND hProgMan=FindWindow(TEXT("Progman"),NULL);
		if (IsWindow(hProgMan)) 
		{
			SetParent(pcli->hwndContactList,hProgMan);
			SetParentForContainers(hProgMan);
			IsOnDesktop=1;
		}
	} 
	else 
	{
		SetParent(pcli->hwndContactList,NULL);
		SetParentForContainers(NULL);
		IsOnDesktop=0;
	}

	//7- if it was visible - show
	if (storedVisMode) 
	{
		ShowWindowNew(pcli->hwndContactList,SW_SHOW);
		OnShowHide(pcli->hwndContactList,1);
	}
	if (!LayeredFlag)
	{
		HRGN hRgn1;
		RECT r;
		int v,h;
		int w=10;
		GetWindowRect(pcli->hwndContactList,&r);
		h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
		v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
		h=(h<v)?h:v;
		hRgn1=CreateRoundRectRgn(0,0,(r.right-r.left+1),(r.bottom-r.top+1),h,h);
		if ((DBGetContactSettingByte(NULL,"CLC","RoundCorners",0)) && (!CallService(MS_CLIST_DOCKINGISDOCKED,0,0)))
			SetWindowRgn(pcli->hwndContactList,hRgn1,1); 
		else SetWindowRgn(pcli->hwndContactList,NULL,1);
		RedrawWindow(pcli->hwndContactList,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME|RDW_UPDATENOW|RDW_ALLCHILDREN);   
	} 			
	IsInChangingMode=FALSE;
	firstTimeCallFlag=TRUE;
}

int UpdateTimer(BYTE BringIn)
{  
	if (BehindEdge_State==0)
	{
		KillTimer(pcli->hwndContactList,TM_BRINGOUTTIMEOUT);
		SetTimer(pcli->hwndContactList,TM_BRINGOUTTIMEOUT,BehindEdgeHideDelay*100,NULL);
	}
	if (show_event_started==0 && BehindEdge_State>0 ) 
	{
		KillTimer(pcli->hwndContactList,TM_BRINGINTIMEOUT);
		SetTimer(pcli->hwndContactList,TM_BRINGINTIMEOUT,BehindEdgeShowDelay*100,NULL);
		TRACE ("Bring in Timer re-started\n");
		show_event_started=1;
	}
	return 0;
}

int BehindEdge_Hide()
{
	int method=BehindEdgeSettings;
	if (method)
	{
		// if (DBGetContactSettingByte(NULL, "ModernData", "BehindEdge", 0)==0)
		{
			RECT rcScreen;
			RECT rcWindow;
			int bordersize=0;
			//Need to be moved out of screen
			show_event_started=0;
			//1. get work area rectangle 
			Docking_GetMonitorRectFromWindow(pcli->hwndContactList,&rcScreen);
			//SystemParametersInfo(SPI_GETWORKAREA,0,&rcScreen,FALSE);
			//2. move out
			bordersize=BehindEdgeBorderSize;
			GetWindowRect(pcli->hwndContactList,&rcWindow);
			switch (method)
			{
			case 1: //left
				rcWindow.left=rcScreen.left-(rcWindow.right-rcWindow.left)+bordersize;
				break;
			case 2: //right
				rcWindow.left=rcScreen.right-bordersize;
				break;
			}
			dock_prevent_moving=0;
			SetWindowPos(pcli->hwndContactList,NULL,rcWindow.left,rcWindow.top,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
			OnMoving(pcli->hwndContactList,&rcWindow);
			dock_prevent_moving=1;

			//3. store setting
			DBWriteContactSettingByte(NULL, "ModernData", "BehindEdge",method);
			BehindEdge_State=method;
			return 1;
		}
		return 2;
	}
	return 0;
}


extern int OnTrayRightClick;
int BehindEdge_Show()
{
	int method=BehindEdgeSettings;
	show_event_started=0;
	if (OnTrayRightClick) 
	{
		OnTrayRightClick=0;
		return 0;
	}
	if (method)// && (DBGetContactSettingByte(NULL, "ModernData", "BehindEdge", 0)==0))
	{
		RECT rcScreen;
		RECT rcWindow;
		int bordersize=0;
		//Need to be moved out of screen

		//1. get work area rectangle 
		//SystemParametersInfo(SPI_GETWORKAREA,0,&rcScreen,FALSE);
		Docking_GetMonitorRectFromWindow(pcli->hwndContactList,&rcScreen);
		//2. move out
		bordersize=BehindEdgeBorderSize;
		GetWindowRect(pcli->hwndContactList,&rcWindow);
		switch (method)
		{
		case 1: //left
			rcWindow.left=rcScreen.left;
			break;
		case 2: //right
			rcWindow.left=rcScreen.right-(rcWindow.right-rcWindow.left);
			break;
		}
		dock_prevent_moving=0;
		SetWindowPos(pcli->hwndContactList,NULL,rcWindow.left,rcWindow.top,0,0,SWP_NOZORDER|SWP_NOSIZE);
		OnMoving(pcli->hwndContactList,&rcWindow);
		dock_prevent_moving=1;

		//3. store setting
		DBWriteContactSettingByte(NULL, "ModernData", "BehindEdge",0);
		BehindEdge_State=0;
	}
	return 0;
}


BOOL skinInvalidateRect(HWND hWnd, CONST RECT* lpRect,BOOL bErase )
{

	if (IsInMainWindow(hWnd) && LayeredFlag)// && IsWindowVisible(hWnd))
	{
		if (IsWindowVisible(hWnd))
			return SkinInvalidateFrame(hWnd,lpRect,bErase);
		else
		{
			NeedToBeFullRepaint=1;
			return 0;
		}
	}
	else return InvalidateRect(hWnd,lpRect,bErase);
	return 1;
}

typedef struct tCheckFilling
{
	HDC hdc;
	RECT rect;
} sCheckFilling;

int FillAlphaChannel(HWND hwnd, HDC hdc, RECT * ParentRect, BYTE alpha)
{
	HRGN hrgn;
	RECT bndRect;
	//    HBITMAP hbitmap;
	RECT wrect;
	int res;
	//    BITMAP bmp;
	//    DWORD x,y;
	DWORD d;
	RGNDATA * rdata;
	DWORD rgnsz;
	RECT * rect;
	hrgn=CreateRectRgn(0,0,1,1);
	res=GetWindowRgn(hwnd,hrgn);     
	GetWindowRect(hwnd,&wrect);
	if (res==0)
	{
		DeleteObject(hrgn);
		hrgn=CreateRectRgn(wrect.left ,wrect.top ,wrect.right,wrect.bottom);
	}
	OffsetRgn(hrgn,-ParentRect->left,-ParentRect->top);
	res=GetRgnBox(hrgn,&bndRect);
	if (bndRect.bottom-bndRect.top*bndRect.right-bndRect.left ==0) return 0;
	rgnsz=GetRegionData(hrgn,0,NULL);
	rdata=(RGNDATA *) mir_alloc(rgnsz);
	GetRegionData(hrgn,rgnsz,rdata);
	rect=(RECT *)rdata->Buffer;
	for (d=0; d<rdata->rdh.nCount; d++)
	{
		FillRect255Alpha(hdc,&rect[d]);
	}

	mir_free(rdata);
	DeleteObject(hrgn);
	return 1;
}

BOOL CALLBACK EnumSubChildProc(HWND hwnd,LPARAM lParam)
{
	sCheckFilling * r=(sCheckFilling *)lParam;
	FillAlphaChannel(hwnd,r->hdc,&(r->rect),255);
	return 1;
}
BOOL CALLBACK EnumChildProc(HWND hwnd,LPARAM lParam)
{
	int ret;
	sCheckFilling * r=(sCheckFilling *)lParam;
	if (GetParent(hwnd)!=pcli->hwndContactList) return 1;
	ret=SendMessage(hwnd,WM_USER+100,0,0);
	switch (ret)
	{
	case 0:  //not respond full rect should be alpha=255
		{

			FillAlphaChannel(hwnd,r->hdc,&(r->rect),255);
			break;
		}
	case 1:
		EnumChildWindows(hwnd,EnumSubChildProc,(LPARAM)r);
		break;
	case 2:
		break;
	}
	return 1;
}


#define AC_SRC_NO_PREMULT_ALPHA     0x01
#define AC_SRC_NO_ALPHA             0x02
#define AC_DST_NO_PREMULT_ALPHA     0x10
#define AC_DST_NO_ALPHA             0x20

int IsInMainWindow(HWND hwnd)
{
	if (hwnd==pcli->hwndContactList) return 1;
	if (GetParent(hwnd)==pcli->hwndContactList) return 2;
	return 0;
}

//int SkinUpdateWindowProc(HWND hwnd1)
//{
//   
//    /*
//    BYTE alpha=255;
//    struct sUpdatingWindow Update2={0};
//    UpdatePendent=333;
//    if (ANIMATION_IS_IN_PROGRESS)
//    {
//        TRACE("UPDATING FOR ANIMATED\n");
//        UpdateWindowImage(&Update);
//        return 0;
//    }
//    TRACE("PREPEARING FOR NON-ANIMATED\n");
//    if (!PrepeareWindowImage(&Update2)) return 0; 
//    UpdateWindowImage(&Update2);
//    if (DBGetContactSettingByte(NULL,"CList","Transparent",0))
//        {   
//            HWND fg;
//            HWND hwnd=pcli->hwndContactList;
//            int f=0;
//            fg=GetForegroundWindow();
//            if (fg==hwnd || GetParent(fg)==hwnd || GetParent(GetParent(fg))==hwnd) f=1;
//            alpha=(f==1)?(BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT):(BYTE)DBGetContactSettingByte(NULL,"CList","AutoAlpha",SETTING_ALPHA_DEFAULT);
//        }
//    UpdateWindowWithImage(&Update2, alpha);
//    FreeWindowImage(&Update2);
//    return 0;
//    */
//}
int PrepeareWindowImage(struct sUpdatingWindow * sUpdate)
{
	HDC hdc,dcScreen;
	HBITMAP oldbmp,bmp;
	//    HFONT hfo,hf;
	LONG cx,cy;
	SIZE sz;
	HWND hwnd;
	//    DWORD ws;
	//    HDC hd2;
	RECT rect,r1;
	DWORD UpdateTime=GetTickCount();
	POINT dest={0}, src={0};
	if (!sUpdate) return 0;
	TRACE("---------------- PrepeareWindowImage ==============\n");

	hwnd=pcli->hwndContactList;

	GetWindowRect(hwnd,&rect);
	dest.x=rect.left;
	dest.y=rect.top;
	cx=rect.right-rect.left;//LOWORD(lParam);
	cy=rect.bottom-rect.top;//HIWORD(lParam);
	r1=rect;
	OffsetRect(&r1,-r1.left,-r1.top);
	sz.cx=cx; sz.cy=cy;
	dcScreen=GetDC(hwnd);
	hdc=CreateCompatibleDC(dcScreen);
	bmp=CreateBitmap32(cx,cy);
	oldbmp=SelectObject(hdc,bmp);   
	sUpdate->currentBitmap=bmp;
	sUpdate->oldBitmap=oldbmp;
	sUpdate->offscreenDC=hdc;
	sUpdate->screenDC=dcScreen; 
	sUpdate->sizeX=cx;
	sUpdate->sizeY=cy;
	return 0;
}
//int UpdateWindowImage(struct sUpdatingWindow * sUpdate)
//    {
//        HDC hdc=sUpdate->offscreenDC;
//        HDC dcScreen=sUpdate->screenDC;
//        HBITMAP bmp=sUpdate->currentBitmap;
//        HFONT hfo,hf;
//        LONG cx,cy;
// //     SIZE sz;
//        HWND hwnd;
//        DWORD ws;
// //     HDC hd2;
//        RECT rect,r1;
//        DWORD UpdateTime=GetTickCount();
//        POINT dest={0}, src={0};
//        if (!sUpdate) return 0;
//        TRACE("---------------- Update Window Image ==============\n");
//
//        hwnd=pcli->hwndContactList;
//
//        GetWindowRect(hwnd,&rect);
//        dest.x=rect.left;
//        dest.y=rect.top;
//        cx=rect.right-rect.left;
//        cy=rect.bottom-rect.top;
//        if (cx!=sUpdate->sizeX || cy!=sUpdate->sizeY)
//        {
//            FreeWindowImage(sUpdate);
//            PrepeareWindowImage(sUpdate);
//            hdc=sUpdate->offscreenDC=hdc;
//            dcScreen=sUpdate->screenDC;
//            bmp=sUpdate->currentBitmap;
//            cx=rect.right-rect.left;
//            cy=rect.bottom-rect.top;
//
//        }      
//        r1=rect;
//        OffsetRect(&r1,-r1.left,-r1.top);
//        r1.right=r1.left+cx;
//        r1.bottom=r1.top+cy;
////        sz.cx=cx; sz.cy=cy;
//        SkinDrawWindowBack(hwnd,hdc,&r1,"Main Window/Backgrnd");
//        ws=GetWindowLong(hwnd,GWL_STYLE);
//        //Draw scroll bars
//        {
//            SCROLLBARINFO si;
//            HWND contacts;
//            contacts=(HWND)CallService(MS_CLUI_GETHWNDTREE,0,0);
//            si.cbSize=sizeof(SCROLLBARINFO);
//
//            if (GetWindowLong(contacts,GWL_STYLE)&WS_VSCROLL)   
//            {
//                RECT rc;
//                RECT wrc;
//                RECT tb;
//                GetScrollBarInfo(contacts,OBJID_VSCROLL,&si);
//                GetWindowRect(hwnd,&wrc);
//                rc=si.rcScrollBar;
//                OffsetRect(&rc,-dest.x,-dest.y);
//                tb=rc;
//                
//                SkinDrawGlyph(hdc,&rc,&rc,"Main Window/ScrollBar/Backgrnd");
//                
//                rc.bottom=si.xyThumbBottom+tb.top;
//                rc.top=si.xyThumbTop+tb.top;
//                SkinDrawGlyph(hdc,&rc,&rc,"Main Window/ScrollBar/Thumb");
//                rc.top=tb.top;
//                rc.bottom=tb.top+si.dxyLineButton;
//                SkinDrawGlyph(hdc,&rc,&rc,"Main Window/ScrollBar/UpButton");
//
//                rc.top=tb.bottom-si.dxyLineButton;
//                rc.bottom=tb.bottom;
//                SkinDrawGlyph(hdc,&rc,&rc,"Main Window/ScrollBar/DownButton");
//
//            }
//            if (ws&WS_CAPTION)
//            {
//                TITLEBARINFO ti;
//                RECT r;
//                ti.cbSize=sizeof(TITLEBARINFO);
//                GetTitleBarInfo(hwnd,&ti);
//                r=ti.rcTitleBar;
//                OffsetRect(&r,-dest.x,-dest.y);
//                if (ws&WS_SYSMENU)
//                {
//                    MENUBARINFO mbi;
//                    RECT rc,rc1;
//                    mbi.cbSize =sizeof(MENUBARINFO);
//                    GetMenuBarInfo(hwnd, OBJID_SYSMENU,0,&mbi);
//                    rc=mbi.rcBar;
//                    if (rc.bottom-rc.top*rc.right-rc.left!=0)
//                    {
//                        OffsetRect(&rc,-dest.x,-dest.y);
//                        rc1=rc;
//                        rc1.right=r.right;
//                        SkinDrawGlyph(hdc,&rc1,&rc1,"Main Window/Caption/Backgrnd");
//                    }
//                    else SkinDrawGlyph(hdc,&r,&r,"Main Window/Caption/Backgrnd");
//                }
//                else
//                    SkinDrawGlyph(hdc,&r,&r,"Main Window/Caption/Backgrnd");
//                {
//                    RECT r2;
//                    WORD sd;
//                    sd=r.bottom-r.top-2;
//                    r2.top=r.top+1;
//                    r2.bottom=r2.top+sd;
//                    r2.right=r.right;
//                    r2.left=r2.right-sd;
//                               
//                    if (ti.rgstate[5]==0 ||ti.rgstate[5]&STATE_SYSTEM_PRESSED)
//                    {
//                        if (GetWindowLong(hwnd,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)
//                            SkinDrawGlyph(hdc,&r2,&r2,"Main Window/Caption/MinimizeButton");
//                        else
//                        {
//                            SkinDrawGlyph(hdc,&r2,&r2,"Main Window/Caption/CloseButton");
//                            r2.right-=(sd)*2+1;
//                            r2.left=r2.right-sd;
//                            SkinDrawGlyph(hdc,&r2,&r2,"Main Window/Caption/MinimizeButton");
//                        }
//                    }
//                }
//            }
//        }
//        SkinDrawMenu(hwnd,hdc);
//        SendMessage(hwnd,WM_PRINT,(WPARAM)hdc,PRF_CHECKVISIBLE|PRF_CLIENT|PRF_CHILDREN);
//
//        {
//            sCheckFilling r;
//            r.hdc=hdc;
//            r.rect=rect;
//    //       EnumChildWindows(hwnd,EnumChildProc,(LPARAM)&r);
//        }
//    return 1;
//    }
//int UpdateWindowWithImage(struct sUpdatingWindow * sUpdate, BYTE constantAlpha)
//{
//    int res;
//    BLENDFUNCTION bf={AC_SRC_OVER, 0,constantAlpha, AC_SRC_ALPHA };
//    POINT dest={0}, src={0};
//    RECT rect;
//    SIZE sz={0};
//    if (!pcli->hwndContactList) return 0;
//    GetWindowRect(pcli->hwndContactList,&rect);
//    dest.x=rect.left;
//    dest.y=rect.top;
//    sz.cx=rect.right-rect.left;
//    sz.cy=rect.bottom-rect.top;
//    if (!(GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE)&WS_EX_LAYERED))
//        SetWindowLong(pcli->hwndContactList,GWL_EXSTYLE, GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE) |WS_EX_LAYERED);
//    {
//        char b[100];
//        sprintf(b,"Try to set alpha %d",constantAlpha);
//        TRACE(b);
//    }
//    SetAlpha(constantAlpha);
//    res=UpdateLayeredWindow(pcli->hwndContactList,sUpdate->screenDC,&dest,&sz,sUpdate->offscreenDC,&src,0,&bf,ULW_ALPHA);
//    if (!res&&0)
//    {
//
//        char szBuf[80]; 
//        LPVOID lpMsgBuf;
//        DWORD dw = GetLastError(); 
//
//
//        FormatMessage(
//            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
//            FORMAT_MESSAGE_FROM_SYSTEM,
//            NULL,
//            dw,
//            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//            (LPTSTR) &lpMsgBuf,
//            0, NULL );
//
//        sprintf(szBuf,  "UPDATE LAYERED WINDOW  failed with error %d: %s\n",  dw, lpMsgBuf); 
//
//        TRACE(szBuf);
//        MessageBoxA(NULL, szBuf, "UPDATE LAYERED WINDOW FAILURE", MB_OK); 
//        DebugBreak();
//
//    }
//
//    else 
//    {
//        TRACE("---Success\n");
//        alphaCurr=constantAlpha;
//    }
//    return 1;
//}
//
int FreeWindowImage(struct sUpdatingWindow * sUpdate)
{   
	TRACE("--- Free update image--------\n"); 
	SelectObject(sUpdate->offscreenDC,sUpdate->oldBitmap);
	DeleteDC(sUpdate->offscreenDC);
	ReleaseDC(pcli->hwndContactList,sUpdate->screenDC);
	DeleteObject(sUpdate->currentBitmap);

	return 0;
}

/*
int SkinUpdateWindow (HWND hwnd, HWND Caller)
{
if (IsWindowVisible(pcli->hwndContactList) )
if (UpdatePendent!=2) 
{
if (ANIMATION_IS_IN_PROGRESS)
{
UpdateWindowImage(&Update);
return 0;
}
if (PostMessage(pcli->hwndContactList,UM_UPDATE,(WPARAM)hwnd,(LPARAM)Caller)) 
{       

TRACE ("UPDATE WAS POSTED\n");          		
UpdatePendent=2;
}
else 
{
char szBuf[80]; 
LPVOID lpMsgBuf;
DWORD dw = GetLastError(); 

FormatMessage(
FORMAT_MESSAGE_ALLOCATE_BUFFER | 
FORMAT_MESSAGE_FROM_SYSTEM,
NULL,
dw,
MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
(LPTSTR) &lpMsgBuf,
0, NULL );

sprintf(szBuf,  "failed with error %d: %s",  dw, lpMsgBuf); 

MessageBoxA(NULL, szBuf, "POST MESSAGE FAILURE", MB_OK); 
DebugBreak();
}

}
return 0; 
}

*/

//void RegisterCLCRowObjects(int mode)
//{
//    TRACE("REGISTERING CL OBJECTS\n");
//    if (mode==1)
//    {  
//        //NORMAL
//        {
//            CreateGlyphedObject("CL/Group/Opened");
//            CreateGlyphedObject("CL/Group/Opened/FirstInList");
//            CreateGlyphedObject("CL/Group/Opened/SingleInList");
//            CreateGlyphedObject("CL/Group/Opened/LastInParent");
//
//            CreateGlyphedObject("CL/Group/Closed");
//            CreateGlyphedObject("CL/Group/Closed/FirstInList");
//            CreateGlyphedObject("CL/Group/Closed/SingleInList");
//            CreateGlyphedObject("CL/Group/Closed/LastInParent");
//
//            CreateGlyphedObject("CL/SubGroup/Opened");
//            CreateGlyphedObject("CL/SubGroup/Opened/LastInParent");
//
//            CreateGlyphedObject("CL/SubGroup/Closed");
//            CreateGlyphedObject("CL/SubGroup/Closed/LastInParent");
//
//            CreateGlyphedObject("CL/Contact/InRoot/First");
//            CreateGlyphedObject("CL/Contact/InRoot/Mid");
//            CreateGlyphedObject("CL/Contact/InRoot/Last");
//            CreateGlyphedObject("CL/Contact/InRoot/Single");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/First");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Mid");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Last");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Single");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/First");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Mid");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Last");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Single");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/First");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Mid");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Last");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Single");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Single");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Last");
//
//            CreateGlyphedObject("CL/Contact/InGroup/First");
//            CreateGlyphedObject("CL/Contact/InGroup/Mid");
//            CreateGlyphedObject("CL/Contact/InGroup/Last");
//            CreateGlyphedObject("CL/Contact/InGroup/Single");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/First");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Mid");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Last");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Single");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/First");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Mid");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Last");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Single");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/First");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Mid");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Last");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Single");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Single");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Last");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/First");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Mid");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Last");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Single");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/First");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Mid");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Last");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Single");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/First");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Mid");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Last");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Single");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/First");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Mid");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Last");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Single");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Single");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Last");
//        }
//        //Selected
//        {
//            CreateGlyphedObject("CL/Group/Opened/Selected");
//            CreateGlyphedObject("CL/Group/Opened/FirstInList/Selected");
//            CreateGlyphedObject("CL/Group/Opened/SingleInList/Selected");
//            CreateGlyphedObject("CL/Group/Opened/LastInParent/Selected");
//
//            CreateGlyphedObject("CL/Group/Closed/Selected");
//            CreateGlyphedObject("CL/Group/Closed/FirstInList/Selected");
//            CreateGlyphedObject("CL/Group/Closed/SingleInList/Selected");
//            CreateGlyphedObject("CL/Group/Closed/LastInParent/Selected");
//
//            CreateGlyphedObject("CL/SubGroup/Opened/Selected");
//            CreateGlyphedObject("CL/SubGroup/Opened/LastInParent/Selected");
//
//            CreateGlyphedObject("CL/SubGroup/Closed/Selected");
//            CreateGlyphedObject("CL/SubGroup/Closed/LastInParent/Selected");
//
//
//            CreateGlyphedObject("CL/Contact/InRoot/First/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/First/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/First/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/First/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Single/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Single/Selected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Last/Selected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/First/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/First/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/First/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/First/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Single/Selected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Last/Selected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/First/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/First/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/First/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/First/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Mid/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Last/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Single/Selected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Single/Selected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Last/Selected");
//        }
//        //Hovered
//        {
//            CreateGlyphedObject("CL/Group/Opened/Hovered");
//            CreateGlyphedObject("CL/Group/Opened/FirstInList/Hovered");
//            CreateGlyphedObject("CL/Group/Opened/SingleInList/Hovered");
//            CreateGlyphedObject("CL/Group/Opened/LastInParent/Hovered");
//
//            CreateGlyphedObject("CL/Group/Closed/Hovered");
//            CreateGlyphedObject("CL/Group/Closed/FirstInList/Hovered");
//            CreateGlyphedObject("CL/Group/Closed/SingleInList/Hovered");
//            CreateGlyphedObject("CL/Group/Closed/LastInParent/Hovered");
//
//            CreateGlyphedObject("CL/SubGroup/Opened/Hovered");
//            CreateGlyphedObject("CL/SubGroup/Opened/LastInParent/Hovered");
//
//            CreateGlyphedObject("CL/SubGroup/Closed/Hovered");
//            CreateGlyphedObject("CL/SubGroup/Closed/LastInParent/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InRoot/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Single/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Single/Hovered");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Last/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InGroup/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Single/Hovered");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Last/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/First/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Mid/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Last/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Single/Hovered");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Single/Hovered");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Last/Hovered");
//
//        }
//        //HoverOnSelected
//        {
//            CreateGlyphedObject("CL/Group/Opened/HoverOnSelected");
//            CreateGlyphedObject("CL/Group/Opened/FirstInList/HoverOnSelected");
//            CreateGlyphedObject("CL/Group/Opened/SingleInList/HoverOnSelected");
//            CreateGlyphedObject("CL/Group/Opened/LastInParent/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Group/Closed/HoverOnSelected");
//            CreateGlyphedObject("CL/Group/Closed/FirstInList/HoverOnSelected");
//            CreateGlyphedObject("CL/Group/Closed/SingleInList/HoverOnSelected");
//            CreateGlyphedObject("CL/Group/Closed/LastInParent/HoverOnSelected");
//
//            CreateGlyphedObject("CL/SubGroup/Opened/HoverOnSelected");
//            CreateGlyphedObject("CL/SubGroup/Opened/LastInParent/HoverOnSelected");
//
//            CreateGlyphedObject("CL/SubGroup/Closed/HoverOnSelected");
//            CreateGlyphedObject("CL/SubGroup/Closed/LastInParent/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InRoot/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Meta/Expanded/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/Single/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Single/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InRoot/Subcontact/LastInGroup/Last/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Meta/Expanded/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Single/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InGroup/Subcontact/LastInGroup/Last/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Meta/Expanded/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/First/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Mid/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Last/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/Single/HoverOnSelected");
//
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Single/HoverOnSelected");
//            CreateGlyphedObject("CL/Contact/InSubgroup/Subcontact/LastInGroup/Last/HoverOnSelected");
//        }
//    }
//}
//
#include "mod_skin_selector.h"
extern void LoadSkinFromDB(void);
int OnSkinLoad(WPARAM wParam, LPARAM lParam)
{

	TRACE("OnSkinLoad\n");
	LoadSkinFromDB();

	//    CreateGlyphedObject("Main Window/ScrollBar Up Button");
	//    CreateGlyphedObject("Main Window/ScrollBar Down Button");
	//    CreateGlyphedObject("Main Window/ScrollBar Thumb");       
	//    CreateGlyphedObject("Main Window/Minimize Button");
	//    CreateGlyphedObject("Main Window/Frame Backgrnd");
	//    CreateGlyphedObject("Main Window/Frame Titles Backgrnd");
	//    CreateGlyphedObject("Main Window/Backgrnd");

	//RegisterCLCRowObjects(1);
	//    {
	//        int i=-1;
	//        i=AddStrModernMaskToList("CL%status=Online%type=2%even=*even*","Main Window/Backgrnd",MainModernMaskList,NULL);
	//        i=AddStrModernMaskToList("CL%status=Online%type=1%even=*even*","Main Window/Frame Titles Backgrnd",MainModernMaskList,NULL);
	//   }


	return 0;
}

static int CluiModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize=MENUITEMINFO_V4_SIZE;
	mii.fMask=MIIM_SUBMENU;
	mii.hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	SetMenuItemInfo(hMenuMain,0,TRUE,&mii);
	mii.hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	SetMenuItemInfo(hMenuMain,1,TRUE,&mii);

	CheckProtocolOrder();
	CluiProtocolStatusChanged(0,0);
	SleepEx(0,TRUE);
	OnModulesLoadedCalled=TRUE;	
	///pcli->pfnInvalidateDisplayNameCacheEntry(INVALID_HANDLE_VALUE);   
	SendMessage(pcli->hwndContactList,M_CREATECLC,0,0); //$$$
	return 0;
}

pProtoTicks GetProtoTicksByProto(char * szProto)
{
	int i;

	for (i=0;i<64;i++)
	{
		if (CycleStartTick[i].szProto==NULL) break;
		if (MyStrCmp(CycleStartTick[i].szProto,szProto)) continue;
		return(&CycleStartTick[i]);
	}
	for (i=0;i<64;i++)
	{
		if (CycleStartTick[i].szProto==NULL)
		{
			CycleStartTick[i].szProto=mir_strdup(szProto);
			CycleStartTick[i].CycleStartTick=0;
			CycleStartTick[i].n=i;
			return(&CycleStartTick[i]);
		}
	}
	return (NULL);
}

int GetConnectingIconForProtoCount(char *szProto)
{
	char file[MAX_PATH],fileFull[MAX_PATH],szFullPath[MAX_PATH];
	char szPath[MAX_PATH];
	char *str;
	int ret;

	GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
	str=strrchr(szPath,'\\');
	if(str!=NULL) *str=0;
	_snprintf(szFullPath, sizeof(szFullPath), "%s\\Icons\\proto_conn_%s.dll", szPath, szProto);

	lstrcpynA(file,szFullPath,sizeof(file));
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)file, (LPARAM)fileFull);
	ret=ExtractIconExA(fileFull,-1,NULL,NULL,1);
	if (ret==0) ret=11;
	return ret;

}

static HICON ExtractIconFromPath(const char *path)
{
	char *comma;
	char file[MAX_PATH],fileFull[MAX_PATH];
	int n;
	HICON hIcon;
	/*	{
	char buf[512];
	sprintf(buf,"LoadIcon %s\r\n",path);
	TRACE(buf);
	}
	*/
	lstrcpynA(file,path,sizeof(file));
	comma=strrchr(file,',');
	if(comma==NULL) n=0;
	else {n=atoi(comma+1); *comma=0;}
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)file, (LPARAM)fileFull);
	/*	
	#ifdef _DEBUG
	{
	char buf[512];
	sprintf(buf,"LoadIconFull %d %s\r\n",n,fileFull);
	TRACE(buf);
	}
	#endif
	*/
	hIcon=NULL;
	ExtractIconExA(fileFull,n,NULL,&hIcon,1);
	return hIcon;
}

HICON LoadIconFromExternalFile(char *filename,int i,boolean UseLibrary,boolean registerit,char *IconName,char *SectName,char *Description,int internalidx)
{
	char szPath[MAX_PATH],szMyPath[MAX_PATH], szFullPath[MAX_PATH],*str;
	HICON hIcon=NULL;
	SKINICONDESC sid;

	GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
	GetModuleFileNameA(g_hInst, szMyPath, MAX_PATH);
	str=strrchr(szPath,'\\');
	if(str!=NULL) *str=0;
	_snprintf(szFullPath, sizeof(szFullPath), "%s\\Icons\\%s,%d", szPath, filename, i);

	if (!UseLibrary||!ServiceExists(MS_SKIN2_ADDICON))
	{		
		hIcon=ExtractIconFromPath(szFullPath);
		if (hIcon) return hIcon;
	}
	else
	{
		if (registerit&&IconName!=NULL&&SectName!=NULL)	
		{
			sid.cbSize = sizeof(sid);
			sid.pszSection = Translate(SectName);				
			sid.pszName=IconName;
			sid.pszDescription=Description;
			sid.pszDefaultFile=szMyPath;
			sid.iDefaultIndex=internalidx;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}
		return ((HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IconName));
	}




	return (HICON)0;
}

HICON GetConnectingIconForProto(char *szProto,int b)
{
	char szFullPath[MAX_PATH];
	HICON hIcon=NULL;

	b=b-1;
	_snprintf(szFullPath, sizeof(szFullPath), "proto_conn_%s.dll",szProto);
	hIcon=LoadIconFromExternalFile(szFullPath,b+1,FALSE,FALSE,NULL,NULL,NULL,0);
	if (hIcon) return hIcon;

	/*#ifdef _DEBUG
	{
	char buf [256];
	sprintf(buf,"IconNotFound %s %d\r\n",szProto,b);
	if (!MyStrCmp(szProto,"MSN"))
	TRACE(buf);
	}
	#endif
	*/
	//if (!MyStrCmp(szProto,"ICQ"))
	{
		hIcon=(LoadIconA(g_hInst,(char *)(IDI_ICQC1+b+1)));
	}

	return(hIcon);
}



//wParam = szProto
int GetConnectingIconService(WPARAM wParam,LPARAM lParam)
{
	int b;						
	ProtoTicks *pt=NULL;
	HICON hIcon=NULL;

	char *szProto=(char *)wParam;
	if (!szProto) return 0;

	pt=GetProtoTicksByProto(szProto);

	if (pt!=NULL)
	{
		if (pt->CycleStartTick==0)
		{
			CreateTimerForConnectingIcon(ID_STATUS_CONNECTING,wParam);
			pt=GetProtoTicksByProto(szProto);
		}
	}
	if (pt!=NULL)
	{
		if (pt->CycleStartTick!=0&&pt->IconsCount!=0) 
		{					
			b=((GetTickCount()-pt->CycleStartTick)/(DefaultStep))%(pt->IconsCount);
			hIcon=GetConnectingIconForProto(szProto,b);
		};
	}


	return (int)hIcon;
};


int CreateTimerForConnectingIcon(WPARAM wParam,LPARAM lParam)
{

	int status=(int)wParam;
	char *szProto=(char *)lParam;					
	if (!szProto) return (0);
	if (!status) return (0);

	if ((DBGetContactSettingByte(NULL,"CLUI","UseConnectingIcon",1)==1)&&status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES)
	{

		ProtoTicks *pt=NULL;
		int cnt;

		pt=GetProtoTicksByProto(szProto);
		if (pt!=NULL)
		{


			if (pt->CycleStartTick==0) 
			{					
				//	sprintf(buf,"SetTimer %d\r\n",pt->n);
				//	TRACE(buf);

				KillTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->n);
				cnt=GetConnectingIconForProtoCount(szProto);
				if (cnt!=0)
				{
					DefaultStep=DBGetContactSettingWord(NULL,"CLUI","DefaultStepConnectingIcon",100);
					pt->IconsCount=cnt;
					SetTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->n,(int)(DefaultStep)/1,0);
					pt->TimerCreated=1;
					pt->CycleStartTick=GetTickCount();
				}

			};
		};
	}
	return 0;
}
// Restore protocols to the last global status.
// Used to reconnect on restore after standby.
static void RestoreMode()
{

	int nStatus;


	nStatus = DBGetContactSettingWord(NULL, "CList", "Status", ID_STATUS_OFFLINE);
	if (nStatus != ID_STATUS_OFFLINE)
	{
		PostMessage(pcli->hwndContactList, WM_COMMAND, nStatus, 0);
	}

}

int ReloadCLUIOptions()
{
	KillTimer(pcli->hwndContactList,TM_UPDATEBRINGTIMER);
	BehindEdgeSettings=DBGetContactSettingByte(NULL, "ModernData", "HideBehind", 0);
	BehindEdgeShowDelay=DBGetContactSettingWord(NULL,"ModernData","ShowDelay",3);
	BehindEdgeHideDelay=DBGetContactSettingWord(NULL,"ModernData","HideDelay",3);
	BehindEdgeBorderSize=DBGetContactSettingWord(NULL,"ModernData","HideBehindBorderSize",1);


	//   if (BehindEdgeSettings)  SetTimer(pcli->hwndContactList,TM_UPDATEBRINGTIMER,250,NULL);
	return 0;
}

int OnSettingChanging(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING *)lParam;

	if (wParam==0)
	{
		if ((dbcws->value.type==DBVT_BYTE)&&!MyStrCmp(dbcws->szModule,"CLUI"))
		{
			if (!MyStrCmp(dbcws->szSetting,"SBarShow"))
			{	
				showOpts=dbcws->value.bVal;	
				return(0);
			};
		}
	}
	else
	{		
		if (dbcws==NULL){return(0);};

		if (dbcws->value.type==DBVT_ASCIIZ&&(!MyStrCmp(dbcws->szSetting,"e-mail") || !MyStrCmp(dbcws->szSetting,"Mye-mail0")))
		{
			SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
			return(0);
		};
		if (dbcws->value.type==DBVT_ASCIIZ&&!MyStrCmp(dbcws->szSetting,"Cellular"))
		{		
			SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
			return(0);
		};

		if (dbcws->value.type==DBVT_ASCIIZ&&!MyStrCmp(dbcws->szModule,"UserInfo"))
		{
			if (!MyStrCmp(dbcws->szSetting,(HANDLE)"MyPhone0"))
			{		
				SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
				return(0);
			};
			if (!MyStrCmp(dbcws->szSetting,(HANDLE)"Mye-mail0"))
			{	
				SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);	
				return(0);
			};
		};
	}
	return(0);
};


// Disconnect all protocols.
// Happens on shutdown and standby.
static void DisconnectAll()
{

	PROTOCOLDESCRIPTOR** ppProtoDesc;
	int nProtoCount;
	int nProto;


	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&nProtoCount, (LPARAM)&ppProtoDesc);
	for (nProto = 0; nProto < nProtoCount; nProto++)
	{
		if (ppProtoDesc[nProto]->type == PROTOTYPE_PROTOCOL)
			CallProtoService(ppProtoDesc[nProto]->szName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
	}

}

int PreCreateCLC(HWND parent)
{
	pcli->hwndContactTree=CreateWindow(CLISTCONTROL_CLASS,TEXT(""),
		WS_CHILD|WS_CLIPCHILDREN|CLS_CONTACTLIST
		|(DBGetContactSettingByte(NULL,"CList","UseGroups",SETTING_USEGROUPS_DEFAULT)?CLS_USEGROUPS:0)
		//|CLS_HIDEOFFLINE
		|(DBGetContactSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT)?CLS_HIDEOFFLINE:0)
		|(DBGetContactSettingByte(NULL,"CList","HideEmptyGroups",SETTING_HIDEEMPTYGROUPS_DEFAULT)?CLS_HIDEEMPTYGROUPS:0
		|CLS_MULTICOLUMN
		//|DBGetContactSettingByte(NULL,"CLUI","ExtraIconsAlignToLeft",1)?CLS_EX_MULTICOLUMNALIGNLEFT:0
		),
		0,0,0,0,parent,NULL,g_hInst,NULL);
	//   SetWindowLong(pcli->hwndContactTree, GWL_EXSTYLE,GetWindowLong(pcli->hwndContactTree, GWL_EXSTYLE) | WS_EX_TRANSPARENT);


	return((int)pcli->hwndContactTree);
};

int CreateCLC(HWND parent)
{

	SleepEx(0,TRUE);
	{	
		//  char * s;
		// create contact list frame
		CLISTFrame Frame;
		memset(&Frame,0,sizeof(Frame));
		Frame.cbSize=sizeof(CLISTFrame);
		Frame.hWnd=pcli->hwndContactTree;
		Frame.align=alClient;
		Frame.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		//LoadIcon(hInst,MAKEINTRESOURCEA(IDI_MIRANDA));
		Frame.Flags=F_VISIBLE|F_SHOWTB|F_SHOWTBTIP|F_NO_SUBCONTAINER;
		Frame.name=Translate("My Contacts");
		hFrameContactTree=(HWND)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,(LPARAM)0);
		CallService(MS_SKINENG_REGISTERPAINTSUB,(WPARAM)Frame.hWnd,(LPARAM)ClcPaintCallbackProc);

		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBTIPNAME,hFrameContactTree),(LPARAM)Translate("My Contacts"));	
	}

	ReloadExtraIcons();
	{
		lastreqh=0;
		{
			CallService(MS_CLIST_SETHIDEOFFLINE,(WPARAM)oldhideoffline,0);
		}

		{	int state=DBGetContactSettingByte(NULL,"CList","State",SETTING_STATE_NORMAL);
		//--if(state==SETTING_STATE_NORMAL) ShowWindowNew(pcli->hwndContactList, SW_SHOW);
		//--else if(state==SETTING_STATE_MINIMIZED) ShowWindowNew(pcli->hwndContactList, SW_SHOWMINIMIZED);
		}


		lastreqh=0;
		disableautoupd=0;

	}  

	hSettingChangedHook=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,OnSettingChanging);
	return(0);
};


//int GetStatsuBarProtoRect(HWND hwnd,char *szProto,RECT *rc)
//{
//  int nParts,nPanel;
//  ProtocolData *PD;
//  //	int startoffset=DBGetContactSettingDword(NULL,"StatusBar","FirstIconOffset",0);
//
//  //	if (!UseOwnerDrawStatusBar) startoffset=0;
//
//  nParts=SendMessage(hwnd,SB_GETPARTS,0,0);
//  FillMemory(rc,sizeof(RECT),0);
//
//  for (nPanel=0;nPanel<nParts;nPanel++)
//  {
//    PD=(ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,(LPARAM)0);
//    if(PD==NULL){
//      return(0);
//    };
//
//
//    if (!MyStrCmp(szProto,PD->RealName))
//    {
//      SendMessage(hwnd,SB_GETRECT,(WPARAM)nPanel,(LPARAM)rc);
//      //			rc->left+=startoffset;
//      //			rc->right+=startoffset;
//      return(0);
//    };
//  };
//  return (0);
//};


int MirMenuState=0;
int StatusMenuState=0;

int SkinDrawMenu(HWND hwnd, HDC hdc)
{
	/*  RECT ra,r1;
	HFONT hfo;
	int fontHeight;
	struct ClcData * dat;
	dat=(struct ClcData*)GetWindowLong(pcli->hwndContactTree,0);
	if (!dat) return 1;
	hfo=GetCurrentObject(hdc,OBJ_FONT);
	ChangeToFont(hdc,dat,FONTID_MENUTEXT,&fontHeight);

	GetWindowRect(hwnd,&ra);
	{
	MENUBARINFO mbi;
	HMENU hmenu;
	mbi.cbSize=sizeof(MENUBARINFO);
	GetMenuBarInfo(hwnd,OBJID_MENU, 0, &mbi);
	if (!(mbi.rcBar.right-mbi.rcBar.left>0 && mbi.rcBar.bottom-mbi.rcBar.top>0)) return 1;
	r1=mbi.rcBar;  
	OffsetRect(&r1,-ra.left,-ra.top);
	SkinDrawGlyph(hdc,&r1,&r1,"Main Window/Menu Bar");
	GetMenuBarInfo(hwnd,OBJID_MENU, 1, &mbi);
	r1=mbi.rcBar;  
	OffsetRect(&r1,-ra.left,-ra.top);
	{
	if (MirMenuState&(ODS_SELECTED|ODS_HOTLIGHT))
	SkinDrawGlyph(hdc,&r1,&r1,"Main Window/Menu Bar/Main Menu/Selected");
	else               
	SkinDrawGlyph(hdc,&r1,&r1,"Main Window/Menu Bar/Main Menu/Normal");
	DrawIconExS(hdc,(r1.right+r1.left-GetSystemMetrics(SM_CXSMICON))/2+((MirMenuState&(ODS_SELECTED|ODS_HOTLIGHT))?1:0),(r1.bottom+r1.top-GetSystemMetrics(SM_CYSMICON))/2+((MirMenuState&(ODS_SELECTED|ODS_HOTLIGHT))?1:0),LoadSkinnedIcon(SKINICON_OTHER_MIRANDA),GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
	}
	GetMenuBarInfo(hwnd,OBJID_MENU, 2, &mbi);
	r1=mbi.rcBar;  
	OffsetRect(&r1,-ra.left,-ra.top);
	{
	if (StatusMenuState&(ODS_SELECTED|ODS_HOTLIGHT))
	SkinDrawGlyph(hdc,&r1,&r1,"Main Window/Menu Bar/Status Menu/Selected");
	else               
	SkinDrawGlyph(hdc,&r1,&r1,"Main Window/Menu Bar/Status Menu/Normal");
	SetTextColor(hdc,StatusMenuState&(ODS_SELECTED|ODS_HOTLIGHT)?dat->MenuTextHiColor:dat->MenuTextColor);
	DrawTextS(hdc,Translate("Status"),-1,&r1,DT_VCENTER|DT_CENTER|DT_SINGLELINE);
	}
	SelectObject(hdc,hfo);
	}

	*/
	return 0;
}
int DrawMenuBackGround(HWND hwnd, HDC hdc, int item)
{
	RECT ra,r1;
	//    HBRUSH hbr;
	HRGN treg,treg2;
	struct ClcData * dat;

	dat=(struct ClcData*)GetWindowLong(pcli->hwndContactTree,0);
	if (!dat) return 1;
	GetWindowRect(hwnd,&ra);
	{
		MENUBARINFO mbi;
		mbi.cbSize=sizeof(MENUBARINFO);
		GetMenuBarInfo(hwnd,OBJID_MENU, 0, &mbi);
		if (!(mbi.rcBar.right-mbi.rcBar.left>0 && mbi.rcBar.bottom-mbi.rcBar.top>0)) return 1;
		r1=mbi.rcBar;  
		r1.bottom+= !DBGetContactSettingByte(NULL,"CLUI","LineUnderMenu",0);
		if (item<1)
		{           
			treg=CreateRectRgn(mbi.rcBar.left,mbi.rcBar.top,mbi.rcBar.right,r1.bottom);
			if (item==0)  //should remove item clips
			{
				int t;
				for (t=1; t<=2; t++)
				{
					GetMenuBarInfo(hwnd,OBJID_MENU, t, &mbi);
					treg2=CreateRectRgn(mbi.rcBar.left,mbi.rcBar.top,mbi.rcBar.right,mbi.rcBar.bottom);
					CombineRgn(treg,treg,treg2,RGN_DIFF);
					DeleteObject(treg2);  
				}

			}
		}
		else
		{
			GetMenuBarInfo(hwnd,OBJID_MENU, item, &mbi);
			treg=CreateRectRgn(mbi.rcBar.left,mbi.rcBar.top,mbi.rcBar.right,mbi.rcBar.bottom+!DBGetContactSettingByte(NULL,"CLUI","LineUnderMenu",0));
		}
		OffsetRgn(treg,-ra.left,-ra.top);
		r1.left-=ra.left;
		r1.top-=ra.top;
		r1.bottom-=ra.top;
		r1.right-=ra.left;
	}   
	//SelectClipRgn(hdc,NULL);
	SelectClipRgn(hdc,treg);
	DeleteObject(treg); 
	{
		RECT rc;
		HWND hwnd=(HWND)CallService(MS_CLUI_GETHWND,0,0);
		GetWindowRect(hwnd,&rc);
		OffsetRect(&rc,-rc.left, -rc.top);
		FillRect(hdc,&r1,GetSysColorBrush(COLOR_MENU));
		FillRect255Alpha(hdc,&r1);
		//BltBackImage(hwnd,hdc,&r1);
	}
	SkinDrawGlyph(hdc,&r1,&r1,"Main,ID=MenuBar");
	/*   New Skin Engine
	if (dat->hMenuBackground)
	{
	BITMAP bmp;
	HBITMAP oldbm;
	HDC hdcBmp;
	int x,y;
	int maxx,maxy;
	int destw,desth;
	RECT clRect=r1;


	// XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
	SetStretchBltMode(hdc, HALFTONE);

	GetObject(dat->hMenuBackground,sizeof(bmp),&bmp);
	hdcBmp=CreateCompatibleDC(hdc);
	oldbm=SelectObject(hdcBmp,dat->hMenuBackground);
	y=clRect.top;
	x=clRect.left;
	maxx=dat->MenuBmpUse&CLBF_TILEH?maxx=r1.right:x+1;
	maxy=dat->MenuBmpUse&CLBF_TILEV?maxy=r1.bottom:y+1;
	switch(dat->MenuBmpUse&CLBM_TYPE) {
	case CLB_STRETCH:
	if(dat->MenuBmpUse&CLBF_PROPORTIONAL) {
	if(clRect.right-clRect.left*bmp.bmHeight<clRect.bottom-clRect.top*bmp.bmWidth) 
	{
	desth=clRect.bottom-clRect.top;
	destw=desth*bmp.bmWidth/bmp.bmHeight;
	}
	else 
	{
	destw=clRect.right-clRect.left;
	desth=destw*bmp.bmHeight/bmp.bmWidth;
	}
	}
	else {
	destw=clRect.right-clRect.left;
	desth=clRect.bottom-clRect.top;
	}
	break;
	case CLB_STRETCHH:
	if(dat->MenuBmpUse&CLBF_PROPORTIONAL) {
	destw=clRect.right-clRect.left;
	desth=destw*bmp.bmHeight/bmp.bmWidth;
	}
	else {
	destw=clRect.right-clRect.left;
	desth=bmp.bmHeight;
	}
	break;
	case CLB_STRETCHV:
	if(dat->MenuBmpUse&CLBF_PROPORTIONAL) {
	desth=clRect.bottom-clRect.top;
	destw=desth*bmp.bmWidth/bmp.bmHeight;
	}
	else {
	destw=bmp.bmWidth;
	desth=clRect.bottom-clRect.top;
	}
	break;
	default:    //clb_topleft
	destw=bmp.bmWidth;
	desth=bmp.bmHeight;					
	break;
	}
	if (desth && destw)
	for(y=clRect.top;y<maxy;y+=desth) {
	for(x=clRect.left;x<maxx;x+=destw)
	StretchBlt(hdc,x,y,destw,desth,hdcBmp,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
	}
	SelectObject(hdcBmp,oldbm);
	DeleteDC(hdcBmp);

	}                  

	else
	{
	hbr=CreateSolidBrush(dat->MenuBkColor);
	FillRect(hdc,&r1,hbr);
	DeleteObject(hbr);    
	}
	*/
	SelectClipRgn(hdc,NULL);
	return 0;
}
SIZE oldSize={0,0};
POINT oldPos={0,0};
extern BOOL dock_prevent_moving;
RECT new_window_rect={0};
RECT old_window_rect={0};
BOOL during_sizing=0;
//int GetWindowRect(HWND hwnd,RECT *rect)
//{
//  if (!IsRectEmpty(&new_window_rect)&&during_sizing)
//  {
//    if (hwnd==pcli->hwndContactList) 
//    {
//      *rect=new_window_rect;
//      return 1;
//    }
//  }
//  GetWindowRect(hwnd,rect);
//  if (hwnd==pcli->hwndContactList) 
//  {
//    new_window_rect=*rect;
//#ifdef _DEBUG
//    {
//      char buf [255];
//      sprintf(buf,"===--- New Window rect is set in NewGetWindowRect to:%d,%d  %d,%d\n",new_window_rect.left,new_window_rect.top,new_window_rect.right,new_window_rect.bottom);
//      TRACE(buf);    
//    }
//#endif
//  }
//  return 1;
//}
//int GetWindowRect(HWND hwnd,RECT *rect)
//{
// // if (hwnd!=pcli->hwndContactList) returnGetWindowRect(hwnd,rect);
// // if (oldSize.cy*oldSize.cx) return SetRect(rect,oldPos.x,oldPos.y,oldPos.x+oldSize.cx,oldPos.y+oldSize.cy);
//  return GetWindowRect(hwnd,rect);  
//}
BYTE Break_Sizing=0;
BYTE delayedSizing=0;
RECT timer_window_rect={0};
RECT sizing_rect={0};
RECT Get_Window_size={0};
RECT correct_size={0};
//int sizeSide
int SizingGetWindowRect(HWND hwnd,RECT * rc)
{
	if (during_sizing && hwnd==pcli->hwndContactList)
		*rc=sizing_rect;
	else
		GetWindowRect(hwnd,rc);
	return 1;
}
BOOL need_to_fix_sizing_rect=0;
BYTE called_from_cln=0;
BYTE called_from_me=0;

#ifdef _DEBUG
int TRACERECT(RECT * rect, char * text)
{
	char buf[255];
	_snprintf(buf,sizeof(buf),"%s : %d,%d %d,%d\n",text,rect->left,rect->top,rect->right,rect->bottom);
	TRACE(buf);
	return 0;
}
#else
#define TRACERECT(a,b)
#endif

extern LRESULT ( CALLBACK *saveContactListWndProc )(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{    
	/*
	This registers a window message with RegisterWindowMessage() and then waits for such a message,
	if it gets it, it tries to open a file mapping object and then maps it to this process space,
	it expects 256 bytes of data (incl. NULL) it will then write back the profile it is using the DB to fill in the answer.

	The caller is expected to create this mapping object and tell us the ID we need to open ours.	
	*/
	if (msg==hMsgGetProfile && wParam != 0) { /* got IPC message */
		HANDLE hMap;
		char szName[MAX_PATH];
		int rc=0;
		_snprintf(szName,sizeof(szName),"Miranda::%u", wParam); // caller will tell us the ID of the map
		hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS,FALSE,szName);
		if (hMap != NULL) {
			void *hView=NULL;
			hView=MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MAX_PATH);
			if (hView) {
				char szFilePath[MAX_PATH], szProfile[MAX_PATH];
				CallService(MS_DB_GETPROFILEPATH,MAX_PATH,(LPARAM)&szFilePath);
				CallService(MS_DB_GETPROFILENAME,MAX_PATH,(LPARAM)&szProfile);
				_snprintf(hView,MAX_PATH,"%s\\%s",szFilePath,szProfile);
				UnmapViewOfFile(hView);
				rc=1;
			}
			CloseHandle(hMap);
		}
		return rc;
	}

	if (LayeredFlag)
	{
		LRESULT res=0;
		if (msg==WM_SIZING)
		{
			static a=0;
			RECT* wp=(RECT*)lParam;
			if (need_to_fix_sizing_rect && (correct_size.bottom!=0||correct_size.top!=0))
			{
				if(wParam!=WMSZ_BOTTOM) wp->bottom=correct_size.bottom;
				if(wParam!=WMSZ_TOP) wp->top=correct_size.top;       
			}
			need_to_fix_sizing_rect=0;
			sizing_rect=*wp;
			TRACERECT(&sizing_rect,"Sizing RECT is:");
			during_sizing=1;
			return 1;
		}

		if (msg==WM_WINDOWPOSCHANGING)
		{

			WINDOWPOS * wp;
			HDWP PosBatch;
			RECT work_rect={0};
			RECT temp_rect={0};
			wp=(WINDOWPOS *)lParam;
			GetWindowRect(hwnd,&old_window_rect);
			if ((old_window_rect.bottom-old_window_rect.top!=wp->cy || old_window_rect.right-old_window_rect.left !=wp->cx)&&!(wp->flags&SWP_NOSIZE))
			{
				TRACE("SizePosChanging started\n");
				TRACERECT(&old_window_rect,"Old window is:");
				//determine new_window_size and = work_rect
				{         
					if (!(wp->flags&SWP_NOMOVE)) 
					{
						new_window_rect.left=wp->x;
						new_window_rect.top=wp->y;
					}
					else
					{
						new_window_rect.left=old_window_rect.left;
						new_window_rect.top=old_window_rect.top;
					}
					new_window_rect.right=new_window_rect.left+wp->cx;  
					new_window_rect.bottom=new_window_rect.top+wp->cy;
					work_rect=new_window_rect;
				}
				TRACERECT(&work_rect,"Work RECT_1 is:");       
				//resize frames (batch)
				{
					PosBatch=BeginDeferWindowPos(1);
					SizeFramesByWindowRect(&work_rect,&PosBatch,0);
				}
				//Check rect after frames resize
				{
					GetWindowRect(hwnd,&temp_rect);
				}
				TRACERECT(&temp_rect,"After frame size Window RECT is:");
				//Here work_rect should be changed to fit possible changes in cln_listsizechange
				if(need_to_fix_sizing_rect)
				{
					work_rect=sizing_rect;
					wp->x=work_rect.left;
					wp->y=work_rect.top;
					wp->cx=work_rect.right-work_rect.left;
					wp->cy=work_rect.bottom-work_rect.top;
					wp->flags&=~(SWP_NOMOVE);
				}
				TRACERECT(&work_rect,"Work RECT_2 is:");
				//reposition buttons and new size applying
				{
					ReposButtons(hwnd,FALSE,&work_rect);
					PrepeareImageButDontUpdateIt(&work_rect);
					dock_prevent_moving=0;			
					UpdateWindowImageRect(&work_rect);        
					EndDeferWindowPos(PosBatch);
					dock_prevent_moving=1;
				}       

				Sleep(0);               
				TRACE("SizePosChanging completed\n");
				// if (called_from_cln) return DefWindowProc(hwnd,msg,wParam,lParam);
				//if (need_to_fix_sizing_rect && !called_from_me) 
				//{
				//  called_from_me=1;
				//    SetWindowPos(hwnd,NULL,wp->x,wp->y,wp->cx,wp->cy,SWP_NOZORDER|SWP_NOACTIVATE);
				//  called_from_me=0;
				//  DefWindowProc(hwnd,msg,wParam,lParam);
				//}
				during_sizing=0; 
				return SendMessage(hwnd,WM_WINDOWPOSCHANGED,wParam,lParam);
			}
			else
			{
				SetRect(&correct_size,0,0,0,0);
				// need_to_fix_sizing_rect=0;
			}
			return DefWindowProc(hwnd,msg,wParam,lParam); 
	}	}

	switch (msg) {
	case UM_UPDATE:
		if (POST_WAS_CANCELED) return 0;
		return ValidateFrameImageProc(NULL);               

		//        case WM_SIZING:
		//            ON_EDGE_SIZING=wParam;
		//            ON_EDGE_SIZING_POS=*((LPRECT)lParam);
		//            return DefWindowProc(hwnd, msg, wParam, lParam); 
	case WM_INITMENU:
		{
			if (ServiceExists(MS_CLIST_MENUBUILDMAIN)){CallService(MS_CLIST_MENUBUILDMAIN,0,0);};
			return(0);
		};

	case WM_PRINT:  
	case WM_NCPAINT:
	case WM_NCACTIVATE:
		{
			int r = DefWindowProc(hwnd, msg, wParam, lParam);
			if (DBGetContactSettingByte(NULL,"CLUI","ShowMainMenu",SETTING_SHOWMAINMENU_DEFAULT))
			{
				HDC hdc;
				hdc=NULL;
				if (msg==WM_PRINT)
					hdc=(HDC)wParam;
				if (!hdc) hdc=GetWindowDC(hwnd);
				DrawMenuBackGround(hwnd,hdc,0);
				if (msg!=WM_PRINT) ReleaseDC(hwnd,hdc);
			}
			return r;
		}
	case WM_ERASEBKGND:
		return 0;

	case WM_NCCREATE:
		{	LPCREATESTRUCT p = (LPCREATESTRUCT)lParam;
			p->style &= ~(CS_HREDRAW | CS_VREDRAW);
		}
		break;

	case WM_PAINT:
		if (!LayeredFlag && IsWindowVisible(hwnd))
		{
			RECT w={0};
			RECT w2={0};
			PAINTSTRUCT ps={0};
			HDC hdc;
			HBITMAP hbmp,oldbmp;
			HDC paintDC;

			GetClientRect(hwnd,&w);
			//OffsetRect(&w,-w.left,-w.top);
			paintDC = GetDC(hwnd);


			//if ((ps.rcPaint.bottom-ps.rcPaint.top)*(ps.rcPaint.right-ps.rcPaint.left)==0)
			w2=w;
			//else
			//	w2=ps.rcPaint;
			hdc=CreateCompatibleDC(paintDC);
			hbmp=CreateBitmap32(w.right,w.bottom);
			oldbmp=SelectObject(hdc,hbmp);
			//FillRect(hdc,&w2,GetSysColorBrush(COLOR_BTNFACE));
			//FillRect255Alpha(ps.hdc,&w2);
			ReCreateBackImage(FALSE,NULL);
			BitBlt(paintDC,w2.left,w2.top,w2.right-w2.left,w2.bottom-w2.top,cachedWindow->hBackDC,w2.left,w2.top,SRCCOPY);
			//SkinDrawGlyph(hdc,&w,&w2,"Main,ID=Background,Opt=Non-Layered");
			//BitBlt(paintDC,w2.left,w2.top,w2.right-w2.left,w2.bottom-w2.top,hdc,w2.left,w2.top,SRCCOPY);
			SelectObject(hdc,oldbmp);
			DeleteObject(hbmp);
			DeleteDC(hdc);
			ReleaseDC(hwnd,paintDC);
			ValidateRect(hwnd,NULL);
			//SkinDrawWindowBack(hwnd,ps.hdc,&ps.rcPaint,"Main Window/Backgrnd");
			TRACE("WM_PAINT in CLUI: ");
#ifdef _DEBUG
			{
				char buf[256];
				_snprintf(buf,sizeof(buf),"%d,%d %d,%d\n",ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom);
				TRACE(buf);
			}
#endif
			ps.fErase=FALSE;
			EndPaint(hwnd,&ps); 
		}
		if (0&&(DBGetContactSettingDword(NULL,"CLUIFrames","GapBetweenFrames",1) || DBGetContactSettingDword(NULL,"CLUIFrames","GapBetweenTitleBar",1)))
		{
			// PAINTSTRUCT ps;
			TRACE("CLUI WM_PAINT\n");

			//       BeginPaint(hwnd,&ps);
			if (IsWindowVisible(hwnd))
				if (LayeredFlag)
				{
					SkinInvalidateFrame(hwnd,NULL,0);				
					TRACE("SkinUpdateWindow on CLUI WM_PAINT\n");
				}
				else 
				{
					RECT w={0};
					RECT w2={0};
					PAINTSTRUCT ps={0};
					GetWindowRect(hwnd,&w);
					OffsetRect(&w,-w.left,-w.top);
					BeginPaint(hwnd,&ps);
					if ((ps.rcPaint.bottom-ps.rcPaint.top)*(ps.rcPaint.right-ps.rcPaint.left)==0)
						w2=w;
					else
						w2=ps.rcPaint;
					SkinDrawGlyph(ps.hdc,&w,&w2,"Main,ID=Background,Opt=Non-Layered");
					//SkinDrawWindowBack(hwnd,ps.hdc,&ps.rcPaint,"Main Window/Backgrnd");
					ps.fErase=FALSE;
					EndPaint(hwnd,&ps); 
				}
				//SkinDrawWindowBack(hwnd,ps.hdc,&ps.rcPaint,"Main Window/Backgrnd");
				//     ps.fErase=FALSE;
				//     EndPaint(hwnd,&ps); 
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CREATE:
		CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)GetMenu(hwnd),0);
		DrawMenuBar(hwnd);
		showOpts=DBGetContactSettingByte(NULL,"CLUI","SBarShow",7);		

		//create the status wnd
		//pcli->hwndStatus = CreateStatusWindow(WS_CHILD | (DBGetContactSettingByte(NULL,"CLUI","ShowSBar",1)?WS_VISIBLE:0), "", hwnd, 0);	
		CluiProtocolStatusChanged(0,0);

		{	MENUITEMINFO mii;
			ZeroMemory(&mii,sizeof(mii));
			mii.cbSize=MENUITEMINFO_V4_SIZE;
			mii.fMask=MIIM_TYPE|MIIM_DATA;
			himlMirandaIcon=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,1);
			ImageList_AddIcon(himlMirandaIcon,LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
			mii.dwItemData=MENU_MIRANDAMENU;
			mii.fType=MFT_OWNERDRAW;
			mii.dwTypeData=NULL;
			SetMenuItemInfo(GetMenu(hwnd),0,TRUE,&mii);

			// mii.fMask=MIIM_TYPE;
			mii.fType=MFT_OWNERDRAW;
			mii.dwItemData=MENU_STATUSMENU;
			SetMenuItemInfo(GetMenu(hwnd),1,TRUE,&mii);
		}

		//delay creation of CLC so that it can get the status icons right the first time (needs protocol modules loaded)
		//PostMessage(hwnd,M_CREATECLC,0,0);

		hMsgGetProfile=RegisterWindowMessage(TEXT("Miranda::GetProfile")); // don't localise

		/*	if (DBGetContactSettingByte(NULL,"CList","Transparent",0))
		{
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
		}
		*/
		transparentFocus=1;

		{
			#ifndef _DEBUG
				int nStatus = DBGetContactSettingWord(NULL, "CList", "Status", ID_STATUS_OFFLINE);
				if (nStatus != ID_STATUS_OFFLINE)
					PostMessage(hwnd, WM_COMMAND, nStatus, 0);
			#endif
		}

		return FALSE;

	case M_SETALLEXTRAICONS:
		return 0;

	case M_CREATECLC:
		CreateCLC(hwnd);
		ShowHide((WPARAM)hwnd,(LPARAM)1); 
		SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
		return 0;

	case  WM_LBUTTONDOWN:
		{
			POINT pt;
			int k=0;
			pt.x = LOWORD(lParam); 
			pt.y = HIWORD(lParam); 
			ClientToScreen(hwnd,&pt);

			k=SizingOnBorder(pt,1);
			if (k)
			{
				IgnoreActivation=1;
				return 0;
			}

		}
		break;
	case  WM_PARENTNOTIFY:      
		if (wParam==WM_LBUTTONDOWN) {   
			POINT pt;
			int k=0;
			pt.x = LOWORD(lParam); 
			pt.y = HIWORD(lParam); 
			ClientToScreen(hwnd,&pt);

			k=SizingOnBorder(pt,1);
			wParam=0;
			lParam=0;

			if (k) {
				IgnoreActivation=1;
				return 0;
			}
			//	ActivateSubContainers(1);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);;

	case WM_SIZING:
		return TRUE;

	case WM_MOVE:
		{
			RECT rc;
			TRACE("WM_MOVE in CLUI\n");

			during_sizing=0;      
			GetWindowRect(hwnd, &rc);

			CheckFramesPos(&rc);
			ReposButtons(hwnd,1,&rc);
			OnMoving(hwnd,&rc);

			if(!IsIconic(hwnd)) {

				//case WM_SIZING:
				if(!CallService(MS_CLIST_DOCKINGISDOCKED,0,0))
				{ //if docked, dont remember pos (except for width)
					DBWriteContactSettingDword(NULL,"CList","Height",(DWORD)(rc.bottom - rc.top));
					DBWriteContactSettingDword(NULL,"CList","x",(DWORD)rc.left);
					DBWriteContactSettingDword(NULL,"CList","y",(DWORD)rc.top);
				}
				DBWriteContactSettingDword(NULL,"CList","Width",(DWORD)(rc.right - rc.left));
			}
			return TRUE;
		}
	case WM_SIZE:    
		{   
			RECT rc;
			if (ON_SIZING_CYCLE) return 0;
			TRACE("WM_SIZE\n");
			if(wParam!=SIZE_MINIMIZED /*&& IsWindowVisible(hwnd)*/) 
			{
				if ( pcli->hwndContactList == NULL )
					return 0;

				if (!LayeredFlag)
					ReCreateBackImage(TRUE,NULL);

				GetWindowRect(hwnd, &rc);
				CheckFramesPos(&rc);
				ReposButtons(hwnd,FALSE,&rc);
				ReposButtons(hwnd,7,NULL);
				if (LayeredFlag)
					UpdateFrameImage((WPARAM)hwnd,0);
				if (!LayeredFlag)
				{
					ON_SIZING_CYCLE=1;
					CLUIFramesOnClistResize2((WPARAM)hwnd,(LPARAM)0,1);
					CLUIFramesApplyNewSizes(2);
					CLUIFramesApplyNewSizes(1);
					TRACE("CLUIFramesOnClistResize(0,2); is called from WM_SIZE\n");
					SendMessage(hwnd,CLN_LISTSIZECHANGE,0,0);				  
					ON_SIZING_CYCLE=0;
				}

				//       RedrawCompleteWindow();
				if(!CallService(MS_CLIST_DOCKINGISDOCKED,0,0))
				{ //if docked, dont remember pos (except for width)
					DBWriteContactSettingDword(NULL,"CList","Height",(DWORD)(rc.bottom - rc.top));
					DBWriteContactSettingDword(NULL,"CList","x",(DWORD)rc.left);
					DBWriteContactSettingDword(NULL,"CList","y",(DWORD)rc.top);
				}
				else SetWindowRgn(hwnd,NULL,0);
				DBWriteContactSettingDword(NULL,"CList","Width",(DWORD)(rc.right - rc.left));	                               

				if (!LayeredFlag)
				{
					HRGN hRgn1;
					RECT r;
					int v,h;
					int w=10;
					GetWindowRect(hwnd,&r);
					h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
					v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
					h=(h<v)?h:v;
					hRgn1=CreateRoundRectRgn(0,0,(r.right-r.left+1),(r.bottom-r.top+1),h,h);
					if ((DBGetContactSettingByte(NULL,"CLC","RoundCorners",0)) && (!CallService(MS_CLIST_DOCKINGISDOCKED,0,0)))
						SetWindowRgn(hwnd,hRgn1,1); 
					else SetWindowRgn(hwnd,NULL,1);
					RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME|RDW_UPDATENOW|RDW_ALLCHILDREN);   
				} 			
			}
			else {
				if(DBGetContactSettingByte(NULL,"CList","Min2Tray",SETTING_MIN2TRAY_DEFAULT)) {
					ShowWindowNew(hwnd, SW_HIDE);
					DBWriteContactSettingByte(NULL,"CList","State",SETTING_STATE_HIDDEN);
				}
				else DBWriteContactSettingByte(NULL,"CList","State",SETTING_STATE_MINIMIZED);
			}

			return TRUE;
		}

	case WM_SETFOCUS:
		if (hFrameContactTree) {					
			int isfloating = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLOATING,hFrameContactTree),0);
			if ( !isfloating )
				SetFocus(pcli->hwndContactTree);
		}
		UpdateWindow(hwnd);
		return 0;

	case WM_ACTIVATE:
		{
			BOOL IsOption=FALSE;
			SetCursor(LoadCursor(NULL, IDC_ARROW));

			TRACE("CLUI------- WM_ACTIVATE\n");
			if (DBGetContactSettingByte(NULL, "ModernData", "HideBehind", 0))
			{
				if(wParam==WA_INACTIVE && ((HWND)lParam!=hwnd) && GetParent((HWND)lParam)!=hwnd && !IsOption) 
				{
					//hide
					//BehindEdge_Hide();
					if (!CALLED_FROM_SHOWHIDE) UpdateTimer(0);
				}
				else
				{
					//show
					if (!CALLED_FROM_SHOWHIDE) BehindEdge_Show();
				}
			}

			if (!IsWindowVisible(hwnd) || SHOWHIDE_CALLED_FROM_ANIMATION) 
			{            
				KillTimer(hwnd,TM_AUTOALPHA);
				return 0;
			}
			if(wParam==WA_INACTIVE && ((HWND)lParam!=hwnd) && GetParent((HWND)lParam)!=hwnd && !IsOption) 
			{
				//if (!DBGetContactSettingByte(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT))
				//{
				//  //---+++SetWindowPos(pcli->hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE|SWP_NOACTIVATE);
				//  //ActivateSubContainers(0);					
				//}
				//if (IS_WM_MOUSE_DOWN_IN_TRAY) //check cursor is over trayicon
				//{
				//	IS_WM_MOUSE_DOWN_IN_TRAY=0;
				//	return DefWindowProc(hwnd,msg,wParam,lParam);
				//}
				//else
				if(TransparentFlag)
					if(transparentFocus)
						SetTimer(hwnd, TM_AUTOALPHA,250,NULL);

			}
			else {
				if (!DBGetContactSettingByte(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT))
				{			
					//---+++SetWindowPos(pcli->hwndContactList, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);				
					ActivateSubContainers(1);
				}
				if(TransparentFlag) {
					KillTimer(hwnd,TM_AUTOALPHA);
					//        TRACE("SMOTH ALPHA CALLED FROM ACTIVATION_1\n");
					SmoothAlphaTransition(hwnd, DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
					//if (MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
					transparentFocus=1;
				}
			}
			//InvalidateRectZ(hwnd,NULL, TRUE);
			RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
			if(TransparentFlag)
			{
				BYTE alpha;
				if (wParam!=WA_INACTIVE || IsOption || ((HWND)lParam==hwnd) || GetParent((HWND)lParam)==hwnd) alpha=DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT);
				else 
					alpha=DBGetContactSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT);
				SmoothAlphaTransition(hwnd, alpha, 1);
				if(IsOption) DefWindowProc(hwnd,msg,wParam,lParam);
				else   return 1; 	
			}
			//DefWindowProc(hwnd,msg,wParam,lParam);
			return  DefWindowProc(hwnd,msg,wParam,lParam);
		}
	case  WM_SETCURSOR:
		{
			int k=0;
			HWND gf=GetForegroundWindow();
			if (BehindEdge_State>=0)  UpdateTimer(1);
			if(TransparentFlag) {
				if (!transparentFocus && gf!=hwnd)
				{
					SmoothAlphaTransition(hwnd, DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
					transparentFocus=1;
					SetTimer(hwnd, TM_AUTOALPHA,250,NULL);
				}
			}
			k=TestCursorOnBorders();               
			return k?k:1;//DefWindowProc(hwnd,msg,wParam,lParam);
		}
	case WM_MOUSEACTIVATE:
		{
			int r;
			if (IgnoreActivation) 
			{   
				IgnoreActivation=0;
				return(MA_NOACTIVATEANDEAT);                   
			}
			r=DefWindowProc(hwnd,msg,wParam,lParam);
			RepaintSubContainers();
			return r;
		}

	case WM_NCLBUTTONDOWN:
		{   
			POINT pt;
			int k=0;
			pt.x = LOWORD(lParam); 
			pt.y = HIWORD(lParam); 
			//ClientToScreen(hwnd,&pt);
			k=SizingOnBorder(pt,1);
			//if (!k) after_syscommand=1;
			return k?k:DefWindowProc(hwnd,msg,wParam,lParam);

			break;
		}
	case WM_NCHITTEST:
		{
			LRESULT result;
			result=DefWindowProc(hwnd,WM_NCHITTEST,wParam,lParam);
			if(result==HTSIZE || result==HTTOP || result==HTTOPLEFT || result==HTTOPRIGHT ||
				result==HTBOTTOM || result==HTBOTTOMRIGHT || result==HTBOTTOMLEFT)
				if(DBGetContactSettingByte(NULL,"CLUI","AutoSize",0)) return HTCLIENT;
			if (result==HTMENU) 
			{
				int t;
				POINT pt;
				pt.x=LOWORD(lParam);
				pt.y=HIWORD(lParam);
				t=MenuItemFromPoint(hwnd,hMenuMain,pt);
				if (t==-1) return HTCAPTION;
			}
			if (result==HTCLIENT)
			{
				POINT pt;
				int k;
				pt.x=LOWORD(lParam);
				pt.y=HIWORD(lParam);			
				k=SizingOnBorder(pt,0);
				if (!k)	return HTCAPTION;
				else return k+9;
			}
			return result;
		}

	case WM_TIMER:
		if ((int)wParam>=TM_STATUSBARUPDATE&&(int)wParam<=TM_STATUSBARUPDATE+64)
		{

			int status,i;

			ProtoTicks *pt=NULL;
			for (i=0;i<64;i++)
			{

				pt=&CycleStartTick[i];

				if (pt->szProto!=NULL&&pt->TimerCreated==1)
				{

					status=CallProtoService(pt->szProto,PS_GETSTATUS,0,0);

					if (!(status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES))
					{													
						pt->CycleStartTick=0;
						KillTimer(hwnd,TM_STATUSBARUPDATE+pt->n);
						pt->TimerCreated=0;
					}
				}

			};

			pt=&CycleStartTick[wParam-TM_STATUSBARUPDATE];
			{
				// RECT rc;
				// GetStatsuBarProtoRect(pcli->hwndStatus,pt->szProto,&rc);
				// rc.right=rc.left+GetSystemMetrics(SM_CXSMICON)+1;
				// rc.top=0;
#ifdef _DEBUG
				{
					//char buf[512];
					//sprintf(buf,"Invalidate left: %d right: %d\r\n",rc.left,rc.right);
					//TRACE(buf);
				}
#endif

				if(IsWindowVisible(pcli->hwndStatus)) SkinInvalidateFrame(pcli->hwndStatus,NULL,0);//InvalidateRectZ(pcli->hwndStatus,NULL,TRUE);
				if (DBGetContactSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)!=SETTING_TRAYICON_CYCLE) TrayIconUpdateBase(pt->szProto);

			}
			SkinInvalidateFrame(pcli->hwndStatus,NULL,0);


			//SendMessage(pcli->hwndStatus,WM_PAINT,0,0);
			//UpdateWindow(pcli->hwndStatus);

			//SkinUpdateWindow(NULL,pcli->hwndStatus); 
			//TRACE("SkinUpdateWindow on WM_TIMER\n");
			break;
		}
		else if ((int)wParam==TM_SMOTHALPHATRANSITION)
		{
			TRACE ("CallAnimation\n");
			SmoothAlphaTransition(hwnd, 0, 2);
		}
		else if ((int)wParam==TM_AUTOALPHA)
		{	int inwnd;

		if (GetForegroundWindow()==hwnd) {
			KillTimer(hwnd,TM_AUTOALPHA);
			inwnd=1;
		}
		else {
			POINT pt;
			HWND hwndPt,thwnd;
			pt.x=(short)LOWORD(GetMessagePos());
			pt.y=(short)HIWORD(GetMessagePos());
			hwndPt=WindowFromPoint(pt);
			inwnd=0;
			{
				thwnd=hwndPt;
				do
				{
					if (thwnd==hwnd) inwnd=TRUE;
					if (!inwnd)
						thwnd=GetParent(thwnd);
				}while (thwnd && !inwnd); 
			}
		}
		if (inwnd!=transparentFocus)
		{ //change
			transparentFocus=inwnd;
			if(transparentFocus) SmoothAlphaTransition(hwnd, (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
			else  
			{
				SmoothAlphaTransition(hwnd, (BYTE)DBGetContactSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT), 1);
			}
		}
		if(!transparentFocus) KillTimer(hwnd,TM_AUTOALPHA);
		}     
		else if ((int)wParam==TM_DELAYEDSIZING && delayedSizing)
		{
			if (!during_sizing)
			{
				delayedSizing=0;
				KillTimer(hwnd,TM_DELAYEDSIZING);
				{
					char as[100];
					sprintf(as,"DELAYED SIZING %d,%d, %d, %d\n",timer_window_rect.left,timer_window_rect.top,timer_window_rect.right,timer_window_rect.bottom );
					TRACE(as);
				}
				pcli->pfnClcBroadcast( INTM_SCROLLBARCHANGED,0,0);
			}
		}
		else if ((int)wParam==TM_BRINGOUTTIMEOUT)
		{
			//hide
			POINT pt;
			HWND hAux;
			int mouse_in_window=0;
			KillTimer(hwnd,TM_BRINGINTIMEOUT);
			KillTimer(hwnd,TM_BRINGOUTTIMEOUT);
			GetCursorPos(&pt);
			hAux = WindowFromPoint(pt);
			while(hAux != NULL) 
			{
				if (hAux == hwnd) {mouse_in_window=1; break;}
				hAux = GetParent(hAux);
			}
			if (!mouse_in_window && GetForegroundWindow()!=hwnd)
			{
				BehindEdge_Hide(); 
			}
			TRACE("BRINGOUT TIMER\n");
		}
		else if ((int)wParam==TM_BRINGINTIMEOUT)
		{
			//show
			POINT pt;
			HWND hAux;
			int mouse_in_window=0;
			KillTimer(hwnd,TM_BRINGINTIMEOUT);
			KillTimer(hwnd,TM_BRINGOUTTIMEOUT);
			GetCursorPos(&pt);
			hAux = WindowFromPoint(pt);
			while(hAux != NULL) 
			{
				if (hAux == hwnd) {mouse_in_window=1; break;}
				hAux = GetParent(hAux);
			}
			if (mouse_in_window)
			{       
				BehindEdge_Show();
			}
			TRACE("BRINGIN TIMER\n");
		}
		else if ((int)wParam==TM_UPDATEBRINGTIMER)
		{
			//recheck timer
			//    POINT pt;
			//    HWND hAux;
			//    int mouse_in_window=0;
			//    if (BehindEdge_State==-1 && BehindEdgeSettings)
			//      {
			//        UpdateTimer(0);
			//        return TRUE;
			//      }
			//    GetCursorPos(&pt);
			//    hAux = WindowFromPoint(pt);
			//while(hAux != NULL) 
			//    {
			//      if (hAux == hwnd) {mouse_in_window=1; break;}
			//      hAux = GetParent(hAux);
			//    }
			//    if (!mouse_in_window)
			{       
				UpdateTimer(0);
			}
			//      TRACE("UpdatetimerTIMER\n");
		}

		return TRUE;


	case WM_SHOWWINDOW:
		{	
			BYTE gAlpha;
			if (lParam) break;
			if (SHOWHIDE_CALLED_FROM_ANIMATION) return 1; 
			{

				if (!wParam) gAlpha=0;
				else 
					gAlpha=(DBGetContactSettingByte(NULL,"CList","Transparent",0)?DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT):255);
				if (wParam) 
				{
					CURRENT_ALPHA=0;
					//if (BehindEdgeSettings) 
					OnShowHide(pcli->hwndContactList,1);
					RedrawCompleteWindow();
				}
				TRACE("ANIMATION FROM SHOW WINDOW\n");
				SmoothAlphaTransition(hwnd, gAlpha, 1);
			}
			return 0;
		}


		//case WM_MENURBUTTONUP: /* this API is so badly documented at MSDN!! */
		//  {
		//    UINT id=0;
		//    id=GetMenuItemID((HMENU)lParam,LOWORD(wParam)); /* LOWORD(wParam) contains the menu pos in its parent menu */
		// DefWindowProc(hwnd, msg, wParam, lParam);
		// EndMenu();
		//    if (id != (-1)) PostMessage(hwnd,WM_COMMAND,MAKEWPARAM(id,0),0);
		//    // if (id != (-1)) PostMessage(hwnd,WM_MENUCOMMAND,wParam,lParam);
		//    //return 
		//    break;
		//  }
	case WM_SYSCOMMAND:
		if(wParam==SC_MAXIMIZE) return 0;

		DefWindowProc(hwnd, msg, wParam, lParam);
		if (DBGetContactSettingByte(NULL,"CList","OnDesktop",0))
			ActivateSubContainers(TRUE);
		return 0;

	case WM_KEYDOWN:
		if (wParam==VK_F5)
			SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
		break;

	case WM_GETMINMAXINFO:
		DefWindowProc(hwnd,msg,wParam,lParam);
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x=max(DBGetContactSettingWord(NULL,"CLUI","MinWidth",18),max(18,DBGetContactSettingByte(NULL,"CLUI","LeftClientMargin",0)+DBGetContactSettingByte(NULL,"CLUI","RightClientMargin",0)+18));
		if (requr==0)
		{
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y=CLUIFramesGetMinHeight();
		}
		return 0;

	case WM_MOVING:
		if (1) //showcontents is turned on
		{
			OnMoving(hwnd,(RECT*)lParam);
			//if (!LayeredFlag) UpdateWindowImage();
		}
		return TRUE;


		//MSG FROM CHILD CONTROL
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == pcli->hwndContactTree) {
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
				{
					NMCLISTCONTROL *nm=(NMCLISTCONTROL *)lParam;
					if (nm!=NULL) SetAllExtraIcons(pcli->hwndContactTree,nm->hItem );
					return 0;
				}
			case CLN_LISTREBUILT:
				SetAllExtraIcons(pcli->hwndContactTree,0);
				return(FALSE);

			case CLN_LISTSIZECHANGE:
				{
					NMCLISTCONTROL *nmc=(NMCLISTCONTROL*)lParam;
					static RECT rcWindow,rcTree,rcTree2,rcWorkArea,rcOld;
					int maxHeight,newHeight;
					int winstyle;
					if (disableautoupd==1){return 0;};
					if (during_sizing)
						rcWindow=sizing_rect;
					else					
						GetWindowRect(hwnd,&rcWindow);
					if(!DBGetContactSettingByte(NULL,"CLUI","AutoSize",0)){return 0;}
					if(CallService(MS_CLIST_DOCKINGISDOCKED,0,0)) return 0;
					if (hFrameContactTree==0)return 0;
					maxHeight=DBGetContactSettingByte(NULL,"CLUI","MaxSizeHeight",75);
					rcOld=rcWindow;
					GetWindowRect(pcli->hwndContactTree,&rcTree);
					//
					{
						wndFrame* frm=FindFrameByItsHWND(pcli->hwndContactTree);
						if (frm) 
							rcTree2=frm->wndSize;
						else
							SetRect(&rcTree2,0,0,0,0);
					}
					winstyle=GetWindowLong(pcli->hwndContactTree,GWL_STYLE);

					SystemParametersInfo(SPI_GETWORKAREA,0,&rcWorkArea,FALSE);
					if (nmc->pt.y>(rcWorkArea.bottom-rcWorkArea.top)) 
					{
						nmc->pt.y=(rcWorkArea.bottom-rcWorkArea.top);
						//break;
					};
					if ((nmc->pt.y)==lastreqh)
					{
						//	break;
					}
					lastreqh=nmc->pt.y;
					newHeight=max(CLUIFramesGetMinHeight(),max(nmc->pt.y,3)+1+((winstyle&WS_BORDER)?2:0)+(rcWindow.bottom-rcWindow.top)-(rcTree.bottom-rcTree.top));
					if (newHeight==(rcWindow.bottom-rcWindow.top)) return 0;

					if(newHeight>(rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100)
						newHeight=(rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100;

					if(DBGetContactSettingByte(NULL,"CLUI","AutoSizeUpward",0)) {
						rcWindow.top=rcWindow.bottom-newHeight;
						if(rcWindow.top<rcWorkArea.top) rcWindow.top=rcWorkArea.top;
					}
					else {
						rcWindow.bottom=rcWindow.top+newHeight;
						if(rcWindow.bottom>rcWorkArea.bottom) rcWindow.bottom=rcWorkArea.bottom;
					}
					if (requr==1){return 0;};
					requr=1;					
					if (during_sizing)
					{
						need_to_fix_sizing_rect=1;           
						sizing_rect.top=rcWindow.top;
						sizing_rect.bottom=rcWindow.bottom;
						correct_size=sizing_rect;						
					}
					else
					{
						need_to_fix_sizing_rect=0;
					}
					called_from_cln=1;
					if (!during_sizing)
						SetWindowPos(hwnd,0,rcWindow.left,rcWindow.top,rcWindow.right-rcWindow.left,rcWindow.bottom-rcWindow.top,SWP_NOZORDER|SWP_NOACTIVATE);
					else
					{
						SetWindowPos(hwnd,0,rcWindow.left,rcWindow.top,rcWindow.right-rcWindow.left,rcWindow.bottom-rcWindow.top,SWP_NOZORDER|SWP_NOACTIVATE);
					}
					requr=0;
					called_from_cln=0;
					return 0;
				}
			case NM_CLICK:
				{	NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
					DWORD hitFlags;
					HANDLE hItem = (HANDLE)SendMessage(pcli->hwndContactTree,CLM_HITTEST,(WPARAM)&hitFlags,MAKELPARAM(nm->pt.x,nm->pt.y));

					if (hitFlags&CLCHT_ONITEMEXTRA)
					{					
						int v,e,w;
						pdisplayNameCacheEntry pdnce; 
						v=ExtraToColumnNum(EXTRA_ICON_PROTO);
						e=ExtraToColumnNum(EXTRA_ICON_EMAIL);
						w=ExtraToColumnNum(EXTRA_ICON_WEB);

						if (!IsHContactGroup(hItem)&&!IsHContactInfo(hItem))
						{
							pdnce=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(nm->hItem);
							if (pdnce==NULL) return 0;

							if(nm->iColumn==v) {
								CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)nm->hItem,0);
							};
							if(nm->iColumn==e) {
								//CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)nm->hItem,0);
								char *email,buf[4096];
								email=DBGetStringA(nm->hItem,"UserInfo", "Mye-mail0");
								if (!email)
									email=DBGetStringA(nm->hItem, pdnce->szProto, "e-mail");																						
								if (email)
								{
									sprintf(buf,"mailto:%s",email);
									mir_free(email);
									ShellExecuteA(hwnd,"open",buf,NULL,NULL,SW_SHOW);
								}											
							};	
							if(nm->iColumn==w) {
								char *homepage;
								homepage=DBGetStringA(pdnce->hContact,"UserInfo", "Homepage");
								if (!homepage)
									homepage=DBGetStringA(pdnce->hContact,pdnce->szProto, "Homepage");
								if (homepage!=NULL)
								{											
									ShellExecuteA(hwnd,"open",homepage,NULL,NULL,SW_SHOW);
									mir_free(homepage);
								}
							}
						}
					};	
					if(hItem) break;
					if((hitFlags&(CLCHT_NOWHERE|CLCHT_INLEFTMARGIN|CLCHT_BELOWITEMS))==0) break;
					if (DBGetContactSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT)) {
						POINT pt;
						int res;
						pt=nm->pt;
						ClientToScreen(pcli->hwndContactTree,&pt);
						res=PostMessage(hwnd, WM_SYSCOMMAND, SC_MOVE|HTCAPTION,MAKELPARAM(pt.x,pt.y));
						return res;
					}
					/*===================*/
					if (DBGetContactSettingByte(NULL,"CLUI","DragToScroll",0)) 
						return EnterDragToScroll(pcli->hwndContactTree,nm->pt.y);
					/*===================*/
					return 0;
		}	}	}
		break;

	case WM_CONTEXTMENU:
		{	RECT rc;
			POINT pt;

			pt.x=(short)LOWORD(lParam);
			pt.y=(short)HIWORD(lParam);
			// x/y might be -1 if it was generated by a kb click			
			GetWindowRect(pcli->hwndContactTree,&rc);
			if ( pt.x == -1 && pt.y == -1) {
				// all this is done in screen-coords!
				GetCursorPos(&pt);
				// the mouse isnt near the window, so put it in the middle of the window
				if (!PtInRect(&rc,pt)) {
					pt.x = rc.left + (rc.right - rc.left) / 2;
					pt.y = rc.top + (rc.bottom - rc.top) / 2; 
				}
			}
			if(PtInRect(&rc,pt)) {
				HMENU hMenu;
				hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDGROUP,0,0);
				TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,hwnd,NULL);
				return 0;
		}	}
		return 0;

	case WM_MEASUREITEM:
		if(((LPMEASUREITEMSTRUCT)lParam)->itemData==MENU_STATUSMENU) {
			HDC hdc;
			SIZE textSize;
			hdc=GetDC(hwnd);
			GetTextExtentPoint32A(hdc,Translate("Status"),lstrlenA(Translate("Status")),&textSize);	
			((LPMEASUREITEMSTRUCT)lParam)->itemWidth=textSize.cx;
			//GetSystemMetrics(SM_CXSMICON)*4/3;
			((LPMEASUREITEMSTRUCT)lParam)->itemHeight=0;
			ReleaseDC(hwnd,hdc);
			return TRUE;
		}
		break;

	case WM_DRAWITEM:
		{
			struct ClcData * dat=(struct ClcData*)GetWindowLong(pcli->hwndContactTree,0);
			LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;
			if (!dat) return 0;
			//if(dis->hwndItem==pcli->hwndStatus/*&&IsWindowVisible(pcli->hwndStatus)*/) {
			//  //DrawDataForStatusBar(dis);  //possible issue with hangin for some protocol is here
			//  InvalidateFrameImage((WPARAM)pcli->hwndStatus,0);
			//}

			if (dis->CtlType==ODT_MENU) {
				if (dis->itemData==MENU_MIRANDAMENU) {
					if (!LayeredFlag)
					{	
						char buf[255]={0};	
						short dx=1+(dis->itemState&ODS_SELECTED?1:0)-(dis->itemState&ODS_HOTLIGHT?1:0);
						HICON hIcon=ImageList_GetIcon(himlMirandaIcon,0,ILD_NORMAL);
						DrawMenuBackGround(hwnd, dis->hDC, 1);
						_snprintf(buf,sizeof(buf),"Main,ID=MainMenu,Selected=%s,Hot=%s",(dis->itemState&ODS_SELECTED)?"True":"False",(dis->itemState&ODS_HOTLIGHT)?"True":"False");
						SkinDrawGlyph(dis->hDC,&dis->rcItem,&dis->rcItem,buf);
						DrawState(dis->hDC,NULL,NULL,(LPARAM)hIcon,0,(dis->rcItem.right+dis->rcItem.left-GetSystemMetrics(SM_CXSMICON))/2+dx,(dis->rcItem.bottom+dis->rcItem.top-GetSystemMetrics(SM_CYSMICON))/2+dx,0,0,DST_ICON|(dis->itemState&ODS_INACTIVE&&FALSE?DSS_DISABLED:DSS_NORMAL));
						DestroyIcon(hIcon);         
						MirMenuState=dis->itemState;
					} else {
						MirMenuState=dis->itemState;
						SkinInvalidateFrame(hwnd,NULL,0);
					}
					return TRUE;
				}
				else if(dis->itemData==MENU_STATUSMENU) {
					if (!LayeredFlag)
					{
						char buf[255]={0};
						RECT rc=dis->rcItem;
						short dx=1+(dis->itemState&ODS_SELECTED?1:0)-(dis->itemState&ODS_HOTLIGHT?1:0);
						if (dx>1){
							rc.left+=dx;
							rc.top+=dx;
						}else if (dx==0){
							rc.right-=1;
							rc.bottom-=1;
						}
						DrawMenuBackGround(hwnd, dis->hDC, 2);
						SetBkMode(dis->hDC,TRANSPARENT);
						_snprintf(buf,sizeof(buf),"Main,ID=StatusMenu,Selected=%s,Hot=%s",(dis->itemState&ODS_SELECTED)?"True":"False",(dis->itemState&ODS_HOTLIGHT)?"True":"False");
						SkinDrawGlyph(dis->hDC,&dis->rcItem,&dis->rcItem,buf);
						SetTextColor(dis->hDC, (dis->itemState&ODS_SELECTED|dis->itemState&ODS_HOTLIGHT)?dat->MenuTextHiColor:dat->MenuTextColor);
						DrawText(dis->hDC,TranslateT("Status"), lstrlen(TranslateT("Status")),&rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
						StatusMenuState=dis->itemState;
					} else {
						StatusMenuState=dis->itemState;
						SkinInvalidateFrame(hwnd,NULL,0);
					}
					return TRUE;
				}

				return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
			}
			return 0;
		}

	case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS * wp;
			wp=(WINDOWPOS *)lParam; 

			//       if (ON_EDGE_SIZING)
			//           wp->flags|=SWP_NOSIZE|SWP_NOMOVE;
			//       ON_EDGE_SIZING=0;
			if (wp->flags&SWP_HIDEWINDOW && ANIMATION_IS_IN_PROGRESS) 
				return 0;               
		
			//if (ON_EDGE_SIZING) wp->flags|=SWP_NOMOVE;
			if (IsOnDesktop)
				wp->flags|=SWP_NOACTIVATE|SWP_NOZORDER;
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	case WM_DESTROY:
		{
			int state=DBGetContactSettingByte(NULL,"CList","State",SETTING_STATE_NORMAL);
			if (state==SETTING_STATE_NORMAL){ShowWindowNew(hwnd,SW_HIDE);};				
			if(hSettingChangedHook!=0){UnhookEvent(hSettingChangedHook);};
			TrayIconDestroy(hwnd);
			DisconnectAll();
			ANIMATION_IS_IN_PROGRESS=0;  		
			SleepEx(100,TRUE);
			CallService(MS_CLIST_FRAMES_REMOVEFRAME,(WPARAM)hFrameContactTree,(LPARAM)0);
			DestroyWindow(pcli->hwndContactTree);
			UnLoadCLUIFramesModule();		
			ImageList_Destroy(himlMirandaIcon);
			DBWriteContactSettingByte(NULL,"CList","State",(BYTE)state);
			UnloadSkin(&glObjectList);
			FreeLibrary(hUserDll);
			pcli->hwndContactList=NULL;
			PostQuitMessage(0);
			return 0;
	}	}

	return saveContactListWndProc( hwnd, msg, wParam, lParam );
}

int CluiIconsChanged(WPARAM wParam,LPARAM lParam)
{
	ImageList_ReplaceIcon(himlMirandaIcon,0,LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
	DrawMenuBar(pcli->hwndContactList);
	ReloadExtraIcons();
	SetAllExtraIcons(pcli->hwndContactTree,0);
	RedrawCompleteWindow();
	//	pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);
	return 0;
}

static HANDLE hRenameMenuItem;
static HANDLE hShowAvatarMenuItem;
static HANDLE hHideAvatarMenuItem;

static int MenuItem_PreBuild(WPARAM wParam, LPARAM lParam) 
{
	TCHAR cls[128];
	HANDLE hItem;
	HWND hwndClist = GetFocus();
	CLISTMENUITEM mi;

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;
	GetClassName(hwndClist,cls,sizeof(cls));
	hwndClist = (!lstrcmp(CLISTCONTROL_CLASS,cls))?hwndClist:pcli->hwndContactList;
	hItem = (HANDLE)SendMessage(hwndClist,CLM_GETSELECTION,0,0);
	if (!hItem) {
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hRenameMenuItem, (LPARAM)&mi);

	if (!hItem || !IsHContactContact(hItem) || !DBGetContactSettingByte(NULL,"CList","ShowAvatars",0)) 
	{
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowAvatarMenuItem, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideAvatarMenuItem, (LPARAM)&mi);
	}
	else
	{
		int has_avatar;

		if (ServiceExists(MS_AV_GETAVATARBITMAP))
		{
			has_avatar = CallService(MS_AV_GETAVATARBITMAP, (WPARAM)hItem, 0);
		}
		else
		{
			DBVARIANT dbv;
			if (DBGetContactSetting(hItem, "ContactPhoto", "File", &dbv))
			{
				has_avatar = 0;
			}
			else
			{
				has_avatar = 1;
				DBFreeVariant(&dbv);
			}
		}

		if (DBGetContactSettingByte(hItem, "CList", "HideContactAvatar", 0))
		{
			mi.flags = CMIM_FLAGS | (has_avatar ? 0 : CMIF_GRAYED);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowAvatarMenuItem, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideAvatarMenuItem, (LPARAM)&mi);
		}
		else
		{
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowAvatarMenuItem, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | (has_avatar ? 0 : CMIF_GRAYED);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideAvatarMenuItem, (LPARAM)&mi);
		}
	}

	return 0;
}

static int MenuItem_ShowContactAvatar(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;

	DBWriteContactSettingByte(hContact, "CList", "HideContactAvatar", 0);

	pcli->pfnClcBroadcast( INTM_AVATARCHANGED,wParam,0);
	return 0;
}

static int MenuItem_HideContactAvatar(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;

	DBWriteContactSettingByte(hContact, "CList", "HideContactAvatar", 1);

	pcli->pfnClcBroadcast( INTM_AVATARCHANGED,wParam,0);
	return 0;
}

int CList_ShowMainMenu (WPARAM w,LPARAM l)
{
	HMENU hMenu;
	POINT pt;
	hMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,pcli->hwndContactList,NULL);				
	return 0;
}
int CList_ShowStatusMenu(WPARAM w,LPARAM l)
{
	HMENU hMenu;
	POINT pt;
	hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,pcli->hwndContactList,NULL);				
	return 0;
}


void LoadCluiGlobalOpts()
{
  BOOL tLayeredFlag=FALSE;
  tLayeredFlag=IsWinVer2000Plus();
  tLayeredFlag&=DBGetContactSettingByte(NULL, "ModernData", "EnableLayering", tLayeredFlag);

  if(tLayeredFlag)
  {
    if (DBGetContactSettingByte(NULL,"CList","WindowShadow",0)==1)
      DBWriteContactSettingByte(NULL,"CList","WindowShadow",2);    
  }
  else
  {
    if (DBGetContactSettingByte(NULL,"CList","WindowShadow",0)==2)
      DBWriteContactSettingByte(NULL,"CList","WindowShadow",1); 
  }
  savedLoadCluiGlobalOpts();
}

void LoadCLUIModule(void)
{
	hFrameContactTree=0;
	TRACE("Load CLUI Module\n");

	hUserDll = LoadLibrary(TEXT("user32.dll"));
	if (hUserDll)
	{
		MyUpdateLayeredWindow = (BOOL (WINAPI *)(HWND,HDC,POINT*,SIZE*,HDC,POINT*,COLORREF,BLENDFUNCTION*,DWORD))GetProcAddress(hUserDll, "UpdateLayeredWindow");
		LayeredFlag=(MyUpdateLayeredWindow!=NULL);
		SmoothAnimation=IsWinVer2000Plus()&&DBGetContactSettingByte(NULL, "CLUI", "FadeInOut", 1);
		LayeredFlag=LayeredFlag*DBGetContactSettingByte(NULL, "ModernData", "EnableLayering", LayeredFlag);
		MySetLayeredWindowAttributesNew = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyAnimateWindow=(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll,"AnimateWindow");
	}
	uMsgProcessProfile=RegisterWindowMessage(TEXT("Miranda::ProcessProfile"));

	// Call InitGroup menus before
	InitGroupMenus();

	HookEvent(ME_SYSTEM_MODULESLOADED,CluiModulesLoaded);
	HookEvent(ME_SKIN_ICONSCHANGED,CluiIconsChanged);
	HookEvent(ME_OPT_INITIALISE,CluiOptInit);

	hContactDraggingEvent=CreateHookableEvent(ME_CLUI_CONTACTDRAGGING);
	hContactDroppedEvent=CreateHookableEvent(ME_CLUI_CONTACTDROPPED);
	hContactDragStopEvent=CreateHookableEvent(ME_CLUI_CONTACTDRAGSTOP);
	LoadCluiServices();

	CreateServiceFunction("CLUI/GetConnectingIconForProtocol",GetConnectingIconService);

	oldhideoffline=DBGetContactSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT);

	PreCreateCLC(pcli->hwndContactList);

	LoadCLUIFramesModule();
	LoadExtraImageFunc();	

	hMenuMain=GetMenu(pcli->hwndContactList);
	ChangeWindowMode();

	{	CLISTMENUITEM mi;
		ZeroMemory(&mi,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.flags=0;
		mi.pszContactOwner=NULL;    //on every contact
		CreateServiceFunction("CList/ShowContactAvatar",MenuItem_ShowContactAvatar);
		mi.position=2000150000;
		mi.hIcon=LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_SHOW_AVATAR));
		mi.pszName=Translate("Show Contact &Avatar");
		mi.pszService="CList/ShowContactAvatar";
		hShowAvatarMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

		CreateServiceFunction("CList/HideContactAvatar",MenuItem_HideContactAvatar);
		mi.position=2000150001;
		mi.hIcon=LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_HIDE_AVATAR));
		mi.pszName=Translate("Hide Contact &Avatar");
		mi.pszService="CList/HideContactAvatar";
		hHideAvatarMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

		HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MenuItem_PreBuild);
	}

	LoadProtocolOrderModule();
	lastreqh=0;
	//CreateServiceFunction("TestService",TestService);
	CreateServiceFunction(MS_CLUI_SHOWMAINMENU,CList_ShowMainMenu);
	CreateServiceFunction(MS_CLUI_SHOWSTATUSMENU,CList_ShowStatusMenu);
	LoadModernButtonModule();
	ReloadCLUIOptions();
	pcli->hwndStatus=(HWND)CreateModernStatusBar(pcli->hwndContactList);
}

int TestCursorOnBorders()
{
	HWND hwnd=pcli->hwndContactList;
	HCURSOR hCurs1=NULL;
	RECT r;  
	POINT pt;
	int k=0, t=0, fx,fy;
	HWND hAux;
	BOOL mouse_in_window=0;
	HWND gf=GetForegroundWindow();
	if(TransparentFlag) {
		if (!transparentFocus && gf!=hwnd) {
			SmoothAlphaTransition(hwnd, DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
			//MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
			transparentFocus=1;
			SetTimer(hwnd, TM_AUTOALPHA,250,NULL);
		}
	}

	GetCursorPos(&pt);
	IgnoreActivation=0;
	GetWindowRect(hwnd,&r);
	hAux = WindowFromPoint(pt);
	while(hAux != NULL) 
	{
		if (hAux == hwnd) {mouse_in_window=1; break;}
		hAux = GetParent(hAux);
	}
	fx=GetSystemMetrics(SM_CXFULLSCREEN);
	fy=GetSystemMetrics(SM_CYFULLSCREEN);
	if (docked || ((pt.x<fx-1) && (pt.y<fy-1) && pt.x>1 && pt.y>1)) // workarounds for behind the edge.
	{
		//ScreenToClient(hwnd,&pt);
		//GetClientRect(hwnd,&r);
		if (pt.y<=r.bottom && pt.y>=r.bottom-SIZING_MARGIN && !DBGetContactSettingByte(NULL,"CLUI","AutoSize",0)) k=6;
		else if (pt.y>=r.top && pt.y<=r.top+SIZING_MARGIN && !DBGetContactSettingByte(NULL,"CLUI","AutoSize",0)) k=3;
		if (pt.x<=r.right && pt.x>=r.right-SIZING_MARGIN) k+=2;
		else if (pt.x>=r.left && pt.x<=r.left+SIZING_MARGIN)k+=1;
		if (!(pt.x>=r.left && pt.x<=r.right && pt.y>=r.top && pt.y<=r.bottom)) k=0;
		k*=mouse_in_window;
		hCurs1 = LoadCursor(NULL, IDC_ARROW);
		if(BehindEdge_State<=0)
			switch(k)
		{
			case 1: 
			case 2:
				if(!docked||(docked==2 && k==1)||(docked==1 && k==2)){hCurs1 = LoadCursor(NULL, IDC_SIZEWE); break;}
			case 3: if(!docked) {hCurs1 = LoadCursor(NULL, IDC_SIZENS); break;}
			case 4: if(!docked) {hCurs1 = LoadCursor(NULL, IDC_SIZENWSE); break;}
			case 5: if(!docked) {hCurs1 = LoadCursor(NULL, IDC_SIZENESW); break;}
			case 6: if(!docked) {hCurs1 = LoadCursor(NULL, IDC_SIZENS); break;}
			case 7: if(!docked) {hCurs1 = LoadCursor(NULL, IDC_SIZENESW); break;}
			case 8: if(!docked) {hCurs1 = LoadCursor(NULL, IDC_SIZENWSE); break;}
		}
		if (hCurs1) SetCursor(hCurs1);
		return k;
	}

	return 0;
}

int SizingOnBorder(POINT pt, int PerformSize)
{
	if (!(DBGetContactSettingByte(NULL,"CLUI","LockSize",0)))
	{
		RECT r;
		HWND hwnd=pcli->hwndContactList;
		int k=0;
		GetWindowRect(hwnd,&r);
		//ScreenToClient(hwnd,&pt);
		if (pt.y<=r.bottom && pt.y>=r.bottom-SIZING_MARGIN && !DBGetContactSettingByte(NULL,"CLUI","AutoSize",0)) k=6;
		else if (pt.y>=r.top && pt.y<=r.top+SIZING_MARGIN && !DBGetContactSettingByte(NULL,"CLUI","AutoSize",0)) k=3;
		if (pt.x<=r.right && pt.x>=r.right-SIZING_MARGIN) k+=2;
		else if (pt.x>=r.left && pt.x<=r.left+SIZING_MARGIN)k+=1;
		if (!(pt.x>=r.left && pt.x<=r.right && pt.y>=r.top && pt.y<=r.bottom)) k=0;
		//ClientToScreen(hwnd,&pt);
		if (k && PerformSize) 
		{ 
			ReleaseCapture();
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE+k,MAKELPARAM(pt.x,pt.y));					
			return k;
		}
		else return k;
	}
	return 0;
}
extern void ApplyTransluency();
void SmoothAnimationThread(HWND hwnd)
{
	//  return;
	if (!ANIMATION_IS_IN_PROGRESS) return;  /// Should be some locked to avoid painting against contact deletion.
	do
	{
		if (!LOCK_UPDATING)
		{
			SmoothAlphaThreadTransition(hwnd);       
			SleepEx(20,TRUE);
			if (Miranda_Terminated()) return;
		}
		else SleepEx(0,TRUE);

	} while (ANIMATION_IS_IN_PROGRESS);
	return;
}
#define ANIMATION_STEP 20

int SmoothAlphaThreadTransition(HWND hwnd)
{
	int step;
	int a;
	step=(CURRENT_ALPHA>alphaEnd)?-1*ANIMATION_STEP:ANIMATION_STEP;
	a=CURRENT_ALPHA+step;
	if ((step>=0 && a>=alphaEnd) || (step<=0 && a<=alphaEnd))
	{
		ANIMATION_IS_IN_PROGRESS=0;
		CURRENT_ALPHA=alphaEnd;
		if (CURRENT_ALPHA==0)
		{			
			CURRENT_ALPHA=1;
			JustUpdateWindowImage();
			SHOWHIDE_CALLED_FROM_ANIMATION=1;             
			ShowWindowNew(pcli->hwndContactList,0);
			OnShowHide(hwnd,0);
			SHOWHIDE_CALLED_FROM_ANIMATION=0;
			CURRENT_ALPHA=0;
			if (!LayeredFlag) RedrawWindow(pcli->hwndContactList,NULL,NULL,RDW_ERASE|RDW_FRAME);
			return 0;			
		}
	}
	else   CURRENT_ALPHA=a;   
	JustUpdateWindowImage();     
	return 1;
}

int SmoothAlphaTransition(HWND hwnd, BYTE GoalAlpha, BOOL wParam)
{ 

	if ((!LayeredFlag
		&& (!SmoothAnimation && !TransparentFlag))||!MySetLayeredWindowAttributesNew)
	{
		if (GoalAlpha>0 && wParam!=2)
		{
			if (!IsWindowVisible(hwnd))
			{
				SHOWHIDE_CALLED_FROM_ANIMATION=1;
				ShowWindowNew(pcli->hwndContactList,SW_RESTORE);
				OnShowHide(hwnd,1);
				SHOWHIDE_CALLED_FROM_ANIMATION=0;
				CURRENT_ALPHA=GoalAlpha;
				UpdateWindowImage();

			}
		}
		else if (GoalAlpha==0 && wParam!=2)
		{
			if (IsWindowVisible(hwnd))
			{
				SHOWHIDE_CALLED_FROM_ANIMATION=1;
				ShowWindowNew(pcli->hwndContactList,0);
				OnShowHide(hwnd,0);
				CURRENT_ALPHA=GoalAlpha;
				SHOWHIDE_CALLED_FROM_ANIMATION=0;

			}
		}
		return 0;
	}
	//TRACE("Transition---\n");
	if (CURRENT_ALPHA==GoalAlpha &&0)
	{
		if (ANIMATION_IS_IN_PROGRESS)
		{
			//TRACE("- -KillTimer\n");
			KillTimer(hwnd,TM_SMOTHALPHATRANSITION);
			ANIMATION_IS_IN_PROGRESS=0;
		}
		return 0;
	}
	if (SHOWHIDE_CALLED_FROM_ANIMATION) return 0;
	if (wParam!=2)  //not from timer
	{   
		alphaEnd=GoalAlpha;
		if (!ANIMATION_IS_IN_PROGRESS)// && alphaEnd!=CURRENT_ALPHA)
		{

			if ((!IsWindowVisible(hwnd)||CURRENT_ALPHA==0) && alphaEnd>0 )
			{
				SHOWHIDE_CALLED_FROM_ANIMATION=1;            
				ShowWindowNew(pcli->hwndContactList,SW_SHOWNA);			 
				OnShowHide(hwnd,SW_SHOW);
				SHOWHIDE_CALLED_FROM_ANIMATION=0;
				CURRENT_ALPHA=1;
				UpdateWindowImage();
			}
			if (IsWindowVisible(hwnd))
			{
				ANIMATION_IS_IN_PROGRESS=1;
				//TRACE("- -StartTimer\n");
				//			 if (!ALREADYACTIVE)
				if (SmoothAnimation)
				{         
					forkthread(SmoothAnimationThread,0,pcli->hwndContactList);	
				}
				//SetTimer(hwnd,TM_SMOTHALPHATRANSITION,20,NULL); 
			}
		}
	}

	{
		int step;
		int a;
		step=(CURRENT_ALPHA>alphaEnd)?-1*ANIMATION_STEP:ANIMATION_STEP;
		a=CURRENT_ALPHA+step;
		if ((step>=0 && a>=alphaEnd) || (step<=0 && a<=alphaEnd) || CURRENT_ALPHA==alphaEnd || !SmoothAnimation) //stop animation;
		{
			KillTimer(hwnd,TM_SMOTHALPHATRANSITION);
			ANIMATION_IS_IN_PROGRESS=0;
			if (alphaEnd==0) 
			{
				CURRENT_ALPHA=1;
				UpdateWindowImage();
				SHOWHIDE_CALLED_FROM_ANIMATION=1;             
				ShowWindowNew(pcli->hwndContactList,0);
				OnShowHide(pcli->hwndContactList,0);
				SHOWHIDE_CALLED_FROM_ANIMATION=0;
				CURRENT_ALPHA=0;
			}
			else
			{
				CURRENT_ALPHA=alphaEnd;
				UpdateWindowImage();
			}
		}
		else
		{
			CURRENT_ALPHA=a;
			UpdateWindowImage();
		}
	}

	return 0;
}
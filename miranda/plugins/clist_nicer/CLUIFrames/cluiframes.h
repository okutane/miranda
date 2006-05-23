/*
Miranda ICQ: the free icq client for MS Windows 
Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

int LoadCLUIFramesModule(void);
int UnLoadCLUIFramesModule(void);
int CLUIFramesGetMinHeight();
int CLUIFramesOnClistResize(WPARAM wParam,LPARAM lParam);
int SizeFramesByWindowRect(RECT *r, HDWP * PosBatch);
int CLUIFramesResizeFrames(RECT *rc);

typedef struct tagProtocolData {
	char *RealName;
	int protopos;
    int statusbarpos;
	boolean show;
} ProtocolData;

#define OFFSET_PROTOPOS 200
#define OFFSET_VISIBLE 400

#define CLUIFrameTitleBarClassName				"CLUIFrameTitleBar"
#define CLUIFrameModule							"CLUIFrames"

//integrated menu module
#define MS_INT_MENUMEASUREITEM "CLUIFrames/IntMenuMeasureItem"
#define MS_INT_MENUDRAWITEM "CLUIFrames/IntMenuDrawItem"
#define MS_INT_MENUPROCESSCOMMAND "CLUIFrames/IntMenuProcessCommand"
#define MS_INT_MODIFYMENUITEM "CLUIFrames/IntModifyMenuItem"

#define DEFAULT_TITLEBAR_HEIGHT		18

typedef struct _DockOpt {
    HWND    hwndLeft;
    HWND    hwndRight;
}
DockOpt;

typedef struct tagMenuHandles {
    HANDLE MainMenuItem;
    HANDLE MIVisible,MITitle,MITBVisible,MILock,MIColl,MIFloating,MIAlignRoot;
    HANDLE MIAlignTop,MIAlignClient,MIAlignBottom;
    HANDLE MIBorder, MISkinned;
} FrameMenuHandles;

typedef struct tagFrameTitleBar {
    HWND hwnd;
    HWND TitleBarbutt;
    HWND hwndTip;

    char *tbname;
    char *tooltip;
    HMENU hmenu;
    HICON hicon;

    BOOLEAN ShowTitleBar;
    BOOLEAN ShowTitleBarTip;
    COLORREF BackColour;
    COLORREF TextColour;
    int oldstyles;
    POINT oldpos;
    RECT wndSize;
} FrameTitleBar;

typedef struct {
    int id;
    HWND hWnd ;
    RECT wndSize;
    char *name;
    int align;
    int height;
    int dwFlags;
    BOOLEAN Locked;
    BOOLEAN Skinned;
    BOOLEAN visible;
    BOOLEAN needhide;
    BOOLEAN collapsed;
    int prevvisframe;
    int HeightWhenCollapsed;
    FrameTitleBar TitleBar;
    FrameMenuHandles MenuHandles;
    int oldstyles;
    BOOLEAN floating;
    HWND ContainerWnd;
    POINT FloatingPos;
    POINT FloatingSize;
    BOOLEAN minmaxenabled;
    BOOLEAN UseBorder;
    int order;
    DockOpt dockOpt;
    HWND OwnerWindow;
    WNDPROC wndProc;
} wndFrame;


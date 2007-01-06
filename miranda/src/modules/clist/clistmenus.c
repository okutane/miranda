/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project,
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

#include "genmenu.h"
#include "m_clui.h"
#pragma hdrstop

#define FIRSTCUSTOMMENUITEMID	30000
#define MENU_CUSTOMITEMMAIN		0x80000000
//#define MENU_CUSTOMITEMCONTEXT	0x40000000
//#define MENU_CUSTOMITEMFRAME	0x20000000

typedef struct  {
	WORD id;
	int iconId;
	CLISTMENUITEM mi;
}
	CListIntMenuItem,*lpCListIntMenuItem;

//new menu sys
static int hMainMenuObject;
static int hContactMenuObject;
static int hStatusMenuObject;
int MenuModulesLoaded(WPARAM,LPARAM);

int statustopos(int status);
//
//HIMAGELIST hImlMenuIcons;

//static HANDLE hPreBuildContactMenuEvent;
static HANDLE hPreBuildMainMenuEvent,hStatusModeChangeEvent;

static HANDLE hPreBuildContactMenuEvent,hPreBuildStatusMenuEvent,hAckHook;

static HMENU hMainMenu,hStatusMenu = 0;
int currentStatusMenuItem,currentDesiredStatusMode;
static int statusModeList[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_ONTHEPHONE,ID_STATUS_OUTTOLUNCH};
static int skinIconStatusList[]={SKINICON_STATUS_OFFLINE,SKINICON_STATUS_ONLINE,SKINICON_STATUS_AWAY,SKINICON_STATUS_NA,SKINICON_STATUS_OCCUPIED,SKINICON_STATUS_DND,SKINICON_STATUS_FREE4CHAT,SKINICON_STATUS_INVISIBLE,SKINICON_STATUS_ONTHEPHONE,SKINICON_STATUS_OUTTOLUNCH};
static int statusModePf2List[]={0xFFFFFFFF,PF2_ONLINE,PF2_SHORTAWAY,PF2_LONGAWAY,PF2_LIGHTDND,PF2_HEAVYDND,PF2_FREECHAT,PF2_INVISIBLE,PF2_ONTHEPHONE,PF2_OUTTOLUNCH};

HANDLE *hStatusMainMenuHandles;
int hStatusMainMenuHandlesCnt;
typedef struct
{
	int protoindex;
	int protostatus[sizeof(statusModeList)];
	int menuhandle[sizeof(statusModeList)];
}
tStatusMenuHandles,*lpStatusMenuHandles;

lpStatusMenuHandles hStatusMenuHandles;
int hStatusMenuHandlesCnt;



//mainmenu exec param(ownerdata)
typedef struct{
	char *szServiceName;
	TCHAR *szMenuName;
	int Param1;
}
MainMenuExecParam,*lpMainMenuExecParam;
//


//contactmenu exec param(ownerdata)
//also used in checkservice
typedef struct{
	char *szServiceName;
	char *pszContactOwner;//for check proc
	int param;
}
ContactMenuExecParam,*lpContactMenuExecParam;

typedef struct {
	char *szProto;
	int isOnList;
	int isOnline;
}
BuildContactParam;

typedef struct{
	char *proto;
	int protoindex;
	int status;

	BOOL custom;
	char *svc;
	int hMenuItem;
}
StatusMenuExecParam,*lpStatusMenuExecParam;

//////////////////////////////MAIN MENU/////////////////////////

/*
wparam=handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/
static int RemoveMainMenuItem(WPARAM wParam,LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM,wParam,0);
	return 0;
}


static int BuildMainMenu(WPARAM wParam,LPARAM lParam)
{
	int tick;
	HMENU hMenu;
	ListParam param;

	memset(&param,0,sizeof(param));
	param.MenuObjectHandle=hMainMenuObject;
	param.rootlevel=-1;

	hMenu=hMainMenu;
	tick=GetTickCount();
	NotifyEventHooks(hPreBuildMainMenuEvent,(WPARAM)0,(LPARAM)0);

	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	DrawMenuBar((HWND)CallService("CLUI/GetHwnd",(WPARAM)0,(LPARAM)0));
	tick=GetTickCount()-tick;
	return (int)hMenu;
}

static int AddMainMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;

	if ( mi->cbSize == sizeof( CLISTMENUITEM )) {
		TMO_MenuItem tmi = { 0 };
		tmi.cbSize   = sizeof(tmi);
		tmi.flags    = mi->flags;
		tmi.hIcon    = mi->hIcon;
		tmi.hotKey   = mi->hotKey;
		tmi.ptszName = TranslateTS(mi->ptszName);
		tmi.position = mi->position;

		//pszPopupName for new system mean root level
		//pszPopupName for old system mean that exists popup
		tmi.root = (int)mi->pszPopupName;
		{	
			lpMainMenuExecParam mmep;
			mmep = ( lpMainMenuExecParam )mir_alloc( sizeof( MainMenuExecParam ));
			if ( mmep == NULL )
				return 0;

			//we need just one parametr.
			mmep->szServiceName = mir_strdup(mi->pszService);
			mmep->Param1 = mi->popupPosition;
			mmep->szMenuName = tmi.ptszName;
			tmi.ownerdata=mmep;
		}
		{
			OptParam op;
			op.Handle=CallService(MO_ADDNEWMENUITEM,(WPARAM)hMainMenuObject,(LPARAM)&tmi);
			op.Setting=OPT_MENUITEMSETUNIQNAME;
			op.Value=(int)mi->pszService;
			CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
			return op.Handle;
		}
	}
	else return 0;
}

int MainMenuCheckService(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
int MainMenuExecService(WPARAM wParam,LPARAM lParam)
{
	if (wParam!=0)
	{
		lpMainMenuExecParam mmep=(lpMainMenuExecParam)wParam;
		if (!lstrcmpA(mmep->szServiceName,"Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1=0;
		}
		CallService(mmep->szServiceName,mmep->Param1,lParam);


	}
	return 1;
}

int FreeOwnerDataMainMenu (WPARAM wParam,LPARAM lParam)
{
	lpMainMenuExecParam mmep;
	mmep=(lpMainMenuExecParam)lParam;
	if (mmep!=NULL){
		FreeAndNil(&mmep->szServiceName);
		FreeAndNil(&mmep);
	}
	return 0;
}

//////////////////////////////END MAIN MENU/////////////////////////



//////////////////////////////CONTACT MENU/////////////////////////

static int RemoveContactMenuItem(WPARAM wParam,LPARAM lParam)
{
	//identical

	CallService(MO_REMOVEMENUITEM,wParam,0);
	return 0;
}

static int AddContactMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	TMO_MenuItem tmi;
	OptParam op;

	if ( mi->cbSize != sizeof( CLISTMENUITEM ))
		return 0;

	memset(&tmi,0,sizeof(tmi));
	tmi.cbSize=sizeof(tmi);
	tmi.flags=mi->flags;
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.position=mi->position;
	tmi.ptszName = mi->ptszName;
	tmi.root=(int)mi->pszPopupName;

	if (!(mi->flags&CMIF_ROOTPOPUP||mi->flags&CMIF_CHILDPOPUP)) {
		//old system
		tmi.flags|=CMIF_CHILDPOPUP;
		tmi.root=-1;
	}

	{
		//owner data
		lpContactMenuExecParam cmep;
		cmep=(lpContactMenuExecParam)mir_alloc(sizeof(ContactMenuExecParam));
		memset(cmep,0,sizeof(ContactMenuExecParam));
		cmep->szServiceName=mir_strdup(mi->pszService);
		if (mi->pszContactOwner!=NULL) cmep->pszContactOwner=mir_strdup(mi->pszContactOwner);
		cmep->param=mi->popupPosition;
		tmi.ownerdata=cmep;
	}

	op.Handle=(int)CallService(MO_ADDNEWMENUITEM,(WPARAM)hContactMenuObject,(LPARAM)&tmi);
	op.Setting=OPT_MENUITEMSETUNIQNAME;
	{
		char buf[256];
		sprintf(buf,"%s/%s",mi->pszContactOwner?mi->pszContactOwner:"",mi->pszService?mi->pszService:"");
		op.Value=(int)buf;
		CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
	}
	return op.Handle;
}

static int BuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	HMENU hMenu;
	int isOnline,isOnList;
	HANDLE hContact=(HANDLE)wParam;
	char *szProto;
	BuildContactParam bcp;
	ListParam param;

	NotifyEventHooks(hPreBuildContactMenuEvent,(WPARAM)hContact,0);

	szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	isOnList=0==DBGetContactSettingByte(hContact,"CList","NotOnList",0);
	isOnline=szProto!=NULL && ID_STATUS_OFFLINE!=DBGetContactSettingWord(hContact,szProto,"Status",ID_STATUS_OFFLINE);

	bcp.szProto=szProto;
	bcp.isOnList=isOnList;
	bcp.isOnline=isOnline;

	memset(&param,0,sizeof(param));

	param.MenuObjectHandle=hContactMenuObject;
	param.rootlevel=-1;
	param.wParam=(WPARAM)&bcp;

	hMenu=CreatePopupMenu();
	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);

	return (int)hMenu;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
int ContactMenuExecService(WPARAM wParam,LPARAM lParam) {
	if (wParam!=0) {
		lpContactMenuExecParam cmep=(lpContactMenuExecParam)wParam;
		//call with wParam=(WPARAM)(HANDLE)hContact,lparam=popupposition
		CallService(cmep->szServiceName,lParam,cmep->param);
	}
	return 0;
}

//true - ok,false ignore
int ContactMenuCheckService(WPARAM wParam,LPARAM lParam) {

	PCheckProcParam pcpp=(PCheckProcParam)wParam;
	BuildContactParam *bcp=NULL;
	lpContactMenuExecParam cmep=NULL;
	TMO_MenuItem mi;

	if (pcpp==NULL)
		return FALSE;

	bcp=(BuildContactParam *)pcpp->wParam;
	if (bcp==NULL)
		return FALSE;

	cmep=pcpp->MenuItemOwnerData;
	if (cmep==NULL) //this is root...build it
		return TRUE;

	if (cmep->pszContactOwner!=NULL) {
		if (bcp->szProto==NULL) return FALSE;
		if (strcmp(cmep->pszContactOwner,bcp->szProto)) return FALSE;
	}
	if (MO_GetMenuItem((WPARAM)pcpp->MenuItemHandle,(LPARAM)&mi)==0) {
		if (mi.flags&CMIF_HIDDEN) return FALSE;
		if (mi.flags&CMIF_NOTONLIST && bcp->isOnList) return FALSE;
		if (mi.flags&CMIF_NOTOFFLIST && !(bcp->isOnList)) return FALSE;
		if (mi.flags&CMIF_NOTONLINE && (bcp->isOnline)) return FALSE;
		if (mi.flags&CMIF_NOTOFFLINE && !(bcp->isOnline)) return FALSE;
	}
	return TRUE;
}

int FreeOwnerDataContactMenu (WPARAM wParam,LPARAM lParam)
{

	lpContactMenuExecParam cmep;

	cmep=(lpContactMenuExecParam)lParam;
	if (cmep!=NULL) {
		FreeAndNil(&cmep->szServiceName);
		FreeAndNil(&cmep->pszContactOwner);
		FreeAndNil(&cmep);
	}
	return 0;
}


//////////////////////////////END CONTACT MENU/////////////////////////


//wparam MenuItemHandle
static int ModifyCustomMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	TMO_MenuItem tmi;

	if (lParam==0) return -1;
	if (mi->cbSize!=sizeof(CLISTMENUITEM)) return 1;

	tmi.cbSize=sizeof(tmi);
	tmi.flags=mi->flags;
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.ptszName=mi->ptszName;
	//todo new flags for this parametrs
	//tmi.ownerdata
	//tmi.position
	return CallService(MO_MODIFYMENUITEM,wParam,(LPARAM)&tmi);
}

int MenuProcessCommand(WPARAM wParam,LPARAM lParam)
{
	//process old menu sys
	if ( HIWORD(wParam) & MPCF_CONTACTMENU ) {
		//make faked globalid
		return CallService( MO_PROCESSCOMMAND, getGlobalId( hContactMenuObject, LOWORD(wParam)), lParam );
	}

	if ( HIWORD(wParam) & MPCF_MAINMENU ) {
		int hst = LOWORD( wParam );
		if ( hst >= ID_STATUS_OFFLINE && hst <= ID_STATUS_OUTTOLUNCH ) {
			int pos = statustopos( hst );
			if ( pos != -1 && hStatusMainMenuHandles != NULL )
				return CallService( MO_PROCESSCOMMAND, ( WPARAM )hStatusMainMenuHandles[ pos ], lParam );
	}	}
	
	//unknown old menu
	return CallService(MO_PROCESSCOMMANDBYMENUIDENT,LOWORD(wParam),lParam);
}

char* GetUniqueProtoName(char * proto)
{
	int i, count;
	PROTOCOLDESCRIPTOR **protos;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM) & count, (LPARAM) & protos);
	for (i=0; i<count; i++)
		if (!_strcmpi(proto,protos[i]->szName))
			return protos[i]->szName;
	return NULL;
}

typedef struct _MenuItemHandles
{
	HMENU OwnerMenu;
	int position;
}
MenuItemData;

BOOL __inline wildcmp(char * name, char * mask, BYTE option)
{
	char * last='\0';
	for(;; mask++, name++)
	{
		if(*mask != '?' && *mask != *name) break;
		if(*name == '\0') return ((BOOL)!*mask);
	}
	if(*mask != '*') return FALSE;
	for(;; mask++, name++)
	{
		while(*mask == '*')
		{
			last = mask++;
			if(*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if(*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if(*mask != '?' && *mask != *name) name -= (size_t)(mask - last) - 1, mask = last;
	}
}

BOOL FindMenuHanleByGlobalID(HMENU hMenu, int globalID, MenuItemData* itdat)
{
	int i;
	PMO_IntMenuItem pimi;	
	MENUITEMINFO mii = {0};
	BOOL inSub=FALSE;

	if ( !itdat )
		return FALSE;

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU | MIIM_DATA;
	for ( i = GetMenuItemCount( hMenu )-1; i >= 0; i-- ) {
		GetMenuItemInfo( hMenu, i, TRUE, &mii );
		if ( mii.fType == MFT_SEPARATOR )
			continue;

		if ( mii.hSubMenu ) 
			inSub = FindMenuHanleByGlobalID( mii.hSubMenu, globalID, itdat );
		if (inSub)
			return inSub;

		pimi = MO_GetIntMenuItem(mii.dwItemData);
		if ( pimi != NULL ) {	
			if ( pimi->globalid == globalID ) {
				itdat->OwnerMenu = hMenu;
				itdat->position = i;
				return TRUE;
	}	}	}

	return FALSE;
}

int FreeOwnerDataStatusMenu (WPARAM wParam,LPARAM lParam)
{
	lpStatusMenuExecParam smep;
	smep=(lpStatusMenuExecParam)lParam;

	if (smep!=NULL) {
		FreeAndNil(&smep->proto);
		FreeAndNil(&smep->svc);
		FreeAndNil(&smep);
	}
	return (0);
}

static int MenuProcessHotkey(WPARAM vKey,LPARAM lParam)
{
	int res;
	res=CallService(MO_PROCESSHOTKEYS,(WPARAM)hStatusMenuObject,(LPARAM)vKey);
	if (res) return res;
	res=CallService(MO_PROCESSHOTKEYS,(WPARAM)hMainMenuObject,(LPARAM)vKey);
	if (res) return res;
	return 0;
}

static int MenuIconsChanged(WPARAM wParam,LPARAM lParam)
{
	//just rebuild menu
	MenuModulesLoaded(0,0);
	return 0;
}

static int MeasureMenuItem(WPARAM wParam,LPARAM lParam)
{
	return CallService(MO_MEASUREMENUITEM,(WPARAM)0,lParam);
}

static int DrawMenuItem(WPARAM wParam,LPARAM lParam)
{
	return CallService(MO_DRAWMENUITEM,(WPARAM)0,lParam);
}

int RecursiveDeleteMenu(HMENU hMenu)
{
	int i=GetMenuItemCount(hMenu);
	while (i>0)
	{
		HMENU submenu=GetSubMenu(hMenu,0);
		if (submenu)
		{
			RecursiveDeleteMenu(submenu);
			DestroyMenu(submenu);
		}
		DeleteMenu(hMenu,0,MF_BYPOSITION);
		i=GetMenuItemCount(hMenu);
	}
	return 0;
}

static int MenuGetMain(WPARAM wParam,LPARAM lParam)
{
	RecursiveDeleteMenu(hMainMenu);
	BuildMainMenu(0,0);
	return (int)hMainMenu;
}

static int BuildStatusMenu(WPARAM wParam,LPARAM lParam)
{
	HMENU hMenu;
	ListParam param;

	memset(&param,0,sizeof(param));
	param.MenuObjectHandle=hStatusMenuObject;
	param.rootlevel=-1;

	hMenu=hStatusMenu;
	RecursiveDeleteMenu(hStatusMenu);
	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	return (int)hMenu;
}

static int SetStatusMode(WPARAM wParam, LPARAM lParam)
{
	MenuProcessCommand(MAKEWPARAM(LOWORD(wParam), MPCF_MAINMENU), 0);
	return 0;
}

static int MenuGetStatus(WPARAM wParam,LPARAM lParam)
{
	return BuildStatusMenu(0,0);
}

int GetProtocolVisibility(char *ProtoName)
{
	int i;
	int res=0;
	DBVARIANT dbv;
	char buf2[10];
	int count;

	if(ProtoName == NULL)
		return 0;

	count = (int)DBGetContactSettingDword(0, "Protocols", "ProtoCount", -1);
	if (count == -1)
		return 1;
	for (i = 0; i < count; i++) {
		_itoa(i, buf2, 10);
		if (!DBGetContactSetting(NULL, "Protocols", buf2, &dbv)) {
			if (strcmp(ProtoName, dbv.pszVal) == 0) {
				mir_free(dbv.pszVal);
				_itoa(i + 400, buf2, 10);
				res= DBGetContactSettingDword(NULL, "Protocols", buf2, 0);
				return res;
			}
			mir_free(dbv.pszVal);
		}
	}
	return 0;
}

int GetProtoIndexByPos(PROTOCOLDESCRIPTOR ** proto, int protoCnt, int Pos)
{
	int p;
	char buf[10];
	DBVARIANT dbv;

	_itoa( Pos, buf, 10 );
	if ( !DBGetContactSetting( NULL, "Protocols", buf, &dbv )) {
		for ( p=0; p < protoCnt; p++ ) {
			if ( lstrcmpA( proto[p]->szName, dbv.pszVal ) == 0 ) {
				DBFreeVariant( &dbv );
				return p;
		}	}

		DBFreeVariant( &dbv );
	}	

	return -1;
}

int AllocedProtos=0;
MenuProto* menusProto=NULL;
MenuProto  menusProtoSingle={0};
int MenuModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	int i,j,protoCount=0,networkProtoCount,s;
	int storedProtoCount;
	int visnetworkProtoCount=0;
	PROTOCOLDESCRIPTOR **proto;
	DWORD statusFlags=0,flags;
	TMO_MenuItem tmi;
	TMenuParam tmp;
	int pos=0;
	OptParam op;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&protoCount,(LPARAM)&proto);
	networkProtoCount=0;

	//clear statusmenu
	while ( GetMenuItemCount(hStatusMenu) > 0 )
		DeleteMenu(hStatusMenu,0,MF_BYPOSITION);

	//status menu
	if (hStatusMenuObject!=0) {
		CallService(MO_REMOVEMENUOBJECT,hStatusMenuObject,0);
		if (hStatusMainMenuHandles!=NULL)
			mir_free(hStatusMainMenuHandles);

		if (hStatusMenuHandles!=NULL)
			mir_free(hStatusMenuHandles);
	}
	memset(&tmp,0,sizeof(tmp));
	tmp.cbSize=sizeof(tmp);
	tmp.ExecService="StatusMenuExecService";
	tmp.CheckService="StatusMenuCheckService";
	//tmp.
	tmp.name=Translate("StatusMenu");

	hStatusMenuObject=(int)CallService(MO_CREATENEWMENUOBJECT,(WPARAM)0,(LPARAM)&tmp);
	{
		op.Handle=hStatusMenuObject;
		op.Setting=OPT_MENUOBJECT_SET_FREE_SERVICE;
		op.Value=(int)"CLISTMENUS/FreeOwnerDataStatusMenu";
		CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

		op.Handle=hStatusMenuObject;
		op.Setting=OPT_USERDEFINEDITEMS;
		op.Value=(int)TRUE;
		CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);
	}

	hStatusMainMenuHandles=(HANDLE*)mir_alloc(sizeof(statusModeList));
	hStatusMainMenuHandlesCnt = SIZEOF(statusModeList);
	for (i=0;i<protoCount;i++)
		if (proto[i]->type==PROTOTYPE_PROTOCOL)
			networkProtoCount++;

	memset(hStatusMainMenuHandles,0,sizeof(statusModeList));
	hStatusMenuHandles=(tStatusMenuHandles*)mir_alloc(sizeof(tStatusMenuHandles)*protoCount);
	hStatusMenuHandlesCnt=protoCount;

	memset(hStatusMenuHandles,0,sizeof(tStatusMenuHandles)*protoCount);
	storedProtoCount=DBGetContactSettingDword(0,"Protocols","ProtoCount",-1);
	memset(&menusProtoSingle,0,sizeof(MenuProto));
	menusProtoSingle.menuID=(HANDLE)-1;

	if (menusProto)  //free menusProto
	{
		int i;
		for (i=0; i<AllocedProtos;i++)
			if (menusProto[i].szProto) mir_free (menusProto[i].szProto);
		mir_free(menusProto);
		menusProto=NULL;
		AllocedProtos=0;
	}

	for (s=0;s<storedProtoCount;s++)
	{
		i=GetProtoIndexByPos(proto,protoCount,s);
		if (i==-1) continue;
		if (!((proto[i]->type!=PROTOTYPE_PROTOCOL) || (DBGetContactSettingByte(NULL,"CLUI","DontHideStatusMenu",0)==0&&GetProtocolVisibility(proto[i]->szName)==0)))  visnetworkProtoCount++;
	}

	for(s=0;s<storedProtoCount;s++) {
		pos=0;
		i=GetProtoIndexByPos(proto,protoCount,s);
		if (i==-1) continue;
		if ((proto[i]->type!=PROTOTYPE_PROTOCOL) || (DBGetContactSettingByte(NULL,"CLUI","DontHideStatusMenu",0)==0&&GetProtocolVisibility(proto[i]->szName)==0)) continue;

		flags=CallProtoService(proto[i]->szName,PS_GETCAPS,PFLAGNUM_2,0);
		if (visnetworkProtoCount>1) {
			char protoName[128];
			int j;
			int rootmenu;
			HICON ic;
			//adding root
			memset(&tmi,0,sizeof(tmi));
			memset(protoName,0,128);
			tmi.cbSize=sizeof(tmi);
			tmi.flags=CMIF_CHILDPOPUP|CMIF_ROOTPOPUP;
			tmi.position=pos++;
			tmi.hIcon=(HICON)CallProtoService(proto[i]->szName,PS_LOADICON,PLI_PROTOCOL|PLIF_SMALL,0);
			ic=tmi.hIcon;
			tmi.root=-1;

			CallProtoService(proto[i]->szName,PS_GETNAME,sizeof(protoName),(LPARAM)protoName);
			tmi.pszName=protoName;
			rootmenu=CallService(MO_ADDNEWMENUITEM,(WPARAM)hStatusMenuObject,(LPARAM)&tmi);
			memset(&tmi,0,sizeof(tmi));
			tmi.cbSize=sizeof(tmi);
			tmi.flags=CMIF_CHILDPOPUP;
			//if (statusModeList[j]==ID_STATUS_OFFLINE){tmi.flags|=CMIF_CHECKED;};
			tmi.root=rootmenu;
			tmi.position=pos++;
			tmi.pszName=protoName;
			{
				//owner data
				lpStatusMenuExecParam smep;
				smep=(lpStatusMenuExecParam)mir_alloc(sizeof(StatusMenuExecParam));
				smep->custom = FALSE;
				smep->status=0;
				smep->svc=NULL;
				smep->protoindex=0;
				smep->proto=mir_strdup(protoName);
				tmi.ownerdata=smep;
			};

			tmi.hIcon=ic;
			if (DBGetContactSettingByte(NULL,GetUniqueProtoName(protoName),"LockMainStatus",0)) tmi.flags|=CMIF_CHECKED;
			if (tmi.flags&CMIF_CHECKED)
			{
				char buf[256];
				_snprintf(buf,sizeof(buf),Translate("%s (locked)"),protoName);
				tmi.pszName=mir_strdup(buf);
			}

			op.Handle=CallService(MO_ADDNEWMENUITEM,(WPARAM)hStatusMenuObject,(LPARAM)&tmi);
			((lpStatusMenuExecParam)tmi.ownerdata)->protoindex=(int)op.Handle;
			CallService(MO_MODIFYMENUITEM,(WPARAM)op.Handle,(LPARAM)&tmi);
			if (tmi.flags&CMIF_CHECKED)
			{
				mir_free(tmi.pszName);
			}
			{
				if (menusProto)
					menusProto=(MenuProto*)mir_realloc(menusProto,sizeof(MenuProto)*(AllocedProtos+1));
				else
					menusProto=(MenuProto*)mir_alloc(sizeof(MenuProto));
				memset(&(menusProto[AllocedProtos]),0,sizeof(MenuProto));
				menusProto[AllocedProtos].menuID=(HANDLE)rootmenu;
				menusProto[AllocedProtos].szProto=mir_strdup(protoName);
				AllocedProtos++;
			}
			{
				char buf[256];
				sprintf(buf,"RootProtocolIcon_%s",proto[i]->szName);
				op.Value=(int)buf;
				op.Setting=OPT_MENUITEMSETUNIQNAME;
				CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
			}
			DestroyIcon(tmi.hIcon);
			pos+=100000;

			for(j=0; j < SIZEOF(statusModeList); j++) {
				if (!(flags&statusModePf2List[j])) {
					//	DeleteMenu(hMenu,statusModeList[j],MF_BYCOMMAND);
				}
				else {

					char *st;
					st=(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,statusModeList[j],0);
					//adding
					memset(&tmi,0,sizeof(tmi));
					tmi.cbSize=sizeof(tmi);
					tmi.flags=CMIF_CHILDPOPUP;
					if (statusModeList[j]==ID_STATUS_OFFLINE)
						tmi.flags|=CMIF_CHECKED;
					tmi.root=rootmenu;
					tmi.position=pos++;
					tmi.pszName=st;
					tmi.hIcon=LoadSkinnedProtoIcon(proto[i]->szName,statusModeList[j]);
					{
						//owner data
						lpStatusMenuExecParam smep;
						smep=(lpStatusMenuExecParam)mir_alloc(sizeof(StatusMenuExecParam));
						smep->custom = FALSE;
						smep->status=statusModeList[j];
						smep->protoindex=s;
						smep->proto=mir_strdup(proto[i]->szName);
						smep->svc=NULL;
						tmi.ownerdata=smep;
					}

					hStatusMenuHandles[i].protoindex=i;
					hStatusMenuHandles[i].protostatus[j]=statusModeList[j];
					hStatusMenuHandles[i].menuhandle[j]=CallService(MO_ADDNEWMENUITEM,(WPARAM)hStatusMenuObject,(LPARAM)&tmi);

					{
						char buf[256];
						op.Handle=hStatusMenuHandles[i].menuhandle[j];
						sprintf(buf,"ProtocolIcon_%s_%s",proto[i]->szName,tmi.pszName);
						op.Value=(int)buf;
						op.Setting=OPT_MENUITEMSETUNIQNAME;
						CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
					}
				}
			}
		}
		statusFlags|=flags;
	}
	NotifyEventHooks(hPreBuildStatusMenuEvent, 0, 0);
	pos=200000;
	{
		//add to root menu
		for(j=0;j<sizeof(statusModeList)/sizeof(statusModeList[0]);j++) {
			for(i=0;i<protoCount;i++) {
				if (proto[i]->type!=PROTOTYPE_PROTOCOL)//  || (DBGetContactSettingByte(NULL,"CLUI","DontHideStatusMenu",1) == 0)) // && GetProtocolVisibility(proto[i]->szName)==0))
					continue;
				flags=CallProtoService(proto[i]->szName,PS_GETCAPS,PFLAGNUM_2,0);
				if (flags&statusModePf2List[j]){
					//DeleteMenu(hMenu,statusModeList[j],MF_BYCOMMAND)
					char buf[256];
					memset(&tmi,0,sizeof(tmi));
					memset(&buf,0,256);
					tmi.cbSize=sizeof(tmi);
					tmi.flags=CMIF_CHILDPOPUP;
					if (statusModeList[j]==ID_STATUS_OFFLINE)
						tmi.flags|=CMIF_CHECKED;

					tmi.hIcon=LoadSkinnedIcon(skinIconStatusList[j]);
					tmi.position=pos++;
					tmi.root=-1;
					tmi.hotKey=MAKELPARAM(MOD_CONTROL,'0'+j);
					tmi.pszName=(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,statusModeList[j],0);

					wsprintf((LPTSTR)buf, _T("%s\tCtrl-%c"), tmi.pszName,'0'+j);
					tmi.pszName=(char *)buf;
					{
						//owner data
						lpStatusMenuExecParam smep;
						smep=(lpStatusMenuExecParam)mir_alloc(sizeof(StatusMenuExecParam));
						smep->custom = FALSE;
						smep->status=statusModeList[j];
						smep->proto=NULL;
						smep->svc=NULL;
						tmi.ownerdata=smep;
					}
					hStatusMainMenuHandles[j]=(void **)(CallService(MO_ADDNEWMENUITEM,(WPARAM)hStatusMenuObject,(LPARAM)&tmi));
					{
						char buf[256];
						op.Handle=(int)hStatusMainMenuHandles[j];
						sprintf(buf,"Root2ProtocolIcon_%s_%s",proto[i]->szName,tmi.pszName);
						op.Value=(int)buf;
						op.Setting=OPT_MENUITEMSETUNIQNAME;
						CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
					}

					break;
				}
			}
		}
	}

	BuildStatusMenu(0,0);
	return 0;
}

int statustopos(int status)
{
	int j;
	if (statusModeList==NULL) return (-1);
	for(j=0; j < SIZEOF(statusModeList); j++)
		if (status==statusModeList[j])
			return j;

	return -1;
}

static int MenuProtoAck(WPARAM wParam,LPARAM lParam)
{
	int protoCount,i,networkProtoCount;
	PROTOCOLDESCRIPTOR **proto;
	ACKDATA *ack=(ACKDATA*)lParam;
	int overallStatus=0,thisStatus;
	TMO_MenuItem tmi;

	if (ack->type!=ACKTYPE_STATUS) return 0;
	if (ack->result!=ACKRESULT_SUCCESS) return 0;
	if (hStatusMainMenuHandles==NULL) return 0;
	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&protoCount,(LPARAM)&proto);
	networkProtoCount=0;
	for(i=0;i<protoCount;i++)
		if (proto[i]->type==PROTOTYPE_PROTOCOL) {
			thisStatus=CallProtoService(proto[i]->szName,PS_GETSTATUS,0,0);
			if (overallStatus==0) overallStatus=thisStatus;
			else if (overallStatus!=thisStatus) overallStatus=-1;
			networkProtoCount++;
		}
		memset(&tmi,0,sizeof(tmi));
		tmi.cbSize=sizeof(tmi);
		if (overallStatus>ID_STATUS_CONNECTING) {
			int pos = statustopos(currentStatusMenuItem);
			if (pos==-1) pos=0;
			{   // reset all current possible checked statuses
				int pos2;
				for (pos2=0; pos2<hStatusMainMenuHandlesCnt; pos2++)
				{
					if (pos2>=0 && pos2 < hStatusMainMenuHandlesCnt)
					{
						tmi.flags=CMIM_FLAGS|CMIF_CHILDPOPUP;
						CallService(MO_MODIFYMENUITEM,(WPARAM)hStatusMainMenuHandles[pos2],(LPARAM)&tmi);
					};
				}
			}
			currentStatusMenuItem=overallStatus;
			pos = statustopos(currentStatusMenuItem);
			if (pos>=0 && pos < hStatusMainMenuHandlesCnt) {
				tmi.flags=CMIM_FLAGS|CMIF_CHILDPOPUP|CMIF_CHECKED;
				CallService(MO_MODIFYMENUITEM,(WPARAM)hStatusMainMenuHandles[pos],(LPARAM)&tmi);
			};
			currentDesiredStatusMode=currentStatusMenuItem;
		}
		else {
			int pos = statustopos(currentStatusMenuItem);
			if (pos==-1) pos=0;
			if (pos>=0 && pos < hStatusMainMenuHandlesCnt) {
				tmi.flags=CMIM_FLAGS|CMIF_CHILDPOPUP;
				CallService(MO_MODIFYMENUITEM,(WPARAM)hStatusMainMenuHandles[pos],(LPARAM)&tmi);
			};
			//SetMenuDefaultItem(hStatusMenu,-1,FALSE);
			currentStatusMenuItem=0;
		}
		if (networkProtoCount<=1)
			return 0;

		for ( i=0; i < protoCount; i++ )
			if ( !lstrcmpA( proto[i]->szName,ack->szModule ))
				break;

		//hProcess is previous mode, lParam is new mode
		if (((int)ack->hProcess>=ID_STATUS_OFFLINE||(int)ack->hProcess==0) && (int)ack->hProcess<ID_STATUS_OFFLINE+sizeof(statusModeList)/sizeof(statusModeList[0]))
		{
			int pos = statustopos((int)ack->hProcess);
			if (pos==-1) pos=0;
			for(pos=0; pos < sizeof(statusModeList)/sizeof(statusModeList[0]); pos++)
			{
				tmi.flags=CMIM_FLAGS|CMIF_CHILDPOPUP;
				CallService(MO_MODIFYMENUITEM,(WPARAM)hStatusMenuHandles[i].menuhandle[pos],(LPARAM)&tmi);
			};
		};

		if (ack->lParam>=ID_STATUS_OFFLINE && ack->lParam<ID_STATUS_OFFLINE+sizeof(statusModeList)/sizeof(statusModeList[0]))
		{
			int pos = statustopos((int)ack->lParam);
			if (pos>=0 && pos < sizeof(statusModeList)/sizeof(statusModeList[0])) {
				tmi.flags=CMIM_FLAGS|CMIF_CHILDPOPUP|CMIF_CHECKED;
				CallService(MO_MODIFYMENUITEM,(WPARAM)hStatusMenuHandles[i].menuhandle[pos],(LPARAM)&tmi);
			}

		}
		BuildStatusMenu(0,0);
		return 0;
}

static int AddStatusMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	TMO_MenuItem tmi;
	OptParam op;
	long pos=1000;
	MenuProto * mp=NULL;
	int i;
	char buf[MAX_PATH+64];
	BOOL val=(wParam!=0 && !IsBadStringPtrA(mi->pszContactOwner,130)); //proto name should be less than 128
	if (mi->cbSize!=sizeof(CLISTMENUITEM)) return 0;

	for (i=0; i<AllocedProtos; i++) {
		if (!val)
			_snprintf(buf,sizeof(buf),Translate("%s Custom Status"),menusProto[i].szProto);
		if (  menusProto[i].menuID &&
			( (val && lstrcmpiA(menusProto[i].szProto,mi->pszContactOwner))||
			(wParam==0 && lstrcmpiA(buf,mi->pszPopupName)) ) )
		{
			mp=&menusProto[i];
			break;
	}	}

	if (AllocedProtos==0) {
		if (!val)
			_snprintf(buf,sizeof(buf),Translate("%s Custom Status"),menusProtoSingle.szProto);
		if ( (val && lstrcmpiA(menusProtoSingle.szProto,mi->pszContactOwner))||
			(wParam==0 && lstrcmpiA(buf,mi->pszPopupName)) )
		{
			mp=&menusProtoSingle;
	}	}
	// End

	if (mp && !mp->hasAdded) {
		memset(&tmi,0,sizeof(tmi));
		tmi.cbSize=sizeof(tmi);
		tmi.flags=CMIF_CHILDPOPUP|CMIF_ROOTPOPUP;
		tmi.position=pos++;
		tmi.root=(int)mp->menuID;
		tmi.hIcon=NULL;
		tmi.pszName=mi->pszPopupName;
		mp->hasAdded=(HANDLE)CallService(MO_ADDNEWMENUITEM,(WPARAM)hStatusMenuObject,(LPARAM)&tmi);
	}

	if (wParam) {
		int * res=(int*)wParam;
		*res=mp?(int)mp->hasAdded:0;
	}

	memset(&tmi,0,sizeof(tmi));
	tmi.cbSize=sizeof(tmi);
	tmi.flags=(mp?CMIF_CHILDPOPUP:0)|mi->flags;
	tmi.root=(mp?(int)mp->hasAdded:(int)mi->pszPopupName);
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.position=mi->position;
	tmi.pszName=mi->pszName;
	{
		//owner data
		lpStatusMenuExecParam smep;
		smep=(lpStatusMenuExecParam)mir_alloc(sizeof(StatusMenuExecParam));
		memset(smep,0,sizeof(StatusMenuExecParam));
		smep->custom = TRUE;
		smep->svc=mir_strdup(mi->pszService);
		{
			char *buf=mir_strdup(mi->pszService);
			int i=0;
			while(buf[i]!='\0' && buf[i]!='/') i++;
			buf[i]='\0';
			smep->proto=mir_strdup(buf);
			mir_free(buf);
		}
		tmi.ownerdata=smep;
	}

	op.Handle = ((lpStatusMenuExecParam)(tmi.ownerdata))->hMenuItem = 
		(int)CallService(MO_ADDNEWMENUITEM,(WPARAM)hStatusMenuObject,(LPARAM)&tmi);
	op.Setting=OPT_MENUITEMSETUNIQNAME;
	{
		char buf[256];
		sprintf(buf,"%s/%s",mi->pszPopupName?mi->pszPopupName:"",mi->pszService?mi->pszService:"");
		op.Value=(int)buf;
		CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
	}
	return op.Handle;
}

int MenuModulesShutdown(WPARAM wParam,LPARAM lParam)
{
	RecursiveDeleteMenu(hStatusMenu);
	RecursiveDeleteMenu(hMainMenu);
	DestroyMenu(hMainMenu);
	DestroyMenu(hStatusMenu);
	UnhookEvent(hAckHook);
	return 0;
}

int InitCustomMenus(void)
{
	TMenuParam tmp;
	OptParam op;

	CreateServiceFunction("MainMenuExecService",MainMenuExecService);

	CreateServiceFunction("ContactMenuExecService",ContactMenuExecService);
	CreateServiceFunction("ContactMenuCheckService",ContactMenuCheckService);

	//CreateServiceFunction("StatusMenuExecService",StatusMenuExecService);
	//CreateServiceFunction("StatusMenuCheckService",StatusMenuCheckService);
	//CreateServiceFunction("CloseAction",CloseAction);

	//free services
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataMainMenu",FreeOwnerDataMainMenu);
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataContactMenu",FreeOwnerDataContactMenu);
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataStatusMenu",FreeOwnerDataStatusMenu);

	CreateServiceFunction(MS_CLIST_SETSTATUSMODE, SetStatusMode);

	CreateServiceFunction(MS_CLIST_ADDMAINMENUITEM,AddMainMenuItem);
	CreateServiceFunction(MS_CLIST_ADDSTATUSMENUITEM,AddStatusMenuItem);
	CreateServiceFunction(MS_CLIST_MENUGETMAIN,MenuGetMain);
	CreateServiceFunction(MS_CLIST_REMOVEMAINMENUITEM,RemoveMainMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDMAIN,BuildMainMenu);

	CreateServiceFunction(MS_CLIST_ADDCONTACTMENUITEM,AddContactMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDCONTACT,BuildContactMenu);
	CreateServiceFunction(MS_CLIST_REMOVECONTACTMENUITEM,RemoveContactMenuItem);

	CreateServiceFunction(MS_CLIST_MODIFYMENUITEM,ModifyCustomMenuItem);
	CreateServiceFunction(MS_CLIST_MENUMEASUREITEM,MeasureMenuItem);
	CreateServiceFunction(MS_CLIST_MENUDRAWITEM,DrawMenuItem);

	CreateServiceFunction(MS_CLIST_MENUGETSTATUS,MenuGetStatus);
	CreateServiceFunction(MS_CLIST_MENUPROCESSCOMMAND,MenuProcessCommand);
	CreateServiceFunction(MS_CLIST_MENUPROCESSHOTKEY,MenuProcessHotkey);

	hPreBuildContactMenuEvent=CreateHookableEvent(ME_CLIST_PREBUILDCONTACTMENU);
	hPreBuildMainMenuEvent=CreateHookableEvent(ME_CLIST_PREBUILDMAINMENU);

	hAckHook=(HANDLE)HookEvent(ME_PROTO_ACK,MenuProtoAck);

	hMainMenu=GetSubMenu(LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_CLISTMENU)),0);
	hStatusMenu=GetSubMenu(LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_CLISTMENU)),1);
	CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)hMainMenu,0);
	CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)hStatusMenu,0);

	hStatusMainMenuHandles=NULL;
	hStatusMainMenuHandlesCnt=0;

	hStatusMenuHandles=NULL;
	hStatusMenuHandlesCnt=0;

	//new menu sys
	InitGenMenu();

	//main menu
	memset(&tmp,0,sizeof(tmp));
	tmp.cbSize=sizeof(tmp);
	tmp.CheckService=NULL;
	tmp.ExecService="MainMenuExecService";
	tmp.name=Translate("MainMenu");
	hMainMenuObject=CallService(MO_CREATENEWMENUOBJECT,(WPARAM)0,(LPARAM)&tmp);

	op.Handle=hMainMenuObject;
	op.Setting=OPT_USERDEFINEDITEMS;
	op.Value=(int)TRUE;
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	op.Handle=hMainMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_FREE_SERVICE;
	op.Value=(int)"CLISTMENUS/FreeOwnerDataMainMenu";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	//contact menu
	memset(&tmp,0,sizeof(tmp));
	tmp.cbSize=sizeof(tmp);
	tmp.CheckService="ContactMenuCheckService";
	tmp.ExecService="ContactMenuExecService";
	tmp.name="Contact Menu";
	hContactMenuObject=CallService(MO_CREATENEWMENUOBJECT,(WPARAM)0,(LPARAM)&tmp);

	op.Handle=hContactMenuObject;
	op.Setting=OPT_USERDEFINEDITEMS;
	op.Value=(int)TRUE;
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	op.Handle=hContactMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_FREE_SERVICE;
	op.Value=(int)"CLISTMENUS/FreeOwnerDataContactMenu";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	{   //add  exit command to menu
		CLISTMENUITEM mi;
		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=0x7fffffff;
		mi.pszService="CloseAction";
		mi.pszName=Translate("E&xit");
		AddMainMenuItem((WPARAM)0,(LPARAM)&mi);
	}

	currentStatusMenuItem=ID_STATUS_OFFLINE;
	currentDesiredStatusMode=ID_STATUS_OFFLINE;

	if (IsWinVer98Plus())
		HookEvent(ME_SKIN_ICONSCHANGED,MenuIconsChanged);

	HookEvent(ME_SYSTEM_MODULESLOADED,MenuModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN,MenuModulesShutdown);
	return 0;
}

void UninitCustomMenus(void)
{
	int i;
	if (!menusProto) return;
	for (i=0; i<AllocedProtos; i++)
		if (menusProto[i].szProto)
			mir_free(menusProto[i].szProto);
	if (menusProto) mir_free(menusProto);

	if (hStatusMainMenuHandles!=NULL)
		mir_free(hStatusMainMenuHandles);

	hStatusMainMenuHandles=NULL;

	if ( hStatusMenuHandles != NULL )
		mir_free(hStatusMenuHandles);

	hStatusMenuHandles=NULL;
	if (hMainMenuObject) CallService(MO_REMOVEMENUOBJECT,hMainMenuObject,0);
	if (hStatusMenuObject) CallService(MO_REMOVEMENUOBJECT,hMainMenuObject,0);
}

#include "commonheaders.h"

HANDLE hModulesLoaded,hOnCntMenuBuild;
HANDLE prevmenu=0;
extern char *DBGetString(HANDLE hContact,const char *szModule,const char *szSetting);

extern HWND hwndContactList;
HWND hwndTopToolBar=0;

//service
//wparam - hcontact
//lparam .popupposition from CLISTMENUITEM

#define MTG_MOVE								"MoveToGroup/Move"
/*
char *DBGetString(HANDLE hContact,const char *szModule,const char *szSetting)
{
	char *str=NULL;
	DBVARIANT dbv;
	DBGetContactSetting(hContact,szModule,szSetting,&dbv);
	if(dbv.type==DBVT_ASCIIZ)
		str=strdup(dbv.pszVal);
	DBFreeVariant(&dbv);
	return str;
}
*/
static int OnContactMenuBuild(WPARAM wParam,LPARAM lParam)
{
	
	CLISTMENUITEM mi;
	HANDLE menuid;
	int i,grpid;
	boolean grpexists;
	char *grpname;
	char *intname;
	
	if (prevmenu!=0){
		CallService(MS_CLIST_REMOVECONTACTMENUITEM,(WPARAM)prevmenu,(LPARAM)0);
	};
	
	
	ZeroMemory(&mi,sizeof(mi));
	
	mi.cbSize=sizeof(mi);
	mi.hIcon=NULL;//LoadIcon(hInst,MAKEINTRESOURCE(IDI_MIRANDA));
	mi.pszPopupName=(char *)-1;
	mi.position=100000;
	mi.pszName=Translate("&Move to Group");
	mi.flags=CMIF_ROOTPOPUP;
	mi.pszContactOwner=(char *)0;
	menuid=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,wParam,(LPARAM)&mi);
	
	prevmenu=menuid;

	grpexists=TRUE;
	i=0;
	intname=(char *)malloc(20);
	grpid=1000;
while (TRUE) 
{
	itoa(i,intname,10);
	grpname=DBGetString(0,"CListGroups",intname);

	if (grpname==NULL ){break;};
	if (strlen(grpname)==0)
	{
		break;
	};
	if (grpname[0]==0)
	{
		break;
	};
	mi.cbSize=sizeof(mi);
	mi.hIcon=NULL;//LoadIcon(hInst,MAKEINTRESOURCE(IDI_MIRANDA));
	mi.pszPopupName=(char *)menuid;
	mi.popupPosition=i+1;
	mi.position=grpid++;
	mi.pszName=&(grpname[1]);
	mi.flags=CMIF_CHILDPOPUP;
	mi.pszContactOwner=(char *)0;
	mi.pszService=MTG_MOVE;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,wParam,(LPARAM)&mi);
	i++;
	free(grpname);
};
free(intname);
return 0;
};

static int MTG_DOMOVE(WPARAM wParam,LPARAM lParam)
{
	char *grpname,*correctgrpname;
	char *intname;


if (lParam==0)
{
	MessageBox(0,"Wrong version of New menu system - please update.","MoveToGroup",0);
	return(0);
};
lParam--;
	intname=(char *)malloc(20);
	itoa(lParam,intname,10);
	grpname=DBGetString(0,"CListGroups",intname);
	if (grpname!=0)
	{
		correctgrpname=&(grpname[1]);
		DBWriteContactSettingString((HANDLE)wParam,"CList","Group",correctgrpname);
		free(grpname);
	};
	
	
	free (intname);
return 0;
};
static int OnmodulesLoad(WPARAM wParam,LPARAM lParam)
{
//hwndContactList=CallService(MS_CLUI_GETHWND,0,0);

if (!ServiceExists(MS_CLIST_REMOVECONTACTMENUITEM))
{
	MessageBox(0,"New menu system not found - plugin disabled.","MoveToGroup",0);
	return(0);
};
	hOnCntMenuBuild=HookEvent(ME_CLIST_PREBUILDCONTACTMENU,OnContactMenuBuild);	
	CreateServiceFunction(MTG_MOVE,MTG_DOMOVE);
	//hFrameTopWindow=addTopToolBarWindow(hwndContactList);
	//LoadInternalButtons(CallService(MS_CLUI_GETHWNDTREE,0,0));

return(0);
};


int LoadMoveToGroup()
{

	hModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,OnmodulesLoad);	
	
	return 0;
}

int UnloadMoveToGroup(void)
{
	//if (hFrameTopWindow!=NULL){CallService(MS_CLIST_FRAMES_REMOVEFRAME,(WPARAM)hFrameTopWindow,(LPARAM)0);};

	//DestroyServiceFunction();
	UnhookEvent(hModulesLoaded);
	
	return 0;
}	
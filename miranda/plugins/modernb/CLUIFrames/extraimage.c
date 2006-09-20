#include "..\commonheaders.h"
#include "..\SkinEngine.h"

BOOL g_mutex_bSetAllExtraIconsCycle=0;

//int ImageList_AddIcon_FixAlpha(HIMAGELIST himl,HICON hicon);
extern int LoadPositionsFromDB(BYTE * OrderPos);
#define EXTRACOLUMNCOUNT 10
int EnabledColumnCount=0;
boolean visar[EXTRACOLUMNCOUNT];
#define ExtraImageIconsIndexCount 6
int ExtraImageIconsIndex[ExtraImageIconsIndexCount];

static HANDLE hExtraImageListRebuilding,hExtraImageApplying;

static HIMAGELIST hExtraImageList;
extern HINSTANCE g_hInst;
extern HIMAGELIST hCListImages;

extern int CluiIconsChanged(WPARAM,LPARAM);
extern int ClcIconsChanged(WPARAM,LPARAM);
extern BOOL CLUI__cliInvalidateRect(HWND hWnd, CONST RECT* lpRect,BOOL bErase);

void ExtraImage_SetAllExtraIcons(HWND hwndList,HANDLE hContact);
void ExtraImage_LoadModule();
extern HICON CLUI_LoadIconFromExternalFile (char *filename,int i,boolean UseLibrary,boolean registerit,char *IconName,char *SectName,char *Description,int internalidx, BOOL * needFree);
boolean ImageCreated=FALSE;
void ExtraImage_ReloadExtraIcons();
BYTE ExtraOrder[]=
{
	1, // EXTRA_ICON_EMAIL	
	2, // EXTRA_ICON_PROTO	
	3, // EXTRA_ICON_SMS	
	4, // EXTRA_ICON_ADV1	
	5, // EXTRA_ICON_ADV2	
	6, // EXTRA_ICON_WEB	
	6, // EXTRA_ICON_CLIENT 
	0, // EXTRA_ICON_VISMODE 
	8, // EXTRA_ICON_ADV3	
	9, // EXTRA_ICON_ADV4	
};
boolean isColumnVisible(int extra)
{
	int i=0;
	for (i=0; i<sizeof(ExtraOrder)/sizeof(ExtraOrder[0]); i++)
		if (ExtraOrder[i]==extra)
		{
			switch(i+1)
			{
				case EXTRA_ICON_EMAIL:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_EMAIL",1));
				case EXTRA_ICON_PROTO:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_PROTO",1));
				case EXTRA_ICON_SMS:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_SMS",1));
				case EXTRA_ICON_ADV1:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV1",1));
				case EXTRA_ICON_ADV2:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV2",1));	
				case EXTRA_ICON_WEB:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_WEB",1));
				case EXTRA_ICON_CLIENT:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_CLIENT",1));
				case EXTRA_ICON_VISMODE:	return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_VISMODE",1));
				case EXTRA_ICON_ADV3:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV3",0));
				case EXTRA_ICON_ADV4:		return(DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV4",0));			
			}
			break;
		}
	return(FALSE);
}

void GetVisColumns()
{
	visar[0]=isColumnVisible(0);
	visar[1]=isColumnVisible(1);
	visar[2]=isColumnVisible(2);
	visar[3]=isColumnVisible(3);
	visar[4]=isColumnVisible(4);
	visar[5]=isColumnVisible(5);
	visar[6]=isColumnVisible(6);
	visar[7]=isColumnVisible(7);
	visar[8]=isColumnVisible(8);
	visar[9]=isColumnVisible(9);
};

__inline int bti(boolean b)
{
	return(b?1:0);
};
int colsum(int from,int to)
{
	int i,sum;
	if (from<0||from>=EXTRACOLUMNCOUNT){return(-1);};
	if (to<0||to>=EXTRACOLUMNCOUNT){return(-1);};
	if (to<from){return(-1);};

	sum=0;
	for (i=from;i<=to;i++)
	{
		sum+=bti(visar[i]);
	};
	return(sum);
};




int ExtraImage_ExtraIDToColumnNum(int extra)
{
	int cnt=EnabledColumnCount;
	int extracnt=EXTRACOLUMNCOUNT-1;
	int ord=ExtraOrder[extra-1];
    if (!visar[ord]) return -1;
	return (colsum(0,ord)-1);
};

int SetIconForExtraColumn(WPARAM wParam,LPARAM lParam)
{
	pIconExtraColumn piec;
	int icol;
	HANDLE hItem;

	if (pcli->hwndContactTree==0){return(-1);};
	if (wParam==0||lParam==0){return(-1);};
	piec=(pIconExtraColumn)lParam;

	if (piec->cbSize!=sizeof(IconExtraColumn)){return(-1);};
	icol=ExtraImage_ExtraIDToColumnNum(piec->ColumnType);
	if (icol==-1){return(-1);};
	hItem=(HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)wParam,0);
	if (hItem==0){return(-1);};

	SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(icol,piec->hImage));	
	return(0);
};

//wparam=hIcon
//return hImage on success,-1 on failure
int AddIconToExtraImageList(WPARAM wParam,LPARAM lParam)
{
	if (hExtraImageList==0||wParam==0){return(-1);};
	return((int)ImageList_AddIcon(hExtraImageList,(HICON)wParam));

};

void SetNewExtraColumnCount()
{
	int newcount;
	LoadPositionsFromDB(ExtraOrder);
	GetVisColumns();
	newcount=colsum(0,EXTRACOLUMNCOUNT-1);
	DBWriteContactSettingByte(NULL,CLUIFrameModule,"EnabledColumnCount",(BYTE)newcount);
	EnabledColumnCount=newcount;
	SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNS,EnabledColumnCount,0);
};

int OnIconLibIconChanged(WPARAM wParam,LPARAM lParam)
{
	HICON hicon;
	BOOL needFree;
    if (MirandaExiting()) return 0;
	hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",0,TRUE,FALSE,"Email","Contact List",Translate("Email Icon"),-IDI_EMAIL, &needFree);
	ExtraImageIconsIndex[0]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[0],hicon );		
    if (needFree) DestroyIcon_protect(hicon);

	hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",1,TRUE,FALSE,"Sms","Contact List",Translate("Sms Icon"),-IDI_SMS,&needFree);
	ExtraImageIconsIndex[1]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[1],hicon );		
	if (needFree) DestroyIcon_protect(hicon);

	hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",4,TRUE,FALSE,"Web","Contact List",Translate("Web Icon"),-IDI_GLOBUS,&needFree);
	ExtraImageIconsIndex[2]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[2],hicon );		
	if (needFree) DestroyIcon_protect(hicon);

	hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",5,TRUE,FALSE,"AlwaysVis","Contact List",Translate("Always Visible"),-IDI_ALWAYSVIS, &needFree);
	ExtraImageIconsIndex[3]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[3],hicon );		
	if (needFree) DestroyIcon_protect(hicon);

	hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",6,TRUE,FALSE,"NeverVis","Contact List",Translate("Never Visible"),-IDI_NEVERVIS, &needFree);
	ExtraImageIconsIndex[4]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[4],hicon );		
	if (needFree) DestroyIcon_protect(hicon);

	hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",7,TRUE,TRUE,"ChatActivity","Contact List",Translate("Chat Activity"),-IDI_CHAT, &needFree);
	ExtraImageIconsIndex[5]=ImageList_ReplaceIcon(hExtraImageList,ExtraImageIconsIndex[5],hicon );
	if (needFree) DestroyIcon_protect(hicon);

	CluiIconsChanged(wParam,lParam);
	NotifyEventHooks(ME_SKIN_ICONSCHANGED,0,0);
	pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);
  
	return 0;
};


void ExtraImage_ReloadExtraIcons()
{
	{	
		int count,i;
		PROTOCOLDESCRIPTOR **protos;
		HICON hicon;
		BOOL needFree;

		SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNSSPACE,DBGetContactSettingByte(NULL,"CLUI","ExtraColumnSpace",18),0);					
		SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,0,(LPARAM)NULL);		
		if (hExtraImageList){ImageList_Destroy(hExtraImageList);};
		hExtraImageList=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,256);

		//adding protocol icons
		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&count,(LPARAM)&protos);

		//loading icons
		hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",0,TRUE,TRUE,"Email","Contact List",Translate("Email Icon"),-IDI_EMAIL,&needFree);
		if (!hicon) {hicon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_EMAIL)); needFree=TRUE;}
		ExtraImageIconsIndex[0]=ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",1,TRUE,TRUE,"Sms","Contact List",Translate("Sms Icon"),-IDI_SMS,&needFree);
		if (!hicon) {hicon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_SMS)); needFree=TRUE;} 
		ExtraImageIconsIndex[1]=ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",4,TRUE,TRUE,"Web","Contact List",Translate("Web Icon"),-IDI_GLOBUS,&needFree);
		if (!hicon) {hicon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_GLOBUS)); needFree=TRUE;}
		ExtraImageIconsIndex[2]=ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		//calc only needed protocols
		for(i=0;i<count;i++) {
			if(protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
			ImageList_AddIcon(hExtraImageList,LoadSkinnedProtoIcon(protos[i]->szName,ID_STATUS_ONLINE));
		}

		hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",5,TRUE,TRUE,"AlwaysVis","Contact List",Translate("Always Visible"),-IDI_ALWAYSVIS,&needFree);
		if (!hicon) {hicon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_ALWAYSVIS));needFree=TRUE;}
		ExtraImageIconsIndex[3]=ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",6,TRUE,TRUE,"NeverVis","Contact List",Translate("Never Visible"),-IDI_NEVERVIS,&needFree);
		if (!hicon) {hicon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_NEVERVIS));needFree=TRUE;}
		ExtraImageIconsIndex[4]=ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		hicon=CLUI_LoadIconFromExternalFile("clisticons.dll",7,TRUE,TRUE,"ChatActivity","Contact List",Translate("Chat Activity"),-IDI_CHAT,&needFree);
		if (!hicon) {hicon=LoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_CHAT));needFree=TRUE;}
		ExtraImageIconsIndex[5]=ImageList_AddIcon(hExtraImageList,hicon );
		if (needFree) DestroyIcon_protect(hicon);

		SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGELIST,0,(LPARAM)hExtraImageList);		
		//ExtraImage_SetAllExtraIcons(hImgList);
		SetNewExtraColumnCount();
		NotifyEventHooks(hExtraImageListRebuilding,0,0);
		ImageCreated=TRUE;
	}

};
void ClearExtraIcons();

void ReAssignExtraIcons()
{
	ClearExtraIcons();
	SetNewExtraColumnCount();
	ExtraImage_SetAllExtraIcons(pcli->hwndContactTree,0);
	SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
}

void ClearExtraIcons()
{
	int i;
	HANDLE hContact,hItem;

	//EnabledColumnCount=DBGetContactSettingByte(NULL,CLUIFrameModule,"EnabledColumnCount",5);
	//SendMessage(pcli->hwndContactTree,CLM_SETEXTRACOLUMNS,EnabledColumnCount,0);
	SetNewExtraColumnCount();

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do {

		hItem=(HANDLE)SendMessage(pcli->hwndContactTree,CLM_FINDCONTACT,(WPARAM)hContact,0);
		if (hItem==0){continue;};
		for (i=0;i<EnabledColumnCount;i++)
		{
			SendMessage(pcli->hwndContactTree,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(i,0xFF));	
		};

	} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
};

void ExtraImage_SetAllExtraIcons(HWND hwndList,HANDLE hContact)
{
	HANDLE hItem;
	int locApparentMode=0;
	char * locApparentModeProto=NULL;
	boolean hcontgiven=FALSE;
	char *szProto;
	char *(ImgIndex[64]);
	int maxpr,count,i;
	PROTOCOLDESCRIPTOR **protos;
	pdisplayNameCacheEntry pdnce;
	int em,pr,sms,a1,a2,w1,c1;
	int tick=0;
	int inphcont=(int)hContact;
	g_mutex_bSetAllExtraIconsCycle=1;
	hcontgiven=(hContact!=0);

	if (pcli->hwndContactTree==0){return;};
	tick=GetTickCount();
	if (ImageCreated==FALSE) ExtraImage_ReloadExtraIcons();

	SetNewExtraColumnCount();
	{
		em=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_EMAIL);	
		pr=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_PROTO);
		sms=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_SMS);
		a1=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_ADV1);
		a2=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_ADV2);
		w1=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_WEB);
		c1=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_CLIENT);
	};

	memset(&ImgIndex,0,sizeof(&ImgIndex));
	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&count,(LPARAM)&protos);
	maxpr=0;
	//calc only needed protocols
	for(i=0;i<count;i++) {
		if(protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
		ImgIndex[maxpr]=protos[i]->szName;
		maxpr++;
	}

	if (hContact==NULL)
	{
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	}	

	do {

		szProto=NULL;
		hItem=hContact;
		if (hItem==0){continue;};
		pdnce=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hItem);
		if (pdnce==NULL) {continue;};

		//		szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);		
		szProto=pdnce->szProto;
		{
			boolean showweb;	
			showweb=FALSE;     
			if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_WEB)!=-1)
			{

				if (szProto != NULL)
				{
					char *homepage;
					homepage=DBGetStringA(pdnce->hContact,"UserInfo", "Homepage");
					if (!homepage)
						homepage=DBGetStringA(pdnce->hContact,pdnce->szProto, "Homepage");
					if (homepage!=NULL)
					{											
						showweb=TRUE;				
						mir_free(homepage);
					}
				}
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_WEB),(showweb)?2:0xFF));	
			}
		}		
		{
			DBVARIANT dbv={0};
			boolean showemail;	
			showemail=TRUE;
			if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_EMAIL)!=-1)
			{

				if (szProto == NULL || DBGetContactSetting(hContact, szProto, "e-mail",&dbv)) 
				{
					DBFreeVariant(&dbv);
					if (DBGetContactSetting(hContact, "UserInfo", "Mye-mail0", &dbv))
						showemail=FALSE;					
				}
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_EMAIL),(showemail)?0:0xFF));	
				DBFreeVariant(&dbv);
			}
		}

		{
			DBVARIANT dbv={0};
			boolean showsms;	
			showsms=TRUE;
			if (ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_SMS)!=-1)
			{
				if (szProto == NULL || DBGetContactSetting(hContact, szProto, "Cellular",&dbv)) 
				{
					DBFreeVariant(&dbv);
					if (DBGetContactSetting(hContact, "UserInfo", "MyPhone0", &dbv))
						showsms=FALSE;
				}
				SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_SMS),(showsms)?1:0xFF));	
				DBFreeVariant(&dbv);
			}
		}		

		if(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_PROTO)!=-1) 
		{					
			for (i=0;i<maxpr;i++)
			{
				if(!mir_strcmp(ImgIndex[i],szProto))
				{
					SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_PROTO),i+3));	
					break;
				};
			};				
		};
		if(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_VISMODE)!=-1) 
		{
			BYTE iconIndex=0xFF;
			if (szProto != NULL)
			{
				if (!DBGetContactSettingByte(hContact, szProto, "ChatRoom", 0))		
				{
					if (pdnce->ApparentMode==ID_STATUS_OFFLINE)
						iconIndex=ExtraImageIconsIndex[4];	
					else if (pdnce->ApparentMode==ID_STATUS_ONLINE)
					{
						if (szProto!=locApparentModeProto)
						{
							locApparentModeProto=szProto;
							locApparentMode=CallProtoService(locApparentModeProto,PS_GETSTATUS,0,0);
						}
						if (locApparentMode==ID_STATUS_INVISIBLE) 
							iconIndex=ExtraImageIconsIndex[3];				
					}
				}
				else 
				{
					if (pdnce->ApparentMode==ID_STATUS_OFFLINE)
						iconIndex=ExtraImageIconsIndex[5];	
					else iconIndex=255;	
				}
			}
			SendMessage(hwndList,CLM_SETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_VISMODE),iconIndex));	
		}
		NotifyEventHooks(hExtraImageApplying,(WPARAM)hContact,0);
		if (hcontgiven) break;
		Sleep(0);
	} while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));

	tick=GetTickCount()-tick;
	g_mutex_bSetAllExtraIconsCycle=0;
	CLUI__cliInvalidateRect(hwndList,NULL,FALSE);
	Sleep(0);
}


void ExtraImage_LoadModule()
{
	CreateServiceFunction(MS_CLIST_EXTRA_SET_ICON,SetIconForExtraColumn); 
	CreateServiceFunction(MS_CLIST_EXTRA_ADD_ICON,AddIconToExtraImageList); 

	hExtraImageListRebuilding=CreateHookableEvent(ME_CLIST_EXTRA_LIST_REBUILD);
	hExtraImageApplying=CreateHookableEvent(ME_CLIST_EXTRA_IMAGE_APPLY);
	HookEvent(ME_SKIN2_ICONSCHANGED,OnIconLibIconChanged);


};


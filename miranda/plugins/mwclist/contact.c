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
#include "m_clui.h"
#include "clist.h"

extern HANDLE hContactIconChangedEvent;
extern int GetContactCachedStatus(HANDLE hContact);
extern char *GetContactCachedProtocol(HANDLE hContact);

int sortByStatus;
static int sortByProto;
static int sortNoOfflineBottom;
struct {
	int status,order;
} statusModeOrder[]={
	{ID_STATUS_OFFLINE,500},
	{ID_STATUS_ONLINE,0},
	{ID_STATUS_AWAY,200},
	{ID_STATUS_DND,400},
	{ID_STATUS_NA,450},
	{ID_STATUS_OCCUPIED,100},
	{ID_STATUS_FREECHAT,50},
	{ID_STATUS_INVISIBLE,20},
	{ID_STATUS_ONTHEPHONE,150},
	{ID_STATUS_OUTTOLUNCH,425}};

static int GetContactStatus(HANDLE hContact)
{
	/*
	
	char *szProto;

	szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	if(szProto==NULL) return ID_STATUS_OFFLINE;
	return DBGetContactSettingWord(hContact,szProto,"Status",ID_STATUS_OFFLINE);
	*/
	return (GetContactCachedStatus(hContact));
}

void ChangeContactIcon(HANDLE hContact,int iIcon,int add)
{
	//clui MS_CLUI_CONTACTADDED MS_CLUI_CONTACTSETICON this methods is null
	//CallService(add?MS_CLUI_CONTACTADDED:MS_CLUI_CONTACTSETICON,(WPARAM)hContact,iIcon);
	NotifyEventHooks(hContactIconChangedEvent,(WPARAM)hContact,iIcon);
}

static int GetStatusModeOrdering(int statusMode)
{
	int i;
	for(i=0; i< SIZEOF(statusModeOrder); i++)
		if(statusModeOrder[i].status==statusMode) return statusModeOrder[i].order;
	return 1000;
}

void LoadContactTree(void)
{
	HANDLE hContact;
	int i,hideOffline,status,tick;
	pdisplayNameCacheEntry cacheEntry;
		


	tick=GetTickCount();
	CallService(MS_CLUI_LISTBEGINREBUILD,0,0);
	for(i=1;;i++) {
		if((char*)CallService(MS_CLIST_GROUPGETNAME2,i,(LPARAM)(int*)NULL)==NULL) break;
		CallService(MS_CLUI_GROUPADDED,i,0);
	}

	hideOffline=DBGetContactSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT);
	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);

	while(hContact!=NULL) {
		cacheEntry=GetContactFullCacheEntry(hContact);
		if (cacheEntry==NULL)
		{
			MessageBoxA(0,"Fail To Get CacheEntry for hContact","!!!!!",0);
			break;
		}
		status=cacheEntry->status;
		if((!hideOffline || status!=ID_STATUS_OFFLINE) && !cacheEntry->Hidden)
			ChangeContactIcon(hContact,ExtIconFromStatusMode(hContact,(char*)cacheEntry->szProto,status),1);
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}
	sortByStatus=DBGetContactSettingByte(NULL,"CList","SortByStatus",SETTING_SORTBYSTATUS_DEFAULT);
	sortByProto=DBGetContactSettingByte(NULL,"CList","SortByProto",SETTING_SORTBYPROTO_DEFAULT);
	sortNoOfflineBottom=DBGetContactSettingByte(NULL,"CList","NoOfflineBottom",SETTING_NOOFFLINEBOTTOM_DEFAULT);

	CallService(MS_CLUI_SORTLIST,0,0);
	CallService(MS_CLUI_LISTENDREBUILD,0,0);
	
	tick=GetTickCount()-tick;
	{
	char buf[255];
	//sprintf(buf,"%s %s took %i ms",__FILE__,__LINE__,tick);
	sprintf(buf,"LoadContactTree %d \r\n",tick);

	OutputDebugStringA(buf);
	}
}

#define SAFESTRING(a) a?a:""

int CompareContacts( const struct ClcContact *contact1, const struct ClcContact *contact2 )
{
	HANDLE a = contact1->hContact, b = contact2->hContact;
	TCHAR *namea,*nameb;
	int statusa,statusb;
	char *szProto1,*szProto2;
	int rc;

	GetContactInfosForSort(a,&szProto1,&namea,&statusa);
	GetContactInfosForSort(b,&szProto2,&nameb,&statusb);

	if (sortByProto) {

		/* deal with statuses, online contacts have to go above offline */
		if (sortNoOfflineBottom==0) 
			if((statusa==ID_STATUS_OFFLINE)!=(statusb==ID_STATUS_OFFLINE)) {
			return 2*(statusa==ID_STATUS_OFFLINE)-1;
		}
		/* both are online, now check protocols */
		rc=strcmp(SAFESTRING(szProto1),SAFESTRING(szProto2)); /* strcmp() doesn't like NULL so feed in "" as needed */
		if (rc != 0 && (szProto1 != NULL && szProto2 != NULL)) return rc;
		/* protocols are the same, order by display name */
	} 

	if(sortByStatus) {
		int ordera,orderb;
		ordera=GetStatusModeOrdering(statusa);
		orderb=GetStatusModeOrdering(statusb);
		if(ordera!=orderb) return ordera-orderb;
	}
	else {
		//one is offline: offline goes below online
		if (sortNoOfflineBottom==0)
			{
			if((statusa==ID_STATUS_OFFLINE)!=(statusb==ID_STATUS_OFFLINE)) {
				return 2*(statusa==ID_STATUS_OFFLINE)-1;
			}
		}
	}

	//otherwise just compare names
	return _tcsicmp(namea,nameb);
}

#undef SAFESTRING

static UINT_PTR resortTimerId=0;
static VOID CALLBACK SortContactsTimer(HWND hwnd,UINT message,UINT_PTR idEvent,DWORD dwTime)
{
	KillTimer(NULL,resortTimerId);
	resortTimerId=0;
	CallService(MS_CLUI_SORTLIST,0,0);
}

void SortContacts(void)
{
	//avoid doing lots of resorts in quick succession
	sortByStatus=DBGetContactSettingByte(NULL,"CList","SortByStatus",SETTING_SORTBYSTATUS_DEFAULT);
	sortByProto=DBGetContactSettingByte(NULL,"CList","SortByProto",SETTING_SORTBYPROTO_DEFAULT);
	if(resortTimerId) KillTimer(NULL,resortTimerId);
	resortTimerId=SetTimer(NULL,0,50,SortContactsTimer);
}

INT_PTR ContactChangeGroup(WPARAM wParam,LPARAM lParam)
{
	CallService(MS_CLUI_CONTACTDELETED,wParam,0);
	if((HANDLE)lParam==NULL)
		DBDeleteContactSetting((HANDLE)wParam,"CList","Group");
	else
		DBWriteContactSettingString((HANDLE)wParam,"CList","Group",(char*)CallService(MS_CLIST_GROUPGETNAME2,lParam,(LPARAM)(int*)NULL));
	CallService(MS_CLUI_CONTACTADDED,wParam,ExtIconFromStatusMode((HANDLE)wParam,(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0),GetContactStatus((HANDLE)wParam)));
	return 0;
}

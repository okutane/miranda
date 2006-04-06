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
#include "clc.h"
#include "commonprototypes.h"

#define CacheArrSize 255
struct ClcGroup *CacheIndex[CacheArrSize]={NULL};
boolean CacheIndexClear=TRUE;

/* the CLC uses 3 different ways to identify elements in its list, this file
contains routines to convert between them.

1) struct ClcContact/struct ClcGroup pair. Only ever used within the duration
of a single operation, but used at some point in nearly everything
2) index integer. The 0-based number of the item from the top. Only visible
items are counted (ie not closed groups). Used for saving selection and drag
highlight
3) hItem handle. Either the hContact or (hGroup|HCONTACT_ISGROUP). Used
exclusively externally

1->2: GetRowsPriorTo()
1->3: ContactToHItem()
3->1: FindItem()
2->1: GetRowByIndex()
*/

int GetContactIndex(struct ClcGroup *group,struct ClcContact *contact)
{
  int i=0;
  for (i=0; i<group->cl.count; i++)
    if (group->cl.items[i]->hContact==contact->hContact)  return i;
  return -1;
}

int GetRowsPriorTo(struct ClcGroup *group,struct ClcGroup *subgroup,int contactIndex)
{
	int count=0;
	BYTE k;
	k=DBGetContactSettingByte(NULL,"CLC","MetaExpanding",1);
	group->scanIndex=0;
	for(;;) {
		if(group->scanIndex==group->cl.count) {
			group=group->parent;
			if(group==NULL) break;
			group->scanIndex++;
			continue;
		}
		if(group==subgroup && contactIndex==group->scanIndex) return count;
		count++;
		/*		if ((group->cl.items[group->scanIndex]->type==CLCIT_CONTACT) && (group->cl.items[group->scanIndex].flags & CONTACTF_STATUSMSG)) {
		count++;
		}
		*/
		if(group->cl.items[group->scanIndex]->type==CLCIT_GROUP) {
			if(group->cl.items[group->scanIndex]->group==subgroup && contactIndex==-1)
				return count-1;
			if(group->cl.items[group->scanIndex]->group->expanded) {
				group=group->cl.items[group->scanIndex]->group;
				group->scanIndex=0;
				continue;
			}
		}
		if(group->cl.items[group->scanIndex]->type==CLCIT_CONTACT)
		{
			count+=(group->cl.items[group->scanIndex]->SubAllocated*group->cl.items[group->scanIndex]->SubExpanded*k);
		}
		group->scanIndex++;
	}
	return -1;
}

int sfnFindItem(HWND hwnd,struct ClcData *dat,HANDLE hItem,struct ClcContact **contact,struct ClcGroup **subgroup,int *isVisible)
{
  return FindItem(hwnd,dat, hItem,contact,subgroup,isVisible,FALSE);
}

int FindItem(HWND hwnd,struct ClcData *dat,HANDLE hItem,struct ClcContact **contact,struct ClcGroup **subgroup,int *isVisible, BOOL isIgnoreSubcontacts)
{
	int index=0, i;
	int nowVisible=1;
	struct ClcGroup *group;
//	EnterCriticalSection(&(dat->lockitemCS));
	group=&dat->list;

	group->scanIndex=0;
	group=&dat->list;

	for(;;) {
		if(group->scanIndex==group->cl.count) {
			struct ClcGroup *tgroup;
			group=group->parent;
			if(group==NULL) break;
			nowVisible=1;
			for(tgroup=group;tgroup;tgroup=tgroup->parent)
			{
				if(!tgroup->expanded) 
				{
					nowVisible=0; 
					break;
				}
			}
			group->scanIndex++;
			continue;
		}
		if(nowVisible) index++;
		if((IsHContactGroup(hItem) && group->cl.items[group->scanIndex]->type==CLCIT_GROUP && ((unsigned)hItem&~HCONTACT_ISGROUP)==group->cl.items[group->scanIndex]->groupId) ||
			(IsHContactContact(hItem) && group->cl.items[group->scanIndex]->type==CLCIT_CONTACT && group->cl.items[group->scanIndex]->hContact==hItem) ||
			(IsHContactInfo(hItem) && group->cl.items[group->scanIndex]->type==CLCIT_INFO && group->cl.items[group->scanIndex]->hContact==(HANDLE)((unsigned)hItem&~HCONTACT_ISINFO))) 
		{
			if(isVisible) {
				if(!nowVisible) *isVisible=0;
				else {
					int posy = RowHeights_GetItemTopY(dat,index+1);
					if(posy<dat->yScroll) 
						*isVisible=0;
					else {
						RECT clRect;
						GetClientRect(hwnd,&clRect);
						if(posy>=dat->yScroll+clRect.bottom) *isVisible=0;
						else *isVisible=1;
					}
				}
			}
			if(contact) *contact=group->cl.items[group->scanIndex];
			if(subgroup) *subgroup=group;
		//	LeaveCriticalSection(&(dat->lockitemCS));
			return 1;
		}
		if (!isIgnoreSubcontacts && 
			IsHContactContact(hItem) &&
			group->cl.items[group->scanIndex]->type == CLCIT_CONTACT &&
			group->cl.items[group->scanIndex]->SubAllocated > 0)
		{
			for (i=0; i<group->cl.items[group->scanIndex]->SubAllocated; i++)
			{
				if (group->cl.items[group->scanIndex]->subcontacts[i].hContact == hItem)
				{	
#ifdef _DEBUG
					if (IsBadWritePtr(&group->cl.items[group->scanIndex]->subcontacts[i], sizeof(struct ClcContact)))
					{
						log1("FindIltem->IsBadWritePtr | 2o  [%08x]", &group->cl.items[group->scanIndex]->subcontacts[i]);
						PostMessage(hwnd,CLM_AUTOREBUILD,0,0);
	//					LeaveCriticalSection(&(dat->lockitemCS));
						return 0;
					}
#endif
					if(contact) *contact=&group->cl.items[group->scanIndex]->subcontacts[i];
					if(subgroup) *subgroup=group;
	//				LeaveCriticalSection(&(dat->lockitemCS));
					return 1;
				}
			}
		}

		if(group->cl.items[group->scanIndex]->type==CLCIT_GROUP) {
			group=group->cl.items[group->scanIndex]->group;
			group->scanIndex=0;
			nowVisible&=group->expanded;
			continue;
		}
		group->scanIndex++;
	}
//	LeaveCriticalSection(&(dat->lockitemCS));
	return 0;
}

void ClearRowByIndexCache()
{
	if (!CacheIndexClear) 
	{
		memset(CacheIndex,0,sizeof(CacheIndex));
		CacheIndexClear=TRUE;
	};
}
int GetRowByIndex(struct ClcData *dat,int testindex,struct ClcContact **contact,struct ClcGroup **subgroup)
{
	int index=0,i;
	struct ClcGroup *group=&dat->list;

	if (testindex<0) return (-1);
	{
		group->scanIndex=0;
		for(;;) {
			if(group->scanIndex==group->cl.count) {
				group=group->parent;
				if(group==NULL) break;
				group->scanIndex++;
				continue;
			}
			if ((index>0) && (index<CacheArrSize)) 
			{
				CacheIndex[index]=group;
				CacheIndexClear=FALSE;
			};

			if(testindex==index) {
				if(contact) *contact=group->cl.items[group->scanIndex];
				if(subgroup) *subgroup=group;
				return index;
			}

			if (group->cl.items[group->scanIndex]->type==CLCIT_CONTACT)
				if (group->cl.items[group->scanIndex]->SubAllocated)
					if (group->cl.items[group->scanIndex]->SubExpanded && dat->expandMeta)
					{
						for (i=0;i<group->cl.items[group->scanIndex]->SubAllocated;i++)
						{
							if ((index>0) && (index<CacheArrSize)) 
							{
								CacheIndex[index]=group;
								CacheIndexClear=FALSE;
							};
							index++;
							if(testindex==index) {
								if(contact) 
								{
									*contact=&group->cl.items[group->scanIndex]->subcontacts[i];
									(*contact)->subcontacts=group->cl.items[group->scanIndex];
								}

								if(subgroup) *subgroup=group;
								return index;
							}
						}

					}
					index++;
					if(group->cl.items[group->scanIndex]->type==CLCIT_GROUP && group->cl.items[group->scanIndex]->group->expanded) {
						group=group->cl.items[group->scanIndex]->group;
						group->scanIndex=0;
						continue;
					}
					group->scanIndex++;
		}
	};
	return -1;
}

HANDLE ContactToHItem(struct ClcContact *contact)
{
	switch(contact->type) {
case CLCIT_CONTACT:
	return contact->hContact;
case CLCIT_GROUP:
	return (HANDLE)(contact->groupId|HCONTACT_ISGROUP);
case CLCIT_INFO:
	return (HANDLE)((DWORD)contact->hContact|HCONTACT_ISINFO);
	}
	return NULL;
}

HANDLE ContactToItemHandle(struct ClcContact *contact,DWORD *nmFlags)
{
	switch(contact->type) {
case CLCIT_CONTACT:
	return contact->hContact;
case CLCIT_GROUP:
	if(nmFlags) *nmFlags|=CLNF_ISGROUP;
	return (HANDLE)contact->groupId;
case CLCIT_INFO:
	if(nmFlags) *nmFlags|=CLNF_ISINFO;
	return (HANDLE)((DWORD)contact->hContact|HCONTACT_ISINFO);
	}
	return NULL;
}
// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright � 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001,2002 Jon Keating, Richard Hughes
// Copyright � 2002,2003,2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004,2005 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $Source$
// Revision       : $Revision$
// Last change on : $Date$
// Last change by : $Author$
//
// DESCRIPTION:
//
//  Functions that handles list of used server IDs, sends low-level packets for SSI information
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"



extern BYTE gbSsiEnabled;
extern HANDLE ghServerNetlibUser;
extern BOOL bIsSyncingCL;
extern char gpszICQProtoName[MAX_PATH];
extern int gnCurrentStatus;

static HANDLE hHookSettingChanged = NULL;
static HANDLE hHookContactDeleted = NULL;
static WORD* pwIDList = NULL;
static int nIDListCount = 0;
static int nIDListSize = 0;



// Add a server ID to the list of reserved IDs.
// To speed up the process, IDs cannot be removed, and if
// you try to reserve an ID twice, it will be added again.
// You should call CheckServerID before reserving an ID.
void ReserveServerID(WORD wID)
{
	if (nIDListCount >= nIDListSize)
	{
		nIDListSize += 100;
		pwIDList = (WORD*)realloc(pwIDList, nIDListSize * sizeof(WORD));
	}

	pwIDList[nIDListCount] = wID;
	nIDListCount++;
	
}


// Remove a server ID from the list of reserved IDs.
// Used for deleting contacts and other modifications.
void FreeServerID(WORD wID)
{
  int i, j;

  if (pwIDList)
  {
    for (i = 0; i<nIDListCount; i++)
		{
			if (pwIDList[i] == wID)
      { // we found it, so remove
        for (j = i+1; j<nIDListCount; j++)
        {
          pwIDList[j-1] = pwIDList[j];
        }
        nIDListCount--;
      }
		}
	}
  
}


// Returns true if dwID is reserved
BOOL CheckServerID(WORD wID, int wCount)
{
	int i;
	BOOL bFound = FALSE;
	
	if (pwIDList)
	{
		for (i = 0; i<nIDListCount; i++)
		{
			if ((pwIDList[i] >= wID) && (pwIDList[i] <= wID + wCount))
				bFound = TRUE;
		}
	}
	
	return bFound;	
}


void FlushServerIDs()
{
	
	SAFE_FREE(&pwIDList);
	nIDListCount = 0;
	nIDListSize = 0;
	
}


// Load all known server IDs from DB to list
void LoadServerIDs()
{
  HANDLE hContact;
  WORD wSrvID;

  if (wSrvID = DBGetContactSettingWord(NULL, gpszICQProtoName, "SrvAvatarID", 0))
    ReserveServerID(wSrvID);
  if (wSrvID = DBGetContactSettingWord(NULL, gpszICQProtoName, "SrvVisibilityID", 0))
    ReserveServerID(wSrvID);

  
  hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

  while (hContact)
  { // search all contacts, reserve their server IDs
    if (wSrvID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvGroupId", 0))
      ReserveServerID(wSrvID);
    if (wSrvID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvContactId", 0))
      ReserveServerID(wSrvID);
    if (wSrvID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvDenyId", 0))
      ReserveServerID(wSrvID);
    if (wSrvID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvPermitId", 0))
      ReserveServerID(wSrvID);
    if (wSrvID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvIgnoreId", 0))
      ReserveServerID(wSrvID);

    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
  }

  return;
}


WORD GenerateServerId(VOID)
{

	WORD wId;


	while (TRUE)
	{
		// Randomize a new ID
		// Max value is probably 0x7FFF, lowest value is unknown.
		// We use range 0x1000-0x7FFF.
		wId = (WORD)RandRange(0x1000, 0x7FFF);

		if (!CheckServerID(wId, 0))
			break;
	}
	
	ReserveServerID(wId);

	return wId;
}

// Generate server ID with wCount IDs free after it, for sub-groups.
WORD GenerateServerIdPair(int wCount)
{
  WORD wId;

  while (TRUE)
  {
    // Randomize a new ID
    // Max value is probably 0x7FFF, lowest value is unknown.
    // We use range 0x1000-0x7FFF.
    wId = (WORD)RandRange(0x1000, 0x7FFF);

    if (!CheckServerID(wId, wCount))
      break;
	}
	
	ReserveServerID(wId);

	return wId;
}


/***********************************************
 *
 *  --- Low-level packet sending functions ---
 *
 */



DWORD icq_sendBuddy(DWORD dwCookie, WORD wAction, DWORD dwUin, WORD wGroupId, WORD wContactId, const char *szNick, const char*szNote, int authRequired, WORD wItemType)
{
  icq_packet packet;
  char szUin[10];
  int nUinLen;
  int nNickLen;
  int nNoteLen;
  char* szUtfNick = NULL;
  char* szUtfNote = NULL;
  WORD wTLVlen;

  // Prepare UIN
  _itoa(dwUin, szUin, 10);
  nUinLen = strlen(szUin);

  // Prepare custom utf-8 nick name
  if (szNick && (strlen(szNick) > 0))
  {
    int nResult;

    nResult = utf8_encode(szNick, &szUtfNick);
    nNickLen = strlen(szUtfNick);
  }
  else
  {
    nNickLen = 0;
  }

  // Prepare custom utf-8 note
  if (szNote && (strlen(szNote) > 0))
  {
    int nResult;

    nResult = utf8_encode(szNote, &szUtfNote);
    nNoteLen = strlen(szUtfNote);
  }
  else
  {
    nNoteLen = 0;
  }


  // Build the packet
  packet.wLen = nUinLen + 20;	
  if (nNickLen > 0)
    packet.wLen += nNickLen + 4;
  if (nNoteLen > 0)
    packet.wLen += nNoteLen + 4;
  if (authRequired)
    packet.wLen += 4;

  write_flap(&packet, ICQ_DATA_CHAN);
  packFNACHeader(&packet, ICQ_LISTS_FAMILY, wAction, 0, dwCookie);
  packWord(&packet, (WORD)nUinLen);
  packBuffer(&packet, szUin, (WORD)nUinLen);
  packWord(&packet, wGroupId);
  packWord(&packet, wContactId);
  packWord(&packet, wItemType);

  wTLVlen = ((nNickLen>0) ? 4+nNickLen : 0) + ((nNoteLen>0) ? 4+nNoteLen : 0) + (authRequired?4:0);
  packWord(&packet, wTLVlen);
  if (authRequired)
    packDWord(&packet, 0x00660000);  // "Still waiting for auth" TLV
  if (nNickLen > 0)
  {
    packWord(&packet, 0x0131);	// Nickname TLV
    packWord(&packet, (WORD)nNickLen);
    packBuffer(&packet, szUtfNick, (WORD)nNickLen);
  }
  if (nNoteLen > 0)
  {
    packWord(&packet, 0x013C);	// Comment TLV
    packWord(&packet, (WORD)nNoteLen);
    packBuffer(&packet, szUtfNote, (WORD)nNoteLen);
  }

	// Send the packet and return the cookie
  sendServPacket(&packet);

  SAFE_FREE(&szUtfNick);
  SAFE_FREE(&szUtfNote);

  return dwCookie;
}

DWORD icq_sendGroup(DWORD dwCookie, WORD wAction, WORD wGroupId, const char *szName, void *pContent, int cbContent)
{
  icq_packet packet;
  int nNameLen;
  char* szUtfName = NULL;
  WORD wTLVlen;

  // Prepare custom utf-8 group name
  if (szName && (strlen(szName) > 0))
  {
    int nResult;

    nResult = utf8_encode(szName, &szUtfName);
    nNameLen = strlen(szUtfName);
  }
  else
  {
    nNameLen = 0;
  }
  if (nNameLen == 0 && wGroupId != 0) return 0; // without name we could not change the group

  // Build the packet
  packet.wLen = nNameLen + 20;	
  if (cbContent > 0)
    packet.wLen += cbContent + 4;

  write_flap(&packet, ICQ_DATA_CHAN);
  packFNACHeader(&packet, ICQ_LISTS_FAMILY, wAction, 0, dwCookie);
  packWord(&packet, (WORD)nNameLen);
  if (nNameLen) packBuffer(&packet, szUtfName, (WORD)nNameLen);
  packWord(&packet, wGroupId);
  packWord(&packet, 0); // ItemId is always 0 for groups
  packWord(&packet, 1); // ItemType 1 = group

  wTLVlen = ((cbContent>0) ? 4+cbContent : 0);
  packWord(&packet, wTLVlen);
  if (cbContent > 0)
  {
    packWord(&packet, 0x0C8);	// Groups TLV
    packWord(&packet, (WORD)cbContent);
    packBuffer(&packet, pContent, (WORD)cbContent);
  }

	// Send the packet and return the cookie
  sendServPacket(&packet);

  SAFE_FREE(&szUtfName);

  return dwCookie;
}

DWORD icq_sendUploadContactServ(DWORD dwUin, WORD wGroupId, WORD wContactId, const char *szNick, int authRequired, WORD wItemType)
{

 /* icq_packet packet;
	char szUin[10];
	int nUinLen;
	int nNickLen;*/
	DWORD dwSequence;
	/*char* szUtfNick = NULL;
	WORD wTLVlen;


	// Prepare UIN
	_itoa(dwUin, szUin, 10);
	nUinLen = strlen(szUin);


	// Prepare custom nick name
	if (szNick && (strlen(szNick) > 0))
	{

		int nResult;

		nResult = utf8_encode(szNick, &szUtfNick);
		nNickLen = strlen(szUtfNick);

	}
	else
	{
		nNickLen = 0;
	}
*/

	// Cookie
	dwSequence = GenerateCookie(0);
/*
	// Build the packet
	packet.wLen = nUinLen + 20;	
	if (nNickLen > 0)
		packet.wLen += nNickLen + 4;
	if (authRequired)
		packet.wLen += 4;

	write_flap(&packet, ICQ_DATA_CHAN);
	packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_ADDTOLIST, 0, dwSequence);
	packWord(&packet, (WORD)nUinLen);
	packBuffer(&packet, szUin, (WORD)nUinLen);
	packWord(&packet, wGroupId);
	packWord(&packet, wContactId);
	packWord(&packet, wItemType);

	wTLVlen = ((nNickLen>0) ? 4+nNickLen : 0) + (authRequired?4:0);
	packWord(&packet, wTLVlen);
	if (authRequired)
		packDWord(&packet, 0x00660000);  // "Still waiting for auth" TLV
	if (nNickLen > 0)
	{
		packWord(&packet, 0x0131);	// Nickname TLV
		packWord(&packet, (WORD)nNickLen);
		packBuffer(&packet, szUtfNick, (WORD)nNickLen);
	}

	// Send the packet and return the cookie
	sendServPacket(&packet);

	SAFE_FREE(&szUtfNick);
*/
  icq_sendBuddy(dwSequence, ICQ_LISTS_ADDTOLIST, dwUin, wGroupId, wContactId, szNick, NULL, authRequired, wItemType);

	return dwSequence;
	
}


DWORD icq_sendDeleteServerContactServ(DWORD dwUin, WORD wGroupId, WORD wContactId, WORD wItemType)
{
/*
  	icq_packet packet;
	char szUin[10];
	int nUinLen;*/
	DWORD dwSequence;
/*
	_itoa(dwUin, szUin, 10);
	nUinLen = strlen(szUin);
*/
	dwSequence = GenerateCookie(0);
/*
	packet.wLen = nUinLen + 20;
	write_flap(&packet, ICQ_DATA_CHAN);
	packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_REMOVEFROMLIST, 0, dwSequence);
	packWord(&packet, (WORD)nUinLen);
	packBuffer(&packet, szUin, (WORD)nUinLen);
	packWord(&packet, wGroupId);
	packWord(&packet, wContactId);
	packWord(&packet, wItemType);
	packWord(&packet, 0x0000);     // Length of additional data

	sendServPacket(&packet);
*/
  icq_sendBuddy(dwSequence, ICQ_LISTS_REMOVEFROMLIST, dwUin, wGroupId, wContactId, NULL, NULL, 0, wItemType);
	return dwSequence;

}


/*****************************************
 *
 *     --- Contact DB Utilities ---
 *
 */

static int GroupNamesEnumProc(const char *szSetting,LPARAM lParam)
{ // we do nothing here, just return zero
  return 0;
}


int IsServerGroupsDefined()
{
  DBCONTACTENUMSETTINGS dbces;

  char szModule[MAX_PATH+6];

  strcpy(szModule, gpszICQProtoName);
  strcat(szModule, "Groups");

  dbces.pfnEnumProc = &GroupNamesEnumProc;
  dbces.szModule = szModule;

  if (CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)NULL, (LPARAM)&dbces))
    return 0; // no groups defined
  else 
    return 1;
}


// Look thru DB and collect all ContactIDs from a group
void* collectBuddyGroup(WORD wGroupID, int *count)
{
  WORD* buf = NULL;
  int cnt = 0;
  HANDLE hContact;
  WORD wItemID;

  hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

  while (hContact)
  { // search all contacts
    if (wGroupID == DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvGroupId", 0))
    { // add only buddys from specified group
      wItemID = DBGetContactSettingWord(hContact, gpszICQProtoName, "ServerId", 0);

      if (wItemID)
      { // valid ID, add
        cnt++;
        buf = (WORD*)realloc(buf, cnt*sizeof(WORD));
        buf[cnt-1] = wItemID;
      }
    }

    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
  }

  *count = cnt<<1; // we return size in bytes
  return buf;
}



// Look thru DB and collect all GroupIDs
void* collectGroups(int *count)
{
  WORD* buf = NULL;
  int cnt = 0;
  int i;
  HANDLE hContact;
  WORD wGroupID;

  hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

  while (hContact)
  { // search all contacts
    if (wGroupID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvGroupId", 0))
    { // add only valid IDs
      for (i = 0; i<cnt; i++)
      { // check for already added ids
        if (buf[i] == wGroupID) break;
      }

      if (i == cnt)
      { // not preset, add
        cnt++;
        buf = (WORD*)realloc(buf, cnt*sizeof(WORD));
        buf[i] = wGroupID;
      }
    }

    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
  }

  *count = cnt<<1;
  return buf;
}



char* getServerGroupName(WORD wGroupID)
{
  DBVARIANT dbv;
  char szModule[MAX_PATH+6];
  char szGroup[16];
  char *szRes;

  strcpy(szModule, gpszICQProtoName);
  strcat(szModule, "Groups");
  _itoa(wGroupID, szGroup, 0x10);

  if (DBGetContactSetting(NULL, szModule, szGroup, &dbv))
    szRes = NULL;
  else
    szRes = _strdup(dbv.pszVal);
  DBFreeVariant(&dbv);

  return szRes;
}



void setServerGroupName(WORD wGroupID, const char* szGroupName)
{
  char szModule[MAX_PATH+6];
  char szGroup[16];

  strcpy(szModule, gpszICQProtoName);
  strcat(szModule, "Groups");
  _itoa(wGroupID, szGroup, 0x10);

  if (szGroupName)
    DBWriteContactSettingString(NULL, szModule, szGroup, szGroupName);
  else
    DBDeleteContactSetting(NULL, szModule, szGroup);

  return;
}
 


WORD getServerGroupID(const char* szPath)
{
  char szModule[MAX_PATH+6];

  strcpy(szModule, gpszICQProtoName);
  strcat(szModule, "Groups");

  return DBGetContactSettingWord(NULL, szModule, szPath, 0);
}



void setServerGroupID(const char* szPath, WORD wGroupID)
{
  char szModule[MAX_PATH+6];

  strcpy(szModule, gpszICQProtoName);
  strcat(szModule, "Groups");

  DBWriteContactSettingWord(NULL, szModule, szPath, wGroupID);

  return;
}



// copied from groups.c - horrible, but only possible as this is not available as service
static int GroupNameExists(const char *name,int skipGroup)
{
	char idstr[33];
	DBVARIANT dbv;
	int i;

	for(i=0;;i++) {
		if(i==skipGroup) continue;
		itoa(i,idstr,10);
		if(DBGetContactSetting(NULL,"CListGroups",idstr,&dbv)) break;
		if(!strcmp(dbv.pszVal+1,name)) {
			DBFreeVariant(&dbv);
			return 1;
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}



// utility function which counts > on start of a server group name
int countGroupLevel(WORD wGroupId)
{
  char* szGroupName = getServerGroupName(wGroupId);
  int nNameLen = strlen(szGroupName);
  int i = 0;

  while (i<nNameLen)
  {
    if (szGroupName[i] != '>')
    {
      SAFE_FREE(&szGroupName);
      return i;
    }
    i++;
  }
  SAFE_FREE(&szGroupName);
  return -1; // something went wrong
}



// demangle group path
char* makeGroupPath(WORD wGroupId, DWORD bCanCreate)
{
  char* szGroup = NULL;

  if (szGroup = getServerGroupName(wGroupId))
  { // this groupid is not valid
    while (strstr(szGroup, "\\")!=NULL) *strstr(szGroup, "\\") = '_'; // remove invalid char
    if (getServerGroupID(szGroup) == wGroupId)
    { // this grouppath is known and is for this group, set user group
      return szGroup;
    }
    else
    {
      if (strlen(szGroup) && (szGroup[0] == '>'))
      { // it is probably a sub-group
        WORD wId = wGroupId-1;
        int level = countGroupLevel(wGroupId);
        int levnew = countGroupLevel(wId);
        char* szTempGroup;

        while ((levnew >= level) && (levnew != -1))
        { // we look for parent group
          wId--;
          levnew = countGroupLevel(wId);
        }
        if (levnew == -1)
        { // that was not a sub-group, it was just a group starting with >
          int hGroup;

          if (!GroupNameExists(szGroup, -1) && bCanCreate)
          { // if the group does not exist, create it
            hGroup = CallService(MS_CLIST_GROUPCREATE, 0, 0);
            CallService(MS_CLIST_GROUPRENAME, hGroup, (LPARAM)szGroup);
          }
          setServerGroupID(szGroup, wGroupId); // set grouppath id
          return szGroup;
        }

        szTempGroup = makeGroupPath(wId, bCanCreate);

        szTempGroup = realloc(szTempGroup, strlen(szGroup)+strlen(szTempGroup)+2);
        strcat(szTempGroup, "\\");
        strcat(szTempGroup, szGroup);
        SAFE_FREE(&szGroup);
        szGroup = szTempGroup;
        
        if (getServerGroupID(szGroup) == wGroupId)
        { // known path, give
          return szGroup;
        }
        else
        { // unknown path, create
          int hGroup;

          if (!GroupNameExists(szGroup, -1) && bCanCreate)
          { // if the group does not exist, create it
            hGroup = CallService(MS_CLIST_GROUPCREATE, 0, 0);
            CallService(MS_CLIST_GROUPRENAME, hGroup, (LPARAM)szGroup);
          }
          setServerGroupID(szGroup, wGroupId); // set grouppath id
          return szGroup;
        }
      }
      else
      { // create that group
        int hGroup;

        if (!GroupNameExists(szGroup, -1) && bCanCreate)
        { // if the group does not exist, create it
          hGroup = CallService(MS_CLIST_GROUPCREATE, 0, 0);
          CallService(MS_CLIST_GROUPRENAME, hGroup, (LPARAM)szGroup);
        }
        setServerGroupID(szGroup, wGroupId); // set grouppath id
        return szGroup;
      }
    }
  }
  return NULL;
}



// create group with this path, a bit complex task
// this supposes that all server groups are known
WORD makeGroupId(const char* szGroupPath, GROUPADDCALLBACK ofCallback, servlistcookie* lParam)
{
  WORD wGroupID = 0;
  char* szGroup = (char*)szGroupPath;

  if (!szGroup || szGroup[0]=='\0') szGroup = "General"; // TODO: make some default group name

  if (wGroupID = getServerGroupID(szGroup))
  {
    if (ofCallback) ofCallback(szGroup, wGroupID, (LPARAM)lParam);
    return wGroupID; // if the path is known give the id
  }

  if (!strstr(szGroup, "\\"))
  { // a root group can be simply created without problems
    servlistcookie* ack;
    DWORD dwCookie;

    if (ack = (servlistcookie*)malloc(sizeof(servlistcookie)))
    { // we have cookie good, go on
      ack->hContact = NULL;
      ack->wContactId = 0;
      ack->wGroupId = GenerateServerId();
      ack->szGroupName = NULL;
      ack->dwAction = SSA_GROUP_ADD;
      ack->dwUin = 0;
      ack->ofCallback = ofCallback;
      ack->lParam = (LPARAM)lParam;
      dwCookie = AllocateCookie(ICQ_LISTS_ADDTOLIST, 0, ack);

      sendAddStart();
      icq_sendGroup(dwCookie, ICQ_LISTS_ADDTOLIST, ack->wGroupId, szGroup, NULL, 0);

      return 0;
    }
  }
  else
  { // this is a sub-group
    // TODO: create subgroup, recursive, event-driven, possibly relocate 
  }
  
  if (strstr(szGroup, "\\") != NULL)
  { // we failed to get grouppath, trim it to root group
    strstr(szGroup, "\\")[0] = '\0';
    return makeGroupId(szGroupPath, ofCallback, lParam);
  }

  // TODO: remove this, it is now only as a last resort
  wGroupID = (WORD)DBGetContactSettingWord(NULL, gpszICQProtoName, "SrvDefGroupId", 0); 
  if (ofCallback) ofCallback(szGroupPath, wGroupID, (LPARAM)lParam);
  
  return wGroupID;
}


/*****************************************
 *
 *    --- Server-List Operations ---
 *
 */

void addServContactReady(const char* pszGroupPath, WORD wGroupID, LPARAM lParam)
{
  WORD wItemID;
  DWORD dwUin;
  servlistcookie* ack;
  DWORD dwCookie;

  if (!wGroupID) return; // something went wrong
	
  ack = (servlistcookie*)lParam;
  if (!ack) return;

  wItemID = DBGetContactSettingWord(ack->hContact, gpszICQProtoName, "ServerId", 0);

  if (wItemID)
  { // Only add the contact if it doesnt already have an ID
    Netlib_Logf(ghServerNetlibUser, "Failed to add contact to server side list (already there)");
    return;
  }

	// Get UIN
	if (!(dwUin = DBGetContactSettingDword(ack->hContact, gpszICQProtoName, UNIQUEIDSETTING, 0)))
  {
    // Could not do anything without uin
    Netlib_Logf(ghServerNetlibUser, "Failed to add contact to server side list (no UIN)");
    return;
  }

	wItemID = GenerateServerId();

  ack->dwAction = SSA_CONTACT_ADD;
  ack->dwUin = dwUin;
  ack->wGroupId = wGroupID;
  ack->wContactId = wItemID;

  dwCookie = AllocateCookie(ICQ_LISTS_ADDTOLIST, dwUin, ack);

  sendAddStart();
  icq_sendBuddy(dwCookie, ICQ_LISTS_ADDTOLIST, dwUin, wGroupID, wItemID, ack->szGroupName, NULL, 0, SSI_ITEM_BUDDY);
}



// Called when contact should be added to server list, if group does not exist, create one
DWORD addServContact(HANDLE hContact, const char *pszNick, const char *pszGroup)
{
  servlistcookie* ack;

  if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
  { // Could not do anything without cookie
    Netlib_Logf(ghServerNetlibUser, "Failed to add contact to server side list (malloc failed)");
    return 0;
  }
  else
  {
    ack->hContact = hContact;
    ack->szGroupName = _strdup(pszNick); // we need this for resending

    makeGroupId(pszGroup, addServContactReady, ack);
    return 1;
  }
}



// Called when contact should be removed from server list, remove group if it remain empty
DWORD removeServContact(HANDLE hContact)
{
  WORD wGroupID;
  WORD wItemID;
  DWORD dwUin;
  servlistcookie* ack;
  DWORD dwCookie;

  // Get the contact's group ID
  if (!(wGroupID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvGroupId", 0)))
  {
    // Could not find a usable group ID
    Netlib_Logf(ghServerNetlibUser, "Failed to remove contact from server side list (no group ID)");
    return 0;
  }

  // Get the contact's item ID
  if (!(wItemID = DBGetContactSettingWord(hContact, gpszICQProtoName, "ServerId", 0)))
  {
    // Could not find usable item ID
    Netlib_Logf(ghServerNetlibUser, "Failed to remove contact from server side list (no item ID)");
    return 0;
  }

	// Get UIN
	if (!(dwUin = DBGetContactSettingDword(hContact, gpszICQProtoName, UNIQUEIDSETTING, 0)))
  {
    // Could not do anything without uin
    Netlib_Logf(ghServerNetlibUser, "Failed to remove contact from server side list (no UIN)");
    return 0;
  }

  if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
  { // Could not do anything without cookie
    Netlib_Logf(ghServerNetlibUser, "Failed to remove contact from server side list (malloc failed)");
    return 0;
//    dwCookie = GenerateCookie(ICQ_LISTS_REMOVEFROMLIST);
  }
  else
  {
    ack->dwAction = SSA_CONTACT_REMOVE;
    ack->dwUin = dwUin;
    ack->hContact = hContact;
    ack->wGroupId = wGroupID;
    ack->wContactId = wItemID;

    dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, dwUin, ack);
  }

  sendAddStart();
  icq_sendBuddy(dwCookie, ICQ_LISTS_REMOVEFROMLIST, dwUin, wGroupID, wItemID, NULL, NULL, 0, SSI_ITEM_BUDDY);

  return 0;
}



void moveServContactReady(const char* pszGroupPath, WORD wNewGroupID, LPARAM lParam)
{
  WORD wItemID;
  WORD wGroupID;
  DWORD dwUin;
  servlistcookie* ack;
  DWORD dwCookie, dwCookie2;
  DBVARIANT dbvNote;
  char* pszNote;
  char* pszNick;
  BYTE bAuth;

  if (!wNewGroupID) return; // something went wrong
	
  ack = (servlistcookie*)lParam;
  if (!ack) return;

  wItemID = DBGetContactSettingWord(ack->hContact, gpszICQProtoName, "ServerId", 0);
  wGroupID = DBGetContactSettingWord(ack->hContact, gpszICQProtoName, "SrvGroupId", 0);

  if (!wItemID) // TODO: do not fail here, just add the user to the correct group
  { // Only move the contact if it had an ID
    Netlib_Logf(ghServerNetlibUser, "Failed to move contact to group on server side list (no ID)");
    return;
  }

  if (!wGroupID)
  { // Only move the contact if it had an GroupID
    Netlib_Logf(ghServerNetlibUser, "Failed to move contact to group on server side list (no Group)");
    return;
  }

  if (wGroupID == wNewGroupID)
  { // Only move the contact if it had different GroupID
    Netlib_Logf(ghServerNetlibUser, "Failed to move contact to group on server side list (same Group)");
    return;
  }

	// Get UIN
	if (!(dwUin = DBGetContactSettingDword(ack->hContact, gpszICQProtoName, UNIQUEIDSETTING, 0)))
  {
    // Could not do anything without uin
    Netlib_Logf(ghServerNetlibUser, "Failed to move contact to group on server side list (no UIN)");
    return;
  }

  // Read comment from DB
  if (DBGetContactSetting(ack->hContact, "UserInfo", "MyNotes", &dbvNote))
    pszNote = NULL; // if not read, no note
  else
    pszNote = dbvNote.pszVal;

  bAuth = DBGetContactSettingByte(ack->hContact, gpszICQProtoName, "Auth", 0);

  pszNick = ack->szGroupName;

  ack->szGroupName = NULL;
  ack->dwAction = SSA_CONTACT_SET_GROUP;
  ack->dwUin = dwUin;
  ack->wGroupId = wGroupID;
  ack->wContactId = wItemID;
  ack->wNewContactId = GenerateServerId(); // icq5 recreates also this, imitate
  ack->wNewGroupId = wNewGroupID;
  ack->lParam = 0; // we use this as a sign

  dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, dwUin, ack);
  dwCookie2 = AllocateCookie(ICQ_LISTS_ADDTOLIST, dwUin, ack);

  sendAddStart();
  icq_sendBuddy(dwCookie, ICQ_LISTS_REMOVEFROMLIST, dwUin, wGroupID, wItemID, ack->szGroupName, pszNote, bAuth, SSI_ITEM_BUDDY);
  icq_sendBuddy(dwCookie2, ICQ_LISTS_ADDTOLIST, dwUin, wNewGroupID, ack->wNewContactId, ack->szGroupName, pszNote, bAuth, SSI_ITEM_BUDDY);

  DBFreeVariant(&dbvNote);
  SAFE_FREE(&pszNick);
}



// Called when contact should be moved from one group to another, create new, remove empty
DWORD moveServContactGroup(HANDLE hContact, const char *pszNewGroup)
{
  servlistcookie* ack;
  char* pszNick;

  if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
  { // Could not do anything without cookie
    Netlib_Logf(ghServerNetlibUser, "Failed to add contact to server side list (malloc failed)");
    return 0;
  }
  else
  {
    DBVARIANT dbvNick;

    // Read nick name from DB
    if (DBGetContactSetting(hContact, "CList", "MyHandle", &dbvNick))
      pszNick = NULL; // if not read, no nick
    else
      pszNick = dbvNick.pszVal;
    
    ack->hContact = hContact;
    ack->szGroupName = _strdup(pszNick); // we need this for sending

    DBFreeVariant(&dbvNick);

    makeGroupId(pszNewGroup, moveServContactReady, ack);
    return 1;
  }
}



// Is called when a contact has been renamed locally to update
// the server side nick name.
DWORD renameServContact(HANDLE hContact, const char *pszNick)
{
  WORD wGroupID;
  WORD wItemID;
  DWORD dwUin;
  BOOL bAuthRequired;
  DBVARIANT dbvNote;
  char* pszNote;
  servlistcookie* ack;
  DWORD dwCookie;

  // Get the contact's group ID
  if (!(wGroupID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvGroupId", 0)))
  {
    // Could not find a usable group ID
    Netlib_Logf(ghServerNetlibUser, "Failed to upload new nick name to server side list (no group ID)");
    return 0;
  }

  // Get the contact's item ID
  if (!(wItemID = DBGetContactSettingWord(hContact, gpszICQProtoName, "ServerId", 0)))
  {
    // Could not find usable item ID
    Netlib_Logf(ghServerNetlibUser, "Failed to upload new nick name to server side list (no item ID)");
    return 0;
  }

  // Check if contact is authorized
  bAuthRequired = (DBGetContactSettingByte(hContact, gpszICQProtoName, "Auth", 0) == 1);

  // Read comment from DB
  if (DBGetContactSetting(hContact, "UserInfo", "MyNotes", &dbvNote))
    pszNote = NULL; // if not read, no note
  else
    pszNote = dbvNote.pszVal;

	// Get UIN
	if (!(dwUin = DBGetContactSettingDword(hContact, gpszICQProtoName, UNIQUEIDSETTING, 0)))
  {
    // Could not set nickname on server without uin
    Netlib_Logf(ghServerNetlibUser, "Failed to upload new nick name to server side list (no UIN)");

    DBFreeVariant(&dbvNote);
    return 0;
  }
  
  if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
  {
    // Could not allocate cookie - use old fake
    Netlib_Logf(ghServerNetlibUser, "Failed to allocate cookie");

    dwCookie = GenerateCookie(ICQ_LISTS_UPDATEGROUP);
  }
  else
  {
    ack->dwAction = SSA_CONTACT_RENAME;
    ack->wContactId = wItemID;
    ack->wGroupId = wGroupID;
    ack->dwUin = dwUin;
    ack->hContact = hContact;

    dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, dwUin, ack);
  }

  // There is no need to send ICQ_LISTS_CLI_MODIFYSTART or
  // ICQ_LISTS_CLI_MODIFYEND when just changing nick name
  icq_sendBuddy(dwCookie, ICQ_LISTS_UPDATEGROUP, dwUin, wGroupID, wItemID, pszNick, pszNote, bAuthRequired, 0 /* contact */);

  DBFreeVariant(&dbvNote);

  return dwCookie;
}



// Is called when a contact's note was changed to update
// the server side comment.
DWORD setServContactComment(HANDLE hContact, const char *pszNote)
{
  WORD wGroupID;
  WORD wItemID;
  DWORD dwUin;
  BOOL bAuthRequired;
  DBVARIANT dbvNick;
  char* pszNick;
  servlistcookie* ack;
  DWORD dwCookie;

  // Get the contact's group ID
  if (!(wGroupID = DBGetContactSettingWord(hContact, gpszICQProtoName, "SrvGroupId", 0)))
  {
    // Could not find a usable group ID
    Netlib_Logf(ghServerNetlibUser, "Failed to upload new comment to server side list (no group ID)");
    return 0;
  }

  // Get the contact's item ID
  if (!(wItemID = DBGetContactSettingWord(hContact, gpszICQProtoName, "ServerId", 0)))
  {
    // Could not find usable item ID
    Netlib_Logf(ghServerNetlibUser, "Failed to upload new comment to server side list (no item ID)");
    return 0;
  }

  // Check if contact is authorized
  bAuthRequired = (DBGetContactSettingByte(hContact, gpszICQProtoName, "Auth", 0) == 1);

  // Read nick name from DB
  if (DBGetContactSetting(hContact, "CList", "MyHandle", &dbvNick))
    pszNick = NULL; // if not read, no nick
  else
    pszNick = dbvNick.pszVal;

	// Get UIN
	if (!(dwUin = DBGetContactSettingDword(hContact, gpszICQProtoName, UNIQUEIDSETTING, 0)))
  {
    // Could not set comment on server without uin
    Netlib_Logf(ghServerNetlibUser, "Failed to upload new comment to server side list (no UIN)");

    DBFreeVariant(&dbvNick);
    return 0;
  }
  
  if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
  {
    // Could not allocate cookie - use old fake
    Netlib_Logf(ghServerNetlibUser, "Failed to allocate cookie");

    dwCookie = GenerateCookie(ICQ_LISTS_UPDATEGROUP);
  }
  else
  {
    ack->dwAction = SSA_CONTACT_COMMENT; 
    ack->wContactId = wItemID;
    ack->wGroupId = wGroupID;
    ack->dwUin = dwUin;
    ack->hContact = hContact;

    dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, dwUin, ack);
  }

  // There is no need to send ICQ_LISTS_CLI_MODIFYSTART or
  // ICQ_LISTS_CLI_MODIFYEND when just changing nick name
  icq_sendBuddy(dwCookie, ICQ_LISTS_UPDATEGROUP, dwUin, wGroupID, wItemID, pszNick, pszNote, bAuthRequired, 0 /* contact */);

  DBFreeVariant(&dbvNick);

  return dwCookie;
}


/*****************************************
 *
 *   --- Miranda Contactlist Hooks ---
 *
 */



static int ServListDbSettingChanged(WPARAM wParam, LPARAM lParam)
{
  DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;


  // We can't upload changes to NULL contact
  if ((HANDLE)wParam == NULL)
    return 0;

  // TODO: Queue changes that occur while offline
  if (!icqOnline || !gbSsiEnabled || bIsSyncingCL)
    return 0;

  if (!strcmp(cws->szModule, "CList"))
  {
    // Has a temporary contact just been added permanently?
    if (!strcmp(cws->szSetting, "NotOnList") &&
      (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0)) &&
      DBGetContactSettingByte(NULL, gpszICQProtoName, "ServerAddRemove", DEFAULT_SS_ADDREMOVE))
    {
      char* szProto;
      DWORD dwUin;

      dwUin = DBGetContactSettingDword((HANDLE)wParam, gpszICQProtoName, UNIQUEIDSETTING, 0);
      szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)wParam, 0);
			
      // Is this a ICQ contact and does it have a UIN?
      if (szProto && !strcmp(szProto, gpszICQProtoName) && dwUin)
      {
        char *pszNick;
        char *pszGroup;
        DBVARIANT dbvNick;
        DBVARIANT dbvGroup;
        
        // Read nick name from DB
        if (DBGetContactSetting((HANDLE)wParam, "CList", "MyHandle", &dbvNick))
          pszNick = NULL; // if not read, no nick
        else
          pszNick = dbvNick.pszVal;

        // Read group from DB
        if (DBGetContactSetting((HANDLE)wParam, "CList", "Group", &dbvGroup))
          pszGroup = NULL; // if not read, no group
        else
          pszGroup = dbvGroup.pszVal;

        addServContact((HANDLE)wParam, pszNick, pszGroup);

        DBFreeVariant(&dbvNick);
        DBFreeVariant(&dbvGroup);
      }
    }

    // Has contact been renamed?
    if (!strcmp(cws->szSetting, "MyHandle") &&
      DBGetContactSettingByte(NULL, gpszICQProtoName, "StoreServerDetails", DEFAULT_SS_STORE))
    {
      if (cws->value.type == DBVT_ASCIIZ && cws->value.pszVal != 0)
      {
        renameServContact((HANDLE)wParam, cws->value.pszVal);
      }
      else
      {
        renameServContact((HANDLE)wParam, NULL);
      }
    }

    // Has contact been moved to another group?
    if (!strcmp(cws->szSetting, "Group") &&
      DBGetContactSettingByte(NULL, gpszICQProtoName, "StoreServerDetails", DEFAULT_SS_STORE))
    {
      if (cws->value.type == DBVT_ASCIIZ && cws->value.pszVal != 0)
      {
        moveServContactGroup((HANDLE)wParam, cws->value.pszVal);
      }
      else
      {
        moveServContactGroup((HANDLE)wParam, NULL);
      }
    }		

  }

  if (!strcmp(cws->szModule, "UserInfo"))
  {
    if (!strcmp(cws->szSetting, "MyNotes") &&
      DBGetContactSettingByte(NULL, gpszICQProtoName, "StoreServerDetails", DEFAULT_SS_STORE))
    {
      if (cws->value.type == DBVT_ASCIIZ && cws->value.pszVal != 0)
      {
        setServContactComment((HANDLE)wParam, cws->value.pszVal);
      }
      else
      {
        setServContactComment((HANDLE)wParam, NULL);
      }
    }
  }

  return 0;
}



static int ServListDbContactDeleted(WPARAM wParam, LPARAM lParam)
{
	
	if (!icqOnline || !gbSsiEnabled)
		return 0;
	
	if (DBGetContactSettingByte(NULL, gpszICQProtoName, "ServerAddRemove", DEFAULT_SS_ADDREMOVE))
	{

		WORD wContactID;
		WORD wGroupID;
		WORD wVisibleID;
		WORD wInvisibleID;
    WORD wIgnoreID;
		DWORD dwUIN;

		
		wContactID = DBGetContactSettingWord((HANDLE)wParam, gpszICQProtoName, "ServerId", 0);
		wGroupID = DBGetContactSettingWord((HANDLE)wParam, gpszICQProtoName, "SrvGroupId", 0);
		wVisibleID = DBGetContactSettingWord((HANDLE)wParam, gpszICQProtoName, "SrvPermitId", 0);
		wInvisibleID = DBGetContactSettingWord((HANDLE)wParam, gpszICQProtoName, "SrvDenyId", 0);
		wIgnoreID = DBGetContactSettingWord((HANDLE)wParam, gpszICQProtoName, "SrvIgnoreId", 0);
		dwUIN = DBGetContactSettingDword((HANDLE)wParam, gpszICQProtoName, UNIQUEIDSETTING, 0);

		if ((wGroupID && wContactID) || wVisibleID || wInvisibleID || wIgnoreID)
		{

			if (wContactID)
      { // delete contact from server
        removeServContact((HANDLE)wParam);
      }

      if (wVisibleID)
      { // detete permit record
        servlistcookie* ack;
        DWORD dwCookie;

        if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
        { // cookie failed, use old fake
          dwCookie = GenerateCookie(ICQ_LISTS_REMOVEFROMLIST);
        }
        else
        {
          ack->dwAction = SSA_PRIVACY_REMOVE;
          ack->dwUin = dwUIN;
          ack->hContact = (HANDLE)wParam;
          ack->wGroupId = 0;
          ack->wContactId = wVisibleID;

          dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, dwUIN, ack);
        }

        icq_sendBuddy(dwCookie, ICQ_LISTS_REMOVEFROMLIST, dwUIN, 0, wVisibleID, NULL, NULL, 0, SSI_ITEM_PERMIT);
      }

      if (wInvisibleID)
      { // delete deny record
        servlistcookie* ack;
        DWORD dwCookie;

        if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
        { // cookie failed, use old fake
          dwCookie = GenerateCookie(ICQ_LISTS_REMOVEFROMLIST);
        }
        else
        {
          ack->dwAction = SSA_PRIVACY_REMOVE;
          ack->dwUin = dwUIN;
          ack->hContact = (HANDLE)wParam;
          ack->wGroupId = 0;
          ack->wContactId = wInvisibleID;

          dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, dwUIN, ack);
        }

        icq_sendBuddy(dwCookie, ICQ_LISTS_REMOVEFROMLIST, dwUIN, 0, wInvisibleID, NULL, NULL, 0, SSI_ITEM_DENY);
      }

      if (wIgnoreID)
      { // delete ignore record
        servlistcookie* ack;
        DWORD dwCookie;

        if (!(ack = (servlistcookie*)malloc(sizeof(servlistcookie))))
        { // cookie failed, use old fake
          dwCookie = GenerateCookie(ICQ_LISTS_REMOVEFROMLIST);
        }
        else
        {
          ack->dwAction = SSA_PRIVACY_REMOVE; // remove privacy item
          ack->dwUin = dwUIN;
          ack->hContact = (HANDLE)wParam;
          ack->wGroupId = 0;
          ack->wContactId = wIgnoreID;

          dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, dwUIN, ack);
        }

        icq_sendBuddy(dwCookie, ICQ_LISTS_REMOVEFROMLIST, dwUIN, 0, wIgnoreID, NULL, NULL, 0, SSI_ITEM_IGNORE);
      }
    }
  }

	return 0;

}



void InitServerLists(void)
{

	hHookSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ServListDbSettingChanged);
	hHookContactDeleted = HookEvent(ME_DB_CONTACT_DELETED, ServListDbContactDeleted);

}



void UninitServerLists(void)
{

	if (hHookSettingChanged)
		UnhookEvent(hHookSettingChanged);

	if (hHookContactDeleted)
		UnhookEvent(hHookContactDeleted);

	FlushServerIDs();

}

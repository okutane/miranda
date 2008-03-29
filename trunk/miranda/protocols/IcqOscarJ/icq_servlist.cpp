// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright � 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001-2002 Jon Keating, Richard Hughes
// Copyright � 2002-2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004-2008 Joe Kucera
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
// File name      : $URL$
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


// SERVER-LIST UPDATE BOARD
//

void CIcqProto::servlistBeginOperation(int operationCount, int bImport)
{
  if (operationCount)
  { // check if we should send operation begin packet
    if (!servlistEditCount)
      icq_sendServerBeginOperation(bImport);
    // update count of active operations
    servlistEditCount += operationCount;
#ifdef _DEBUG
    NetLog_Server("Server-List: Begin operation processed (%d operations active)", servlistEditCount);
#endif
  }
}

void CIcqProto::servlistEndOperation(int operationCount)
{
  if (operationCount)
  {
    if (operationCount > servlistEditCount)
    { // sanity check
      NetLog_Server("Error: Server-List End operation is not paired!");
      operationCount = servlistEditCount;
    }
    // update count of active operations
    servlistEditCount -= operationCount;
    // check if we should send operation end packet
    if (!servlistEditCount)
      icq_sendServerEndOperation();
#ifdef _DEBUG
    NetLog_Server("Server-List: End operation processed (%d operations active)", servlistEditCount);
#endif
  }
}

struct servlistqueuethreadparam {
  struct CIcqProto *ppro;
  int *queueState;
};

static unsigned __stdcall servlistQueueThreadStub(void* lParam)
{
  servlistqueuethreadparam *param = (servlistqueuethreadparam*)lParam;
  CIcqProto *ppro = param->ppro;
  int *queueState = param->queueState;

  SAFE_FREE(&lParam);
  return ppro->servlistQueueThread(queueState);
}

DWORD CIcqProto::servlistQueueThread(int *queueState)
{
#ifdef _DEBUG
  NetLog_Server("Server-List: Starting Update board.");
#endif
  SleepEx(50, FALSE);
  // handle server-list requests queue
  EnterCriticalSection(&servlistQueueMutex);
  while (servlistQueueCount)
  {
    ssiqueueditems* pItem = NULL;
    int bItemDouble;
    WORD wItemAction;
    icq_packet groupPacket = {0};
    icq_packet groupPacket2 = {0}; 
    servlistcookie* pGroupCookie = NULL;
    int nEndOperations;

    // first check if the state is calm
    while (*queueState) 
    {
      int i;
      time_t tNow = time(NULL);
      int bFound = FALSE;
      
      for (i = 0; i < servlistQueueCount; i++)
      { // check if we do not have some expired items to handle, otherwise keep sleeping
        if ((servlistQueueList[i]->tAdded + servlistQueueList[i]->dwTimeout) < tNow)
        { // got expired item, stop sleep even when changes goes on
          bFound = TRUE;
          break;
        }
      }
      if (bFound) break;
      // reset queue state, keep sleeping
      *queueState = FALSE; 
      LeaveCriticalSection(&servlistQueueMutex);
      SleepEx(100, TRUE);
      EnterCriticalSection(&servlistQueueMutex);
    }
    if (!icqOnline())
    { // do not try to send packets if offline
      LeaveCriticalSection(&servlistQueueMutex);
      SleepEx(100, TRUE);
      EnterCriticalSection(&servlistQueueMutex);
      continue;
    }
#ifdef _DEBUG
    NetLog_Server("Server-List: %d items in queue.", servlistQueueCount);
#endif
    // take the oldest item (keep the board FIFO)
    pItem = servlistQueueList[0]; // take first (queue contains at least one item here)
    wItemAction = (WORD)(pItem->pItems[0]->dwOperation & SSOF_ACTIONMASK);
    bItemDouble = pItem->pItems[0]->dwOperation & SSOG_DOUBLE;
    // check item rate - too high -> sleep
    EnterCriticalSection(&ratesMutex);
    {
      WORD wRateGroup = ratesGroupFromSNAC(m_rates, ICQ_LISTS_FAMILY, wItemAction);
      int nRateLevel = bItemDouble ? RML_IDLE_30 : RML_IDLE_10;

      while (ratesNextRateLevel(m_rates, wRateGroup) < ratesGetLimitLevel(m_rates, wRateGroup, nRateLevel))
      { // the rate is higher, keep sleeping
        int nDelay = ratesDelayToLevel(m_rates, wRateGroup, ratesGetLimitLevel(m_rates, wRateGroup, nRateLevel));

        LeaveCriticalSection(&ratesMutex);
        // do not keep the queue frozen
        LeaveCriticalSection(&servlistQueueMutex);
        if (nDelay < 10) nDelay = 10;
#ifdef _DEBUG
        NetLog_Server("Server-List: Delaying %dms [Rates]", nDelay);
#endif
        SleepEx(nDelay, FALSE);
        // check if the rate is now ok
        EnterCriticalSection(&servlistQueueMutex);
        EnterCriticalSection(&ratesMutex);
      }    
    }
    LeaveCriticalSection(&ratesMutex);
    { // setup group packet(s) & cookie
      int totalSize = 0;
      int i;
      servlistcookie *pGroupCookie;
      DWORD dwGroupCookie;
      // determine the total size of the packet
      for(i = 0; i < pItem->nItems; i++)
        totalSize += pItem->pItems[i]->packet.wLen - 0x10;

      // process begin & end operation flags
      {
        int bImportOperation = FALSE;
        int nBeginOperations = 0;

        nEndOperations = 0;
        for(i = 0; i < pItem->nItems; i++)
        { // collect begin & end operation flags
          if (pItem->pItems[i]->dwOperation & SSOF_BEGIN_OPERATION)
            nBeginOperations++;
          if (pItem->pItems[i]->dwOperation & SSOF_END_OPERATION)
            nEndOperations++;
          // check if the operation is import
          if (pItem->pItems[i]->dwOperation & SSOF_IMPORT_OPERATION)
            bImportOperation = TRUE;
        }
        // really begin operation if requested
        if (nBeginOperations)
          servlistBeginOperation(nBeginOperations, bImportOperation);
      }

      if (pItem->nItems > 1)
      { // pack all packet's data, create group cookie
        pGroupCookie = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
        pGroupCookie->dwAction = SSA_ACTION_GROUP;
        pGroupCookie->dwGroupCount = pItem->nItems;
        pGroupCookie->pGroupItems = (servlistcookie**)SAFE_MALLOC(pItem->nItems * sizeof(servlistcookie*));
        for (i = 0; i < pItem->nItems; i++)
        { // build group cookie data - assign cookies datas
          pGroupCookie->pGroupItems[i] = pItem->pItems[i]->cookie;
          // release the separate cookie id
          FreeCookieByData(CKT_SERVERLIST, pItem->pItems[i]->cookie);
        }
        // allocate cookie id
        dwGroupCookie = AllocateCookie(CKT_SERVERLIST, wItemAction, 0, pGroupCookie);
        // prepare packet data
        serverPacketInit(&groupPacket, (WORD)(totalSize + 0x0A)); // FLAP size added inside
        packFNACHeaderFull(&groupPacket, ICQ_LISTS_FAMILY, wItemAction, 0, dwGroupCookie);
        for (i = 0; i < pItem->nItems; i++)
          packBuffer(&groupPacket, pItem->pItems[i]->packet.pData + 0x10, (WORD)(pItem->pItems[i]->packet.wLen - 0x10));

        if (bItemDouble)
        { // prepare second packet
          wItemAction = ((servlistgroupitemdouble*)(pItem->pItems[0]))->wAction2;
          totalSize = 0;
          // determine the total size of the packet
          for(i = 0; i < pItem->nItems; i++)
            totalSize += ((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.wLen - 0x10;

          pGroupCookie = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
          pGroupCookie->dwAction = SSA_ACTION_GROUP;
          pGroupCookie->dwGroupCount = pItem->nItems;
          pGroupCookie->pGroupItems = (servlistcookie**)SAFE_MALLOC(pItem->nItems * sizeof(servlistcookie*));
          for (i = 0; i < pItem->nItems; i++)
            pGroupCookie->pGroupItems[i] = pItem->pItems[i]->cookie;
          // allocate cookie id
          dwGroupCookie = AllocateCookie(CKT_SERVERLIST, wItemAction, 0, pGroupCookie);
          // prepare packet data
          serverPacketInit(&groupPacket2, (WORD)(totalSize + 0x0A)); // FLAP size added inside
          packFNACHeaderFull(&groupPacket2, ICQ_LISTS_FAMILY, wItemAction, 0, dwGroupCookie);
          for (i = 0; i < pItem->nItems; i++)
            packBuffer(&groupPacket2, ((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.pData + 0x10, (WORD)(((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.wLen - 0x10));
        }
      }
      else
      { // just send the one packet, do not create action group
        pGroupCookie = pItem->pItems[0]->cookie;
        memcpy(&groupPacket, &pItem->pItems[0]->packet, sizeof(icq_packet));
        if (bItemDouble)
          memcpy(&groupPacket2, &((servlistgroupitemdouble*)(pItem->pItems[0]))->packet2, sizeof(icq_packet));
      }

      { // remove grouped item from queue & release grouped item
        servlistQueueCount--;
        servlistQueueList[0] = servlistQueueList[servlistQueueCount];

        for (i = 0; i < pItem->nItems; i++)
        { // release memory
          if (pItem->nItems > 1)
          { // free the packet only if we created the group packet
            SAFE_FREE((void**)&pItem->pItems[i]->packet.pData);
            if (pItem->pItems[i]->dwOperation & SSOG_DOUBLE)
              SAFE_FREE((void**)&((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.pData);
          }
          SAFE_FREE((void**)&pItem->pItems[i]);
          break;
        }
        SAFE_FREE((void**)&pItem);
        // resize the queue
        if (servlistQueueSize > servlistQueueCount + 4)
        {
          servlistQueueSize = servlistQueueCount / 4 + 1;
          servlistQueueList = (ssiqueueditems**)SAFE_REALLOC(servlistQueueList, servlistQueueSize * sizeof(ssiqueueditems*));
        }
      }
    }
    LeaveCriticalSection(&servlistQueueMutex);
    // send group packet
    sendServPacket(&groupPacket);
    // send second group packet (if present)
    if (bItemDouble)
      sendServPacket(&groupPacket2);
    // process end operation marks
    if (nEndOperations)
      servlistEndOperation(nEndOperations);
    // loose the loop a bit
    SleepEx(100, TRUE);
    EnterCriticalSection(&servlistQueueMutex);
  }
  // clean-up thread
  CloseHandle(servlistQueueThreadHandle);
  servlistQueueThreadHandle = NULL;
  LeaveCriticalSection(&servlistQueueMutex);
#ifdef _DEBUG
  NetLog_Server("Server-List: Update Board ending.");
#endif
  return 0;
}


void CIcqProto::servlistQueueAddGroupItem(servlistgroupitem* pGroupItem, int dwTimeout)
{
  EnterCriticalSection(&servlistQueueMutex);
  { // add the packet to queue
    DWORD dwMark = pGroupItem->dwOperation & SSOF_GROUPINGMASK;
    ssiqueueditems* pItem = NULL;
    
    // try to find compatible item
    for (int i = 0; i < servlistQueueCount; i++)
    {
      if ((servlistQueueList[i]->pItems[0]->dwOperation & SSOF_GROUPINGMASK) == dwMark && servlistQueueList[i]->nItems < MAX_SERVLIST_PACKET_ITEMS)
      { // found compatible item, check if it does not contain operation for the same server-list item
        pItem = servlistQueueList[i];

        for (int j = 0; j < pItem->nItems; j++)
          if (pItem->pItems[j]->cookie->wContactId == pGroupItem->cookie->wContactId &&
              pItem->pItems[j]->cookie->wGroupId == pGroupItem->cookie->wGroupId)
          {
            pItem = NULL;
            break;
          }
        // cannot send two operations for the same server-list record in one packet, look for another
        if (!pItem) continue;

#ifdef _DEBUG
        NetLog_Server("Server-List: Adding packet to item #%d with operation %x.", i, servlistQueueList[i]->pItems[0]->dwOperation);
#endif
        break;
      }
    }
    if (!pItem)
    { // compatible item was not found, create new one, add to queue
      pItem = (ssiqueueditems*)SAFE_MALLOC(sizeof(ssiqueueditems));
      pItem->tAdded = time(NULL);
      pItem->dwTimeout = dwTimeout;

      if (servlistQueueCount == servlistQueueSize)
      { // resize the queue - it is too small
        servlistQueueSize += 4;
        servlistQueueList = (ssiqueueditems**)SAFE_REALLOC(servlistQueueList, servlistQueueSize * sizeof(ssiqueueditems*));     
      }
      // really add to queue
      servlistQueueList[servlistQueueCount++] = pItem;
#ifdef _DEBUG
      NetLog_Server("Server-List: Adding new item to queue.");
#endif
    } 
    else if (pItem->dwTimeout > dwTimeout)
    { // if the timeout of currently added packet is shorter, update the previous one
      pItem->dwTimeout = dwTimeout;
    }
    // add GroupItem to queueditems (pItem)
    pItem->pItems[pItem->nItems++] = pGroupItem;
  }
  // wake up board thread (keep sleeping or start new one)  
  if (!servlistQueueThreadHandle)
  { // create new board thread
    servlistqueuethreadparam *init = (servlistqueuethreadparam*)SAFE_MALLOC(sizeof(servlistqueuethreadparam));

    if (init)
    {
      init->queueState = &servlistQueueState;
      init->ppro = this;

      servlistQueueThreadHandle = ICQCreateThreadEx(servlistQueueThreadStub, init, NULL);
    }
  }
  else // signal thread, that queue was changed during sleep
    servlistQueueState = TRUE;
  LeaveCriticalSection(&servlistQueueMutex);
}

int CIcqProto::servlistHandlePrimitives(DWORD dwOperation)
{
  if (dwOperation & SSO_BEGIN_OPERATION)
  { // operation starting, no action ready yet
    servlistBeginOperation(1, dwOperation & SSOF_IMPORT_OPERATION);
    return TRUE;
  }
  else if (dwOperation & SSO_END_OPERATION)
  { // operation ending without action
    servlistEndOperation(1);
    return TRUE;
  }

  return FALSE;
}

void CIcqProto::servlistPostPacket(icq_packet* packet, DWORD dwCookie, DWORD dwOperation, DWORD dwTimeout)
{
  servlistcookie* pCookie;

  if (servlistHandlePrimitives(dwOperation))
    return;

  if (!FindCookie(dwCookie, NULL, (void**)&pCookie))
    return; // invalid cookie

  if (dwOperation & SSOF_SEND_DIRECTLY)
  { // send directly - this is for some special cases
    // begin operation if requested
    if (dwOperation & SSOF_BEGIN_OPERATION)
      servlistBeginOperation(1, dwOperation & SSOF_IMPORT_OPERATION);

    // send the packet
    sendServPacket(packet);

    // end operation if requested
    if (dwOperation & SSOF_END_OPERATION)
      servlistEndOperation(1);
  }
  else
  { // add to server-list update board
    servlistgroupitem* pGroupItem;
    
    // prepare group item
    pGroupItem = (servlistgroupitem*)SAFE_MALLOC(sizeof(servlistgroupitem));
    pGroupItem->dwOperation = dwOperation;
    pGroupItem->cookie = pCookie;
    // packet data are alloced, keep them until they are sent
    memcpy(&pGroupItem->packet, packet, sizeof(icq_packet));

    servlistQueueAddGroupItem(pGroupItem, dwTimeout);
  }
}

void CIcqProto::servlistPostPacketDouble(icq_packet* packet1, DWORD dwCookie, DWORD dwOperation, DWORD dwTimeout, icq_packet* packet2, WORD wAction2)
{
  servlistcookie* pCookie;

  if (servlistHandlePrimitives(dwOperation))
    return;

  if (!FindCookie(dwCookie, NULL, (void**)&pCookie))
    return; // invalid cookie

  if (dwOperation & SSOF_SEND_DIRECTLY)
  { // send directly - this is for some special cases
    // begin operation if requested
    if (dwOperation & SSOF_BEGIN_OPERATION)
      servlistBeginOperation(1, dwOperation & SSOF_IMPORT_OPERATION);

    // send the packets
    sendServPacket(packet1);
    sendServPacket(packet2);

    // end operation if requested
    if (dwOperation & SSOF_END_OPERATION)
      servlistEndOperation(1);
  }
  else
  { // add to server-list update board
    servlistgroupitemdouble* pGroupItem;

    // prepare group item
    pGroupItem = (servlistgroupitemdouble*)SAFE_MALLOC(sizeof(servlistgroupitemdouble));
    pGroupItem->dwOperation = dwOperation;
    pGroupItem->cookie = pCookie;
    pGroupItem->wAction2 = wAction2;
    // packets data are alloced, keep them until they are sent
    memcpy(&pGroupItem->packet1, packet1, sizeof(icq_packet));
    memcpy(&pGroupItem->packet2, packet2, sizeof(icq_packet));

    servlistQueueAddGroupItem((servlistgroupitem*)pGroupItem, dwTimeout);
  }
}

void CIcqProto::servlistProcessLogin()
{
  // reset edit state counter
  servlistEditCount = 0;

  /// TODO: preserve queue state in DB! restore here!

  // if the server-list queue contains items and thread is not running, start it
  if (servlistQueueCount && !servlistQueueThreadHandle)
  {
    servlistqueuethreadparam *init = (servlistqueuethreadparam*)SAFE_MALLOC(sizeof(servlistqueuethreadparam));

    if (init)
    {
      init->queueState = &servlistQueueState;
      init->ppro = this;

      servlistQueueThreadHandle = ICQCreateThreadEx(servlistQueueThreadStub, init, NULL);
    }
  }
}

// HERE ENDS SERVER-LIST UPDATE BOARD IMPLEMENTATION //
///////////////////////////////////////////////////////
//===================================================//

// PENDING SERVER-LIST OPERATIONS
//
#define ITEM_PENDING_CONTACT    0x01
#define ITEM_PENDING_GROUP      0x02

#define CALLBACK_RESULT_CONTINUE  0x00
#define CALLBACK_RESULT_POSTPONE  0x0D
#define CALLBACK_RESULT_PURGE     0x10


#define SPOF_AUTO_CREATE_ITEM     0x01

int CIcqProto::servlistPendingFindItem(int nType, HANDLE hContact, const char *pszGroup)
{
  int i;

  if (servlistPendingList)
    for (i = 0; i < servlistPendingCount; i++)
      if (servlistPendingList[i]->nType == nType)
      { 
        if (((nType == ITEM_PENDING_CONTACT) && (servlistPendingList[i]->hContact == hContact)) ||
            ((nType == ITEM_PENDING_GROUP) && (!strcmpnull(servlistPendingList[i]->szGroup, pszGroup))))
          return i;
      }
  return -1;
}


void CIcqProto::servlistPendingAddItem(servlistpendingitem* pItem)
{
  if (servlistPendingCount >= servlistPendingSize) // add new
  {
    servlistPendingSize += 10;
    servlistPendingList = (servlistpendingitem**)SAFE_REALLOC(servlistPendingList, servlistPendingSize * sizeof(servlistpendingitem*));
  }

  servlistPendingList[servlistPendingCount++] = pItem;
}


servlistpendingitem* CIcqProto::servlistPendingRemoveItem(int nType, HANDLE hContact, const char *pszGroup)
{ // unregister pending item, trigger pending operations
  int iItem;
  servlistpendingitem *pItem = NULL;

  EnterCriticalSection(&servlistMutex);

  if ((iItem = servlistPendingFindItem(nType, hContact, pszGroup)) != -1)
  { // found, remove from the pending list
    pItem = servlistPendingList[iItem];

    servlistPendingList[iItem] = servlistPendingList[--servlistPendingCount];

    if (servlistPendingCount + 10 < servlistPendingSize)
    {
      servlistPendingSize -= 5;
      servlistPendingList = (servlistpendingitem**)SAFE_REALLOC(servlistPendingList, servlistPendingSize * sizeof(servlistpendingitem*));
    }
    // was the first operation was created automatically to postpone ItemAdd?
    if (pItem->operations && pItem->operations[0].flags & SPOF_AUTO_CREATE_ITEM)
    { // yes, add new item
      servlistpendingitem *pNewItem = (servlistpendingitem*)SAFE_MALLOC(sizeof(servlistpendingitem));

      if (pNewItem)
      { // move the remaining operations
#ifdef _DEBUG
        if (pItem->nType == ITEM_PENDING_CONTACT)
          NetLog_Server("Server-List: Resuming contact %x operation.", pItem->hContact);
        else
          NetLog_Server("Server-List: Resuming group \"%s\" operation.", pItem->szGroup);
#endif

        pNewItem->nType = pItem->nType;
        pNewItem->hContact = pItem->hContact;
        pNewItem->szGroup = null_strdup(pItem->szGroup);
        pNewItem->wContactID = pItem->wContactID;
        pNewItem->wGroupID = pItem->wGroupID;
        pNewItem->operationsCount = pItem->operationsCount - 1;
        pNewItem->operations = (servlistpendingoperation*)SAFE_MALLOC(pNewItem->operationsCount * sizeof(servlistpendingoperation));
        memcpy(pNewItem->operations, pItem->operations + 1, pNewItem->operationsCount * sizeof(servlistpendingoperation));
        pItem->operationsCount = 1;

        servlistPendingAddItem(pNewItem);
        // clear the flag
        pItem->operations[0].flags &= ~SPOF_AUTO_CREATE_ITEM;
      }
    }
  }
#ifdef _DEBUG
  else
    NetLog_Server("Server-List Error: Trying to remove non-existing pending %s.", nType == ITEM_PENDING_CONTACT ? "contact" : "group");
#endif

  LeaveCriticalSection(&servlistMutex);

  return pItem;
}


void CIcqProto::servlistPendingAddContactOperation(HANDLE hContact, LPARAM param, PENDING_CONTACT_CALLBACK callback, DWORD flags)
{ // add postponed operation (add contact, update contact, regroup resume, etc.)
  // - after contact is added
  int iItem;
  servlistpendingitem *pItem = NULL;

  EnterCriticalSection(&servlistMutex);

  if ((iItem = servlistPendingFindItem(ITEM_PENDING_CONTACT, hContact, NULL)) != -1)
    pItem = servlistPendingList[iItem];

  if (pItem)
  {
    int iOperation = pItem->operationsCount++;

    pItem->operations = (servlistpendingoperation*)SAFE_REALLOC(pItem->operations, pItem->operationsCount * sizeof(servlistpendingoperation));
    pItem->operations[iOperation].param = param;
    pItem->operations[iOperation].callback = (PENDING_GROUP_CALLBACK)callback;
    pItem->operations[iOperation].flags = flags;
  }
  else
  {
    NetLog_Server("Server-List Error: Trying to add pending operation to a non existing contact.");
  }
  LeaveCriticalSection(&servlistMutex);
}


void CIcqProto::servlistPendingAddGroupOperation(const char *pszGroup, LPARAM param, PENDING_GROUP_CALLBACK callback, DWORD flags)
{ // add postponed operation - after group is added
  int iItem;
  servlistpendingitem *pItem = NULL;

  EnterCriticalSection(&servlistMutex);

  if ((iItem = servlistPendingFindItem(ITEM_PENDING_GROUP, NULL, pszGroup)) != -1)
    pItem = servlistPendingList[iItem];

  if (pItem)
  {
    int iOperation = pItem->operationsCount++;

    pItem->operations = (servlistpendingoperation*)SAFE_REALLOC(pItem->operations, pItem->operationsCount * sizeof(servlistpendingoperation));
    pItem->operations[iOperation].param = param;
    pItem->operations[iOperation].callback = callback;
    pItem->operations[iOperation].flags = flags;
  }
  else
  {
    NetLog_Server("Server-List Error: Trying to add pending operation to a non existing group.");
  }
  LeaveCriticalSection(&servlistMutex);
}


int CIcqProto::servlistPendingAddContact(HANDLE hContact, WORD wContactID, WORD wGroupID, LPARAM param, PENDING_CONTACT_CALLBACK callback, int bDoInline, LPARAM operationParam, PENDING_CONTACT_CALLBACK operationCallback)
{
  int iItem;
  servlistpendingitem *pItem = NULL;

  EnterCriticalSection(&servlistMutex);

  if ((iItem = servlistPendingFindItem(ITEM_PENDING_CONTACT, hContact, NULL)) != -1)
    pItem = servlistPendingList[iItem];

  if (pItem)
  {
#ifdef _DEBUG
    NetLog_Server("Server-List: Pending contact %x already in list; adding as operation.", hContact);
#endif
    servlistPendingAddContactOperation(hContact, param, callback, SPOF_AUTO_CREATE_ITEM);

    if (operationCallback)
      servlistPendingAddContactOperation(hContact, operationParam, operationCallback, 0);

    LeaveCriticalSection(&servlistMutex);

    return 0; // Pending
  }

#ifdef _DEBUG
  NetLog_Server("Server-List: Starting contact %x operation.", hContact);
#endif

  pItem = (servlistpendingitem *)SAFE_MALLOC(sizeof(servlistpendingitem));
  pItem->nType = ITEM_PENDING_CONTACT;
  pItem->hContact = hContact;
  pItem->wContactID = wContactID;
  pItem->wGroupID = wGroupID;

  servlistPendingAddItem(pItem);

  if (operationCallback)
    servlistPendingAddContactOperation(hContact, operationParam, operationCallback, 0);

  LeaveCriticalSection(&servlistMutex);

  if (bDoInline)
  { // not postponed, called directly if requested
    (this->*callback)(hContact, wContactID, wGroupID, param, PENDING_RESULT_INLINE);
  }

  return 1; // Ready
}


int CIcqProto::servlistPendingAddGroup(const char *pszGroup, WORD wGroupID, LPARAM param, PENDING_GROUP_CALLBACK callback, int bDoInline, LPARAM operationParam, PENDING_GROUP_CALLBACK operationCallback)
{
  int iItem;
  servlistpendingitem *pItem = NULL;

  EnterCriticalSection(&servlistMutex);

  if ((iItem = servlistPendingFindItem(ITEM_PENDING_GROUP, NULL, pszGroup)) != -1)
    pItem = servlistPendingList[iItem];

  if (pItem)
  {
#ifdef _DEBUG
    NetLog_Server("Server-List: Pending group \"%s\" already in list; adding as operation.", pszGroup);
#endif
    servlistPendingAddGroupOperation(pszGroup, param, callback, SPOF_AUTO_CREATE_ITEM);

    if (operationCallback)
      servlistPendingAddGroupOperation(pszGroup, operationParam, operationCallback, 0);

    LeaveCriticalSection(&servlistMutex);

    return 0; // Pending
  }

#ifdef _DEBUG
  NetLog_Server("Server-List: Starting group \"%s\" operation.", pszGroup);
#endif

  pItem = (servlistpendingitem *)SAFE_MALLOC(sizeof(servlistpendingitem));
  pItem->nType = ITEM_PENDING_GROUP;
  pItem->szGroup = null_strdup(pszGroup);
  pItem->wGroupID = wGroupID;

  servlistPendingAddItem(pItem);

  if (operationCallback)
    servlistPendingAddGroupOperation(pszGroup, operationParam, operationCallback, 0);

  LeaveCriticalSection(&servlistMutex);

  if (bDoInline)
  { // not postponed, called directly if requested
    (this->*callback)(pszGroup, wGroupID, param, PENDING_RESULT_INLINE);
  }

  return 1; // Ready
}


void CIcqProto::servlistPendingRemoveContact(HANDLE hContact, WORD wContactID, WORD wGroupID, int nResult)
{
#ifdef _DEBUG
  NetLog_Server("Server-List: %s contact %x operation.", (nResult != PENDING_RESULT_PURGE) ? "Ending" : "Purging", hContact);
#endif

  servlistpendingitem *pItem = servlistPendingRemoveItem(ITEM_PENDING_CONTACT, hContact, NULL);

  if (pItem)
  { // process pending operations
    if (pItem->operations)
    {
      int i;

      for (i = 0; i < pItem->operationsCount; i++)
      {
        int nCallbackState = (this->*(PENDING_CONTACT_CALLBACK)(pItem->operations[i].callback))(hContact, wContactID, wGroupID, pItem->operations[i].param, nResult);

        if (nResult != PENDING_RESULT_PURGE && nCallbackState == CALLBACK_RESULT_POSTPONE)
        { // any following pending operations cannot be processed now, move them to the new pending contact
          int j;

          for (j = i + 1; j < pItem->operationsCount; j++)
            servlistPendingAddContactOperation(hContact, pItem->operations[j].param, (PENDING_CONTACT_CALLBACK)(pItem->operations[j].callback), pItem->operations[j].flags);
          break;
        }
        else if (nCallbackState == CALLBACK_RESULT_PURGE)
        { // purge all following operations - fatal failure occured
          nResult = PENDING_RESULT_PURGE;
        }
      }
      SAFE_FREE((void**)&pItem->operations);
    }
    // release item's memory
    SAFE_FREE((void**)&pItem);
  }
  else
    NetLog_Server("Server-List Error: Trying to remove a non existing pending contact.");
}


void CIcqProto::servlistPendingRemoveGroup(const char *pszGroup, WORD wGroupID, int nResult)
{
#ifdef _DEBUG
  NetLog_Server("Server-List: %s group \"%s\" operation.", (nResult != PENDING_RESULT_PURGE) ? "Ending" : "Purging", pszGroup);
#endif

  servlistpendingitem *pItem = servlistPendingRemoveItem(ITEM_PENDING_GROUP, NULL, pszGroup);

  if (pItem)
  { // process pending operations
    if (pItem->operations)
    {
      int i;

      for (i = 0; i < pItem->operationsCount; i++)
      {
        int nCallbackState = (this->*pItem->operations[i].callback)(pItem->szGroup, wGroupID, pItem->operations[i].param, nResult);

        if (nResult != PENDING_RESULT_PURGE && nCallbackState == CALLBACK_RESULT_POSTPONE)
        { // any following pending operations cannot be processed now, move them to the new pending group
          int j;

          for (j = i + 1; j < pItem->operationsCount; j++)
            servlistPendingAddGroupOperation(pItem->szGroup, pItem->operations[j].param, pItem->operations[j].callback, pItem->operations[j].flags);
          break;
        }
        else if (nCallbackState == CALLBACK_RESULT_PURGE)
        { // purge all following operations - fatal failure occured
          nResult = PENDING_RESULT_PURGE;
        }
      }
      SAFE_FREE((void**)&pItem->operations);
    }
    // release item's memory
    SAFE_FREE((void**)&pItem->szGroup);
    SAFE_FREE((void**)&pItem);
  }
  else
    NetLog_Server("Server-List Error: Trying to remove a non existing pending group.");
}


// Remove All pending operations
void CIcqProto::servlistPendingFlushOperations()
{
  int i;

  EnterCriticalSection(&servlistMutex);

  for (i = servlistPendingCount; i; i--)
  { // purge all items
    servlistpendingitem *pItem = servlistPendingList[i - 1];

    if (pItem->nType == ITEM_PENDING_CONTACT)
      servlistPendingRemoveContact(pItem->hContact, 0, 0, PENDING_RESULT_PURGE);
    else if (pItem->nType == ITEM_PENDING_GROUP)
      servlistPendingRemoveGroup(pItem->szGroup, 0, PENDING_RESULT_PURGE);
  }
  // release the list completely
  SAFE_FREE((void**)&servlistPendingList);
  servlistPendingCount = 0;
  servlistPendingSize = 0;

  LeaveCriticalSection(&servlistMutex);
}

// END OF SERVER-LIST PENDING OPERATIONS
////


// used for adding new contacts to list - sync with visible items
void CIcqProto::AddJustAddedContact(HANDLE hContact)
{
	EnterCriticalSection(&servlistMutex);
	if (nJustAddedCount >= nJustAddedSize)
	{
		nJustAddedSize += 10;
		pdwJustAddedList = (HANDLE*)SAFE_REALLOC(pdwJustAddedList, nJustAddedSize * sizeof(HANDLE));
	}

	pdwJustAddedList[nJustAddedCount] = hContact;
	nJustAddedCount++;  
	LeaveCriticalSection(&servlistMutex);
}

// was the contact added during this serv-list load
BOOL CIcqProto::IsContactJustAdded(HANDLE hContact)
{
	int i;

	EnterCriticalSection(&servlistMutex);
	if (pdwJustAddedList)
	{
		for (i = 0; i<nJustAddedCount; i++)
		{
			if (pdwJustAddedList[i] == hContact)
			{
				LeaveCriticalSection(&servlistMutex);
				return TRUE;
			}
		}
	}
	LeaveCriticalSection(&servlistMutex);

	return FALSE;
}

void CIcqProto::FlushJustAddedContacts()
{
	EnterCriticalSection(&servlistMutex);
	SAFE_FREE((void**)&pdwJustAddedList);
	nJustAddedSize = 0;
	nJustAddedCount = 0;
	LeaveCriticalSection(&servlistMutex);
}


// Add a server ID to the list of reserved IDs.
// To speed up the process, no checks is done, if
// you try to reserve an ID twice, it will be added again.
// You should call CheckServerID before reserving an ID.
void CIcqProto::ReserveServerID(WORD wID, int bGroupId)
{
	EnterCriticalSection(&servlistMutex);
	if (nIDListCount >= nIDListSize)
	{
		nIDListSize += 100;
		pwIDList = (DWORD*)SAFE_REALLOC(pwIDList, nIDListSize * sizeof(DWORD));
	}

	pwIDList[nIDListCount] = wID | bGroupId << 0x18;
	nIDListCount++;  
	LeaveCriticalSection(&servlistMutex);
}

// Remove a server ID from the list of reserved IDs.
// Used for deleting contacts and other modifications.
void CIcqProto::FreeServerID(WORD wID, int bGroupId)
{
	int i, j;
	DWORD dwId = wID | bGroupId << 0x18;

	EnterCriticalSection(&servlistMutex);
	if (pwIDList)
	{
		for (i = 0; i<nIDListCount; i++)
		{
			if (pwIDList[i] == dwId)
			{ // we found it, so remove
				for (j = i+1; j<nIDListCount; j++)
				{
					pwIDList[j-1] = pwIDList[j];
				}
				nIDListCount--;
			}
		}
	}
	LeaveCriticalSection(&servlistMutex);
}

// Returns true if dwID is reserved
BOOL CIcqProto::CheckServerID(WORD wID, unsigned int wCount)
{
	int i;

	EnterCriticalSection(&servlistMutex);
	if (pwIDList)
	{
		for (i = 0; i<nIDListCount; i++)
		{
			if (((pwIDList[i] & 0xFFFF) >= wID) && ((pwIDList[i] & 0xFFFF) <= wID + wCount))
			{
				LeaveCriticalSection(&servlistMutex);
				return TRUE;
			}
		}
	}
	LeaveCriticalSection(&servlistMutex);

	return FALSE;
}

void CIcqProto::FlushServerIDs()
{
	EnterCriticalSection(&servlistMutex);
	SAFE_FREE((void**)&pwIDList);
	nIDListCount = 0;
	nIDListSize = 0;
	LeaveCriticalSection(&servlistMutex);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct GroupReserveIdsEnumParam
{
	CIcqProto* ppro;
	char* szModule;
};

static int GroupReserveIdsEnumProc(const char *szSetting,LPARAM lParam)
{ 
	if (szSetting && strlennull(szSetting)<5)
	{ 
		// it is probably server group
		GroupReserveIdsEnumParam* param = (GroupReserveIdsEnumParam*)lParam;
		char val[MAX_PATH+2]; // dummy

		DBVARIANT dbv;
		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = val;
		dbv.cchVal = MAX_PATH;

		DBCONTACTGETSETTING cgs;
		cgs.szModule = param->szModule;
		cgs.szSetting = szSetting;
		cgs.pValue = &dbv;
		if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC,(WPARAM)NULL,(LPARAM)&cgs))
		{ // we failed to read setting, try also utf8 - DB bug
			dbv.type = DBVT_UTF8;
			dbv.pszVal = val;
			dbv.cchVal = MAX_PATH;
			if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC,(WPARAM)NULL,(LPARAM)&cgs))
				return 0; // we failed also, invalid setting
		}
		if (dbv.type != DBVT_ASCIIZ)
		{ // it is not a cached server-group name
			return 0;
		}
		param->ppro->ReserveServerID((WORD)strtoul(szSetting, NULL, 0x10), SSIT_GROUP);
#ifdef _DEBUG
		param->ppro->NetLog_Server("Loaded group %u:'%s'", strtoul(szSetting, NULL, 0x10), val);
#endif
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load all known server IDs from DB to list
void CIcqProto::LoadServerIDs()
{
	HANDLE hContact;
	WORD wSrvID;
	int nGroups = 0, nContacts = 0, nPermits = 0, nDenys = 0, nIgnores = 0;

	EnterCriticalSection(&servlistMutex);
	if (wSrvID = getSettingWord(NULL, "SrvAvatarID", 0))
		ReserveServerID(wSrvID, SSIT_ITEM);
	if (wSrvID = getSettingWord(NULL, "SrvPhotoID", 0))
		ReserveServerID(wSrvID, SSIT_ITEM);
	if (wSrvID = getSettingWord(NULL, "SrvVisibilityID", 0))
		ReserveServerID(wSrvID, SSIT_ITEM);

	DBCONTACTENUMSETTINGS dbces;
	int nStart = nIDListCount;

	char szModule[MAX_PATH+9];
	null_snprintf(szModule, sizeof(szModule), "%sSrvGroups", m_szModuleName);

	GroupReserveIdsEnumParam param = { this, szModule };
	dbces.pfnEnumProc = &GroupReserveIdsEnumProc;
	dbces.szModule = szModule;
	dbces.lParam = (LPARAM)&param;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)NULL, (LPARAM)&dbces);

	nGroups = nIDListCount - nStart;

	hContact = FindFirstContact();

	while (hContact)
	{ // search all our contacts, reserve their server IDs
		if (wSrvID = getSettingWord(hContact, "ServerId", 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM);
			nContacts++;
		}
		if (wSrvID = getSettingWord(hContact, "SrvDenyId", 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM);
			nDenys++;
		}
		if (wSrvID = getSettingWord(hContact, "SrvPermitId", 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM);
			nPermits++;
		}
		if (wSrvID = getSettingWord(hContact, "SrvIgnoreId", 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM);
			nIgnores++;
		}

		hContact = FindNextContact(hContact);
	}
	LeaveCriticalSection(&servlistMutex);

	NetLog_Server("Loaded SSI: %d contacts, %d groups, %d permit, %d deny, %d ignore items.", nContacts, nGroups, nPermits, nDenys, nIgnores);
}

WORD CIcqProto::GenerateServerId(int bGroupId)
{
	WORD wId;

	while (TRUE)
	{
		// Randomize a new ID
		// Max value is probably 0x7FFF, lowest value is probably 0x0001 (generated by Icq2Go)
		// We use range 0x1000-0x7FFF.
		wId = (WORD)RandRange(0x1000, 0x7FFF);

		if (!CheckServerID(wId, 0))
			break;
	}

	ReserveServerID(wId, bGroupId);

	return wId;
}

// Generate server ID with wCount IDs free after it, for sub-groups.
WORD CIcqProto::GenerateServerIdPair(int bGroupId, int wCount)
{
	WORD wId;

	while (TRUE)
	{
		// Randomize a new ID
		// Max value is probably 0x7FFF, lowest value is probably 0x0001 (generated by Icq2Go)
		// We use range 0x1000-0x7FFF.
		wId = (WORD)RandRange(0x1000, 0x7FFF);

		if (!CheckServerID(wId, wCount))
			break;
	}

	ReserveServerID(wId, bGroupId);

	return wId;
}


/***********************************************
*
*  --- Low-level packet sending functions ---
*
*/

struct doubleServerItemObject
{
  WORD wAction;
  icq_packet packet;
};

DWORD CIcqProto::icq_sendServerItem(DWORD dwCookie, WORD wAction, WORD wGroupId, WORD wItemId, const char *szName, BYTE *pTLVs, int nTlvLength, WORD wItemType, DWORD dwOperation, DWORD dwTimeout, void **doubleObject)
{ // generic packet
	icq_packet packet;
	int nNameLen;
	WORD wTLVlen = (WORD)nTlvLength;

	// Prepare item name length
	nNameLen = strlennull(szName);

	// Build the packet
	serverPacketInit(&packet, (WORD)(nNameLen + 20 + wTLVlen));
	packFNACHeaderFull(&packet, ICQ_LISTS_FAMILY, wAction, 0, dwCookie);
	packWord(&packet, (WORD)nNameLen);
	if (nNameLen) 
		packBuffer(&packet, (LPBYTE)szName, (WORD)nNameLen);
	packWord(&packet, wGroupId);
	packWord(&packet, wItemId);   
	packWord(&packet, wItemType); 
	packWord(&packet, wTLVlen);
	if (wTLVlen)
		packBuffer(&packet, pTLVs, wTLVlen);

  if (!doubleObject)
  { // Send the packet and return the cookie
    servlistPostPacket(&packet, dwCookie, dwOperation | wAction, dwTimeout);
  }
  else
  {
    if (*doubleObject)
    { // Send both packets and return the cookie
      doubleServerItemObject* helper = (doubleServerItemObject*)*doubleObject;

      servlistPostPacketDouble(&helper->packet, dwCookie, dwOperation | helper->wAction, dwTimeout, &packet, wAction);
      SAFE_FREE(doubleObject);
    }
    else
    { // Create helper object, return the cookie
      doubleServerItemObject* helper = (doubleServerItemObject*)SAFE_MALLOC(sizeof(doubleServerItemObject));

      if (helper)
      {
        helper->wAction = wAction;
        memcpy(&helper->packet, &packet, sizeof(icq_packet));
        *doubleObject = helper;
      }
      else // memory alloc failed
        return 0;
    }
  }

	// Force reload of server-list after change
	setSettingWord(NULL, "SrvRecordCount", 0);

	return dwCookie;
}


DWORD CIcqProto::icq_sendServerContact(HANDLE hContact, DWORD dwCookie, WORD wAction, WORD wGroupId, WORD wContactId, DWORD dwOperation, DWORD dwTimeout, void **doubleObject)
{
	DWORD dwUin;
	uid_str szUid;
	icq_packet pBuffer;
	char *szNick = NULL, *szNote = NULL;
	BYTE *pData = NULL;
	int nNickLen, nNoteLen, nDataLen;
	WORD wTLVlen;
	BYTE bAuth;
	int bDataTooLong = FALSE;

	// Prepare UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{
		NetLog_Server("Buddy upload failed (UID missing).");
		return 0;
	}

	bAuth = getSettingByte(hContact, "Auth", 0);
	szNick = getSettingStringUtf(hContact, "CList", "MyHandle", NULL);
	szNote = getSettingStringUtf(hContact, "UserInfo", "MyNotes", NULL);

	DBVARIANT dbv;

	if (!getSetting(hContact, "ServerData", &dbv))
	{ // read additional server item data
		nDataLen = dbv.cpbVal;
		pData = (BYTE*)_alloca(nDataLen);
		memcpy(pData, dbv.pbVal, nDataLen);

		ICQFreeVariant(&dbv);
	}
	else
	{
		pData = NULL;
		nDataLen = 0;
	}

	nNickLen = strlennull(szNick);
	nNoteLen = strlennull(szNote);

	// Limit the strings
	if (nNickLen > MAX_SSI_TLV_NAME_SIZE)
	{
		bDataTooLong = TRUE;
		nNickLen = null_strcut(szNick, MAX_SSI_TLV_NAME_SIZE);
	}
	if (nNoteLen > MAX_SSI_TLV_COMMENT_SIZE)
	{
		bDataTooLong = TRUE;
		nNoteLen = null_strcut(szNote, MAX_SSI_TLV_COMMENT_SIZE);
	}
	if (bDataTooLong)
	{ // Inform the user
		/// TODO: do something with this for Manage Server-List dialog.
		if (wAction != ICQ_LISTS_REMOVEFROMLIST) // do not report this when removing from list
			icq_LogMessage(LOG_WARNING, LPGEN("The contact's information was too big and was truncated."));
	}

	// Build the packet
	wTLVlen = (nNickLen?4+nNickLen:0) + (nNoteLen?4+nNoteLen:0) + (bAuth?4:0) + nDataLen;

	// Initialize our handy data buffer
	pBuffer.wPlace = 0;
	pBuffer.pData = (BYTE *)_alloca(wTLVlen);
	pBuffer.wLen = wTLVlen;

	if (nNickLen)
		packTLV(&pBuffer, SSI_TLV_NAME, (WORD)nNickLen, (LPBYTE)szNick);  // Nickname TLV

	if (nNoteLen)
		packTLV(&pBuffer, SSI_TLV_COMMENT, (WORD)nNoteLen, (LPBYTE)szNote);  // Comment TLV

	if (pData)
		packBuffer(&pBuffer, pData, (WORD)nDataLen);

	if (bAuth) // icq5 gives this as last TLV
		packDWord(&pBuffer, 0x00660000);  // "Still waiting for auth" TLV

	SAFE_FREE((void**)&szNick);
	SAFE_FREE((void**)&szNote);

	return icq_sendServerItem(dwCookie, wAction, wGroupId, wContactId, strUID(dwUin, szUid), pBuffer.pData, wTLVlen, SSI_ITEM_BUDDY, dwOperation, dwTimeout, doubleObject);
}


DWORD CIcqProto::icq_sendSimpleItem(DWORD dwCookie, WORD wAction, DWORD dwUin, char* szUID, WORD wGroupId, WORD wItemId, WORD wItemType, DWORD dwOperation, DWORD dwTimeout)
{ // for privacy items
	return icq_sendServerItem(dwCookie, wAction, wGroupId, wItemId, strUID(dwUin, szUID), NULL, 0, wItemType, dwOperation, dwTimeout, NULL);
}


DWORD CIcqProto::icq_sendServerGroup(DWORD dwCookie, WORD wAction, WORD wGroupId, const char *szName, void *pContent, int cbContent, DWORD dwOperationFlags)
{
	WORD wTLVlen;
	icq_packet pBuffer; // I reuse the ICQ packet type as a generic buffer
	// I should be ashamed! ;)

	if (strlennull(szName) == 0 && wGroupId != 0)
	{
		NetLog_Server("Group upload failed (GroupName missing).");
		return 0; // without name we could not change the group
	}

	// Calculate buffer size
	wTLVlen = (cbContent?4+cbContent:0);

	// Initialize our handy data buffer
	pBuffer.wPlace = 0;
	pBuffer.pData = (BYTE *)_alloca(wTLVlen);
	pBuffer.wLen = wTLVlen;

	if (wTLVlen)
		packTLV(&pBuffer, SSI_TLV_SUBITEMS, (WORD)cbContent, (LPBYTE)pContent);  // Groups TLV

	return icq_sendServerItem(dwCookie, wAction, wGroupId, 0, szName, pBuffer.pData, wTLVlen, SSI_ITEM_GROUP, SSOP_GROUP_ACTION | dwOperationFlags, 400, NULL);
}


DWORD CIcqProto::icq_modifyServerPrivacyItem(HANDLE hContact, DWORD dwUin, char *szUid, WORD wAction, DWORD dwOperation, WORD wItemId, WORD wType)
{
	servlistcookie *ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
	DWORD dwCookie;

	if (ack)
	{
		ack->dwAction = dwOperation; // remove privacy item
		ack->hContact = hContact;
		ack->wContactId = wItemId;

		dwCookie = AllocateCookie(CKT_SERVERLIST, wAction, hContact, ack);
	}
  else // cookie failed
    return 0;

	return icq_sendSimpleItem(dwCookie, wAction, dwUin, szUid, 0, wItemId, wType, SSOP_ITEM_ACTION, 400);
}


DWORD CIcqProto::icq_removeServerPrivacyItem(HANDLE hContact, DWORD dwUin, char *szUid, WORD wItemId, WORD wType)
{
	return icq_modifyServerPrivacyItem(hContact, dwUin, szUid, ICQ_LISTS_REMOVEFROMLIST, SSA_PRIVACY_REMOVE, wItemId, wType);
}


DWORD CIcqProto::icq_addServerPrivacyItem(HANDLE hContact, DWORD dwUin, char *szUid, WORD wItemId, WORD wType)
{
	return icq_modifyServerPrivacyItem(hContact, dwUin, szUid, ICQ_LISTS_ADDTOLIST, SSA_PRIVACY_ADD, wItemId, wType);
}

/*****************************************
*
*     --- Contact DB Utilities ---
*
*/

static int GroupNamesEnumProc(const char *szSetting,LPARAM lParam)
{
	// if we got pointer, store setting name, return zero
	if (lParam)
	{
		char** block = (char**)SAFE_MALLOC(2*sizeof(char*));
		block[1] = null_strdup(szSetting);
		block[0] = ((char**)lParam)[0];
		((char**)lParam)[0] = (char*)block;
	}
	return 0;
}


void DeleteModuleEntries(const char* szModule)
{
	DBCONTACTENUMSETTINGS dbces;
	char** list = NULL;

	dbces.pfnEnumProc = &GroupNamesEnumProc;
	dbces.szModule = szModule;
	dbces.lParam = (LPARAM)&list;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)NULL, (LPARAM)&dbces);
	while (list)
	{
		void* bet;

		DBDeleteContactSetting(NULL, szModule, list[1]);
		SAFE_FREE((void**)&list[1]);
		bet = list;
		list = (char**)list[0];
		SAFE_FREE((void**)&bet);
	}
}


/// TODO: do not check by plugin version, check by ServListStructures version!
int CIcqProto::IsServerGroupsDefined()
{
	int iRes = 1;

	if (getSettingDword(NULL, "Version", 0) < 0x00030608)
	{ // group cache & linking data too old, flush, reload from server
		char szModule[MAX_PATH+9];

		strcpy(szModule, m_szModuleName);
		strcat(szModule, "Groups"); // flush obsolete linking data
		DeleteModuleEntries(szModule);

		iRes = 0; // no groups defined, or older version
	}
	// store our current version
	setSettingDword(NULL, "Version", ICQ_PLUG_VERSION & 0x00FFFFFF);

	return iRes;
}


void CIcqProto::FlushSrvGroupsCache()
{
	char szModule[MAX_PATH+9];

	strcpy(szModule, m_szModuleName);
	strcat(szModule, "SrvGroups");
	DeleteModuleEntries(szModule);
}


// Look thru DB and collect all ContactIDs from a group
void* CIcqProto::collectBuddyGroup(WORD wGroupID, int *count)
{
	WORD* buf = NULL;
	int cnt = 0;
	HANDLE hContact;
	WORD wItemID;

	hContact = FindFirstContact();

	while (hContact)
	{ // search all contacts
		if (wGroupID == getSettingWord(hContact, "SrvGroupId", 0))
		{ // add only buddys from specified group
			wItemID = getSettingWord(hContact, "ServerId", 0);

			if (wItemID)
			{ // valid ID, add
				cnt++;
				buf = (WORD*)SAFE_REALLOC(buf, cnt*sizeof(WORD));
				buf[cnt-1] = wItemID;
        if (!count) break;
			}
		}

		hContact = FindNextContact(hContact);
	}

  if (count)
	  *count = cnt<<1; // we return size in bytes
	return buf;
}


// Look thru DB and collect all GroupIDs
void* CIcqProto::collectGroups(int *count)
{
	WORD* buf = NULL;
	int cnt = 0;
	int i;
	HANDLE hContact;
	WORD wGroupID;

	hContact = FindFirstContact();

	while (hContact)
	{ // search all contacts
		if (wGroupID = getSettingWord(hContact, "SrvGroupId", 0))
		{ // add only valid IDs
			for (i = 0; i<cnt; i++)
			{ // check for already added ids
				if (buf[i] == wGroupID) break;
			}

			if (i == cnt)
			{ // not preset, add
				cnt++;
				buf = (WORD*)SAFE_REALLOC(buf, cnt*sizeof(WORD));
				buf[i] = wGroupID;
			}
		}

		hContact = FindNextContact(hContact);
	}

	*count = cnt<<1;
	return buf;
}


static int GroupLinksEnumProc(const char *szSetting,LPARAM lParam)
{
	// check link target, add if match
	if (DBGetContactSettingWord(NULL, ((char**)lParam)[2], szSetting, 0) == (WORD)((char**)lParam)[1])
	{
		char** block = (char**)SAFE_MALLOC(2*sizeof(char*));
		block[1] = null_strdup(szSetting);
		block[0] = ((char**)lParam)[0];
		((char**)lParam)[0] = (char*)block;
	}
	return 0;
}

void CIcqProto::removeGroupPathLinks(WORD wGroupID)
{ // remove miranda grouppath links targeting to this groupid
	DBCONTACTENUMSETTINGS dbces;
	char szModule[MAX_PATH+6];
	char* pars[3];

	strcpy(szModule, m_szModuleName);
	strcat(szModule, "Groups");

	pars[0] = NULL;
	pars[1] = (char*)wGroupID;
	pars[2] = szModule;

	dbces.pfnEnumProc = &GroupLinksEnumProc;
	dbces.szModule = szModule;
	dbces.lParam = (LPARAM)pars;

	if (!CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)NULL, (LPARAM)&dbces))
	{ // we found some links, remove them
		char** list = (char**)pars[0];
		while (list)
		{
			void* bet;

			DBDeleteContactSetting(NULL, szModule, list[1]);
			SAFE_FREE((void**)&list[1]);
			bet = list;
			list = (char**)list[0];
			SAFE_FREE((void**)&bet);
		}
	}
}


char *CIcqProto::getServListGroupName(WORD wGroupID)
{
	char szModule[MAX_PATH+9];
	char szGroup[16];

	if (!wGroupID)
	{
		NetLog_Server("Warning: Cannot get group name (Group ID missing)!");
		return NULL;
	}

	strcpy(szModule, m_szModuleName);
	strcat(szModule, "SrvGroups");
	_itoa(wGroupID, szGroup, 0x10);

	if (!CheckServerID(wGroupID, 0))
	{ // check if valid id, if not give empty and remove
		NetLog_Server("Removing group %u from cache...", wGroupID);
		DBDeleteContactSetting(NULL, szModule, szGroup);
		return NULL;
	}

	return getSettingStringUtf(NULL, szModule, szGroup, NULL);
}


void CIcqProto::setServListGroupName(WORD wGroupID, const char *szGroupName)
{
	char szModule[MAX_PATH+9];
	char szGroup[16];

	if (!wGroupID)
	{
		NetLog_Server("Warning: Cannot set group name (Group ID missing)!");
		return;
	}

	strcpy(szModule, m_szModuleName);
	strcat(szModule, "SrvGroups");
	_itoa(wGroupID, szGroup, 0x10);

	if (szGroupName)
		setSettingStringUtf(NULL, szModule, szGroup, szGroupName);
	else
	{
		DBDeleteContactSetting(NULL, szModule, szGroup);
		removeGroupPathLinks(wGroupID);
	}
	return;
}


WORD CIcqProto::getServListGroupLinkID(const char *szPath)
{
	char szModule[MAX_PATH+6];
	WORD wGroupId;

	strcpy(szModule, m_szModuleName);
	strcat(szModule, "Groups");

	wGroupId = DBGetContactSettingWord(NULL, szModule, szPath, 0);

	if (wGroupId && !CheckServerID(wGroupId, 0))
	{ // known, check if still valid, if not remove
		NetLog_Server("Removing group \"%s\" from cache...", szPath);
		DBDeleteContactSetting(NULL, szModule, szPath);
		wGroupId = 0;
	}

	return wGroupId;
}


void CIcqProto::setServListGroupLinkID(const char *szPath, WORD wGroupID)
{
	char szModule[MAX_PATH+6];

	strcpy(szModule, m_szModuleName);
	strcat(szModule, "Groups");

	if (wGroupID)
		DBWriteContactSettingWord(NULL, szModule, szPath, wGroupID);
	else
		DBDeleteContactSetting(NULL, szModule, szPath);
}


// this function takes all backslashes in szGroup as group-level separators
int CIcqProto::getCListGroupHandle(const char *szGroup)
{
  char *pszGroup = (char*)szGroup;
  TCHAR *tszGroup = NULL;
  int hParentGroup = 0;
  int hGroup;

  if (!strlennull(szGroup)) return 0; // no group

  if (strrchr(szGroup, '\\'))
  { // create parent group
    char *szSeparator = (char*)strrchr(szGroup, '\\');

    *szSeparator = '\0';
    hParentGroup = getCListGroupHandle(szGroup);
    *szSeparator = '\\';
    // take only sub-group name
    pszGroup = ++szSeparator;
  }

  if (gbUnicodeCore)
    tszGroup = (TCHAR*)make_unicode_string(pszGroup);
  else
    utf8_decode(pszGroup, (char**)&tszGroup);

  hGroup = CallService(MS_CLIST_GROUPCREATE, hParentGroup, (LPARAM)tszGroup); // 0.7+
  SAFE_FREE((void**)&tszGroup);

#ifdef _DEBUG
  NetLog_Server("Obtained CList group \"%s\" handle %x", szGroup, hGroup);
#endif

  return hGroup;
}

// determine if the specified clist group path exists
//!! this function is not thread-safe due to the use of cli->pfnGetGroupName()
int CIcqProto::getCListGroupExists(const char *szGroup)
{
  int hGroup = 0;
  CLIST_INTERFACE *clint = NULL;

  if (ServiceExists(MS_CLIST_RETRIEVE_INTERFACE))
    clint = (CLIST_INTERFACE*)CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, 0);

  if (gbUnicodeCore && clint && clint->version >= 1)
  { // we've got unicode interface, use it
    int i;
    WCHAR *usGroup = make_unicode_string(szGroup);

    if (usGroup)
    {
      for (i = 1; TRUE; i++)
      {
        WCHAR *pusGroup = (WCHAR*)clint->pfnGetGroupName(i, NULL);

        if (!pusGroup) break;

        if (!wcscmp(usGroup, pusGroup))
        { // we found the group
          hGroup = i;
          break;
        }
      }
    }
    SAFE_FREE((void**)&usGroup);
  }
  else
  { // old ansi version - no other way
    int size = strlennull(szGroup) + 2;
    char* aszGroup = (char*)_alloca(size);
    int i;

    utf8_decode_static(szGroup, aszGroup, size);

    for (i = 1; TRUE; i++)
    {
      char *paszGroup = (char*)CallService(MS_CLIST_GROUPGETNAME, i, 0);

      if (!paszGroup) break;

      if (!strcmpnull(aszGroup, paszGroup))
      { // we found the group
        hGroup = i;
        break;
      }
    }
  }

  return hGroup;
}


int CIcqProto::moveContactToCListGroup(HANDLE hContact, const char *szGroup)
{
  int hGroup = getCListGroupHandle(szGroup);

  if (ServiceExists(MS_CLIST_CONTACTCHANGEGROUP))
    return CallService(MS_CLIST_CONTACTCHANGEGROUP, (WPARAM)hContact, hGroup);
  else /// TODO: is this neccessary ?
    return setSettingStringUtf(hContact, "CList", "Group", szGroup);
}


// utility function which counts > on start of a server group name
static int countGroupNameLevel(const char *szGroupName)
{
	int nNameLen = strlennull(szGroupName);
	int i = 0;

	while (i<nNameLen)
	{
		if (szGroupName[i] != '>')
			return i;

		i++;
	}
	return -1;
}

static int countCListGroupLevel(const char *szClistName)
{
	int nNameLen = strlennull(szClistName);
	int i, level = 0;

	for (i = 0; i < nNameLen; i++)
		if (szClistName[i] == '\\') level++;

	return level;
}

int CIcqProto::getServListGroupLevel(WORD wGroupId)
{
	char *szGroupName = getServListGroupName(wGroupId);
	int cnt = -1;

	if (szGroupName)
	{ // groupid is valid count group name level
		if (m_bSsiSimpleGroups)
			cnt = countCListGroupLevel(szGroupName);
		else
			cnt = countGroupNameLevel(szGroupName);

		SAFE_FREE((void**)&szGroupName);
	}

	return cnt;
}


// demangle group path
char *CIcqProto::getServListGroupCListPath(WORD wGroupId)
{
	char *szGroup = NULL;

	if (szGroup = getServListGroupName(wGroupId))
	{ // this groupid is valid
		if (!m_bSsiSimpleGroups)
			while (strstrnull(szGroup, "\\")) *strstrnull(szGroup, "\\") = '_'; // remove invalid char

		if (getServListGroupLinkID(szGroup) == wGroupId)
		{ // this grouppath is known and is for this group, set user group
			return szGroup;
		}
		else if (m_bSsiSimpleGroups)
		{ // with simple groups it is mapped 1:1, give real serv-list group name
			setServListGroupLinkID(szGroup, wGroupId);
			return szGroup;
		}
		else
		{ // advanced groups, determine group level
			int nGroupLevel = getServListGroupLevel(wGroupId);

			if (nGroupLevel > 0)
			{ // it is probably a sub-group locate parent group
				WORD wParentGroupId = wGroupId;
				int nParentGroupLevel;

				do
				{ // we look for parent group at the correct level
					wParentGroupId--;
					nParentGroupLevel = getServListGroupLevel(wParentGroupId);
				} while ((nParentGroupLevel >= nGroupLevel) && (nParentGroupLevel != -1));

				if (nParentGroupLevel == -1)
				{ // that was not a sub-group, it was just a group starting with >
					setServListGroupLinkID(szGroup, wGroupId);
					return szGroup;
				}

				{ // recursively determine parent group clist path
					char *szParentGroup = getServListGroupCListPath(wParentGroupId);

          /// FIXME: properly handle ~N suffixes
					szParentGroup = (char*)SAFE_REALLOC(szParentGroup, strlennull(szGroup) + strlennull(szParentGroup) + 2);
					strcat(szParentGroup, "\\");
					strcat(szParentGroup, (char*)szGroup + nGroupLevel);
/*          if (strstrnull(szGroup, "~"))
          { // check if the ~ was not added to obtain unique servlist item name
            char *szUniqueMark = strrchr(szParentGroup, '~');

            *szUniqueMark = '\0';
            // not the same group without ~, return it
            if (getServListGroupLinkID(szParentGroup) != wGroupId)
              *szUniqueMark = '~';
          } */ /// FIXME: this is necessary, but needs group loading changes
					SAFE_FREE((void**)&szGroup);
					szGroup = szParentGroup;


					if (getServListGroupLinkID(szGroup) == wGroupId)
					{ // known path, give
						return szGroup;
					}
					else
					{ // unknown path, setup a link
						setServListGroupLinkID(szGroup, wGroupId);
						return szGroup;
					}
				}
			}
			else
			{ // normal group, setup a link
				setServListGroupLinkID(szGroup, wGroupId);
				return szGroup;
			}
		}
	}
	return NULL;
}



static int SrvGroupNamesEnumProc(const char *szSetting, LPARAM lParam)
{ // check server-group cache item
  const char **params = (const char**)lParam;
  CIcqProto *ppro = (CIcqProto*)params[0];
  char *szGroupName = ppro->getSettingStringUtf(NULL, params[3], szSetting, NULL);

  if (!strcmpnull(szGroupName, params[2]))
    params[1] = szSetting; // do not need the real value, just arbitrary non-NULL

  SAFE_FREE((void**)&szGroupName);
  return 0;
}

char* CIcqProto::getServListUniqueGroupName(const char *szGroupName, int bAlloced)
{ // enum ICQSrvGroups and create unique name if neccessary
  DBCONTACTENUMSETTINGS dbces;
  char szModule[MAX_PATH+9];
  char *pars[4];
  int uniqueID = 1;
  char *szGroupNameBase = (char*)szGroupName;
  char *szNewGroupName = NULL;

  if (!bAlloced)
    szGroupNameBase = null_strdup(szGroupName);
  null_strcut(szGroupNameBase, m_wServerListRecordNameMaxLength);

  strcpy(szModule, m_szModuleName);
  strcat(szModule, "SrvGroups");

  do {
    pars[0] = (char*)this;
    pars[1] = NULL;
    pars[2] = szNewGroupName ? szNewGroupName : szGroupNameBase;
    pars[3] = szModule;

    dbces.pfnEnumProc = &SrvGroupNamesEnumProc;
    dbces.szModule = szModule;
    dbces.lParam = (LPARAM)pars;

    CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)NULL, (LPARAM)&dbces);

    if (pars[1])
    { // the groupname already exists, create another
      SAFE_FREE((void**)&szNewGroupName);

      char szUnique[10];
      _itoa(uniqueID++, szUnique, 10);
      null_strcut(szGroupNameBase, m_wServerListRecordNameMaxLength - strlennull(szUnique) - 1);
      szNewGroupName = (char*)SAFE_MALLOC(strlennull(szUnique) + strlennull(szGroupNameBase) + 2);
      if (szNewGroupName)
      {
        strcpy(szNewGroupName, szGroupNameBase);
        strcat(szNewGroupName, "~");
        strcat(szNewGroupName, szUnique);
      }
    }
  } while (pars[1] && szNewGroupName);

  if (szNewGroupName)
  {
    SAFE_FREE((void**)&szGroupNameBase);
    return szNewGroupName;
  }
  if (szGroupName != szGroupNameBase)
  {
    SAFE_FREE((void**)&szGroupNameBase);
    return (char*)szGroupName;
  }
  else
    return szGroupNameBase;
}


// this is the second part of recursive event-driven procedure
int CIcqProto::servlistCreateGroup_gotParentGroup(const char *szGroup, WORD wGroupID, LPARAM param, int nResult)
{
	servlistcookie* clue = (servlistcookie*)param;
  char *szSubGroupName = clue->szGroupName;
  char *szSubGroup;
  int wSubGroupLevel = -1;
  WORD wSubGroupID;

	SAFE_FREE((void**)&clue);

  if (nResult == PENDING_RESULT_PURGE)
  { // only cleanup
    return CALLBACK_RESULT_CONTINUE;
  }

  szSubGroup = (char*)SAFE_MALLOC(strlennull(szGroup) + strlennull(szSubGroupName) + 2);
  if (szSubGroup)
  {
    strcpy(szSubGroup, szGroup);
    strcat(szSubGroup, "\\");
    strcat(szSubGroup, szSubGroupName);
  }

  if (nResult == PENDING_RESULT_SUCCESS) // if we got an id count level
    wSubGroupLevel = getServListGroupLevel(wGroupID);

  if (wSubGroupLevel == -1)
  { // something went wrong, give the id and go away
    servlistPendingRemoveGroup(szSubGroup, wGroupID, PENDING_RESULT_FAILED);

    SAFE_FREE((void**)&szSubGroupName);
    SAFE_FREE((void**)&szSubGroup);
    return CALLBACK_RESULT_CONTINUE;
  }
  wSubGroupLevel++; // we are a sub-group
  wSubGroupID = wGroupID + 1;

	// check if on that id is not group of the same or greater level, if yes, try next
  while (CheckServerID(wSubGroupID, 0) && (getServListGroupLevel(wSubGroupID) >= wSubGroupLevel))
  {
    wSubGroupID++;
  }

	if (!CheckServerID(wSubGroupID, 0))
	{ // the next id is free, so create our group with that id
		servlistcookie *ack;
		DWORD dwCookie;
		char *szSubGroupItem = (char*)SAFE_MALLOC(strlennull(szSubGroupName) + wSubGroupLevel + 1);

		if (szSubGroupItem)
		{
			int i;

      for (i=0; i < wSubGroupLevel; i++)
        szSubGroupItem[i] = '>';

      strcpy(szSubGroupItem + wSubGroupLevel, szSubGroupName);
      szSubGroupItem[strlennull(szSubGroupName) + wSubGroupLevel] = '\0';
      SAFE_FREE((void**)&szSubGroupName);
      // check and create unique group name (Miranda does allow more subgroups with the same name!)
      szSubGroupItem = getServListUniqueGroupName(szSubGroupItem, TRUE);

			if (ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie)))
			{ // we have cookie good, go on
#ifdef _DEBUG
        NetLog_Server("Server-List: Creating sub-group \"%s\", parent group \"%s\".", szSubGroupItem, szGroup);
#endif
				ReserveServerID(wSubGroupID, SSIT_GROUP);

				ack->wGroupId = wSubGroupID;
				ack->szGroupName = szSubGroupItem; // we need that name
        ack->szGroup = szSubGroup;
				ack->dwAction = SSA_GROUP_ADD;
				dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, 0, ack);

        icq_sendServerGroup(dwCookie, ICQ_LISTS_ADDTOLIST, ack->wGroupId, szSubGroupItem, NULL, 0, SSOF_BEGIN_OPERATION);
        return CALLBACK_RESULT_CONTINUE;
			}
			SAFE_FREE((void**)&szSubGroupItem);
		}
	}
	// we failed to create sub-group give parent groupid
  icq_LogMessage(LOG_ERROR, "Failed to create the correct sub-group, the using closest parent group.");

  servlistPendingRemoveGroup(szSubGroup, wGroupID, PENDING_RESULT_FAILED);

  SAFE_FREE((void**)&szSubGroupName);
	SAFE_FREE((void**)&szSubGroup);
	return CALLBACK_RESULT_CONTINUE;
}


int CIcqProto::servlistCreateGroup_Ready(const char *szGroup, WORD groupID, LPARAM param, int nResult)
{
  WORD wGroupID = 0;

  if (nResult == PENDING_RESULT_PURGE)
    return CALLBACK_RESULT_CONTINUE;

  if (wGroupID = getServListGroupLinkID(szGroup))
  { // the path is known, continue the process
    servlistPendingRemoveGroup(szGroup, wGroupID, PENDING_RESULT_SUCCESS);
    return CALLBACK_RESULT_CONTINUE;
  }

  if (!strstrnull(szGroup, "\\") || m_bSsiSimpleGroups)
  { // a root group can be simply created without problems; simple groups are mapped directly
    servlistcookie* ack;
    DWORD dwCookie;

    if (ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie)))
    { // we have cookie good, go on
#ifdef _DEBUG
      NetLog_Server("Server-List: Creating root group \"%s\".", szGroup);
#endif
      ack->wGroupId = GenerateServerId(SSIT_GROUP);
      ack->szGroup = null_strdup(szGroup); // we need that name
      // check if the groupname is unique - just to be sure, Miranda should handle that!
      ack->szGroupName = getServListUniqueGroupName(ack->szGroup, FALSE);
      ack->dwAction = SSA_GROUP_ADD;
      dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, 0, ack);

      icq_sendServerGroup(dwCookie, ICQ_LISTS_ADDTOLIST, ack->wGroupId, ack->szGroup, NULL, 0, SSOF_BEGIN_OPERATION);

      return CALLBACK_RESULT_POSTPONE;
    }
  }
  else
  { // this is a sub-group
    char* szSub = null_strdup(szGroup); // create subgroup, recursive, event-driven, possibly relocate
    servlistcookie* ack;
    char *szLast;

    if (strstrnull(szSub, "\\"))
    { // determine parent group
      szLast = strrchr(szSub, '\\') + 1;

      szLast[-1] = '\0'; 
    }
    // make parent group id
    ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
    if (ack)
    {
      ack->szGroupName = null_strdup(szLast); // groupname
      servlistCreateGroup(szSub, (LPARAM)ack, &CIcqProto::servlistCreateGroup_gotParentGroup);
      SAFE_FREE((void**)&szSub);

      return CALLBACK_RESULT_POSTPONE;
    }

    SAFE_FREE((void**)&szSub); 
  }
  servlistPendingRemoveGroup(szGroup, groupID, PENDING_RESULT_FAILED);

  return CALLBACK_RESULT_CONTINUE;
}


// create group with this path, a bit complex task
// this supposes that all server groups are known
void CIcqProto::servlistCreateGroup(const char* szGroupPath, LPARAM param, PENDING_GROUP_CALLBACK callback)
{
  char* szGroup = (char*)szGroupPath;

  if (!strlennull(szGroup)) szGroup = DEFAULT_SS_GROUP;

  servlistPendingAddGroup(szGroup, 0, 0, &CIcqProto::servlistCreateGroup_Ready, TRUE, param, callback);
}


/*****************************************
*
*    --- Server-List Operations ---
*
*/

int CIcqProto::servlistAddContact_gotGroup(const char *szGroup, WORD wGroupID, LPARAM lParam, int nResult)
{
	WORD wItemID;
  servlistcookie* ack = (servlistcookie*)lParam;
  DWORD dwCookie;

  if (ack) SAFE_FREE((void**)&ack->szGroup);

  if (nResult == PENDING_RESULT_PURGE)
  { // only cleanup
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

	if (!ack || !wGroupID) // something went wrong
	{
    if (ack) servlistPendingRemoveContact(ack->hContact, 0, wGroupID, PENDING_RESULT_FAILED);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
	}

	wItemID = getSettingWord(ack->hContact, "ServerId", 0);

	if (wItemID) /// TODO: redundant ???
	{ // Only add the contact if it doesnt already have an ID
    servlistPendingRemoveContact(ack->hContact, wItemID, wGroupID, PENDING_RESULT_SUCCESS);
		NetLog_Server("Failed to add contact to server side list (%s)", "already there");
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
	}

	wItemID = GenerateServerId(SSIT_ITEM);

	ack->dwAction = SSA_CONTACT_ADD;
	ack->wGroupId = wGroupID;
	ack->wContactId = wItemID;

	dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, ack->hContact, ack);

  icq_sendServerContact(ack->hContact, dwCookie, ICQ_LISTS_ADDTOLIST, wGroupID, wItemID, SSOP_ITEM_ACTION | SSOF_CONTACT | SSOF_BEGIN_OPERATION, 400, NULL);

  return CALLBACK_RESULT_CONTINUE;
}


// Need to be called when Pending Contact is active
int CIcqProto::servlistAddContact_Ready(HANDLE hContact, WORD wContactID, WORD wGroupID, LPARAM lParam, int nResult)
{
  WORD wItemID;
  servlistcookie* ack = (servlistcookie*)lParam;

  if (nResult == PENDING_RESULT_PURGE)
  { // removing obsolete items, just free the memory
    SAFE_FREE((void**)&ack->szGroup);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  wItemID = getSettingWord(ack->hContact, "ServerId", 0);

  if (wItemID)
  { // Only add the contact if it doesn't already have an ID
    servlistPendingRemoveContact(ack->hContact, wItemID, getSettingWord(hContact, "SrvGroupId", 0), PENDING_RESULT_SUCCESS);
    NetLog_Server("Failed to add contact to server side list (%s)", "already there");
    SAFE_FREE((void**)&ack->szGroup);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }
  
  // obtain a correct groupid first
  servlistCreateGroup(ack->szGroup, lParam, &CIcqProto::servlistAddContact_gotGroup);

  return CALLBACK_RESULT_POSTPONE;
}


// Called when contact should be added to server list, if group does not exist, create one
void CIcqProto::servlistAddContact(HANDLE hContact, const char *pszGroup)
{
  DWORD dwUin;
  uid_str szUid;
	servlistcookie* ack;

  // Get UID
  if (getContactUid(hContact, &dwUin, &szUid))
  { // Could not do anything without uid
    NetLog_Server("Failed to add contact to server side list (%s)", "no UID");
    return;
  }

	if (!(ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie))))
	{ // Could not do anything without cookie
		NetLog_Server("Failed to add contact to server side list (%s)", "malloc failed");
		return;
	}
  else
  {
    ack->hContact = hContact;
    ack->szGroup = null_strdup(pszGroup);
    // call thru pending operations - makes sure the contact is ready to be added
    servlistPendingAddContact(hContact, 0, 0, 0, &CIcqProto::servlistAddContact_Ready, TRUE);
    return;
  }
}


int CIcqProto::servlistRemoveContact_Ready(HANDLE hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult)
{
  WORD wGroupID;
  WORD wItemID;
  servlistcookie* ack = (servlistcookie*)lParam;
  DWORD dwCookie;

  if (nResult == PENDING_RESULT_PURGE)
  {
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  // Get the contact's group ID
  if (!(wGroupID = getSettingWord(hContact, "SrvGroupId", 0)))
  { // Could not find a usable group ID
    servlistPendingRemoveContact(hContact, contactID, groupID, PENDING_RESULT_FAILED);

    NetLog_Server("Failed to remove contact from server side list (%s)", "no group ID");
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  // Get the contact's item ID
  if (!(wItemID = getSettingWord(hContact, "ServerId", 0)))
  { // Could not find usable item ID
    servlistPendingRemoveContact(hContact, contactID, wGroupID, PENDING_RESULT_FAILED);

    NetLog_Server("Failed to remove contact from server side list (%s)", "no item ID");
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  ack->dwAction = SSA_CONTACT_REMOVE;
  ack->hContact = hContact;
  ack->wGroupId = wGroupID;
  ack->wContactId = wItemID;

  dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_REMOVEFROMLIST, hContact, ack);

  icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_REMOVEFROMLIST, wGroupID, wItemID, SSOP_ITEM_ACTION | SSOF_CONTACT | SSOF_BEGIN_OPERATION, 400, NULL);

  return CALLBACK_RESULT_POSTPONE;
}


// Called when contact should be removed from server list, remove group if it remain empty
void CIcqProto::servlistRemoveContact(HANDLE hContact)
{
	DWORD dwUin;
	uid_str szUid;
	servlistcookie* ack;

	// Get UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{
		// Could not do anything without uid
		NetLog_Server("Failed to remove contact from server side list (%s)", "no UID");
		return;
	}

	if (!(ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie))))
	{ // Could not do anything without cookie
		NetLog_Server("Failed to remove contact from server side list (%s)", "malloc failed");
		return;
	}
	else
	{
    ack->hContact = hContact;
    // call thru pending operations - makes sure the contact is ready to be removed
    servlistPendingAddContact(hContact, 0, 0, 0, &CIcqProto::servlistRemoveContact_Ready, TRUE);
    return;
	}
}


int CIcqProto::servlistMoveContact_gotTargetGroup(const char *szGroup, WORD wNewGroupID, LPARAM lParam, int nResult)
{
	WORD wItemID;
	WORD wGroupID;
	servlistcookie* ack = (servlistcookie*)lParam;
	DWORD dwCookie, dwCookie2;

  if (ack) SAFE_FREE((void**)&ack->szGroup);

  if (nResult == PENDING_RESULT_PURGE)
  { // removing obsolete items, just free the memory
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

	if (!ack || !wNewGroupID || !ack->hContact) // something went wrong
	{
    if (ack) servlistPendingRemoveContact(ack->hContact, 0, 0, PENDING_RESULT_FAILED);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
	}

	wItemID = getSettingWord(ack->hContact, "ServerId", 0);
	wGroupID = getSettingWord(ack->hContact, "SrvGroupId", 0);

	if (!wItemID) 
	{ // We have no ID, so try to simply add the contact to serv-list 
		NetLog_Server("Unable to move contact (no ItemID) -> trying to add");
		// we know the GroupID, so directly call add
    return servlistAddContact_gotGroup(szGroup, wNewGroupID, lParam, nResult);
	}

	if (wGroupID == wNewGroupID)
	{ // Only move the contact if it had different GroupID
    servlistPendingRemoveContact(ack->hContact, wItemID, wNewGroupID, PENDING_RESULT_SUCCESS);
		NetLog_Server("Contact not moved to group on server side list (same Group)");
		return CALLBACK_RESULT_CONTINUE;
	}

	ack->szGroupName = NULL;
	ack->dwAction = SSA_CONTACT_SET_GROUP;
	ack->wGroupId = wGroupID;
	ack->wContactId = wItemID;
	ack->wNewContactId = GenerateServerId(SSIT_ITEM); // icq5 recreates also this, imitate
	ack->wNewGroupId = wNewGroupID;
	ack->lParam = 0; // we use this as a sign

	dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_REMOVEFROMLIST, ack->hContact, ack);
	dwCookie2 = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, ack->hContact, ack);

  { // imitate icq5, previously here was different order, but AOL changed and it ceased to work
    void* doubleObject = NULL;

    icq_sendServerContact(ack->hContact, dwCookie, ICQ_LISTS_REMOVEFROMLIST, wGroupID, wItemID, SSO_CONTACT_SETGROUP | SSOF_BEGIN_OPERATION, 500, &doubleObject);
    icq_sendServerContact(ack->hContact, dwCookie2, ICQ_LISTS_ADDTOLIST, wNewGroupID, ack->wNewContactId, SSO_CONTACT_SETGROUP | SSOF_BEGIN_OPERATION, 500, &doubleObject);
  }
  return CALLBACK_RESULT_CONTINUE;
}


int CIcqProto::servlistMoveContact_Ready(HANDLE hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult)
{
  WORD wItemID;
  WORD wGroupID;
  servlistcookie* ack = (servlistcookie*)lParam;

  if (nResult == PENDING_RESULT_PURGE)
  { // removing obsolete items, just free the memory
    SAFE_FREE((void**)&ack->szGroup);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  wItemID = getSettingWord(ack->hContact, "ServerId", 0);
  wGroupID = getSettingWord(ack->hContact, "SrvGroupId", 0);

  if (!wGroupID && wItemID)
  { // Only move the contact if it had an GroupID
    servlistPendingRemoveContact(ack->hContact, contactID, groupID, PENDING_RESULT_FAILED);

    NetLog_Server("Failed to move contact to group on server side list (%s)", "no Group");
    SAFE_FREE((void**)&ack->szGroup);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }
  
  // obtain a correct target groupid first
  servlistCreateGroup(ack->szGroup, lParam, &CIcqProto::servlistMoveContact_gotTargetGroup);

  return CALLBACK_RESULT_POSTPONE;
}


// Called when contact should be moved from one group to another, create new, remove empty
void CIcqProto::servlistMoveContact(HANDLE hContact, const char *pszNewGroup)
{
  DWORD dwUin;
  uid_str szUid;
	servlistcookie* ack;

  if (!hContact) return; // we do not move us, caused our uin was wrongly added to list

  // Get UID
  if (getContactUid(hContact, &dwUin, &szUid))
  { // Could not do anything without uin
    NetLog_Server("Failed to move contact to group on server side list (%s)", "no UID");
    return;
  }

	if (!getCListGroupExists(pszNewGroup) && (pszNewGroup != NULL) && (pszNewGroup[0]!='\0'))
	{ // the contact moved to non existing group, do not do anything: MetaContact hack
		NetLog_Server("Contact not moved - probably hiding by MetaContacts.");
		return;
	}

	if (!getSettingWord(hContact, "ServerId", 0))
	{ // the contact is not stored on the server, check if we should try to add
		if (!getSettingByte(NULL, "ServerAddRemove", DEFAULT_SS_ADDSERVER) ||
			DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
			return;
	}

	if (!(ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie))))
	{ // Could not do anything without cookie
		NetLog_Server("Failed to add contact to server side list (%s)", "malloc failed");
		return;
	}
	else
	{
		ack->hContact = hContact;
    ack->szGroup = null_strdup(pszNewGroup);
    // call thru pending operations - makes sure the contact is ready to be moved
    servlistPendingAddContact(hContact, 0, 0, (LPARAM)ack, &CIcqProto::servlistMoveContact_Ready, TRUE);
    return;
	}
}


int CIcqProto::servlistUpdateContact_Ready(HANDLE hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult)
{
  WORD wItemID;
  WORD wGroupID;
  servlistcookie* ack = (servlistcookie*)lParam;
  DWORD dwCookie;

  if (nResult == PENDING_RESULT_PURGE)
  { // removing obsolete items, just free the memory
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  // Get the contact's group ID
  if (!(wGroupID = getSettingWord(hContact, "SrvGroupId", 0)))
  {
    servlistPendingRemoveContact(hContact, contactID, groupID, PENDING_RESULT_FAILED);
    // Could not find a usable group ID
    NetLog_Server("Failed to update contact's details on server side list (%s)", "no group ID");
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  // Get the contact's item ID
  if (!(wItemID = getSettingWord(hContact, "ServerId", 0)))
  {
    servlistPendingRemoveContact(hContact, contactID, wGroupID, PENDING_RESULT_FAILED);
    // Could not find usable item ID
    NetLog_Server("Failed to update contact's details on server side list (%s)", "no item ID");
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  ack->dwAction = SSA_CONTACT_UPDATE;
  ack->wContactId = wItemID;
  ack->wGroupId = wGroupID;
  ack->hContact = hContact;

  dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, hContact, ack);

  // There is no need to send ICQ_LISTS_CLI_MODIFYSTART or
  // ICQ_LISTS_CLI_MODIFYEND when just changing nick name
  icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_UPDATEGROUP, wGroupID, wItemID, SSOP_ITEM_ACTION | SSOF_CONTACT, 400, NULL);

  return CALLBACK_RESULT_POSTPONE;
}


// Is called when a contact' details has been changed locally to update
// the server side details.
void CIcqProto::servlistUpdateContact(HANDLE hContact)
{
	DWORD dwUin;
	uid_str szUid;
	servlistcookie* ack;

	// Get UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{
		// Could not set nickname on server without uid
		NetLog_Server("Failed to update contact's details on server side list (%s)", "no UID");
		return;
	}

	if (!(ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie))))
	{
		// Could not allocate cookie - use old fake
    NetLog_Server("Failed to update contact's details on server side list (%s)", "malloc failed");
    return;
	}
	else
	{
    ack->hContact = hContact;
    // call thru pending operations - makes sure the contact is ready to be updated
    servlistPendingAddContact(hContact, 0, 0, (LPARAM)ack, &CIcqProto::servlistUpdateContact_Ready, TRUE);
    return;
  }
}


int CIcqProto::servlistRenameGroup_Ready(const char *szGroup, WORD wGroupID, LPARAM lParam, int nResult)
{
  servlistcookie *ack = (servlistcookie*)lParam;
  DWORD dwCookie;
  void* groupData;
  int groupSize;

  if (nResult == PENDING_RESULT_PURGE)
  { // only cleanup
    if (ack) SAFE_FREE((void**)&ack->szGroupName);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  if (!ack || !wGroupID) // something went wrong
  {
    servlistPendingRemoveGroup(szGroup, wGroupID, PENDING_RESULT_FAILED);

    if (ack) SAFE_FREE((void**)&ack->szGroupName);
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }

  if (groupData = collectBuddyGroup(wGroupID, &groupSize))
  {
    ack->dwAction = SSA_GROUP_RENAME;
    ack->wGroupId = wGroupID;
    ack->szGroup = null_strdup(szGroup); // we need this name
    // check if the new name is unique, create unique groupname if necessary
    ack->szGroupName = getServListUniqueGroupName(ack->szGroupName, TRUE);

    dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, 0, ack);

    icq_sendServerGroup(dwCookie, ICQ_LISTS_UPDATEGROUP, wGroupID, ack->szGroupName, groupData, groupSize, 0);
    SAFE_FREE(&groupData);
  }
  return CALLBACK_RESULT_POSTPONE;
}


void CIcqProto::servlistRenameGroup(char *szGroup, WORD wGroupId, char *szNewGroup)
{
	servlistcookie* ack;
  char *szGroupName, *szNewGroupName, *szLast;
  int nGroupLevel = getServListGroupLevel(wGroupId);
  int i;

	if (nGroupLevel == -1) return; // we failed to prepare group

  if (!m_bSsiSimpleGroups)
  {
    szGroupName = szGroup;
    i = nGroupLevel;
    while (i)
    { // find correct part of grouppath
      szGroupName = strstrnull(szGroupName, "\\") + 1;
      if (!szGroupName) return; // failed to get correct part of the grouppath
      i--;
    }
    szNewGroupName = szNewGroup;
    i = nGroupLevel;
    while (i)
    { // find correct part of new grouppath
      szNewGroupName = strstrnull(szNewGroupName, "\\") + 1;
      if (!szNewGroupName) return; // failed to get correct part of the new grouppath
      i--;
    }
    // truncate possible sub-groups
    szLast = strstrnull(szGroupName, "\\");
    if (szLast)
      szLast[0] = '\0';
    szLast = strstrnull(szNewGroupName, "\\");
    if (szLast)
      szLast[0] = '\0';

    // this group was not changed, nothing to rename
    if (!strcmpnull(szGroupName, szNewGroupName)) return;

    szGroupName = szNewGroupName;
    szNewGroupName = (char*)SAFE_MALLOC(strlennull(szGroupName) + 1 + nGroupLevel);
    if (!szNewGroupName) return; // Failure

    for (i = 0; i < nGroupLevel; i++)
    { // create level prefix
      szNewGroupName[i] = '>';
    }
    strcat(szNewGroupName, szGroupName);
  }
  else // simple groups do not require any conversion
    szNewGroupName = null_strdup(szNewGroup);

	if (!(ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie))))
	{ // cookie failed
    NetLog_Server("Error: Failed to allocate cookie");

    SAFE_FREE((void**)&szNewGroupName);
    return;
	}
  // store new group name for future use
  ack->szGroupName = szNewGroupName;
  // call thru pending operations - makes sure the group is ready for rename
  servlistPendingAddGroup(szGroup, wGroupId, (LPARAM)ack, &CIcqProto::servlistRenameGroup_Ready, TRUE);
}


int CIcqProto::servlistRemoveGroup_Ready(const char *szGroup, WORD groupID, LPARAM lParam, int nResult)
{
  servlistcookie* ack = (servlistcookie*)lParam;
  DWORD dwCookie;
  WORD wGroupID;
  char* szGroupName;

  if (nResult == PENDING_RESULT_PURGE)
  { // only cleanup
    SAFE_FREE((void**)&ack);
    return CALLBACK_RESULT_CONTINUE;
  }
  wGroupID = getServListGroupLinkID(szGroup);
  if (wGroupID && (szGroupName = getServListGroupName(wGroupID)))
  { // the group is valid, check if it is empty
    void* groupData = collectBuddyGroup(wGroupID, NULL);

    if (groupData)
    { // the group is not empty, cannot delete
      SAFE_FREE((void**)&groupData);
      SAFE_FREE((void**)&szGroupName);
      servlistPendingRemoveGroup(szGroup, wGroupID, PENDING_RESULT_SUCCESS);
      SAFE_FREE((void**)&ack);
      return CALLBACK_RESULT_CONTINUE;
    }

    if (!CheckServerID((WORD)(wGroupID+1), 0) || getServListGroupLevel((WORD)(wGroupID+1)) == 0)
    { // is next id an sub-group, if yes, we cannot delete this group
      ack->dwAction = SSA_GROUP_REMOVE;
      ack->wContactId = 0;
      ack->wGroupId = wGroupID;
      ack->hContact = NULL;
      ack->szGroup = null_strdup(szGroup); // we need that name
      ack->szGroupName = szGroupName;
      dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_REMOVEFROMLIST, 0, ack);

      icq_sendServerGroup(dwCookie, ICQ_LISTS_REMOVEFROMLIST, ack->wGroupId, ack->szGroupName, NULL, 0, 0);
    }
    return CALLBACK_RESULT_POSTPONE;
  }
  servlistPendingRemoveGroup(szGroup, groupID, PENDING_RESULT_SUCCESS);
  SAFE_FREE((void**)&ack);
  return CALLBACK_RESULT_CONTINUE;
}


void CIcqProto::servlistRemoveGroup(const char *szGroup, WORD wGroupId)
{
  servlistcookie *ack;

  if (!szGroup) return;

  if (!(ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie))))
  { // cookie failed
    NetLog_Server("Error: Failed to allocate cookie");
    return;
  }

  // call thru pending operations - makes sure the group is ready for removal
  servlistPendingAddGroup(szGroup, wGroupId, (LPARAM)ack, &CIcqProto::servlistRemoveGroup_Ready, TRUE);
}


void CIcqProto::resetServContactAuthState(HANDLE hContact, DWORD dwUin)
{
	WORD wContactId = getSettingWord(hContact, "ServerId", 0);
	WORD wGroupId = getSettingWord(hContact, "SrvGroupId", 0);

	if (wContactId && wGroupId)
	{
		DWORD dwCookie;
		servlistcookie* ack;

		if (ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie)))
		{ // we have cookie good, go on
			ack->hContact = hContact;
			ack->wContactId = wContactId;
			ack->wGroupId = wGroupId;
			ack->dwAction = SSA_CONTACT_FIX_AUTH;
			dwCookie = AllocateCookie(CKT_SERVERLIST, 0, hContact, ack);

      {
        void* doubleObject = NULL;

        icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_REMOVEFROMLIST, wGroupId, wContactId, SSO_CONTACT_FIXAUTH | SSOF_BEGIN_OPERATION | SSOF_END_OPERATION, 200, &doubleObject);
        deleteSetting(hContact, "ServerData");
        icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_ADDTOLIST, wGroupId, wContactId, SSO_CONTACT_FIXAUTH | SSOF_BEGIN_OPERATION | SSOF_END_OPERATION, 200, &doubleObject);
      }
		}
	}
}

/*****************************************
*
*   --- Miranda Contactlist Hooks ---
*
*/

int CIcqProto::ServListDbSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;

	// We can't upload changes to NULL contact
	if ((HANDLE)wParam == NULL)
  { // only note last change of CListGroups - contact/group operation detection
    if (!strcmpnull(cws->szModule, "CListGroups"))
      dwLastCListGroupsChange = time(NULL);

		return 0;
  }

	// TODO: Queue changes that occur while offline
	if ( !icqOnline() || !m_bSsiEnabled || bIsSyncingCL)
		return 0;

	{ // only our contacts will be handled
		if (IsICQContact((HANDLE)wParam))
			;// our contact, fine; otherwise return
		else 
			return 0;
	}

	if (!strcmpnull(cws->szModule, "CList"))
	{
		// Has a temporary contact just been added permanently?
		if (!strcmpnull(cws->szSetting, "NotOnList") &&
			(cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0)) &&
			getSettingByte(NULL, "ServerAddRemove", DEFAULT_SS_ADDSERVER) &&
			!DBGetContactSettingByte((HANDLE)wParam, "CList", "Hidden", 0))
		{ // Add to server-list
			AddServerContact(wParam, 0);
		}

		// Has contact been renamed?
		if (!strcmpnull(cws->szSetting, "MyHandle") &&
			getSettingByte(NULL, "StoreServerDetails", DEFAULT_SS_STORE))
    { // Update contact's details in server-list
      servlistUpdateContact((HANDLE)wParam);
		}

		// Has contact been moved to another group?
		if (!strcmpnull(cws->szSetting, "Group") &&
			getSettingByte(NULL, "StoreServerDetails", DEFAULT_SS_STORE))
    { // Read group from DB
      char* szNewGroup = getContactCListGroup((HANDLE)wParam);

      // it is contact operation only ? no, if CListGroups was changed less than 10 secs ago
      if (szNewGroup && (dwLastCListGroupsChange + 10 < time(NULL)))
        servlistMoveContact((HANDLE)wParam, szNewGroup);

      SAFE_FREE((void**)&szNewGroup);
    }
	}
	else if (!strcmpnull(cws->szModule, "UserInfo"))
	{
		if (!strcmpnull(cws->szSetting, "MyNotes") &&
			getSettingByte(NULL, "StoreServerDetails", DEFAULT_SS_STORE))
    { // Update contact's details in server-list
      servlistUpdateContact((HANDLE)wParam);
		}
	}

	return 0;
}


int CIcqProto::ServListDbContactDeleted(WPARAM wParam, LPARAM lParam)
{
	DeleteFromContactsCache((HANDLE)wParam);

	if ( !icqOnline() && m_bSsiEnabled)
	{ // contact was deleted only locally - retrieve full list on next connect
		setSettingWord((HANDLE)wParam, "SrvRecordCount", 0);
	}

	if ( !icqOnline() || !m_bSsiEnabled)
		return 0;

	{ // we need all server contacts on local buddy list
		WORD wContactID;
		WORD wGroupID;
		WORD wVisibleID;
		WORD wInvisibleID;
		WORD wIgnoreID;
		DWORD dwUIN;
		uid_str szUID;

		if (getContactUid((HANDLE)wParam, &dwUIN, &szUID))
			return 0;

		wContactID = getSettingWord((HANDLE)wParam, "ServerId", 0);
		wGroupID = getSettingWord((HANDLE)wParam, "SrvGroupId", 0);
		wVisibleID = getSettingWord((HANDLE)wParam, "SrvPermitId", 0);
		wInvisibleID = getSettingWord((HANDLE)wParam, "SrvDenyId", 0);
		wIgnoreID = getSettingWord((HANDLE)wParam, "SrvIgnoreId", 0);

		// Close all opened peer connections
		CloseContactDirectConns((HANDLE)wParam);

		if ((wGroupID && wContactID) || wVisibleID || wInvisibleID || wIgnoreID)
		{
			if (wContactID)
			{ // delete contact from server
				servlistRemoveContact((HANDLE)wParam);
			}

			if (wVisibleID)
			{ // detete permit record
				icq_removeServerPrivacyItem((HANDLE)wParam, dwUIN, szUID, wVisibleID, SSI_ITEM_PERMIT);
			}

			if (wInvisibleID)
			{ // delete deny record
				icq_removeServerPrivacyItem((HANDLE)wParam, dwUIN, szUID, wInvisibleID, SSI_ITEM_DENY);
			}

			if (wIgnoreID)
			{ // delete ignore record
				icq_removeServerPrivacyItem((HANDLE)wParam, dwUIN, szUID, wIgnoreID, SSI_ITEM_IGNORE);
			}
		}
	}

	return 0;
}


int CIcqProto::ServListCListGroupChange(WPARAM wParam, LPARAM lParam)
{
  HANDLE hContact = (HANDLE)wParam;
  CLISTGROUPCHANGE* grpchg = (CLISTGROUPCHANGE*)lParam;
	
  if (!icqOnline() || !m_bSsiEnabled || bIsSyncingCL)
    return 0;

  // only change server-list if it is allowed
  if (!getSettingByte(NULL, "StoreServerDetails", DEFAULT_SS_STORE))
    return 0;


  if (hContact == NULL)
  { // change made to group
    if (grpchg->pszNewName == NULL && grpchg->pszOldName != NULL)
    { // group removed
      char* szOldName = mtchar_to_utf8(grpchg->pszOldName);
      WORD wGroupId = getServListGroupLinkID(szOldName);
      if (wGroupId)
      { // the group is known, remove from server
        servlistPostPacket(NULL, 0, SSO_BEGIN_OPERATION, 100); // start server modifications here
        servlistRemoveGroup(szOldName, wGroupId);
      }
			SAFE_FREE((void**)&szOldName);
		}
		else if (grpchg->pszNewName != NULL && grpchg->pszOldName != NULL)
		{ // group renamed
			char* szNewName = mtchar_to_utf8(grpchg->pszNewName);
			char* szOldName = mtchar_to_utf8(grpchg->pszOldName);
			WORD wGroupId = getServListGroupLinkID(szOldName);

#ifdef _DEBUG
      NetLog_Server("CList-Events: Group %x:\"%s\" changed to \"%s\".", wGroupId, szOldName, szNewName);
#endif
			if (wGroupId)
			{ // group is known, rename on server
			  servlistRenameGroup(szOldName, wGroupId, szNewName);
      }
			SAFE_FREE((void**)&szOldName);
			SAFE_FREE((void**)&szNewName);
		}
	}
	else
	{ // change to contact
		if (IsICQContact(hContact))
		{ // our contact, fine move on the server as well
			char* szNewName = grpchg->pszNewName ? mtchar_to_utf8(grpchg->pszNewName) : NULL;
			
      servlistMoveContact(hContact, szNewName);
			SAFE_FREE((void**)&szNewName);
		}
	}
	return 0;
}

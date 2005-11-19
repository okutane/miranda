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
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"


extern DWORD dwLocalInternalIP;
extern DWORD dwLocalExternalIP;
extern WORD wListenPort;

extern void setUserInfo();
extern int GroupNameExistsUtf(const char *name,int skipGroup);

BOOL bIsSyncingCL = FALSE;

static void handleServerCListAck(servlistcookie* sc, WORD wError);
static void handleServerCList(unsigned char *buf, WORD wLen, WORD wFlags);
static void handleRecvAuthRequest(unsigned char *buf, WORD wLen);
static void handleRecvAuthResponse(unsigned char *buf, WORD wLen);
static void handleRecvAdded(unsigned char *buf, WORD wLen);
void sendRosterAck(void);



void handleServClistFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader)
{
  switch (pSnacHeader->wSubtype)
  {

  case ICQ_LISTS_ACK: // UPDATE_ACK
    if (wBufferLength >= 2)
    {
      WORD wError;
      DWORD dwActUin;
      servlistcookie* sc;

      unpackWord(&pBuffer, &wError);

      if (FindCookie(pSnacHeader->dwRef, &dwActUin, &sc))
      { // look for action cookie
#ifdef _DEBUG
        NetLog_Server("Received expected server list ack, action: %d, result: %d", sc->dwAction, wError);
#endif
        FreeCookie(pSnacHeader->dwRef); // release cookie

        handleServerCListAck(sc, wError);
      }
      else
      {
        NetLog_Server("Received unexpected server list ack %u", wError);
      }
    }
    break;

  case ICQ_LISTS_SRV_REPLYLISTS:
    {
      /* received list rights, we just skip them */

      oscar_tlv_chain* chain;

      if (chain = readIntoTLVChain(&pBuffer, wBufferLength, 0))
      {
        oscar_tlv* pTLV;

        if ((pTLV = getTLV(chain, 0x04, 1)) && pTLV->wLen>=30)
        { // limits for item types
          WORD* pMax = (WORD*)pTLV->pData;

          NetLog_Server("SSI: Max %d contacts, %d groups, %d permit, %d deny, %d ignore items.", pMax[0], pMax[1], pMax[2], pMax[3], pMax[14]);
        }

        disposeChain(&chain);
      }
#ifdef _DEBUG
      NetLog_Server("Server sent SNAC(x13,x03) - SRV_REPLYLISTS");
#endif
    }
    break;

  case ICQ_LISTS_LIST: // SRV_REPLYROSTER
  {
    servlistcookie* sc;
    BOOL blWork;

    blWork = bIsSyncingCL;
    bIsSyncingCL = TRUE; // this is not used if cookie takes place

    if (FindCookie(pSnacHeader->dwRef, NULL, &sc))
    { // we do it by reliable cookie
      if (!sc->dwUin)
      { // is this first packet ?
        ResetSettingsOnListReload();
        sc->dwUin = 1;
      }
      handleServerCList(pBuffer, wBufferLength, pSnacHeader->wFlags);
      if (!(pSnacHeader->wFlags & 0x0001))
      { // was that last packet ?
        FreeCookie(pSnacHeader->dwRef); // yes, release cookie
        SAFE_FREE(&sc);
      }
    }
    else
    { // use old fake
      if (!blWork)
      { // this can fail on some crazy situations
        ResetSettingsOnListReload();
      }
      handleServerCList(pBuffer, wBufferLength, pSnacHeader->wFlags);
    }
    break;
  }

  case ICQ_LISTS_UPTODATE: // SRV_REPLYROSTEROK
  {
    servlistcookie* sc;

    bIsSyncingCL = FALSE;

    if (FindCookie(pSnacHeader->dwRef, NULL, &sc))
    { // we requested servlist check
#ifdef _DEBUG
      NetLog_Server("Server stated roster is ok.");
#endif
      FreeCookie(pSnacHeader->dwRef);
      LoadServerIDs();

      SAFE_FREE(&sc);
    }
    else
      NetLog_Server("Server sent unexpected SNAC(x13,x0F) - SRV_REPLYROSTEROK");

    // This will activate the server side list
    sendRosterAck(); // this must be here, cause of failures during cookie alloc
    handleServUINSettings(wListenPort, dwLocalInternalIP);
    break;
  }

  case ICQ_LISTS_CLI_MODIFYSTART:
    NetLog_Server("Server sent SNAC(x13,x11) - Server is modifying contact list");
    break;

  case ICQ_LISTS_CLI_MODIFYEND:
    NetLog_Server("Server sent SNAC(x13,x12) - End of server modification");
    break;

  case ICQ_LISTS_UPDATEGROUP:
    NetLog_Server("Server sent SNAC(x13,x09) - Server updated our contact on list");
    break;

  case ICQ_LISTS_REMOVEFROMLIST:
    NetLog_Server("Server sent SNAC(x13,x0A) - User removed from our contact list");
    break;

  case ICQ_LISTS_ADDTOLIST:
    if (wBufferLength >= 10)
    {
      WORD wNameLen;
      WORD wGroupId, wItemId, wItemType, wTlvLen;

      unpackWord(&pBuffer, &wNameLen);
      if (wBufferLength >= 10 + wNameLen)
      {
        pBuffer += wNameLen;
        wBufferLength -= 10 + wNameLen;
        unpackWord(&pBuffer, &wGroupId);
        unpackWord(&pBuffer, &wItemId);
        unpackWord(&pBuffer, &wItemType);
        unpackWord(&pBuffer, &wTlvLen);
        if (wBufferLength >= wTlvLen && wItemType == SSI_ITEM_IMPORTTIME)
        {
          if (wTlvLen > 0)
          { // parse timestamp
            oscar_tlv_chain *pChain = readIntoTLVChain(&pBuffer, (WORD)(wTlvLen), 0);

            if (pChain) 
            {
              ICQWriteContactSettingDword(NULL, "ImportTS", getDWordFromChain(pChain, 0xD4, 1));
              ICQWriteContactSettingWord(NULL, "SrvImportID", wItemId);
              disposeChain(&pChain);

              NetLog_Server("Server added Import timestamp to list");

              break;
            }
          }
        }
      }
    }
    NetLog_Server("Server sent SNAC(x13,x08) - Server added something to our list");
    break;

  case ICQ_LISTS_AUTHREQUEST:
    handleRecvAuthRequest(pBuffer, wBufferLength);
    break;

  case ICQ_LISTS_SRV_AUTHRESPONSE:
    handleRecvAuthResponse(pBuffer, wBufferLength);
    break;

  case ICQ_LISTS_AUTHGRANTED:
    NetLog_Server("Server sent SNAC(x13,x15) - User granted us future authorization");
    break;

  case ICQ_LISTS_YOUWEREADDED:
    handleRecvAdded(pBuffer, wBufferLength);
    break;

  case ICQ_LISTS_ERROR:
    if (wBufferLength >= 2)
    {
      WORD wError;
      DWORD dwActUin;
      servlistcookie* sc;

      unpackWord(&pBuffer, &wError);

      if (FindCookie(pSnacHeader->dwRef, &dwActUin, &sc))
      { // look for action cookie
#ifdef _DEBUG
        NetLog_Server("Received server list error, action: %d, result: %d", sc->dwAction, wError);
#endif
        FreeCookie(pSnacHeader->dwRef); // release cookie

        if (sc->dwAction==SSA_CHECK_ROSTER)
        { // the serv-list is unavailable turn it off
          icq_LogMessage(LOG_ERROR, Translate("Server contact list is unavailable, Miranda will use local contact list."));
          gbSsiEnabled = 0;
          handleServUINSettings(wListenPort, dwLocalInternalIP);
        }
        SAFE_FREE(&sc);
      }
      else
      {
        LogFamilyError(ICQ_LISTS_FAMILY, wError);
      }
    }
    break;

  default:
    NetLog_Server("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_LISTS_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
    break;
  }
}



static void handleServerCListAck(servlistcookie* sc, WORD wError)
{
  switch (sc->dwAction)
  {
  case SSA_VISIBILITY: 
    {
      if (wError)
        NetLog_Server("Server visibility update failed, error %d", wError);
      break;
    }
  case SSA_CONTACT_RENAME: 
    {
      RemovePendingOperation(sc->hContact, 1);
      if (wError)
      {
        NetLog_Server("Renaming of server contact failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Renaming of server contact failed."));
      }
      SAFE_FREE(&sc->szUID);
      break;
    }
  case SSA_CONTACT_COMMENT: 
    {
      if (wError)
      {
        NetLog_Server("Update of server contact's comment failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Update of server contact's comment failed."));
      }
      SAFE_FREE(&sc->szUID);
      break;
    }
  case SSA_PRIVACY_ADD: 
    {
      if (wError)
      {
        NetLog_Server("Adding of privacy item to server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Adding of privacy item to server list failed."));
      }
      SAFE_FREE(&sc->szUID);
      break;
    }
  case SSA_PRIVACY_REMOVE: 
    {
      if (wError)
      {
        NetLog_Server("Removing of privacy item from server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Removing of privacy item from server list failed."));
      }
      FreeServerID(sc->wContactId, SSIT_ITEM); // release server id
      SAFE_FREE(&sc->szUID);
      break;
    }
  case SSA_CONTACT_ADD:
    {
      if (wError)
      {
        if (wError == 0xE) // server refused to add contact w/o auth, add with
        {
          DWORD dwCookie;

          NetLog_Server("Contact could not be added without authorisation, add with await auth flag.");

          ICQWriteContactSettingByte(sc->hContact, "Auth", 1); // we need auth
          dwCookie = AllocateCookie(ICQ_LISTS_ADDTOLIST, sc->dwUin, sc);
          icq_sendBuddyUtf(dwCookie, ICQ_LISTS_ADDTOLIST, sc->dwUin, sc->szUID, sc->wGroupId, sc->wContactId, sc->szGroupName, NULL, 1, SSI_ITEM_BUDDY);

          sc = NULL; // we do not want it to be freed now
          break;
        }
        FreeServerID(sc->wContactId, SSIT_ITEM);
        SAFE_FREE(&sc->szGroupName); // the the nick
        sendAddEnd(); // end server modifications here
        RemovePendingOperation(sc->hContact, 0);

        NetLog_Server("Adding of contact to server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Adding of contact to server list failed."));
        SAFE_FREE(&sc->szUID);
      }
      else
      {
        void* groupData;
        int groupSize;
        HANDLE hPend = sc->hContact;

        ICQWriteContactSettingWord(sc->hContact, "ServerId", sc->wContactId);
        ICQWriteContactSettingWord(sc->hContact, "SrvGroupId", sc->wGroupId);
        SAFE_FREE(&sc->szGroupName); // free the nick

        if (groupData = collectBuddyGroup(sc->wGroupId, &groupSize))
        { // the group is not empty, just update it
          DWORD dwCookie;

          sc->dwAction = SSA_GROUP_UPDATE;
          sc->wContactId = 0;
          sc->hContact = NULL;
          sc->szGroupName = getServerGroupNameUtf(sc->wGroupId);
          dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, 0, sc);

          icq_sendGroupUtf(dwCookie, ICQ_LISTS_UPDATEGROUP, sc->wGroupId, sc->szGroupName, groupData, groupSize);
          sendAddEnd(); // end server modifications here

          SAFE_FREE(&sc->szUID);
          sc = NULL; // we do not want it to be freed now
        }
        else
        { // this should never happen
          NetLog_Server("Group update failed.");
          sendAddEnd(); // end server modifications here
          SAFE_FREE(&sc->szUID);
        }
        if (hPend) RemovePendingOperation(hPend, 1);
      }
      break;
    }
  case SSA_GROUP_ADD:
    {
      if (wError)
      {
        FreeServerID(sc->wGroupId, SSIT_GROUP);
        NetLog_Server("Adding of group to server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Adding of group to server list failed."));
      }
      else // group added, we need to update master group
      {
        void* groupData;
        int groupSize;
        servlistcookie* ack;
        DWORD dwCookie;

        setServerGroupNameUtf(sc->wGroupId, sc->szGroupName); // add group to namelist
        setServerGroupIDUtf(makeGroupPathUtf(sc->wGroupId), sc->wGroupId); // add group to known

        groupData = collectGroups(&groupSize);
        groupData = realloc(groupData, groupSize+2);
        *(((WORD*)groupData)+(groupSize>>1)) = sc->wGroupId; // add this new group id
        groupSize += 2;

        ack = (servlistcookie*)malloc(sizeof(servlistcookie));
        if (ack)
        {
          ack->wGroupId = 0;
          ack->dwAction = SSA_GROUP_UPDATE;
          ack->szGroupName = NULL;
          dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, 0, ack);

          icq_sendGroupUtf(dwCookie, ICQ_LISTS_UPDATEGROUP, 0, ack->szGroupName, groupData, groupSize);
        }
        sendAddEnd(); // end server modifications here

        SAFE_FREE(&groupData);

        if (sc->ofCallback) // is add contact pending
        {
          sc->ofCallback(sc->wGroupId, (LPARAM)sc->lParam);
         // sc = NULL; // we do not want to be freed here
        }
      }
      SAFE_FREE(&sc->szGroupName);
      break;
    }
  case SSA_CONTACT_REMOVE:
    {
      if (!wError)
      {
        void* groupData;
        int groupSize;

        ICQWriteContactSettingWord(sc->hContact, "ServerId", 0); // clear the values
        ICQWriteContactSettingWord(sc->hContact, "SrvGroupId", 0);

        FreeServerID(sc->wContactId, SSIT_ITEM); 
              
        if (groupData = collectBuddyGroup(sc->wGroupId, &groupSize))
        { // the group is still not empty, just update it
          DWORD dwCookie;

          sc->dwAction = SSA_GROUP_UPDATE;
          sc->wContactId = 0;
          sc->hContact = NULL;
          sc->szGroupName = getServerGroupNameUtf(sc->wGroupId);
          dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, 0, sc);

          icq_sendGroupUtf(dwCookie, ICQ_LISTS_UPDATEGROUP, sc->wGroupId, sc->szGroupName, groupData, groupSize);
          sendAddEnd(); // end server modifications here

          SAFE_FREE(&sc->szUID);
          sc = NULL; // we do not want it to be freed now
        }
        else // the group is empty, delete it if it does not have sub-groups
        {
          DWORD dwCookie;

          if (!CheckServerID((WORD)(sc->wGroupId+1), 0) || countGroupLevel((WORD)(sc->wGroupId+1)) == 0)
          { // is next id an sub-group, if yes, we cannot delete this group
            sc->dwAction = SSA_GROUP_REMOVE;
            sc->wContactId = 0;
            sc->hContact = NULL;
            sc->szGroupName = getServerGroupNameUtf(sc->wGroupId);
            dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, 0, sc);

            icq_sendGroupUtf(dwCookie, ICQ_LISTS_REMOVEFROMLIST, sc->wGroupId, sc->szGroupName, NULL, 0);
            // here the modifications go on
            SAFE_FREE(&sc->szUID);
            sc = NULL; // we do not want it to be freed now
          }
        }
        SAFE_FREE(&groupData); // free the memory
      }
      else
      {
        NetLog_Server("Removing of contact from server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Removing of contact from server list failed."));
        sendAddEnd(); // end server modifications here
        SAFE_FREE(&sc->szUID);
      }
      break;
    }
  case SSA_GROUP_UPDATE:
    {
      if (wError)
      {
        NetLog_Server("Updating of group on server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Updating of group on server list failed."));
      }
      SAFE_FREE(&sc->szGroupName);
      break;
    }
  case SSA_GROUP_REMOVE:
    {
      SAFE_FREE(&sc->szGroupName);
      if (wError)
      {
        NetLog_Server("Removing of group from server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Removing of group from server list failed."));
      }
      else // group removed, we need to update master group
      {
        void* groupData;
        int groupSize;
        DWORD dwCookie;

        setServerGroupNameUtf(sc->wGroupId, NULL); // clear group from namelist
        FreeServerID(sc->wGroupId, SSIT_GROUP);
        removeGroupPathLinks(sc->wGroupId);

        groupData = collectGroups(&groupSize);
        sc->wGroupId = 0;
        sc->dwAction = SSA_GROUP_UPDATE;
        sc->szGroupName = "";
        dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, 0, sc);

        icq_sendGroupUtf(dwCookie, ICQ_LISTS_UPDATEGROUP, 0, sc->szGroupName, groupData, groupSize);
        sendAddEnd(); // end server modifications here

        sc = NULL; // we do not want to be freed here

        SAFE_FREE(&groupData);
      }
      break;
    }
  case SSA_CONTACT_SET_GROUP:
    { // we moved contact to another group
      if (sc->lParam == -1)
      { // the first was an error
        SAFE_FREE(&sc->szUID);
        break;
      }
      if (wError)
      {
        RemovePendingOperation(sc->hContact, 0);
        NetLog_Server("Moving of user to another group on server list failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Moving of user to another group on server list failed."));
        if (!sc->lParam) // is this first ack ?
        {
          sc->lParam = -1;
          sc = NULL; // this can't be freed here
        }
        break;
      }
      if (sc->lParam) // is this the second ack ?
      {
        void* groupData;
        int groupSize;
        int bEnd = 1; // shall we end the sever modifications

        ICQWriteContactSettingWord(sc->hContact, "ServerId", sc->wNewContactId);
        ICQWriteContactSettingWord(sc->hContact, "SrvGroupId", sc->wNewGroupId);
        FreeServerID(sc->wContactId, SSIT_ITEM); // release old contact id

        if (groupData = collectBuddyGroup(sc->wGroupId, &groupSize)) // update the group we moved from
        { // the group is still not empty, just update it
          DWORD dwCookie;
          servlistcookie* ack;

          ack = (servlistcookie*)malloc(sizeof(servlistcookie));
          if (!ack)
          {
            NetLog_Server("Updating of group on server list failed (malloc error)");
            break;
          }
          ack->dwAction = SSA_GROUP_UPDATE;
          ack->wContactId = 0;
          ack->hContact = NULL;
          ack->szGroupName = getServerGroupNameUtf(sc->wGroupId);
          ack->wGroupId = sc->wGroupId;
          dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, 0, ack);

          icq_sendGroupUtf(dwCookie, ICQ_LISTS_UPDATEGROUP, ack->wGroupId, ack->szGroupName, groupData, groupSize);
        }
        else if (!CheckServerID((WORD)(sc->wGroupId+1), 0) || countGroupLevel((WORD)(sc->wGroupId+1)) == 0)
        { // the group is empty and is not followed by sub-groups, delete it
          DWORD dwCookie;
          servlistcookie* ack;

          ack = (servlistcookie*)malloc(sizeof(servlistcookie));
          if (!ack)
          {
            NetLog_Server("Updating of group on server list failed (malloc error)");
            break;
          }
          ack->dwAction = SSA_GROUP_REMOVE;
          ack->wContactId = 0;
          ack->hContact = NULL;
          ack->szGroupName = getServerGroupNameUtf(sc->wGroupId);
          ack->wGroupId = sc->wGroupId;
          dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, 0, ack);

          icq_sendGroupUtf(dwCookie, ICQ_LISTS_REMOVEFROMLIST, ack->wGroupId, ack->szGroupName, NULL, 0);
          bEnd = 0; // here the modifications go on
        }
        SAFE_FREE(&groupData); // free the memory

        groupData = collectBuddyGroup(sc->wNewGroupId, &groupSize); // update the group we moved to
        {
          DWORD dwCookie;
          servlistcookie* ack;

          ack = (servlistcookie*)malloc(sizeof(servlistcookie));
          if (!ack)
          {
            NetLog_Server("Updating of group on server list failed (malloc error)");
            break;
          }
          ack->dwAction = SSA_GROUP_UPDATE;
          ack->wContactId = 0;
          ack->hContact = NULL;
          ack->szGroupName = getServerGroupNameUtf(sc->wNewGroupId);
          ack->wGroupId = sc->wNewGroupId;
          dwCookie = AllocateCookie(ICQ_LISTS_UPDATEGROUP, 0, ack);

          icq_sendGroupUtf(dwCookie, ICQ_LISTS_UPDATEGROUP, ack->wGroupId, ack->szGroupName, groupData, groupSize);
        }
        if (bEnd) sendAddEnd();
        if (sc->hContact) RemovePendingOperation(sc->hContact, 1);
        SAFE_FREE(&sc->szUID);
      }
      else
      {
        sc->lParam = 1;
        sc = NULL; // wait for second ack
      }
      break;
    }
  case SSA_GROUP_RENAME:
    {
      if (wError)
      {
        NetLog_Server("Renaming of server group failed, error %d", wError);
        icq_LogMessage(LOG_WARNING, Translate("Renaming of server group failed."));
      }
      else
      { 
        setServerGroupNameUtf(sc->wGroupId, sc->szGroupName);
        removeGroupPathLinks(sc->wGroupId);
        setServerGroupIDUtf(makeGroupPathUtf(sc->wGroupId), sc->wGroupId);
      }
      RemoveGroupRename(sc->wGroupId);
      break;
    }
  case SSA_SETAVATAR:
    {
      if (wError)
      {
        NetLog_Server("Uploading of avatar hash failed.");
        if (sc->wGroupId) // is avatar added or updated?
        {
          FreeServerID(sc->wContactId, SSIT_ITEM);
          ICQDeleteContactSetting(NULL, "SrvAvatarID"); // to fix old versions
        }
      }
      else
      {
        ICQWriteContactSettingWord(NULL, "SrvAvatarID", sc->wContactId);
      }
      break;
    }
  case SSA_REMOVEAVATAR:
    {
      if (wError)
        NetLog_Server("Removing of avatar hash failed.");
      else
      {
        ICQDeleteContactSetting(NULL, "SrvAvatarID");
        FreeServerID(sc->wContactId, SSIT_ITEM);

        setUserInfo();
      }
      break;
    }
  case SSA_SERVLIST_ACK:
    {
      ICQBroadcastAck(sc->hContact, ICQACKTYPE_SERVERCLIST, wError?ACKRESULT_FAILED:ACKRESULT_SUCCESS, (HANDLE)sc->lParam, wError);
      break;
    }
  case SSA_IMPORT:
    {
      if (wError)
        NetLog_Server("Re-starting import sequence failed, error %d", wError);
      else
      {
        ICQWriteContactSettingWord(NULL, "SrvImportID", 0);
        ICQDeleteContactSetting(NULL, "ImportTS");
      }
      break;
    }
  default:
    NetLog_Server("Server ack cookie type (%d) not recognized.", sc->dwAction);
  }
  SAFE_FREE(&sc); // free the memory

  return;
}



static void handleServerCList(unsigned char *buf, WORD wLen, WORD wFlags)
{
  BYTE bySSIVersion;
  WORD wRecordCount;
  WORD wRecord;
  WORD wRecordNameLen;
  WORD wGroupId;
  WORD wItemId;
  WORD wTlvType;
  WORD wTlvLength;
  BOOL bIsLastPacket;
  uid_str szRecordName;
  oscar_tlv_chain* pChain = NULL;
  oscar_tlv* pTLV = NULL;


  // If flag bit 1 is set, this is not the last
  // packet. If it is 0, this is the last packet
  // and there will be a timestamp at the end.
  if (wFlags & 0x0001)
    bIsLastPacket = FALSE;
  else
    bIsLastPacket = TRUE;


  if (wLen < 3)
    return;

  // Version number of SSI protocol?
  unpackByte(&buf, &bySSIVersion);
  wLen -= 1;

  // Total count of following entries. This is the size of the server
  // side contact list and should be saved and sent with CLI_CHECKROSTER.
  // NOTE: When the entries are split up in several packets, each packet
  // has it's own count and they must be added to get the total size of
  // server list.
  unpackWord(&buf, &wRecordCount);
  wLen -= 2;
  NetLog_Server("SSI: number of entries is %u, version is %u", wRecordCount, bySSIVersion);


  // Loop over all items in the packet
  for (wRecord = 0; wRecord < wRecordCount; wRecord++)
  {
    NetLog_Server("SSI: parsing record %u", wRecord + 1);

    if (wLen < 10)
    {
      // minimum: name length (zero), group ID, item ID, empty TLV
      NetLog_Server("Warning: SSI parsing error (%d)", 0);
      break;
    }

    // The name of the entry. If this is a group header, then this
    // is the name of the group. If it is a plain contact list entry,
    // then it's the UIN of the contact.
    unpackWord(&buf, &wRecordNameLen);
    if (wLen < 10 + wRecordNameLen || wRecordNameLen >= MAX_PATH)
    {
      NetLog_Server("Warning: SSI parsing error (%d)", 1);
      break;
    }
    unpackString(&buf, szRecordName, wRecordNameLen);
    szRecordName[wRecordNameLen] = '\0';

    // The group identifier this entry belongs to. If 0, this is meta information or
    // a contact without a group
    unpackWord(&buf, &wGroupId);

    // The ID of this entry. Group headers have ID 0. Otherwise, this
    // is a random number generated when the user is added to the
    // contact list, or when the user is ignored. See CLI_ADDBUDDY.
    unpackWord(&buf, &wItemId);

    // This field indicates what type of entry this is
    unpackWord(&buf, &wTlvType);

    // The length in bytes of the following TLV chain
    unpackWord(&buf, &wTlvLength);

    NetLog_Server("Name: '%s', GroupID: %u, EntryID: %u, EntryType: %u, TLVlength: %u",
      szRecordName, wGroupId, wItemId, wTlvType, wTlvLength);

    wLen -= (10 + wRecordNameLen);
    if (wLen < wTlvLength)
    {
      NetLog_Server("Warning: SSI parsing error (%d)", 2);
      break;
    }

    // Initialize the tlv chain
    if (wTlvLength > 0)
    {
      pChain = readIntoTLVChain(&buf, (WORD)(wTlvLength), 0);
      wLen -= wTlvLength;
    }
    else
    {
      pChain = NULL;
    }


    switch (wTlvType)
    {

    case SSI_ITEM_BUDDY:
      {
        /* this is a contact */
        HANDLE hContact;
        int bAdded;

        if (!IsStringUIN(szRecordName))
        { // probably AIM contact
          hContact = HContactFromUID(szRecordName, &bAdded);
        }
        else
        { // this should be ICQ number
          DWORD dwUin;

          dwUin = atoi(szRecordName);
          hContact = HContactFromUIN(dwUin, &bAdded);
        }

        if (hContact != INVALID_HANDLE_VALUE)
        {
          int bRegroup = 0;
          int bNicked = 0;
          WORD wOldGroupId;

          if (bAdded)
          { // Not already on list: added
            char* szGroup;

            NetLog_Server("SSI added new %s contact '%s'", "ICQ", szRecordName);

            if (szGroup = makeGroupPathUtf(wGroupId))
            { // try to get Miranda Group path from groupid, if succeeded save to db
              UniWriteContactSettingUtf(hContact, "CList", "Group", szGroup);

              SAFE_FREE(&szGroup);
            }
            AddJustAddedContact(hContact);
          }
          else
          {
            // we should add new contacts and this contact was just added, show it
            if (IsContactJustAdded(hContact))
            {
              SetContactHidden(hContact, 0);
              bAdded = 1; // we want details for new contacts
            }
            else
              NetLog_Server("SSI ignoring existing contact '%s'", szRecordName);
            // Contact on server is always on list
            DBWriteContactSettingByte(hContact, "CList", "NotOnList", 0);
          }

          wOldGroupId = ICQGetContactSettingWord(hContact, "SrvGroupId", 0);
          // Save group and item ID
          ICQWriteContactSettingWord(hContact, "ServerId", wItemId);
          ICQWriteContactSettingWord(hContact, "SrvGroupId", wGroupId);
          ReserveServerID(wItemId, SSIT_ITEM);

          if (!bAdded && (wOldGroupId != wGroupId) && ICQGetContactSettingByte(NULL, "LoadServerDetails", DEFAULT_SS_LOAD))
          { // contact has been moved on the server
            char* szOldGroup = getServerGroupNameUtf(wOldGroupId);
            char* szGroup = getServerGroupNameUtf(wGroupId);

            if (!szOldGroup)
            { // old group is not known, most probably not created subgroup
              char* szTmp = UniGetContactSettingUtf(hContact, "CList", "Group", "");

              if (strlennull(szTmp))
              { // got group from CList
                SAFE_FREE(&szOldGroup);
                szOldGroup = szTmp;
              }
              else
                SAFE_FREE(&szTmp);

              if (!szOldGroup) szOldGroup = null_strdup(DEFAULT_SS_GROUP);
            }

            if (!szGroup || strlennull(szGroup)>=strlennull(szOldGroup) || strnicmp(szGroup, szOldGroup, strlennull(szGroup)))
            { // contact moved to new group or sub-group or not to master group
              bRegroup = 1;
            }
            if (bRegroup && !stricmp(DEFAULT_SS_GROUP, szGroup) && !GroupNameExistsUtf(szGroup, -1))
            { // is it the default "General" group ? yes, does it exists in CL ?
              bRegroup = 0; // if no, do not move to it - cause it would hide the contact
            }
            SAFE_FREE(&szGroup);
            SAFE_FREE(&szOldGroup);
          }

          if (bRegroup || bAdded)
          { // if we should load server details or contact was just added, update its group
            char* szGroup;

            if (szGroup = makeGroupPathUtf(wGroupId))
            { // try to get Miranda Group path from groupid, if succeeded save to db
              UniWriteContactSettingUtf(hContact, "CList", "Group", szGroup);

              SAFE_FREE(&szGroup);
            }
          }

          if (pChain)
          { // Look for nickname TLV and copy it to the db if necessary
            if (pTLV = getTLV(pChain, 0x0131, 1))
            {
              if (pTLV->pData && (pTLV->wLen > 0))
              {
                char* pszNick;
                WORD wNickLength;

                wNickLength = pTLV->wLen;

                pszNick = (char*)malloc(wNickLength + 1);
                if (pszNick)
                {
                  // Copy buffer to utf-8 buffer
                  memcpy(pszNick, pTLV->pData, wNickLength);
                  pszNick[wNickLength] = 0; // Terminate string

                  NetLog_Server("Nickname is '%s'", pszNick);

                  bNicked = 1;

                  // Write nickname to database
                  if (ICQGetContactSettingByte(NULL, "LoadServerDetails", DEFAULT_SS_LOAD) || bAdded)
                  { // if just added contact, save details always - does no harm
                    char *szOldNick;

                    if (szOldNick = UniGetContactSettingUtf(hContact,"CList","MyHandle",""))
                    {
                      if ((strcmpnull(szOldNick, pszNick)) && (strlennull(pszNick) > 0))
                      {
                        // Yes, we really do need to delete it first. Otherwise the CLUI nick
                        // cache isn't updated (I'll look into it)
                        DBDeleteContactSetting(hContact,"CList","MyHandle");
                        UniWriteContactSettingUtf(hContact, "CList", "MyHandle", pszNick);
                      }
                      SAFE_FREE(&szOldNick);
                    }
                    else if (strlennull(pszNick) > 0)
                    {
                      DBDeleteContactSetting(hContact,"CList","MyHandle");
                      UniWriteContactSettingUtf(hContact, "CList", "MyHandle", pszNick);
                    }
                  }
                  SAFE_FREE(&pszNick);
                }
              }
              else
              {
                NetLog_Server("Invalid nickname");
              }
            }
            if (bAdded && !bNicked)
              icq_QueueUser(hContact); // queue user without nick for fast auto info update

            // Look for comment TLV and copy it to the db if necessary
            if (pTLV = getTLV(pChain, 0x013C, 1))
            {
              if (pTLV->pData && (pTLV->wLen > 0))
              {
                char* pszComment;
                WORD wCommentLength;


                wCommentLength = pTLV->wLen;

                pszComment = (char*)malloc(wCommentLength + 1);
                if (pszComment)
                {
                  // Copy buffer to utf-8 buffer
                  memcpy(pszComment, pTLV->pData, wCommentLength);
                  pszComment[wCommentLength] = 0; // Terminate string

                  NetLog_Server("Comment is '%s'", pszComment);

                  // Write comment to database
                  if (ICQGetContactSettingByte(NULL, "LoadServerDetails", DEFAULT_SS_LOAD) || bAdded)
                  { // if just added contact, save details always - does no harm
                    char *szOldComment;

                    if (szOldComment = UniGetContactSettingUtf(hContact,"UserInfo","MyNotes",""))
                    {
                      if ((strcmpnull(szOldComment, pszComment)) && (strlennull(pszComment) > 0))
                      {
                        UniWriteContactSettingUtf(hContact, "UserInfo", "MyNotes", pszComment);
                      }
                      SAFE_FREE(&szOldComment);
                    }
                    else if (strlennull(pszComment) > 0)
                    {
                      UniWriteContactSettingUtf(hContact, "UserInfo", "MyNotes", pszComment);
                    }
                  }

                  SAFE_FREE(&pszComment);
                }
              }
              else
              {
                NetLog_Server("Invalid comment");
              }
            }

            // Look for need-authorization TLV
            if (pTLV = getTLV(pChain, 0x0066, 1))
            {
              ICQWriteContactSettingByte(hContact, "Auth", 1);
              NetLog_Server("SSI contact need authorization");
            }
            else
            {
              ICQWriteContactSettingByte(hContact, "Auth", 0);
            }
          }
        }
        else
        { // failed to add or other error
          NetLog_Server("SSI failed to handle %s Item '%s'", "Buddy", szRecordName);
        }
      }
      break;

    case SSI_ITEM_GROUP:
      if ((wGroupId == 0) && (wItemId == 0))
      {
        /* list of groups. wTlvType=1, data is TLV(C8) containing list of WORDs which */
        /* is the group ids
        /* we don't need to use this. Our processing is on-the-fly */
        /* this record is always sent first in the first packet only, */
      }
      else if (wGroupId != 0)
      {
        /* wGroupId != 0: a group record */
        if (wItemId == 0)
        { /* no item ID: this is a group */
          /* pszRecordName is the name of the group */
          char* pszName = NULL;

          ReserveServerID(wGroupId, SSIT_GROUP);

          setServerGroupNameUtf(wGroupId, szRecordName);

          NetLog_Server("Group %s added to known groups.", szRecordName);

          /* demangle full grouppath, create groups, set it to known */
          pszName = makeGroupPathUtf(wGroupId); 
          SAFE_FREE(&pszName);

          /* TLV contains a TLV(C8) with a list of WORDs of contained contact IDs */
          /* our processing is good enough that we don't need this duplication */
        }
        else
        {
          NetLog_Server("Unhandled type 0x01, wItemID != 0");
        }
      }
      else
      {
        NetLog_Server("Unhandled type 0x01");
      }
      break;

    case SSI_ITEM_PERMIT:
      {
        /* item on visible list */
        /* wItemId not related to contact ID */
        /* pszRecordName is the UIN */
        HANDLE hContact;
        int bAdded;

        if (!IsStringUIN(szRecordName))
        { // probably AIM contact
          hContact = HContactFromUID(szRecordName, &bAdded);
        }
        else
        { // this should be ICQ number
          DWORD dwUin;

          dwUin = atoi(szRecordName);
          hContact = HContactFromUIN(dwUin, &bAdded);
        }

        if (hContact != INVALID_HANDLE_VALUE)
        {
          if (bAdded)
          {
            NetLog_Server("SSI added new %s contact '%s'", "Permit", szRecordName);
            // It wasn't previously in the list, we hide it so it only appears in the visible list
            SetContactHidden(hContact, 1);
            // Add it to the list, so it can be added properly if proper contact
            AddJustAddedContact(hContact);
          }
          else
            NetLog_Server("SSI %s contact already exists '%s'", "Permit", szRecordName);

          // Save permit ID
          ICQWriteContactSettingWord(hContact, "SrvPermitId", wItemId);
          ReserveServerID(wItemId, SSIT_ITEM);
          // Set apparent mode
          ICQWriteContactSettingWord(hContact, "ApparentMode", ID_STATUS_ONLINE);
          NetLog_Server("Visible-contact (%s)", szRecordName);
        }
        else
        { // failed to add or other error
          NetLog_Server("SSI failed to handle %s Item '%s'", "Permit", szRecordName);
        }
      }
      break;

    case SSI_ITEM_DENY:
      {
        /* Item on invisible list */
        /* wItemId not related to contact ID */
        /* pszRecordName is the UIN */
        HANDLE hContact;
        int bAdded;

        if (!IsStringUIN(szRecordName))
        { // probably AIM contact
          hContact = HContactFromUID(szRecordName, &bAdded);
        }
        else
        { // this should be ICQ number
          DWORD dwUin;

          dwUin = atoi(szRecordName);
          hContact = HContactFromUIN(dwUin, &bAdded);
        }

        if (hContact != INVALID_HANDLE_VALUE)
        {
          if (bAdded)
          {
            /* not already on list: added */
            NetLog_Server("SSI added new %s contact '%s'", "Deny", szRecordName);
            // It wasn't previously in the list, we hide it so it only appears in the visible list
            SetContactHidden(hContact, 1);
            // Add it to the list, so it can be added properly if proper contact
            AddJustAddedContact(hContact);
          }
          else
            NetLog_Server("SSI %s contact already exists '%s'", "Deny", szRecordName);

          // Save Deny ID
          ICQWriteContactSettingWord(hContact, "SrvDenyId", wItemId);
          ReserveServerID(wItemId, SSIT_ITEM);

          // Set apparent mode
          ICQWriteContactSettingWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
          NetLog_Server("Invisible-contact (%s)", szRecordName);
        }
        else
        { // failed to add or other error
          NetLog_Server("SSI failed to handle %s Item '%s'", "Deny", szRecordName);
        }
      }
      break;

    case SSI_ITEM_VISIBILITY: /* My visibility settings */
      {
        BYTE bVisibility;

        ReserveServerID(wItemId, SSIT_ITEM);

        // Look for visibility TLV
        if (bVisibility = getByteFromChain(pChain, 0x00CA, 1))
        { // found it, store the id, we do not need current visibility - we do not rely on it
          ICQWriteContactSettingWord(NULL, "SrvVisibilityID", wItemId);
          NetLog_Server("Visibility is %u, ID is %u", bVisibility, wItemId);
        }
      }
      break;

    case SSI_ITEM_IGNORE: 
      {
        /* item on ignore list */
        /* wItemId not related to contact ID */
        /* pszRecordName is the UIN */
        HANDLE hContact;
        int bAdded;

        if (!IsStringUIN(szRecordName))
        { // probably AIM contact
          hContact = HContactFromUID(szRecordName, &bAdded);
        }
        else
        { // this should be ICQ number
          DWORD dwUin;

          dwUin = atoi(szRecordName);
          hContact = HContactFromUIN(dwUin, &bAdded);
        }

        if (hContact != INVALID_HANDLE_VALUE)
        {
          if (bAdded)
          {
            /* not already on list: add */
            NetLog_Server("SSI added new %s contact '%s'", "Ignore", szRecordName);
            // It wasn't previously in the list, we hide it
            SetContactHidden(hContact, 1);
            // Add it to the list, so it can be added properly if proper contact
            AddJustAddedContact(hContact);
          }
          else
            NetLog_Server("SSI %s contact already exists '%s'", "Ignore", szRecordName);

          // Save Ignore ID
          ICQWriteContactSettingWord(hContact, "SrvIgnoreId", wItemId);
          ReserveServerID(wItemId, SSIT_ITEM);

          // Set apparent mode & ignore
          ICQWriteContactSettingWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
          // set ignore all events
          DBWriteContactSettingDword(hContact, "Ignore", "Mask1", 0xFFFF);
          NetLog_Server("Ignore-contact (%s)", szRecordName);
        }
        else
        { // failed to add or other error
          NetLog_Server("SSI failed to handle %s Item '%s'", "Ignore", szRecordName);
        }
      }
      break;

    case SSI_ITEM_UNKNOWN2:
      NetLog_Server("SSI unknown type 0x11");
      break;

    case SSI_ITEM_IMPORTTIME:
      if (wGroupId == 0)
      {
        /* time our list was first imported */
        /* pszRecordName is "Import Time" */
        /* data is TLV(13) {TLV(D4) {time_t importTime}} */
        ICQWriteContactSettingDword(NULL, "ImportTS", getDWordFromChain(pChain, 0xD4, 1));
        ICQWriteContactSettingWord(NULL, "SrvImportID", wItemId);
        NetLog_Server("SSI first import recognized");
      }
      break;

    case SSI_ITEM_BUDDYICON:
      if (wGroupId == 0)
      {
        /* our avatar MD5-hash */
        /* pszRecordName is "1" */
        /* data is TLV(D5) hash */
        /* we ignore this, just save the id */
        /* cause we get the hash again after login */
        ReserveServerID(wItemId, SSIT_ITEM);
        ICQWriteContactSettingWord(NULL, "SrvAvatarID", wItemId);
        NetLog_Server("SSI Avatar item recognized");
      }
      break;

    case SSI_ITEM_UNKNOWN1:
      if (wGroupId == 0)
      {
        /* ICQ2k ShortcutBar Items */
        /* data is TLV(CD) text */
      }

    default:
      NetLog_Server("SSI unhandled item %2x", wTlvType);
      break;
    }

    if (pChain)
      disposeChain(&pChain);

  } // end for

  NetLog_Server("Bytes left: %u", wLen);

  ICQWriteContactSettingWord(NULL, "SrvRecordCount", (WORD)(wRecord + ICQGetContactSettingWord(NULL, "SrvRecordCount", 0)));

  if (bIsLastPacket)
  {
    // No contacts left to sync
    bIsSyncingCL = FALSE;

    icq_RescanInfoUpdate();

    if (wLen >= 4)
    {
      DWORD dwLastUpdateTime;

      /* finally we get a time_t of the last update time */
      unpackDWord(&buf, &dwLastUpdateTime);
      ICQWriteContactSettingDword(NULL, "SrvLastUpdate", dwLastUpdateTime);
      NetLog_Server("Last update of server list was (%u) %s", dwLastUpdateTime, asctime(localtime(&dwLastUpdateTime)));

      sendRosterAck();
      handleServUINSettings(wListenPort, dwLocalInternalIP);
    }
    else
    {
      NetLog_Server("Last packet missed update time...");
    }
    if (ICQGetContactSettingWord(NULL, "SrvRecordCount", 0) == 0)
    { // we got empty serv-list, create master group
      servlistcookie* ack = (servlistcookie*)malloc(sizeof(servlistcookie));
      if (ack)
      { 
        DWORD seq;

        ack->dwAction = SSA_GROUP_UPDATE;
        ack->hContact = NULL;
        ack->wContactId = 0;
        ack->wGroupId = 0;
        ack->szGroupName = "";
        seq = AllocateCookie(ICQ_LISTS_ADDTOLIST, 0, ack);
        icq_sendGroupUtf(seq, ICQ_LISTS_ADDTOLIST, 0, ack->szGroupName, NULL, 0);
      }
    }
    // serv-list sync finished, clear just added contacts
    FlushJustAddedContacts(); 
  }
  else
  {
    NetLog_Server("Waiting for more packets");
  }
}



static void handleRecvAuthRequest(unsigned char *buf, WORD wLen)
{
  WORD wReasonLen;
  DWORD dwUin;
  uid_str szUid;
  HANDLE hcontact;
  CCSDATA ccs;
  PROTORECVEVENT pre;
  char* szReason;
  int nReasonLen;
  char* szNick;
  int nNickLen;
  char* szBlob;
  char* pCurBlob;
  DBVARIANT dbv;
  int bAdded;

  if (!unpackUID(&buf, &wLen, &dwUin, &szUid)) return;

  if (dwUin && IsOnSpammerList(dwUin))
  {
    NetLog_Server("Ignored Message from known Spammer");
    return;
  }

  unpackWord(&buf, &wReasonLen);
  wLen -= 2;
  if (wReasonLen > wLen)
    return;

  if (dwUin) 
    hcontact = HContactFromUIN(dwUin, &bAdded);
  else
    hcontact = HContactFromUID(szUid, &bAdded);

  ccs.szProtoService=PSR_AUTH;
  ccs.hContact=hcontact;
  ccs.wParam=0;
  ccs.lParam=(LPARAM)&pre;
  pre.flags=0;
  pre.timestamp=time(NULL);
  pre.lParam=sizeof(DWORD)+sizeof(HANDLE)+wReasonLen+5;
  szReason = (char*)malloc(wReasonLen+1);
  if (szReason)
  {
    memcpy(szReason, buf, wReasonLen);
    szReason[wReasonLen] = '\0';
    szReason = detect_decode_utf8(szReason); // detect & decode UTF-8
  }
  nReasonLen = strlennull(szReason);
  // Read nick name from DB
  if (dwUin)
  {
    if (ICQGetContactSetting(hcontact, "Nick", &dbv))
      nNickLen = 0;
    else
    {
      szNick = dbv.pszVal;
      nNickLen = strlennull(szNick);
    }
  }
  else
    nNickLen = strlennull(szUid);
  pre.lParam += nNickLen + nReasonLen;

  ICQWriteContactSettingByte(ccs.hContact, "Grant", 1);

  /*blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)*/
  pCurBlob=szBlob=(char *)malloc(pre.lParam);
  memcpy(pCurBlob,&dwUin,sizeof(DWORD)); pCurBlob+=sizeof(DWORD);
  memcpy(pCurBlob,&hcontact,sizeof(HANDLE)); pCurBlob+=sizeof(HANDLE);
  if (nNickLen && dwUin) 
  { // if we have nick we add it, otherwise keep trailing zero
    memcpy(pCurBlob, szNick, nNickLen);
    pCurBlob+=nNickLen;
  }
  else
  {
    memcpy(pCurBlob, szUid, nNickLen);
    pCurBlob+=nNickLen;
  }
  *(char *)pCurBlob = 0; pCurBlob++;
  *(char *)pCurBlob = 0; pCurBlob++;
  *(char *)pCurBlob = 0; pCurBlob++;
  *(char *)pCurBlob = 0; pCurBlob++;
  if (nReasonLen)
  {
    memcpy(pCurBlob, szReason, nReasonLen);
    pCurBlob += nReasonLen;
  }
  else
  {
    memcpy(pCurBlob, buf, wReasonLen); 
    pCurBlob += wReasonLen;
  }
  *(char *)pCurBlob = 0;
  pre.szMessage=(char *)szBlob;

// TODO: Change for new auth system, include all known informations
  CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);

  SAFE_FREE(&szReason);
  ICQFreeVariant(&dbv);
  return;
}



static void handleRecvAdded(unsigned char *buf, WORD wLen)
{
  DWORD dwUin;
  DBEVENTINFO dbei;
  PBYTE pCurBlob;
  HANDLE hContact;
  int bAdded;

  if (!unpackUID(&buf, &wLen, &dwUin, NULL)) return;

  if (IsOnSpammerList(dwUin))
  {
    NetLog_Server("Ignored Message from known Spammer");
    return;
  }

  hContact=HContactFromUIN(dwUin, &bAdded);

  ICQDeleteContactSetting(hContact, "Grant");

  ZeroMemory(&dbei,sizeof(dbei));
  dbei.cbSize=sizeof(dbei);
  dbei.szModule=gpszICQProtoName;
  dbei.timestamp=time(NULL);
  dbei.flags=0;
  dbei.eventType=EVENTTYPE_ADDED;
  dbei.cbBlob=sizeof(DWORD)+sizeof(HANDLE)+4;
  pCurBlob=dbei.pBlob=(PBYTE)malloc(dbei.cbBlob);
  /*blob is: uin(DWORD), hContact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ) */
  memcpy(pCurBlob,&dwUin,sizeof(DWORD)); pCurBlob+=sizeof(DWORD);
  memcpy(pCurBlob,&hContact,sizeof(HANDLE)); pCurBlob+=sizeof(HANDLE);
  *(char *)pCurBlob = 0; pCurBlob++;
  *(char *)pCurBlob = 0; pCurBlob++;
  *(char *)pCurBlob = 0; pCurBlob++;
  *(char *)pCurBlob = 0;
// TODO: Change for new auth system

  CallService(MS_DB_EVENT_ADD,(WPARAM)(HANDLE)NULL,(LPARAM)&dbei);
}



static void handleRecvAuthResponse(unsigned char *buf, WORD wLen)
{
  BYTE bResponse;
  DWORD dwUin;
  HANDLE hContact;
  char* szNick;
  WORD nReasonLen;
  char* szReason;
  int bAdded;

  bResponse = 0xFF;

  if (!unpackUID(&buf, &wLen, &dwUin, NULL)) return;

  if (IsOnSpammerList(dwUin))
  {
    NetLog_Server("Ignored Message from known Spammer");
    return;
  }

  hContact = HContactFromUIN(dwUin, &bAdded);

  if (hContact != INVALID_HANDLE_VALUE) szNick = NickFromHandle(hContact);

  if (wLen > 0)
  {
    unpackByte(&buf, &bResponse);
    wLen -= 1;
  }
  if (wLen >= 2)
  {
    unpackWord(&buf, &nReasonLen);
    wLen -= 2;
    if (wLen >= nReasonLen)
    {
      szReason = malloc(nReasonLen+1);
      unpackString(&buf, szReason, nReasonLen);
      szReason[nReasonLen] = '\0';
    }
  }

  switch (bResponse)
  {

  case 0:
    NetLog_Server("Authorisation request denied by %u", dwUin);
    // TODO: Add to system history as soon as new auth system is ready
    break;

  case 1:
    ICQWriteContactSettingByte(hContact, "Auth", 0);
    NetLog_Server("Authorisation request granted by %u", dwUin);
    // TODO: Add to system history as soon as new auth system is ready
    break;

  default:
    NetLog_Server("Unknown Authorisation request response (%u) from %u", bResponse, dwUin);
    break;

  }
  SAFE_FREE(&szNick);
  SAFE_FREE(&szReason);
}



// Updates the visibility code used while in SSI mode. If a server ID is
// not stored in the local DB, a new ID will be added to the server list.
//
// Possible values are:
//   01 - Allow all users to see you
//   02 - Block all users from seeing you
//   03 - Allow only users in the permit list to see you
//   04 - Block only users in the invisible list from seeing you
//   05 - Allow only users in the buddy list to see you
//
void updateServVisibilityCode(BYTE bCode)
{
  icq_packet packet;
  WORD wVisibilityID;
  WORD wCommand;

  if ((bCode > 0) && (bCode < 6))
  {
    servlistcookie* ack;
    DWORD dwCookie;
    BYTE bVisibility = ICQGetContactSettingByte(NULL, "SrvVisibility", 0);

    if (bVisibility == bCode) // if no change was made, not necescary to update that
      return;
    ICQWriteContactSettingByte(NULL, "SrvVisibility", bCode);

    // Do we have a known server visibility ID? We should, unless we just subscribed to the serv-list for the first time
    if ((wVisibilityID = ICQGetContactSettingWord(NULL, "SrvVisibilityID", 0)) == 0)
    {
      // No, create a new random ID
      wVisibilityID = GenerateServerId(SSIT_ITEM);
      ICQWriteContactSettingWord(NULL, "SrvVisibilityID", wVisibilityID);
      wCommand = ICQ_LISTS_ADDTOLIST;
      NetLog_Server("Made new srvVisibilityID, id is %u, code is %u", wVisibilityID, bCode);
    }
    else
    {
      NetLog_Server("Reused srvVisibilityID, id is %u, code is %u", wVisibilityID, bCode);
      wCommand = ICQ_LISTS_UPDATEGROUP;
    }

    ack = (servlistcookie*)malloc(sizeof(servlistcookie));
    if (!ack) 
    {
      NetLog_Server("Cookie alloc failure.");
      return; // out of memory, go away
    }
    ack->dwAction = 1; // update visibility
    ack->dwUin = 0; // this is ours
    dwCookie = AllocateCookie(wCommand, 0, ack); // take cookie

    // Build and send packet
    packet.wLen = 25;
    write_flap(&packet, ICQ_DATA_CHAN);
    packFNACHeader(&packet, ICQ_LISTS_FAMILY, wCommand, 0, dwCookie);
    packWord(&packet, 0);                   // Name (null)
    packWord(&packet, 0);                   // GroupID (0 if not relevant)
    packWord(&packet, wVisibilityID);       // EntryID
    packWord(&packet, SSI_ITEM_VISIBILITY); // EntryType
    packWord(&packet, 5);                   // Length in bytes of following TLV
    packWord(&packet, 0xCA);                // TLV Type
    packWord(&packet, 0x1);                 // TLV Length
    packByte(&packet, bCode);               // TLV Value (visibility code)
    sendServPacket(&packet);
    // There is no need to send ICQ_LISTS_CLI_MODIFYSTART or
    // ICQ_LISTS_CLI_MODIFYEND when modifying the visibility code
  }
}



// Updates the avatar hash used while in SSI mode. If a server ID is
// not stored in the local DB, a new ID will be added to the server list.
void updateServAvatarHash(char* pHash, int size)
{
  icq_packet packet;
  WORD wAvatarID;
  WORD wCommand;

  if (pHash)
  {
    servlistcookie* ack;
    DWORD dwCookie;

    // Do we have a known server avatar ID? We should, unless we just subscribed to the serv-list for the first time
    if ((wAvatarID = ICQGetContactSettingWord(NULL, "SrvAvatarID", 0)) == 0)
    {
      // No, create a new random ID
      wAvatarID = GenerateServerId(SSIT_ITEM);
      wCommand = ICQ_LISTS_ADDTOLIST;
      NetLog_Server("Made new srvAvatarID, id is %u", wAvatarID);
    }
    else
    {
      NetLog_Server("Reused srvAvatarID, id is %u", wAvatarID);
      wCommand = ICQ_LISTS_UPDATEGROUP;
    }

    ack = (servlistcookie*)malloc(sizeof(servlistcookie));
    if (!ack) 
    {
      NetLog_Server("Cookie alloc failure.");
      return; // out of memory, go away
    }
    ack->dwAction = SSA_SETAVATAR; // update avatar hash
    ack->dwUin = 0; // this is ours
    ack->wContactId = wAvatarID;
    dwCookie = AllocateCookie(wCommand, 0, ack); // take cookie

    // Build and send packet
    packet.wLen = 29 + size;
    write_flap(&packet, ICQ_DATA_CHAN);
    packFNACHeader(&packet, ICQ_LISTS_FAMILY, wCommand, 0, dwCookie);
    packWord(&packet, 1);                   // Name length
    packByte(&packet, '1');                 // Name
    packWord(&packet, 0);                   // GroupID (0 if not relevant)
    packWord(&packet, wAvatarID);           // EntryID
    packWord(&packet, SSI_ITEM_BUDDYICON);  // EntryType
    packWord(&packet, (WORD)(0x8 + size));          // Length in bytes of following TLV
    packWord(&packet, 0x131);               // TLV Type (Name)
    packWord(&packet, 0);                   // TLV Length (empty)
    packWord(&packet, 0xD5);                // TLV Type
    packWord(&packet, (WORD)size);                // TLV Length
    packBuffer(&packet, pHash, (WORD)size);       // TLV Value (avatar hash)
    sendServPacket(&packet);
    // There is no need to send ICQ_LISTS_CLI_MODIFYSTART or
    // ICQ_LISTS_CLI_MODIFYEND when modifying the avatar hash
  }
  else
  {
    servlistcookie* ack;
    DWORD dwCookie;

    // Do we have a known server avatar ID?
    if ((wAvatarID = ICQGetContactSettingWord(NULL, "SrvAvatarID", 0)) == 0)
    {
      return;
    }
    ack = (servlistcookie*)malloc(sizeof(servlistcookie));
    if (!ack) 
    {
      NetLog_Server("Cookie alloc failure.");
      return; // out of memory, go away
    }
    ack->dwAction = SSA_REMOVEAVATAR; // update avatar hash
    ack->dwUin = 0; // this is ours
    ack->wContactId = wAvatarID;
    dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, 0, ack); // take cookie

    // Build and send packet
    packet.wLen = 20;
    write_flap(&packet, ICQ_DATA_CHAN);
    packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_REMOVEFROMLIST, 0, dwCookie);
    packWord(&packet, 0);                   // Name (null)
    packWord(&packet, 0);                   // GroupID (0 if not relevant)
    packWord(&packet, wAvatarID);           // EntryID
    packWord(&packet, SSI_ITEM_BUDDYICON);  // EntryType
    packWord(&packet, 0);                   // Length in bytes of following TLV
    sendServPacket(&packet);
    // There is no need to send ICQ_LISTS_CLI_MODIFYSTART or
    // ICQ_LISTS_CLI_MODIFYEND when modifying the avatar hash
  }
}



// Should be called before the server list is modified. When all
// modifications are done, call sendAddEnd().
void sendAddStart(int bImport)
{
  icq_packet packet;
  WORD wImportID = ICQGetContactSettingWord(NULL, "SrvImportID", 0);

  if (bImport && wImportID)
  { // we should be importing, check if already have import item
    if (ICQGetContactSettingDword(NULL, "ImportTS", 0) + 604800 < ICQGetContactSettingDword(NULL, "LogonTS", 0))
    { // is the timestamp week older, clear it and begin new import
      DWORD dwCookie;
      servlistcookie* ack;

      if (ack = (servlistcookie*)malloc(sizeof(servlistcookie)))
      { // we have cookie good, go on
        ack->dwAction = SSA_IMPORT;
        dwCookie = AllocateCookie(ICQ_LISTS_REMOVEFROMLIST, 0, ack);

        icq_sendBuddyUtf(dwCookie, ICQ_LISTS_REMOVEFROMLIST, 0, "ImportTime", 0, wImportID, NULL, NULL, 0, SSI_ITEM_IMPORTTIME);
      }
    }
  }

  packet.wLen = bImport?14:10;
  write_flap(&packet, ICQ_DATA_CHAN);
  packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_MODIFYSTART, 0, ICQ_LISTS_CLI_MODIFYSTART<<0x10);
  if (bImport) packDWord(&packet, 1<<0x10); 
  sendServPacket(&packet);
}



// Should be called after the server list has been modified to inform
// the server that we are done.
void sendAddEnd(void)
{
  icq_packet packet;

  packet.wLen = 10;
  write_flap(&packet, ICQ_DATA_CHAN);
  packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_MODIFYEND, 0, ICQ_LISTS_CLI_MODIFYEND<<0x10);
  sendServPacket(&packet);
}



// Sent when the last roster packet has been received
void sendRosterAck(void)
{
  icq_packet packet;

  packet.wLen = 10;
  write_flap(&packet, ICQ_DATA_CHAN);
  packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_GOTLIST, 0, ICQ_LISTS_GOTLIST<<0x10);
  sendServPacket(&packet);

#ifdef _DEBUG
  NetLog_Server("Sent SNAC(x13,x07) - CLI_ROSTERACK");
#endif
}

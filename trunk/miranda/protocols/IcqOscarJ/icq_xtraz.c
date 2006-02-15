// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright � 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001,2002 Jon Keating, Richard Hughes
// Copyright � 2002,2003,2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004,2005,2006 Joe Kucera
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
//  Internal Xtraz API
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"


extern HANDLE hsmsgrequest;


void handleXtrazNotify(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szMsg, int nMsgLen, BOOL bThruDC)
{
  char *szWork, *szEnd, *szNotify, *szQuery;
  int nNotifyLen, nQueryLen;
  HANDLE hContact;

  szNotify = strstr(szMsg, "<NOTIFY>");
  szQuery = strstr(szMsg, "<QUERY>");

  hContact = HContactFromUIN(dwUin, NULL);
  if (hContact) // user sent us xtraz, he supports it
    SetContactCapabilities(hContact, CAPF_XTRAZ);

  if (szNotify && szQuery)
  { // valid request
    szNotify += 8;
    szQuery += 7;
    szEnd = strstr(szMsg, "</NOTIFY>");
    if (!szEnd) szEnd = szMsg + nMsgLen;
    nNotifyLen = (szEnd - szNotify);
    szEnd = strstr(szMsg, "</QUERY>");
    if (!szEnd) szEnd = szNotify;
    szNotify = DemangleXml(szNotify, nNotifyLen);
    nQueryLen = (szEnd - szQuery);
    szQuery = DemangleXml(szQuery, nQueryLen);
    szWork = strstr(szQuery, "<PluginID>");
    szEnd = strstr(szQuery, "</PluginID>");
#ifdef _DEBUG
    NetLog_Server("Query: %s", szQuery);
    NetLog_Server("Notify: %s", szNotify);
#endif
    if (szWork && szEnd)
    { // this is our plugin
      szWork += 10;
      *szEnd = '\0';

      if (!stricmp(szWork, "srvMng") && strstr(szNotify, "AwayStat"))
      {
        char* szSender = strstr(szNotify, "<senderId>");
        char* szEndSend = strstr(szNotify, "</senderId>");

        if (szSender && szEndSend)
        {
          szSender += 10;
          *szEndSend = '\0';

          if ((DWORD)atoi(szSender) == dwUin)
          {
            char *szResponse;
            int nResponseLen;
            char *szXName, *szXMsg;
            BYTE dwXId = ICQGetContactSettingByte(NULL, DBSETTING_XSTATUSID, 0);

            if (dwXId && validateStatusMessageRequest(hContact, MTYPE_SCRIPT_NOTIFY))
            { // apply privacy rules
              NotifyEventHooks(hsmsgrequest, (WPARAM)MTYPE_SCRIPT_NOTIFY, (LPARAM)dwUin);

              szXName = ICQGetContactSettingUtf(NULL, DBSETTING_XSTATUSNAME, "");
              szXMsg = ICQGetContactSettingUtf(NULL, DBSETTING_XSTATUSMSG, "");
              
              nResponseLen = 212 + strlennull(szXName) + strlennull(szXMsg) + UINMAXLEN + 2;
              szResponse = (char*)_alloca(nResponseLen + 1);
              // send response
              nResponseLen = null_snprintf(szResponse, nResponseLen, 
                "<ret event='OnRemoteNotification'>"
                "<srv><id>cAwaySrv</id>"
                "<val srv_id='cAwaySrv'><Root>"
                "<CASXtraSetAwayMessage></CASXtraSetAwayMessage>"
                "<uin>%d</uin>"
                "<index>%d</index>"
                "<title>%s</title>"
                "<desc>%s</desc></Root></val></srv></ret>",
                dwLocalUIN, dwXId, szXName, szXMsg);

              SAFE_FREE(&szXName);
              SAFE_FREE(&szXMsg);

              if (gbXStatusEnabled)
              {
                rate_record rr = {0};

                rr.bType = RIT_XSTATUS_RESPONSE;
                rr.dwUin = dwUin;
                rr.dwMid1 = dwMID;
                rr.dwMid2 = dwMID2;
                rr.wCookie = wCookie;
                rr.szData = szResponse;
                rr.bThruDC = bThruDC;
                rr.rate_group = 0x102;
                if (bThruDC || !handleRateItem(&rr, TRUE))
                  SendXtrazNotifyResponse(dwUin, dwMID, dwMID2, wCookie, szResponse, nResponseLen, bThruDC);
              }
              else
                NetLog_Server("Error: XStatus Disabled");
            }
            else
              NetLog_Server("Error: We are not in XStatus, skipping");
          }
          else
            NetLog_Server("Error: Invalid sender information");
        }
        else
          NetLog_Server("Error: Missing sender information");
      }
      else
        NetLog_Server("Error: Unknown plugin \"%s\" in Xtraz message", szWork);
    }
    else 
      NetLog_Server("Error: Missing PluginID in Xtraz message");

    SAFE_FREE(&szNotify);
    SAFE_FREE(&szQuery);
  }
  else 
    NetLog_Server("Error: Invalid Xtraz Notify message");
}



void handleXtrazNotifyResponse(DWORD dwUin, HANDLE hContact, WORD wCookie, char* szMsg, int nMsgLen)
{
  char *szMem, *szRes, *szEnd;
  int nResLen;

#ifdef _DEBUG
  NetLog_Server("Received Xtraz Notify Response");
#endif

  szRes = strstr(szMsg, "<RES>");
  szEnd = strstr(szMsg, "</RES>");

  if (szRes && szEnd)
  { // valid response
    char *szNode, *szWork;

    szRes += 5;
    nResLen = szEnd - szRes;

    szMem = szRes = DemangleXml(szRes, nResLen);

#ifdef _DEBUG
    NetLog_Server("Response: %s", szRes);
#endif

    ICQBroadcastAck(hContact, ICQACKTYPE_XTRAZNOTIFY_RESPONSE, ACKRESULT_SUCCESS, (HANDLE)wCookie, (LPARAM)szRes);

NextVal:
    szNode = strstr(szRes, "<val srv_id='");
    if (szNode) szEnd = strstr(szNode, ">"); else szEnd = NULL;

    if (szNode && szEnd)
    {
      *(szEnd-1) = '\0';
      szNode += 13;
      szWork = szEnd + 1;

      if (!stricmp(szNode, "cAwaySrv"))
      {
        szNode = strstr(szWork, "<uin>");
        szEnd = strstr(szWork, "</uin>");

        if (szNode && szEnd)
        {
          szNode += 5;
          *szEnd = '\0';

          if ((DWORD)atoi(szNode) == dwUin)
          {
            *szEnd = ' ';
            szNode = strstr(szWork, "<index>");
            szEnd = strstr(szWork, "</index>");

            if (szNode && szEnd)
            {
              szNode += 7;
              *szEnd = '\0';
              if (atoi(szNode) != ICQGetContactSettingByte(hContact, DBSETTING_XSTATUSID, 0))
              { // this is strange - but go on
                NetLog_Server("Warning: XStatusIds do not match!");
              }
              *szEnd = ' ';
            }
            szNode = strstr(szWork, "<title>");
            szEnd = strstr(szWork, "</title>");

            if (szNode && szEnd)
            { // we got XStatus title, save it
              szNode += 7;
              *szEnd = '\0';
              if (strlennull(szNode)) // save only non-empty names
                ICQWriteContactSettingUtf(hContact, DBSETTING_XSTATUSNAME, szNode);
              *szEnd = ' ';
            }
            szNode = strstr(szWork, "<desc>");
            szEnd = strstr(szWork, "</desc>");

            if (szNode && szEnd)
            { // we got XStatus mode msg, save it
              szNode += 6;
              *szEnd = '\0';
              ICQWriteContactSettingUtf(hContact, DBSETTING_XSTATUSMSG, szNode);
            }
            ICQBroadcastAck(hContact, ICQACKTYPE_XSTATUS_RESPONSE, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0);
          }
          else
            NetLog_Server("Error: Invalid sender information");
        }
        else
          NetLog_Server("Error: Missing sender information");
      }
      else
      {
        char *szSrvEnd = strstr(szEnd, "</srv>");

        if (szSrvEnd && strstr(szSrvEnd, "<val srv_id='"))
        { // check all values !
          szRes = szSrvEnd + 6; // after first value
          goto NextVal;
        }
        // no next val, we were unable to handle packet, write error
        NetLog_Server("Error: Unknown serverId \"%s\" in Xtraz response", szNode);
      }
    }
    else
      NetLog_Server("Error: Missing serverId in Xtraz response");

    SAFE_FREE(&szMem);
  }
  else
    NetLog_Server("Error: Invalid Xtraz Notify response");
}



DWORD SendXtrazNotifyRequest(HANDLE hContact, char* szQuery, char* szNotify)
{
  char *szQueryBody;
  char *szNotifyBody;
  DWORD dwUin;
  int nBodyLen;
  char *szBody;
  DWORD dwCookie;
  message_cookie_data* pCookieData;

  if (!CheckContactCapabilities(hContact, CAPF_XTRAZ))
    return 0; // Contact does not support xtraz, do not send anything

  if (ICQGetContactSettingUID(hContact, &dwUin, NULL))
    return 0; // Invalid contact

  szQueryBody = MangleXml(szQuery, strlennull(szQuery));
  szNotifyBody = MangleXml(szNotify, strlennull(szNotify));
  nBodyLen = strlennull(szQueryBody) + strlennull(szNotifyBody) + 41;
  szBody = (char*)_alloca(nBodyLen);
  nBodyLen = null_snprintf(szBody, nBodyLen, "<N><QUERY>%s</QUERY><NOTIFY>%s</NOTIFY></N>", szQueryBody, szNotifyBody);
  SAFE_FREE(&szQueryBody);
  SAFE_FREE(&szNotifyBody);

  // Set up the ack type
  pCookieData = CreateMessageCookie(MTYPE_SCRIPT_NOTIFY, ACKTYPE_CLIENT);
  dwCookie = AllocateCookie(CKT_MESSAGE, 0, dwUin, (void*)pCookieData);

  // have we a open DC, send through that
  if (gbDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD))
    icq_sendXtrazRequestDirect(dwUin, hContact, dwCookie, szBody, nBodyLen, MTYPE_SCRIPT_NOTIFY);
  else
    icq_sendXtrazRequestServ(dwUin, dwCookie, szBody, nBodyLen, pCookieData);

  return dwCookie;
}



void SendXtrazNotifyResponse(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szResponse, int nResponseLen, BOOL bThruDC)
{
  char *szResBody = MangleXml(szResponse, nResponseLen);
  int nBodyLen = strlennull(szResBody) + 21;
  char *szBody = (char*)_alloca(nBodyLen);
  HANDLE hContact = HContactFromUIN(dwUin, NULL);

  if (hContact != INVALID_HANDLE_VALUE && !CheckContactCapabilities(hContact, CAPF_XTRAZ))
  {
    SAFE_FREE(&szResBody);
    return; // Contact does not support xtraz, do not send anything
  }

  nBodyLen = null_snprintf(szBody, nBodyLen, "<NR><RES>%s</RES></NR>", szResBody);
  SAFE_FREE(&szResBody);

  // Was request received thru DC and have we a open DC, send through that
  if (bThruDC && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD))
    icq_sendXtrazResponseDirect(dwUin, hContact, wCookie, szBody, nBodyLen, MTYPE_SCRIPT_NOTIFY);
  else
    icq_sendXtrazResponseServ(dwUin, dwMID, dwMID2, wCookie, szBody, nBodyLen, MTYPE_SCRIPT_NOTIFY);
}

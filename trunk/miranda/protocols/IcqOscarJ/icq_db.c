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
//  Internal Database API
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"


static BOOL bUtfReadyDB = FALSE;

void InitDB()
{
  bUtfReadyDB = ServiceExists(MS_DB_CONTACT_GETSETTING_STR);
  if (!bUtfReadyDB)
    NetLog_Server("Warning: DB module does not support Unicode.");
}


BYTE ICQGetContactSettingByte(HANDLE hContact, const char* szSetting, BYTE bDef)
{
  return DBGetContactSettingByte(hContact, gpszICQProtoName, szSetting, bDef);
}


WORD ICQGetContactSettingWord(HANDLE hContact, const char* szSetting, WORD wDef)
{
  return DBGetContactSettingWord(hContact, gpszICQProtoName, szSetting, wDef);
}


DWORD ICQGetContactSettingDword(HANDLE hContact, const char* szSetting, DWORD dwDef)
{
  DBVARIANT dbv;
  DBCONTACTGETSETTING cgs;
  DWORD dwRes;

  cgs.szModule = gpszICQProtoName;
  cgs.szSetting = szSetting;
  cgs.pValue = &dbv;
  if (CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs))
    return dwDef; // not found, give default

  if (dbv.type != DBVT_DWORD)
    dwRes = dwDef; // invalid type, give default
  else // found and valid, give result
    dwRes = dbv.dVal;

  ICQFreeVariant(&dbv);
	return dwRes;
}



DWORD ICQGetContactSettingUIN(HANDLE hContact)
{
  return ICQGetContactSettingDword(hContact, UNIQUEIDSETTING, 0);
}



int ICQGetContactSettingUID(HANDLE hContact, DWORD *pdwUin, uid_str* ppszUid)
{
  DBVARIANT dbv;
  int iRes = 1;

  *pdwUin = 0;
  if (ppszUid) *ppszUid[0] = '\0';

  if (!ICQGetContactSetting(hContact, UNIQUEIDSETTING, &dbv))
  {
    if (dbv.type == DBVT_DWORD)
    {
      *pdwUin = dbv.dVal;
      iRes = 0;
    }
    else if (dbv.type == DBVT_ASCIIZ)
    {
      if (ppszUid && gbAimEnabled) 
      {
        strcpy(*ppszUid, dbv.pszVal);
        iRes = 0;
      }
      else
        NetLog_Server("AOL screennames not accepted");
    }
    ICQFreeVariant(&dbv);
  }
  return iRes;
}



int ICQGetContactSetting(HANDLE hContact, const char* szSetting, DBVARIANT *dbv)
{
  return DBGetContactSetting(hContact, gpszICQProtoName, szSetting, dbv);
}



char* UniGetContactSettingUtf(HANDLE hContact, const char *szModule,const char* szSetting, char* szDef)
{
  DBVARIANT dbv = {DBVT_DELETED};
  char* szRes;

  if (bUtfReadyDB)
  {
    if (DBGetContactSettingStringUtf(hContact, szModule, szSetting, &dbv))
      return null_strdup(szDef);
    
    szRes = null_strdup(dbv.pszVal);
    ICQFreeVariant(&dbv);
  }
  else
  { // old DB, we need to convert the string to UTF-8
    if (DBGetContactSetting(hContact, szModule, szSetting, &dbv))
      return null_strdup(szDef);

    if (strlennull(dbv.pszVal))
    {
      int nResult;

      nResult = utf8_encode(dbv.pszVal, &szRes);
    }
    else 
      szRes = null_strdup("");

    ICQFreeVariant(&dbv);
  }
  return szRes;
}



char* ICQGetContactSettingUtf(HANDLE hContact, const char* szSetting, char* szDef)
{
  return UniGetContactSettingUtf(hContact, gpszICQProtoName, szSetting, szDef);
}



WORD ICQGetContactStatus(HANDLE hContact)
{
  return ICQGetContactSettingWord(hContact, "Status", ID_STATUS_OFFLINE);
}



// (c) by George Hazan
int ICQGetContactStaticString(HANDLE hContact, const char* valueName, char* dest, int dest_len)
{
  DBVARIANT dbv;
  DBCONTACTGETSETTING sVal;

  dbv.pszVal = dest;
  dbv.cchVal = dest_len;
  dbv.type = DBVT_ASCIIZ;

  sVal.pValue = &dbv;
  sVal.szModule = gpszICQProtoName;
  sVal.szSetting = valueName;

  if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&sVal) != 0)
  {
    dbv.pszVal = dest;
    dbv.cchVal = dest_len;
    dbv.type = DBVT_UTF8;

    if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&sVal) != 0)
      return 1; // this is here due to DB module bug...
  }

  return (dbv.type != DBVT_ASCIIZ);
}


int ICQDeleteContactSetting(HANDLE hContact, const char* szSetting)
{
  return DBDeleteContactSetting(hContact, gpszICQProtoName, szSetting);
}


int ICQWriteContactSettingByte(HANDLE hContact, const char* szSetting, BYTE bValue)
{
  return DBWriteContactSettingByte(hContact, gpszICQProtoName, szSetting, bValue);
}


int ICQWriteContactSettingWord(HANDLE hContact, const char* szSetting, WORD wValue)
{
  return DBWriteContactSettingWord(hContact, gpszICQProtoName, szSetting, wValue);
}


int ICQWriteContactSettingDword(HANDLE hContact, const char* szSetting, DWORD dwValue)
{
  return DBWriteContactSettingDword(hContact, gpszICQProtoName, szSetting, dwValue);
}


int ICQWriteContactSettingString(HANDLE hContact, const char* szSetting, char* szValue)
{
  return DBWriteContactSettingString(hContact, gpszICQProtoName, szSetting, szValue);
}


int UniWriteContactSettingUtf(HANDLE hContact, const char *szModule,const char* szSetting, char* szValue)
{
  if (bUtfReadyDB)
    return DBWriteContactSettingStringUtf(hContact, szModule, szSetting, szValue);
  else
  { // old DB, we need to convert the string to Ansi
    char* szAnsi = NULL;

    if (utf8_decode(szValue, &szAnsi))
    {
      int nRes = DBWriteContactSettingString(hContact, szModule, szSetting, szAnsi);

      SAFE_FREE(&szAnsi);
      return nRes;
    }
    // failed to convert - give error

    return 1;
  }
}


int ICQWriteContactSettingUtf(HANDLE hContact, const char* szSetting, char* szValue)
{
  return UniWriteContactSettingUtf(hContact, gpszICQProtoName, szSetting, szValue);
}


/*static int bdCacheTested = 0;
static int bdWorkaroundRequired = 0;

void TestDBBlobIssue()
{
  DBVARIANT dbv = {0};

  bdCacheTested = 1;
  DBDeleteContactSetting(NULL, gpszICQProtoName, "BlobTestItem"); // delete setting
  DBGetContactSetting(NULL, gpszICQProtoName, "BlobTestItem", &dbv); // create crap cache item
  DBWriteContactSettingBlob(NULL, gpszICQProtoName, "BlobTestItem", "Test", 4); // write blob
  if (!DBGetContactSetting(NULL, gpszICQProtoName, "BlobTestItem", &dbv)) // try to read it back
  { // we were able to read it, the DB finally work correctly, hurrah
    ICQFreeVariant(&dbv); 
  }
  else // the crap is still in the cache, we need to use workaround for avatars to work properly
  {
    NetLog_Server("DB Module contains bug #0001177, using workaround");
    bdWorkaroundRequired = 1;
  }
  DBDeleteContactSetting(NULL, gpszICQProtoName, "BlobTestItem");
}*/


int ICQWriteContactSettingBlob(HANDLE hContact,const char *szSetting,const char *val, const int cbVal)
{
  DBCONTACTWRITESETTING cws;

/*  if (!bdCacheTested) TestDBBlobIssue();

  if (bdWorkaroundRequired)
  { // this is workaround for DB blob caching problems - nasty isn't it
    DBWriteContactSettingByte(hContact, gpszICQProtoName, szSetting, 1);
    DBDeleteContactSetting(hContact, gpszICQProtoName, szSetting); 
  }*/

  cws.szModule=gpszICQProtoName;
  cws.szSetting=szSetting;
  cws.value.type=DBVT_BLOB;
  cws.value.pbVal=(char*)val;
  cws.value.cpbVal = cbVal;
  return CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
}



int ICQFreeVariant(DBVARIANT* dbv)
{
  return DBFreeVariant(dbv);
}



HANDLE ICQFindFirstContact()
{
  HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
  char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

  if (!strcmpnull(szProto, gpszICQProtoName))
  {
    return hContact;
  }
  return ICQFindNextContact(hContact);
}



HANDLE ICQFindNextContact(HANDLE hContact)
{
  hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);

  while (hContact != NULL)
  {
    char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
    if (!strcmpnull(szProto, gpszICQProtoName))
    {
      return hContact;
    }
    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
  }
  return hContact;
}

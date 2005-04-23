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
//  Code for User details ICQ specific pages
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"


extern char gpszICQProtoName[MAX_PATH];
extern HANDLE ghServerNetlibUser;
extern int gnCurrentStatus;
extern HANDLE hInst;

extern DWORD dwLocalInternalIP;
extern DWORD dwLocalExternalIP;
extern WORD wListenPort;

extern char* calcMD5Hash(char* szFile);

static BOOL CALLBACK IcqDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK AvatarDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void SetValue(HWND hwndDlg, int idCtrl, HANDLE hContact, char *szModule, char *szSetting, int special);

#define SVS_NORMAL        0
#define SVS_GENDER        1
#define SVS_ZEROISUNSPEC  2
#define SVS_IP            3
#define SVS_COUNTRY       4
#define SVS_MONTH         5
#define SVS_SIGNED        6
#define SVS_TIMEZONE      7
#define SVS_ICQVERSION    8
#define SVS_TIMESTAMP     9



int OnDetailsInit(WPARAM wParam, LPARAM lParam)
{
  char* szProto;
  OPTIONSDIALOGPAGE odp;
  char szAvtCaption[MAX_PATH+8];

  szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, lParam, 0);
  if ((szProto == NULL || strcmp(szProto, gpszICQProtoName)) && lParam)
    return 0;

  odp.cbSize = sizeof(odp);
  odp.hIcon = NULL;
  odp.hInstance = hInst;
  odp.pfnDlgProc = IcqDlgProc;
  odp.position = -1900000000;
  odp.pszTemplate = MAKEINTRESOURCE(IDD_INFO_ICQ);
  odp.pszTitle = Translate(gpszICQProtoName);

  CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);

  if (((lParam != 0) && gbAvatarsEnabled) || (gbSsiEnabled && gbAvatarsEnabled))
  {
    odp.pfnDlgProc = AvatarDlgProc;
    odp.position = -1899999999;
    odp.pszTemplate = MAKEINTRESOURCE(IDD_INFO_AVATAR);
    if (lParam != 0)
      odp.pszTitle = Translate("Avatar");
    else
    {
      _snprintf(szAvtCaption, sizeof(szAvtCaption), Translate("%s Avatar"), gpszICQProtoName);
      odp.pszTitle = szAvtCaption;
    }

    CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
  }

  return 0;
}



static BOOL CALLBACK IcqDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	switch (msg)
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		if ((HANDLE)lParam == NULL)
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHANGEDETAILS), SW_SHOW);
		return TRUE;
		
	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom)
			{
				
			case 0:
				{
					
					switch (((LPNMHDR)lParam)->code)
					{
						
					case PSN_INFOCHANGED:
						{
              char* szProto;
              HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;

              if (hContact == NULL)
                szProto = gpszICQProtoName;
              else
                szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

              if (szProto == NULL)
                break;

              SetValue(hwndDlg, IDC_UIN, hContact, szProto, UNIQUEIDSETTING, 0);
              SetValue(hwndDlg, IDC_ONLINESINCE, hContact, szProto, "LogonTS", SVS_TIMESTAMP);
              SetValue(hwndDlg, IDC_IDLETIME, hContact, szProto, "IdleTS", SVS_TIMESTAMP);

              if (hContact)
              {
                SetValue(hwndDlg, IDC_IP, hContact, szProto, "IP", SVS_IP);
                SetValue(hwndDlg, IDC_REALIP, hContact, szProto, "RealIP", SVS_IP);
                SetValue(hwndDlg, IDC_PORT, hContact, szProto, "UserPort", SVS_ZEROISUNSPEC);
                SetValue(hwndDlg, IDC_VERSION, hContact, szProto, "Version", SVS_ICQVERSION);
                SetValue(hwndDlg, IDC_MIRVER, hContact, szProto, "MirVer", SVS_ZEROISUNSPEC);
                if (DBGetContactSettingDword(hContact, szProto, "ClientID", 0) == 1)
                  DBWriteContactSettingDword(hContact, szProto, "TickTS", 0);
                SetValue(hwndDlg, IDC_SYSTEMUPTIME, hContact, szProto, "TickTS", SVS_TIMESTAMP);
              }
              else
              {
                SetValue(hwndDlg, IDC_IP, hContact, (char*)DBVT_DWORD, (char*)dwLocalExternalIP, SVS_IP);
                SetValue(hwndDlg, IDC_REALIP, hContact, (char*)DBVT_DWORD, (char*)dwLocalInternalIP, SVS_IP);
                SetValue(hwndDlg, IDC_PORT, hContact, (char*)DBVT_WORD, (char*)wListenPort, SVS_ZEROISUNSPEC);
                SetValue(hwndDlg, IDC_VERSION, hContact, (char*)DBVT_WORD, (char*)8, SVS_ICQVERSION);
                SetValue(hwndDlg, IDC_MIRVER, hContact, (char*)DBVT_ASCIIZ, "Miranda IM", SVS_ZEROISUNSPEC);
                SetDlgItemText(hwndDlg, IDC_SUPTIME, Translate("Member since:"));
                SetValue(hwndDlg, IDC_SYSTEMUPTIME, hContact, szProto, "MemberTS", SVS_TIMESTAMP);
              }
            }
            break;
					}
				}
				break;
			}
		}
		break;

  case WM_COMMAND:
    switch(LOWORD(wParam))
    {

    case IDC_CHANGEDETAILS:
      {
        CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://www.icq.com/whitepages/user_details.php");
      }
      break;

    case IDCANCEL:
      SendMessage(GetParent(hwndDlg),msg,wParam,lParam);
      break;
		}
    break;
  }

  return FALSE;	
}

typedef struct AvtDlgProcData_t
{
  HANDLE hContact;
  HANDLE hEventHook;
} AvtDlgProcData;

#define HM_REBIND_AVATAR  (WM_USER + 1024)

static char* ChooseAvatarFileName()
{
  char* szDest = (char*)malloc(MAX_PATH+0x10);
  char str[MAX_PATH];
  char szFilter[512];
  OPENFILENAME ofn = {0};
  str[0] = 0;
  szDest[0]='\0';
  CallService(MS_UTILS_GETBITMAPFILTERSTRINGS,sizeof(szFilter),(LPARAM)szFilter);
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szDest;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  ofn.nMaxFile = MAX_PATH;
  ofn.nMaxFileTitle = MAX_PATH;
  ofn.lpstrDefExt = "jpg";
  if (!GetOpenFileName(&ofn))
  {
    SAFE_FREE(&szDest);
    return NULL;
  }

  return szDest;
}


static BOOL CALLBACK AvatarDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
    TranslateDialogDefault(hwndDlg);
    {
      DBVARIANT dbvHash, dbvSaved;
      AvtDlgProcData* pData = (AvtDlgProcData*)malloc(sizeof(AvtDlgProcData));
      DWORD dwUIN;
      char szAvatar[MAX_PATH];
      DWORD dwPaFormat;
      int bValid = 0;

      pData->hContact = (HANDLE)lParam;

      if (pData->hContact)
        pData->hEventHook = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_REBIND_AVATAR);
      else
      { 
        ShowWindow(GetDlgItem(hwndDlg, IDC_SETAVATAR), SW_SHOW);
        ShowWindow(GetDlgItem(hwndDlg, IDC_DELETEAVATAR), SW_SHOW);
        if (!icqOnline)
        {
          EnableWindow(GetDlgItem(hwndDlg, IDC_SETAVATAR), FALSE);
          EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEAVATAR), FALSE);
        }
      }
      SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)pData);

      dwUIN = DBGetContactSettingDword((HANDLE)lParam, gpszICQProtoName, UNIQUEIDSETTING, 0);

      if (!DBGetContactSetting((HANDLE)lParam, gpszICQProtoName, "AvatarHash", &dbvHash))
      {
        dwPaFormat = DBGetContactSettingByte((HANDLE)lParam, gpszICQProtoName, "AvatarType", PA_FORMAT_UNKNOWN);
        if (!pData->hContact || (dwPaFormat != PA_FORMAT_UNKNOWN))
        { // we do not know avatar format, so neither filename is known, not valid
          if (pData->hContact)
            GetFullAvatarFileName(dwUIN, dwPaFormat, szAvatar, 255);
          else
          {
            DBVARIANT dbvFile;
            if (!DBGetContactSetting(NULL, gpszICQProtoName, "AvatarFile", &dbvFile))
            {
              strcpy(szAvatar, dbvFile.pszVal);
              DBFreeVariant(&dbvFile);
            }
            else
              szAvatar[0] = '\0';
          }

          if (pData->hContact && !DBGetContactSetting((HANDLE)lParam, gpszICQProtoName, "AvatarSaved", &dbvSaved))
          { // not for us
            if (!memcmp(dbvHash.pbVal, dbvSaved.pbVal, 0x14))
            { // if the file exists, we know we have the current avatar
              if (!access(szAvatar, 0)) bValid = 1;
            }
            DBFreeVariant(&dbvSaved);
          }
          else // we do not have saved picture hash, do not rely on it
            if (!access(szAvatar, 0)) bValid = 1;
        }
      }
      else 
        return TRUE;

      if (bValid)
      { //load avatar
        HBITMAP avt = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (WPARAM)szAvatar);
        
        if (avt)
          SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)avt);
      }
      else if (pData->hContact) // only retrieve users avatars
      {
        GetAvatarFileName(dwUIN, szAvatar, 255);
        GetAvatarData((HANDLE)lParam, dwUIN, dbvHash.pbVal, 0x14, szAvatar);
      }

      DBFreeVariant(&dbvHash);
    }
    return TRUE;
	
  case HM_REBIND_AVATAR:
    {	
      AvtDlgProcData* pData = (AvtDlgProcData*)GetWindowLong(hwndDlg, GWL_USERDATA);
      ACKDATA* ack = (ACKDATA*)lParam;

      if (!pData->hContact) break; // we do not use this for us

      if (ack->type == ACKTYPE_AVATAR && ack->hContact == pData->hContact)
      {
        if (ack->result == ACKRESULT_SUCCESS)
        { // load avatar
          PROTO_AVATAR_INFORMATION* AI = (PROTO_AVATAR_INFORMATION*)ack->hProcess;

          HBITMAP avt = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (WPARAM)AI->filename);
        
          if (avt) avt = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)avt);
          if (avt) DeleteObject(avt); // we release old avatar if any
				}
				else if (ack->result == ACKRESULT_STATUS)
        {
          DWORD dwUIN = DBGetContactSettingDword(pData->hContact, gpszICQProtoName, UNIQUEIDSETTING, 0);
          DBVARIANT dbvHash;
          
          if (!DBGetContactSetting(pData->hContact, gpszICQProtoName, "AvatarHash", &dbvHash))
          {
            char szAvatar[MAX_PATH];

            GetAvatarFileName(dwUIN, szAvatar, 255);
            GetAvatarData(pData->hContact, dwUIN, dbvHash.pbVal, 0x14, szAvatar);
          }
          DBFreeVariant(&dbvHash);
        }
      }	
    }
		break;

  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDC_SETAVATAR:
      {
        char* szFile;
        AvtDlgProcData* pData = (AvtDlgProcData*)GetWindowLong(hwndDlg, GWL_USERDATA);
        if (szFile = ChooseAvatarFileName())
        { // user selected file for his avatar
          char szMyFile[MAX_PATH+1];
          int dwPaFormat = DetectAvatarFormat(szFile);

          HBITMAP avt = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (WPARAM)szFile);

          GetFullAvatarFileName(0, dwPaFormat, szMyFile, MAX_PATH);
          if (!CopyFile(szFile, szMyFile, FALSE))
          {
            Netlib_Logf(ghServerNetlibUser, "Failed to copy our avatar to local storage.");
            strcpy(szMyFile, szFile);
          }

          if (avt)
          {
            char* hash;
            hash = calcMD5Hash(szMyFile);
            if (hash)
            {
              char* ihash = malloc(0x14);
              if (ihash)
              { // upload hash to server
                ihash[0] = 0;    //unknown
                ihash[1] = 1;    //hash type
                ihash[2] = 1;    //hash status
                ihash[3] = 0x10; //hash len
                memcpy(ihash+4, hash, 0x10);
                updateServAvatarHash(ihash+2, 0x12);

                if (DBWriteContactSettingBlob(NULL, gpszICQProtoName, "AvatarHash", ihash, 0x14))
                {
                  Netlib_Logf(ghServerNetlibUser, "Failed to save avatar hash.");
                  DeleteObject(avt);
                  avt = 0;
                }
                DBWriteContactSettingString(NULL, gpszICQProtoName, "AvatarFile", szMyFile);

                SAFE_FREE(&ihash);
              }
              SAFE_FREE(&hash);
            }
          }
          SAFE_FREE(&szFile);
          if (avt) avt = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)avt);
          if (avt) DeleteObject(avt); // we release old avatar if any
        }
      }
      break;
    case IDC_DELETEAVATAR:
      {
        HBITMAP avt;
        BYTE bEmptyAvatar[7] = {0x00,0x05,0x02,0x01,0xD2,0x04,0x72};

        avt = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        RedrawWindow(GetDlgItem(hwndDlg, IDC_AVATAR), NULL, NULL, RDW_INVALIDATE);
        if (avt) DeleteObject(avt); // we release old avatar if any
        DBDeleteContactSetting(NULL, gpszICQProtoName, "AvatarFile");
        DBDeleteContactSetting(NULL, gpszICQProtoName, "AvatarHash");
        updateServAvatarHash(bEmptyAvatar, 7); // clear hash on server
      }
      break;
    }
    break;

  case WM_DESTROY:
    {
      AvtDlgProcData* pData = (AvtDlgProcData*)GetWindowLong(hwndDlg, GWL_USERDATA);
      if (pData->hContact)
        UnhookEvent(pData->hEventHook);
      SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)0);
      SAFE_FREE(&pData);
    }
    break;
  }

  return FALSE;
}



static void SetValue(HWND hwndDlg, int idCtrl, HANDLE hContact, char* szModule, char* szSetting, int special)
{
  DBVARIANT dbv = {0};
	char str[80];
	char* pstr;
	int unspecified = 0;

	dbv.type = DBVT_DELETED;

  if ((hContact == NULL) && ((int)szModule<0x100))
  {
    dbv.type = (BYTE)szModule;

    switch((int)szModule)
    {
    case DBVT_BYTE:
      dbv.cVal = (BYTE)szSetting;
      break;
    case DBVT_WORD:
      dbv.wVal = (WORD)szSetting;
      break;
    case DBVT_DWORD:
      dbv.dVal = (DWORD)szSetting;
      break;
    case DBVT_ASCIIZ:
      dbv.pszVal = szSetting;
      break;
    default:
      unspecified = 1;
      dbv.type = DBVT_DELETED;
    }
  }
  else
  {
	  if (szModule == NULL)
		  unspecified = 1;
	  else
		  unspecified = DBGetContactSetting(hContact, szModule, szSetting, &dbv);
  }

	if (!unspecified)
	{
		switch (dbv.type)
		{

		case DBVT_BYTE:
			if (special == SVS_GENDER)
			{
				if (dbv.cVal == 'M')
					pstr = Translate("Male");
				else if (dbv.cVal == 'F')
					pstr = Translate("Female");
				else
					unspecified = 1;
			}
			else if (special == SVS_MONTH)
			{
				if (dbv.bVal>0 && dbv.bVal<=12)
				{
					pstr = str;
					GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1-1+dbv.bVal, str,sizeof(str));
				}
				else
					unspecified = 1;
			}
			else if (special == SVS_TIMEZONE)
			{
				if (dbv.cVal == -100)
					unspecified = 1;
				else
				{
					pstr = str;
					_snprintf(str, 80, dbv.cVal ? "GMT%+d:%02d":"GMT", -dbv.cVal/2, (dbv.cVal&1)*30);
				}
			}
			else
			{
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
				pstr = itoa(special == SVS_SIGNED ? dbv.cVal:dbv.bVal, str, 10);
			}
			break;
			
		case DBVT_WORD:
			if (special == SVS_COUNTRY)
			{
				pstr = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, dbv.wVal, 0);
				unspecified = pstr == NULL;
			}
			else if (special == SVS_ICQVERSION)
			{
				if (dbv.wVal != 0)
				{
					pstr = str;
					_snprintf(str, 80, "%d", dbv.wVal);
				}
				else
					unspecified = 1;
			}
			else
			{
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
				pstr = itoa(special == SVS_SIGNED ? dbv.sVal:dbv.wVal, str, 10);
			}
			break;
			
		case DBVT_DWORD:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
			if (special == SVS_IP)
			{
				struct in_addr ia;
				ia.S_un.S_addr = htonl(dbv.dVal);
				pstr = inet_ntoa(ia);
				if (dbv.dVal == 0)
					unspecified=1;
			}
			else if (special == SVS_TIMESTAMP)
			{
				if (dbv.dVal == 0)
					unspecified = 1;
				else
				{
					pstr = asctime(localtime(&dbv.dVal));
					pstr[24] = '\0'; // Remove newline
				}
			}
			else
				pstr = itoa(special == SVS_SIGNED ? dbv.lVal:dbv.dVal, str, 10);
			break;
			
		case DBVT_ASCIIZ:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			pstr = dbv.pszVal;
			break;
			
		default:
			pstr = str;
			lstrcpy(str,"???");
			break;
		}
	}
	
	EnableWindow(GetDlgItem(hwndDlg, idCtrl), !unspecified);
	if (unspecified)
		SetDlgItemText(hwndDlg, idCtrl, Translate("<not specified>"));
	else
		SetDlgItemText(hwndDlg, idCtrl, pstr);
	
  if (dbv.pszVal!=szSetting) DBFreeVariant(&dbv);
}

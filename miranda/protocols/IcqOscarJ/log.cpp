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
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"


static const unsigned char *szLevelDescr[] = {LPGENUTF("ICQ Note"), LPGENUTF("ICQ Warning"), LPGENUTF("ICQ Error"), LPGENUTF("ICQ Fatal")};

typedef struct {
    unsigned char *szMsg;
    unsigned char *szTitle;
} LogMessageInfo;

static BOOL bErrorVisible = FALSE;

static DWORD __stdcall icq_LogMessageThread(void* arg) 
{
  LogMessageInfo *err = (LogMessageInfo*)arg;

  if (!err) return 0;
  bErrorVisible = TRUE;
  if (err->szMsg&&err->szTitle)
    MessageBoxUtf(NULL, err->szMsg, err->szTitle, MB_OK);
  SAFE_FREE((void**)&err->szMsg);
  SAFE_FREE((void**)&err->szTitle);
  SAFE_FREE((void**)&err);
  bErrorVisible = FALSE;

  return 0;
}



void icq_LogMessage(int level, const unsigned char *szMsg)
{
  int displayLevel;

  NetLog_Server("%s", szMsg);

  displayLevel = ICQGetContactSettingByte(NULL, "ShowLogLevel", LOG_WARNING);
  if (level >= displayLevel)
  {
    LogMessageInfo *lmi;

    if (ICQGetContactSettingByte(NULL, "PopupsLogEnabled", DEFAULT_LOG_POPUPS_ENABLED))
    {
      if (!ShowPopUpMsg(NULL, szLevelDescr[level], szMsg, (BYTE)level))
        return; // Popup showed successfuly
    }
    if (!bErrorVisible || !ICQGetContactSettingByte(NULL, "IgnoreMultiErrorBox", 0))
    { // error not shown or allowed multi - show messagebox
      lmi = (LogMessageInfo*)SAFE_MALLOC(sizeof(LogMessageInfo));
      lmi->szMsg = ICQTranslateUtf(szMsg);
      lmi->szTitle = ICQTranslateUtf(szLevelDescr[level]);
      ICQCreateThread((pThreadFuncEx)icq_LogMessageThread, lmi);
    }
  }
}



void icq_LogUsingErrorCode(int level, DWORD dwError, const unsigned char *szMsg)
{
  unsigned char szBuf[1024];
  unsigned char str[1024];
  unsigned char str2[64];
  unsigned char szErrorMsg[512];
  unsigned char *pszErrorMsg = NULL;
  int bNeedFree = FALSE;


  switch(dwError)
  {
    case ERROR_TIMEOUT:
    case WSAETIMEDOUT:
      pszErrorMsg = LPGENUTF("The server did not respond to the connection attempt within a reasonable time, it may be temporarily down. Try again later.");
      break;

    case ERROR_GEN_FAILURE:
      pszErrorMsg = LPGENUTF("The connection with the server was abortively closed during the connection attempt. You may have lost your local network connection.");
      break;

    case WSAEHOSTUNREACH:
    case WSAENETUNREACH:
      pszErrorMsg = LPGENUTF("Miranda was unable to resolve the name of a server to its numeric address. This is most likely caused by a catastrophic loss of your network connection (for example, your modem has disconnected), but if you are behind a proxy, you may need to use the 'Resolve hostnames through proxy' option in M->Options->Network.");
      break;

    case WSAEHOSTDOWN:
    case WSAENETDOWN:
    case WSAECONNREFUSED:
      pszErrorMsg = LPGENUTF("Miranda was unable to make a connection with a server. It is likely that the server is down, in which case you should wait for a while and try again later.");
      break;

    case ERROR_ACCESS_DENIED:
      pszErrorMsg = LPGENUTF("Your proxy rejected the user name and password that you provided. Please check them in M->Options->Network.");
      break;

    case WSAHOST_NOT_FOUND:
    case WSANO_DATA:
      pszErrorMsg = LPGENUTF("The server to which you are trying to connect does not exist. Check your spelling in M->Options->Network->ICQ.");
      break;

    default:
    {
      char err[512];

      if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, err, sizeof(err), NULL))
      {
        utf8_encode(err, &pszErrorMsg);
        bNeedFree = TRUE;
      }
      break;
    }
  }

  null_snprintf(szBuf, sizeof(szBuf), (unsigned char*)"%s%s%s (%s %d)", 
    szMsg ? ICQTranslateUtfStatic(szMsg, str, 1024) : (unsigned char*)"", szMsg ? "\r\n\r\n" : "", ICQTranslateUtfStatic(pszErrorMsg, szErrorMsg, 512), 
    ICQTranslateUtfStatic(LPGENUTF("error"), str2, 64), dwError);

  if (bNeedFree)
    SAFE_FREE((void**)&pszErrorMsg);
  
  icq_LogMessage(level, szBuf);
}



void icq_LogFatalParam(const unsigned char *szMsg, WORD wError)
{
  unsigned char str[MAX_PATH];
  unsigned char buf[MAX_PATH];

  null_snprintf(buf, MAX_PATH, ICQTranslateUtfStatic(szMsg, str, MAX_PATH), wError);
  icq_LogMessage(LOG_FATAL, buf);
}

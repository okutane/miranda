/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2004 Miranda ICQ/IM project,
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
#ifndef _COMMON_HEADERS_H_
#define _COMMON_HEADERS_H_ 1

//#include "AggressiveOptimize.h"

#if defined( UNICODE ) && !defined( _UNICODE )
#define _UNICODE
#endif

#include <malloc.h>

#ifdef _DEBUG
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#endif

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include "resource.h"
#include "forkthread.h"
#include <win2k.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_plugins.h>
#include "m_genmenu.h"
#include "genmenu.h"
#include "m_clui.h"
#include "m_mwclc.h"
#include "clc.h"
#include "clist.h"
#include "icolib.h"
#include <m_userinfo.h>
#include ".\CLUIFrames\cluiframes.h"
#include ".\CLUIFrames\m_cluiframes.h"
#include  "m_metacontacts.h"
#include "BkgrCfg.h"
#include <m_file.h>
#include <m_addcontact.h>
#include "m_fontservice.h"

// shared vars
extern HINSTANCE g_hInst;

/* most free()'s are invalid when the code is executed from a dll, so this changes
 all the bad free()'s to good ones, however it's still incorrect code. The reasons for not
 changing them include:

  * DBFreeVariant has a CallService() lookup
  * free() is executed in some large loops to do with clist creation of group data
  * easy search and replace

*/

extern struct LIST_INTERFACE li;
extern struct MM_INTERFACE memoryManagerInterface;

#define alloc(n) mir_alloc(n)
//#define free(ptr) mir_free(ptr)
//#define realloc(ptr,size) mir_realloc(ptr,size)


#define mir_alloc(n) memoryManagerInterface.mmi_malloc(n)
#define mir_free(ptr) mir_free_proxy(ptr)
#define mir_realloc(ptr,size) memoryManagerInterface.mmi_realloc(ptr,size)

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

extern int mir_realloc_proxy(void *ptr,int size);
extern int mir_free_proxy(void *ptr);
extern BOOL __cdecl strstri(const char *a, const char *b);
extern BOOL __cdecl boolstrcmpi(const char *a, const char *b);
extern int __cdecl MyStrCmp (const char *a, const char *b);
extern int __cdecl MyStrLen (const char *a);
extern int __cdecl MyStrCmpi(const char *a, const char *b);
extern int __cdecl MyStrCmpiT(const TCHAR *a, const TCHAR *b);
extern __inline void *mir_calloc( size_t num, size_t size );
extern __inline char * mir_strdup(const char * src);
extern __inline wchar_t * mir_strdupW(const wchar_t * src);

#ifdef UNICODE
	#define mir_tstrdup(a) mir_strdupW(a)
#else
	#define mir_tstrdup(a) mir_strdup(a)
#endif

extern void *mir_calloc( size_t num, size_t size );
extern char * mir_strdup(const char * src);

extern char *DBGetStringA(HANDLE hContact,const char *szModule,const char *szSetting);
extern wchar_t *DBGetStringW(HANDLE hContact,const char *szModule,const char *szSetting);
extern DWORD exceptFunction(LPEXCEPTION_POINTERS EP);

//from bkg options

//  Register of plugin's user
//
//  wParam = (WPARAM)szSetting - string that describes a user
//           format: Category/ModuleName,
//           eg: "Contact list background/CLUI",
//               "Status bar background/StatusBar"
//  lParam = (LPARAM)dwFlags
//
#define MS_BACKGROUNDCONFIG_REGISTER "BkgrCfg/Register"

//
//  Notification about changed background
//  wParam = ModuleName
//  lParam = 0
#define ME_BACKGROUNDCONFIG_CHANGED "BkgrCfg/Changed"


#endif
/*
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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

#ifndef __M_TIMEZONES_H
#define __M_TIMEZONES_H

#define MIM_TZ_NAMELEN 64

#define TZF_PLF_CB		1				// UI element is assumed to be a combo box
#define TZF_PLF_LB		2				// UI element is assumed to be a list box
#define TZF_DIFONLY     4
#define TZF_KNOWNONLY   8


typedef struct
{
	size_t cbSize;

	HANDLE  ( *createByName )( LPCTSTR tszName, DWORD dwFlags );
	HANDLE  ( *createByContact )( HANDLE hContact, DWORD dwFlags );
	void    ( *storeByContact )( HANDLE hContact, HANDLE hTZ );

	int     ( *printCurrentTime )( HANDLE hTZ, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags );
	int     ( *printCurrentTimeByContact )( HANDLE hContact, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags );

//	LPTIME_ZONE_INFORMATION ( *getTzi )( HANDLE hTZ );
//	void    ( *printTimeStamp )( HANDLE hTZ, time_t ts, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags );

	int     ( *prepareList )( HANDLE hContact, HWND hWnd, DWORD dwFlags );
	void    ( *storeListResults )( HANDLE hContact, HWND hWnd, DWORD dwFlags );
}
	TIME_API;

/* every protocol should declare this variable to use the Time API */
extern TIME_API tmi;

/*
a service to obtain the Time API 

wParam = 0;
lParam = (LPARAM)(TIME_API*).

returns TRUE if all is Ok, and FALSE otherwise
*/

#define MS_SYSTEM_GET_TMI "Miranda/System/GetTimeApi"

__forceinline int mir_getTMI(TIME_API* dest)
{
	dest->cbSize = sizeof(*dest);
	return CallService(MS_SYSTEM_GET_TMI, 0, (LPARAM)dest);
}

#endif /* __M_TIMEZONES_H */

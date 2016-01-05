// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright � 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001-2002 Jon Keating, Richard Hughes
// Copyright � 2002-2016 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004-2016 Joe Kucera
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
//  Contains global types & variables declarations.
//
// -----------------------------------------------------------------------------


#ifndef __GLOBALS_H
#define __GLOBALS_H


typedef char uid_str[MAX_PATH];

// from init.cpp
extern HINSTANCE hInst;
extern DWORD MIRANDA_VERSION;

extern IcqIconHandle hStaticIcons[];

extern const int moodXStatus[];

// from fam_04message.cpp
struct icq_mode_messages
{
  char *szOnline;
  char *szAway;
  char *szNa;
  char *szDnd;
  char *szOccupied;
  char *szFfc;
};


#endif /* __GLOBALS_H */

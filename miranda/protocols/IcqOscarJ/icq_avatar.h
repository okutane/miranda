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
// File name      : $Source: /cvsroot/miranda/miranda/protocols/IcqOscarJ/icq_avatar.h,v $
// Revision       : $Revision$
// Last change on : $Date$
// Last change by : $Author$
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#ifndef __ICQ_AVATAR_H
#define __ICQ_AVATAR_H

extern BOOL AvatarsReady;

void handleAvatarContactHash(DWORD dwUIN, char* szUID, HANDLE hContact, unsigned char* pHash, unsigned int nHashLen, WORD wOldStatus);

char* loadMyAvatarFileName();
void storeMyAvatarFileName(char* szFile);

void GetFullAvatarFileName(int dwUin, char* szUid, int dwFormat, char* pszDest, int cbLen);
void GetAvatarFileName(int dwUin, char* szUid, char* pszDest, int cbLen);
void AddAvatarExt(int dwFormat, char* pszDest);

int DetectAvatarFormat(char* szFile);

int IsAvatarSaved(HANDLE hContact, char* pHash);

int GetAvatarData(HANDLE hContact, DWORD dwUin, char* szUid, char* hash, unsigned int hashlen, char* file);
int SetAvatarData(HANDLE hContact, WORD wRef, char* data, unsigned int datalen);

void StartAvatarThread(HANDLE hConn, char* cookie, WORD cookieLen);
void StopAvatarThread();


#endif /* __ICQ_AVATAR_H */

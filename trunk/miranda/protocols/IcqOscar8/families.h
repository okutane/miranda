// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright � 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001,2002 Jon Keating, Richard Hughes
// Copyright � 2002,2003,2004,2005 Martin �berg, Sam Kothari, Robert Rainwater
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

#ifndef __FAMILIES_H
#define __FAMILIES_H

typedef struct icq_mode_messages_s
{
	char* szAway;
	char* szNa;
	char* szDnd;
	char* szOccupied;
	char* szFfc;
} icq_mode_messages;

typedef struct snac_header_s
{
	BOOL  bValid;
	WORD  wFamily;
	WORD  wSubtype;
	WORD  wFlags;
	DWORD dwRef;
	WORD  wVersion;
} snac_header;

/*---------* Functions *---------------*/

void handleServiceFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);
void handleLocationFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);
void handleBuddyFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);
void handleMsgFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);
void handleBosFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);
void handleStatusFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);
void handleServClistFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);
void handleIcqExtensionsFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader);

void handleServUINSettings(int nPort, int nIP);
int TypeStringToTypeId(const char* pszType);
void handleMessageTypes(DWORD dwUin, DWORD dwTimestamp, DWORD dwRecvTimestamp, DWORD dwRecvTimestamp2, WORD wCookie, int type, int flags, WORD wAckType, DWORD dwDataLen, WORD wMsgLen, char *pMsg);

#define BUL_ALLCONTACTS   0
#define BUL_VISIBLE       1
#define BUL_INVISIBLE     2
void sendEntireListServ(WORD wFamily, WORD wSubtype, WORD wFlags, int listType);
void updateServVisibilityCode(BYTE bCode);
void sendAddStart(void);
void sendAddEnd(void);
WORD renameServContact(HANDLE hContact, const char *szNick);
void sendTypingNotification(HANDLE hContact, WORD wMTNCode);

#endif /* __FAMILIES_H */
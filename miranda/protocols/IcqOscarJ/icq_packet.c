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



void write_httphdr(icq_packet* pPacket, WORD wType, DWORD dwSeq)
{
	pPacket->wPlace = 0;
	pPacket->wLen += 14;
	pPacket->pData = (BYTE*)calloc(1, pPacket->wLen);

	packWord(pPacket, (WORD)(pPacket->wLen - 2));
	packWord(pPacket, HTTP_PROXY_VERSION);
	packWord(pPacket, wType);
	packDWord(pPacket, 0); // Flags?
	packDWord(pPacket, dwSeq); // Connection sequence ?
}



void write_flap(icq_packet* pPacket, BYTE byFlapChannel)
{
	pPacket->wPlace = 0;
	pPacket->wLen += 6;
	pPacket->pData = (BYTE*)calloc(1, pPacket->wLen);

	packByte(pPacket, FLAP_MARKER);
	packByte(pPacket, byFlapChannel);
	packWord(pPacket, 0);                 // This is the sequence ID, it is filled in during the actual sending
	packWord(pPacket, (WORD)(pPacket->wLen - 6)); // This counter should not include the flap header (thus the -6)
}



void directPacketInit(icq_packet* pPacket, DWORD dwSize)
{
	pPacket->wPlace = 0;
	pPacket->wLen   = (WORD)dwSize;
	pPacket->pData  = (BYTE *)calloc(1, dwSize + 2);

	packLEWord(pPacket, pPacket->wLen);
}



void packByte(icq_packet* pPacket, BYTE byValue)
{
	pPacket->pData[pPacket->wPlace++] = byValue;
}



void packWord(icq_packet* pPacket, WORD wValue)
{
	pPacket->pData[pPacket->wPlace++] = ((wValue & 0xff00) >> 8);
	pPacket->pData[pPacket->wPlace++] = (wValue & 0x00ff);
}



void packDWord(icq_packet* pPacket, DWORD dwValue)
{
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0xff000000) >> 24);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x00ff0000) >> 16);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x0000ff00) >> 8);
	pPacket->pData[pPacket->wPlace++] = (BYTE) (dwValue & 0x000000ff);
}



void packTLV(icq_packet* pPacket, WORD wType, WORD wLength, BYTE* pbyValue)
{
	packWord(pPacket, wType);
	packWord(pPacket, wLength);
	packBuffer(pPacket, pbyValue, wLength);
}



void packTLVWord(icq_packet* pPacket, WORD wType, WORD wValue)
{
	packWord(pPacket, wType);
	packWord(pPacket, 0x02);
	packWord(pPacket, wValue);
}



void packTLVDWord(icq_packet* pPacket, WORD wType, DWORD dwValue)
{
	packWord(pPacket, wType);
	packWord(pPacket, 0x04);
	packDWord(pPacket, dwValue);
}



// Pack a preformatted buffer.
// This can be used to pack strings or any type of raw data.
void packBuffer(icq_packet* pPacket, const BYTE* pbyBuffer, WORD wLength)
{

	while (wLength)
	{
		pPacket->pData[pPacket->wPlace++] = *pbyBuffer++;
		wLength--;
	}

}



// Pack a buffer and prepend it with the size as a LE WORD.
// Commented out since its not actually used anywhere right now.
//void packLEWordSizedBuffer(icq_packet* pPacket, const BYTE* pbyBuffer, WORD wLength)
//{
//
//	packLEWord(pPacket, wLength);
//	packBuffer(pPacket, pbyBuffer, wLength);
//
//}



void packFNACHeader(icq_packet* pPacket, WORD wFamily, WORD wSubtype, WORD wFlags, DWORD dwSeq)
{
	packWord(pPacket, wFamily);  // Family type
	packWord(pPacket, wSubtype); // Family subtype
	packWord(pPacket, wFlags);   // SNAC flags
  packWord(pPacket, (WORD)dwSeq); // SNAC request id (sequence)
	packWord(pPacket, (WORD)(dwSeq>>0x10));  // SNAC request id (command)
}



void packLEWord(icq_packet* pPacket, WORD wValue)
{
	pPacket->pData[pPacket->wPlace++] =  (wValue & 0x00ff);
	pPacket->pData[pPacket->wPlace++] = ((wValue & 0xff00) >> 8);
}



void packLEDWord(icq_packet* pPacket, DWORD dwValue)
{
	pPacket->pData[pPacket->wPlace++] = (BYTE) (dwValue & 0x000000ff);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x0000ff00) >> 8);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x00ff0000) >> 16);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0xff000000) >> 24);
}



void unpackByte(BYTE** pSource, BYTE* byDestination)
{
	if (byDestination)
	{
		*byDestination = *(*pSource)++;
	}
	else
	{
		*pSource += 1;
	}
}



void unpackWord(BYTE** pSource, WORD* wDestination)
{

	if (wDestination)
	{
		*wDestination  = *(*pSource)++ << 8;
		*wDestination |= *(*pSource)++;
	}
	else
	{
		*pSource += 2;
	}

}



void unpackDWord(BYTE** pSource, DWORD* dwDestination)
{

	if (dwDestination)
	{
		*dwDestination  = *(*pSource)++ << 24;
		*dwDestination |= *(*pSource)++ << 16;
		*dwDestination |= *(*pSource)++ << 8;
		*dwDestination |= *(*pSource)++;
	}
	else
	{
		*pSource += 4;
	}

}



void unpackLEWord(unsigned char **buf, WORD *w)
{
	unsigned char *tmp = *buf;

	if (w)
	{
		*w = (*tmp++);
		*w |= ((*tmp++) << 8);
	}
	else
		tmp += 2;

	*buf = tmp;
}



void unpackLEDWord(unsigned char **buf, DWORD *dw)
{
	unsigned char *tmp = *buf;

	if (dw)
	{
		*dw = (*tmp++);
		*dw |= ((*tmp++) << 8);
		*dw |= ((*tmp++) << 16);
		*dw |= ((*tmp++) << 24);
	}
	else
		tmp += 4;

	*buf = tmp;
}



void unpackString(unsigned char **buf, char *string, WORD len)
{
	unsigned char *tmp = *buf;

	while (len)	/* Can have 0x00 so go by len */
	{
		*string++ = *tmp++;
		len--;
	}

	*buf = tmp;
}



void unpackWideString(unsigned char **buf, WCHAR *string, WORD len)
{
	unsigned char *tmp = *buf;

	while (len > 1)
	{
		*string = (*tmp++ << 8);
		*string |= *tmp++;

		string++;
		len -= 2;
	}

	// We have a stray byte at the end, this means that the buffer had an odd length
	// which indicates an error.
	_ASSERTE(len == 0);
	if (len != 0)
	{
		// We dont copy the last byte but we still need to increase the buffer pointer
		// (we assume that 'len' was correct) since the calling function expects
		// that it is increased 'len' bytes.
		*tmp += len;
	}

	*buf = tmp;
}



void unpackTLV(unsigned char **buf, WORD *type, WORD *len, char **string)
{
	WORD wType, wLen;
	unsigned char *tmp = *buf;

	// Unpack type and length
	unpackWord(&tmp, &wType);
	unpackWord(&tmp, &wLen);

	// Make sure we have a good pointer
	if (string != NULL)
	{
		// Unpack and save value
		*string = (char *)malloc(wLen + 1); // Add 1 for \0
		unpackString(&tmp, *string, wLen);
		*(*string + wLen) = '\0';
	}
	else
	{
		*tmp += wLen;
	}

	// Save type and length
	if (type)
		*type = wType;
	if (len)
		*len = wLen;

	// Increase source pointer
	*buf = tmp;

}



BOOL unpackUID(unsigned char** ppBuf, WORD* pwLen, char** ppszUID)
{
	BYTE nUIDLen;
	
	// Sender UIN
	unpackByte(ppBuf, &nUIDLen);
	*pwLen -= 1;
	
	if ((nUIDLen > *pwLen) || (nUIDLen == 0))
		return FALSE;
	
	if (!(*ppszUID = malloc(nUIDLen)))
		return FALSE;
	
	unpackString(ppBuf, *ppszUID, nUIDLen);
	*pwLen -= nUIDLen;
	*ppszUID[nUIDLen] = '\0';
	
//	if (!IsStringUIN(szUin))
//	{
//		Netlib_Logf(ghServerNetlibUser, "Error 2: Malformed UID in message);
//			return;
//	}

	return TRUE;
	
}

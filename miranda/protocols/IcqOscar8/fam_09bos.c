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

#include "icqoscar.h"



static void handlePrivacyRightsReply(unsigned char *pBuffer, WORD wBufferLength);

extern HANDLE ghServerNetlibUser;

void handleBosFam(unsigned char *pBuffer, WORD wBufferLength, snac_header* pSnacHeader)
{

	switch (pSnacHeader->wSubtype)
	{

	case SRV_PRIVACY_RIGHTS_REPLY: // Reply to CLI_REQBOS
		handlePrivacyRightsReply(pBuffer, wBufferLength);
		break;

	default:
		Netlib_Logf(ghServerNetlibUser, "Warning: Ignoring SNAC(x09,x%02x) - Unknown SNAC (Flags: %u, Ref: %u", pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;

	}
	
}



static void handlePrivacyRightsReply(unsigned char *pBuffer, WORD wBufferLength)
{

	if (wBufferLength >= 12)
	{

		oscar_tlv_chain* pChain;


		pChain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

		if (pChain)
		{

			WORD wMaxVisibleContacts;
			WORD wMaxInvisibleContacts;


			wMaxVisibleContacts = getWordFromChain(pChain, 0x0001, 1);
			wMaxInvisibleContacts = getWordFromChain(pChain, 0x0001, 1);

			disposeChain(&pChain);

			Netlib_Logf(ghServerNetlibUser, "SRV_PRIVACY_RIGHTS_REPLY: Max visible %u, max invisible %u", wMaxVisibleContacts, wMaxInvisibleContacts);

			// Success
			return;
		}

	}


	// Failure
	Netlib_Logf(ghServerNetlibUser, "Warning: Malformed SRV_PRIVACY_RIGHTS_REPLY");
		
}
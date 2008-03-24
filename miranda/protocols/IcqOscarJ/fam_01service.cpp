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

extern capstr capXStatus[];

void CIcqProto::handleServiceFam(unsigned char* pBuffer, WORD wBufferLength, snac_header* pSnacHeader, serverthread_info *info)
{
	icq_packet packet;

	switch (pSnacHeader->wSubtype) {

	case ICQ_SERVER_READY:
#ifdef _DEBUG
		NetLog_Server("Server is ready and is requesting my Family versions");
		NetLog_Server("Sending my Families");
#endif

		// This packet is a response to SRV_FAMILIES SNAC(1,3).
		// This tells the server which SNAC families and their corresponding
		// versions which the client understands. This also seems to identify
		// the client as an ICQ vice AIM client to the server.
		// Miranda mimics the behaviour of ICQ 6
		serverPacketInit(&packet, 54);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_FAMILIES);
		packDWord(&packet, 0x00220001);
		packDWord(&packet, 0x00010004);
		packDWord(&packet, 0x00130004);
		packDWord(&packet, 0x00020001);
		packDWord(&packet, 0x00030001);
		packDWord(&packet, 0x00150001);
		packDWord(&packet, 0x00040001);
		packDWord(&packet, 0x00060001);
		packDWord(&packet, 0x00090001);
		packDWord(&packet, 0x000a0001);
		packDWord(&packet, 0x000b0001);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_FAMILIES2:
		/* This is a reply to CLI_FAMILIES and it tells the client which families and their versions that this server understands.
		* We send a rate request packet */
#ifdef _DEBUG
		NetLog_Server("Server told me his Family versions");
		NetLog_Server("Requesting Rate Information");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_REQ_RATE_INFO);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_RATE_INFO:
#ifdef _DEBUG
		NetLog_Server("Server sent Rate Info");
		NetLog_Server("Sending Rate Info Ack");
#endif
		/* init rates management */
		m_rates = ratesCreate(pBuffer, wBufferLength);
		/* ack rate levels */
		serverPacketInit(&packet, 20);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_RATE_ACK);
		packDWord(&packet, 0x00010002);
		packDWord(&packet, 0x00030004);
		packWord(&packet, 0x0005);
		sendServPacket(&packet);

		/* CLI_REQINFO - This command requests from the server certain information about the client that is stored on the server. */
#ifdef _DEBUG
		NetLog_Server("Sending CLI_REQINFO");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_REQINFO);
		sendServPacket(&packet);

		if (m_bSsiEnabled)
		{
			DWORD dwLastUpdate;
			WORD wRecordCount;
			servlistcookie* ack;
			DWORD dwCookie;

			dwLastUpdate = getSettingDword(NULL, "SrvLastUpdate", 0);
			wRecordCount = getSettingWord(NULL, "SrvRecordCount", 0);

			// CLI_REQLISTS - we want to use SSI
#ifdef _DEBUG
			NetLog_Server("Requesting roster rights");
#endif
			serverPacketInit(&packet, 16);
			packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQLISTS);
			packTLVWord(&packet, 0x0B, 0x000F); // mimic ICQ 6
			sendServPacket(&packet);

			if (!wRecordCount) // CLI_REQROSTER
			{ // we do not have any data - request full list
#ifdef _DEBUG
				NetLog_Server("Requesting full roster");
#endif
				serverPacketInit(&packet, 10);
				ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
				if (ack)
				{ // we try to use standalone cookie if available
					ack->dwAction = SSA_CHECK_ROSTER; // loading list
					dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_REQUEST, 0, ack);
				}
				else // if not use that old fake
					dwCookie = ICQ_LISTS_CLI_REQUEST<<0x10;

				packFNACHeaderFull(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQUEST, 0, dwCookie);
				sendServPacket(&packet);
			}
			else // CLI_CHECKROSTER
			{
#ifdef _DEBUG
				NetLog_Server("Requesting roster check");
#endif
				serverPacketInit(&packet, 16);
				ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
				if (ack)  // TODO: rewrite - use get list service for empty list
				{ // we try to use standalone cookie if available
					ack->dwAction = SSA_CHECK_ROSTER; // loading list
					dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_CHECK, 0, ack);
				}
				else // if not use that old fake
					dwCookie = ICQ_LISTS_CLI_CHECK<<0x10;

				packFNACHeaderFull(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_CHECK, 0, dwCookie);
				// check if it was not changed elsewhere (force reload, set that setting to zero)
				if (IsServerGroupsDefined())
				{
					packDWord(&packet, dwLastUpdate);  // last saved time
					packWord(&packet, wRecordCount);   // number of records saved
				}
				else
				{ // we need to get groups info into DB, force receive list
					packDWord(&packet, 0);  // last saved time
					packWord(&packet, 0);   // number of records saved
				}
				sendServPacket(&packet);
			}
		}

		// CLI_REQLOCATION
#ifdef _DEBUG
		NetLog_Server("Requesting Location rights");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_CLI_REQ_RIGHTS);
		sendServPacket(&packet);

		// CLI_REQBUDDY
#ifdef _DEBUG
		NetLog_Server("Requesting Client-side contactlist rights");
#endif
		serverPacketInit(&packet, 16);
		packFNACHeader(&packet, ICQ_BUDDY_FAMILY, ICQ_USER_CLI_REQBUDDY);
		// Query flags: 1 = Enable Avatars
		//              2 = Enable offline status message notification
		//              4 = Enable Avatars for offline contacts
		packTLVWord(&packet, 0x05, 0x0003); // mimic ICQ 6
		sendServPacket(&packet);

		// CLI_REQICBM
#ifdef _DEBUG
		NetLog_Server("Sending CLI_REQICBM");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_CLI_REQICBM);
		sendServPacket(&packet);

		// CLI_REQBOS
#ifdef _DEBUG
		NetLog_Server("Sending CLI_REQBOS");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_BOS_FAMILY, ICQ_PRIVACY_REQ_RIGHTS);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_PAUSE:
		NetLog_Server("Server is going down in a few seconds... (Flags: %u)", pSnacHeader->wFlags);
		// This is the list of groups that we want to have on the next server
		serverPacketInit(&packet, 30);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_PAUSE_ACK);
		packWord(&packet,ICQ_SERVICE_FAMILY);
		packWord(&packet,ICQ_LISTS_FAMILY);
		packWord(&packet,ICQ_LOCATION_FAMILY);
		packWord(&packet,ICQ_BUDDY_FAMILY);
		packWord(&packet,ICQ_EXTENSIONS_FAMILY);
		packWord(&packet,ICQ_MSG_FAMILY);
		packWord(&packet,0x06);
		packWord(&packet,ICQ_BOS_FAMILY);
		packWord(&packet,ICQ_LOOKUP_FAMILY);
		packWord(&packet,ICQ_STATS_FAMILY);
		sendServPacket(&packet);
#ifdef _DEBUG
		NetLog_Server("Sent server pause ack");
#endif
		break;

	case ICQ_SERVER_MIGRATIONREQ:
		{
			oscar_tlv_chain *chain = NULL;

#ifdef _DEBUG
			NetLog_Server("Server migration requested (Flags: %u)", pSnacHeader->wFlags);
#endif
			pBuffer += 2; // Unknown, seen: 0
			wBufferLength -= 2;
			chain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

			if (info->cookieDataLen > 0)
				SAFE_FREE((void**)&info->cookieData);

			info->newServer = (char*)getStrFromChain(chain, 0x05, 1);
			info->cookieData = getStrFromChain(chain, 0x06, 1);
			info->cookieDataLen = getLenFromChain(chain, 0x06, 1);

			if (!info->newServer || !info->cookieData)
			{
				icq_LogMessage(LOG_FATAL, LPGEN("A server migration has failed because the server returned invalid data. You must reconnect manually."));
				SAFE_FREE((void**)&info->newServer);
				SAFE_FREE((void**)&info->cookieData);
				info->cookieDataLen = 0;
				info->newServerReady = 0;
				return;
			}

			disposeChain(&chain);
			NetLog_Server("Migration has started. New server will be %s", info->newServer);

			m_iDesiredStatus = m_iStatus;
			SetCurrentStatus(ID_STATUS_CONNECTING); // revert to connecting state

			info->newServerReady = 1;
			info->isMigrating = 1;
		}
		break;

	case ICQ_SERVER_NAME_INFO: // This is the reply to CLI_REQINFO
		{
			BYTE bUinLen;
			oscar_tlv_chain *chain;

#ifdef _DEBUG
			NetLog_Server("Received self info");
#endif
			unpackByte(&pBuffer, &bUinLen);
			pBuffer += bUinLen;
			pBuffer += 4;      /* warning level & user class */
			wBufferLength -= 5 + bUinLen;

			if (pSnacHeader->dwRef == ICQ_CLIENT_REQINFO<<0x10)
			{ // This is during the login sequence
				DWORD dwValue;

				// TLV(x01) User type?
				// TLV(x0C) Empty CLI2CLI Direct connection info
				// TLV(x0A) External IP
				// TLV(x0F) Number of seconds that user has been online
				// TLV(x03) The online since time.
				// TLV(x0A) External IP again
				// TLV(x22) Unknown
				// TLV(x1E) Unknown: empty.
				// TLV(x05) Member of ICQ since.
				// TLV(x14) Unknown
				chain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

				// Save external IP
				dwValue = getDWordFromChain(chain, 10, 1); 
				setSettingDword(NULL, "IP", dwValue);

				// Save member since timestamp
				dwValue = getDWordFromChain(chain, 5, 1); 
				if (dwValue) setSettingDword(NULL, "MemberTS", dwValue);

				dwValue = getDWordFromChain(chain, 3, 1);
				setSettingDword(NULL, "LogonTS", dwValue ? dwValue : time(NULL));

				disposeChain(&chain);

				// If we are in SSI mode, this is sent after the list is acked instead
				// to make sure that we don't set status before seing the visibility code
				if (!m_bSsiEnabled || info->isMigrating)
					handleServUINSettings(wListenPort, info);
			}
		}
		break;

	case ICQ_SERVER_RATE_CHANGE:

		if (wBufferLength >= 2)
		{
			WORD wStatus;
			WORD wClass;
			DWORD dwLevel;
			// We now have global rate management, although controlled are only some
			// areas. This should not arrive in most cases. If it does, update our
			// local rate levels & issue broadcast.
			unpackWord(&pBuffer, &wStatus);
			unpackWord(&pBuffer, &wClass);
			pBuffer += 20;
			unpackDWord(&pBuffer, &dwLevel);
			EnterCriticalSection(&ratesMutex);
			ratesUpdateLevel(m_rates, wClass, dwLevel);
			LeaveCriticalSection(&ratesMutex);

			if (wStatus == 2 || wStatus == 3)
			{ // this is only the simplest solution, needs rate management to every section
				BroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
				if (wStatus == 2)
					NetLog_Server("Rates #%u: Alert", wClass);
				else
					NetLog_Server("Rates #%u: Limit", wClass);
			}
			else if (wStatus == 4)
			{
				BroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
				NetLog_Server("Rates #%u: Clear", wClass);
			}
		}

		break;

	case ICQ_SERVER_REDIRECT_SERVICE: // reply to family request, got new connection point
		{
			oscar_tlv_chain* pChain = NULL;
			WORD wFamily;
			familyrequest_rec* reqdata;

			if (!(pChain = readIntoTLVChain(&pBuffer, wBufferLength, 0)))
			{
				NetLog_Server("Received Broken Redirect Service SNAC(1,5).");
				break;
			}
			wFamily = getWordFromChain(pChain, 0x0D, 1);

			// pick request data
			if ((!FindCookie(pSnacHeader->dwRef, NULL, (void**)&reqdata)) || (reqdata->wFamily != wFamily))
			{
				disposeChain(&pChain);
				NetLog_Server("Received unexpected SNAC(1,5), skipping.");
				break;
			}

			FreeCookie(pSnacHeader->dwRef);

			{ // new family entry point received
				char* pServer;
				WORD wPort;
				char* pCookie;
				WORD wCookieLen;
				NETLIBOPENCONNECTION nloc = {0};
				HANDLE hConnection;

				pServer = (char*)getStrFromChain(pChain, 0x05, 1);
				pCookie = (char*)getStrFromChain(pChain, 0x06, 1);
				wCookieLen = getLenFromChain(pChain, 0x06, 1);

				if (!pServer || !pCookie)
				{
					NetLog_Server("Server returned invalid data, family unavailable.");

					SAFE_FREE((void**)&pServer);
					SAFE_FREE((void**)&pCookie);
					SAFE_FREE((void**)&reqdata);
					break;
				}

				// Get new family server ip and port
				wPort = info->wServerPort; // get default port
				parseServerAddress(pServer, &wPort);

				// establish connection
				nloc.flags = 0;
				nloc.szHost = pServer; 
				nloc.wPort = wPort;

				hConnection = NetLib_OpenConnection(m_hServerNetlibUser, wFamily == ICQ_AVATAR_FAMILY ? "Avatar " : NULL, &nloc);

				if (hConnection == NULL)
				{
					NetLog_Server("Unable to connect to ICQ new family server.");
				} // we want the handler to be called even if the connecting failed
				(this->*reqdata->familyhandler)(hConnection, pCookie, wCookieLen);

				// Free allocated memory
				// NOTE: "cookie" will get freed when we have connected to the avatar server.
				disposeChain(&pChain);
				SAFE_FREE((void**)&pServer);
				SAFE_FREE((void**)&reqdata);
			}

			break;
		}

	case ICQ_SERVER_EXTSTATUS: // our avatar
		{
			NetLog_Server("Received our avatar hash & status.");

			if (wBufferLength > 4 && pBuffer[1] == AVATAR_HASH_PHOTO)
			{ // skip photo item
				if (wBufferLength >= pBuffer[3] + 4)
				{
					wBufferLength -= pBuffer[3] + 4;
					pBuffer += pBuffer[3] + 4;
				}
				else
				{
					pBuffer += wBufferLength;
					wBufferLength = 0;
				}
			}

			if ((wBufferLength >= 0x14) && m_bAvatarsEnabled)
			{
				if (!info->bMyAvatarInited) // signal the server after login
				{ // this refreshes avatar state - it used to work automatically, but now it does not
					if (getSettingByte(NULL, "ForceOurAvatar", 0))
					{ // keep our avatar
						char* file = loadMyAvatarFileName();

						SetMyAvatar(0, (LPARAM)file);
						SAFE_FREE((void**)&file);
					}
					else // only change avatar hash to the same one
					{
						BYTE hash[0x14];

						memcpy(hash, pBuffer, 0x14);
						hash[2] = 1; // update image status
						updateServAvatarHash(hash, 0x14);
					}
					info->bMyAvatarInited = TRUE;
					break;
				}

				switch (pBuffer[2])
				{
				case 1: // our avatar is on the server
					{
						char* file;
						BYTE* hash;

						setSettingBlob(NULL, "AvatarHash", pBuffer, 0x14);

						setUserInfo();
						// here we need to find a file, check its hash, if invalid get avatar from server
						file = loadMyAvatarFileName();
						if (!file)
						{ // we have no avatar file, download from server
							char szFile[MAX_PATH + 4];
#ifdef _DEBUG
							NetLog_Server("We have no avatar, requesting from server.");
#endif
							GetAvatarFileName(0, NULL, szFile, MAX_PATH);
							GetAvatarData(NULL, m_dwLocalUIN, NULL, pBuffer, 0x14, szFile);
						}
						else
						{ // we know avatar filename
							hash = calcMD5Hash(file);
							if (!hash)
							{ // hash could not be calculated - probably missing file, get avatar from server
								char szFile[MAX_PATH + 4];
#ifdef _DEBUG
								NetLog_Server("We have no avatar, requesting from server.");
#endif
								GetAvatarFileName(0, NULL, szFile, MAX_PATH);
								GetAvatarData(NULL, m_dwLocalUIN, NULL, pBuffer, 0x14, szFile);
							} // check if we had set any avatar if yes set our, if not download from server
							else if (memcmp(hash, pBuffer+4, 0x10))
							{ // we have different avatar, sync that
								if (getSettingByte(NULL, "ForceOurAvatar", 1))
								{ // we want our avatar, update hash
									DWORD dwPaFormat = DetectAvatarFormat(file);
									BYTE* pHash = (BYTE*)_alloca(0x14);

									NetLog_Server("Our avatar is different, set our new hash.");

									pHash[0] = 0;
									pHash[1] = dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC;
									pHash[2] = 1; // state of the hash
									pHash[3] = 0x10; // len of the hash
									memcpy(pHash + 4, hash, 0x10);
									updateServAvatarHash(pHash, 0x14);
								}
								else
								{ // get avatar from server
									char szFile[MAX_PATH + 4];
#ifdef _DEBUG
									NetLog_Server("We have different avatar, requesting new from server.");
#endif
									GetAvatarFileName(0, NULL, szFile, MAX_PATH);
									GetAvatarData(NULL, m_dwLocalUIN, NULL, pBuffer, 0x14, szFile);
								}
							}
							SAFE_FREE((void**)&hash);
							SAFE_FREE((void**)&file);
						}
						break;
					}
				case 0x41: // request to upload avatar data
				case 0x81:
					{ // request to re-upload avatar data
						char* file;
						BYTE* hash;
						DWORD dwPaFormat;

						if (!m_bSsiEnabled) break; // we could not change serv-list if it is disabled...

						file = loadMyAvatarFileName();
						if (!file)
						{ // we have no file to upload, remove hash from server
							NetLog_Server("We do not have avatar, removing hash.");
							SetMyAvatar(0, 0);
							break;
						}
						dwPaFormat = DetectAvatarFormat(file);
						hash = calcMD5Hash(file);
						if (!hash)
						{ // the hash could not be calculated, remove from server
							NetLog_Server("We could not obtain hash, removing hash.");
							SetMyAvatar(0, 0);
						}
						else if (!memcmp(hash, pBuffer+4, 0x10))
						{ // we have the right file
							HANDLE hFile = NULL, hMap = NULL;
							BYTE* ppMap = NULL;
							long cbFileSize = 0;

							NetLog_Server("Uploading our avatar data.");

							if ((hFile = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL )) != INVALID_HANDLE_VALUE)
								if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
									if ((ppMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)) != NULL)
										cbFileSize = GetFileSize( hFile, NULL );

							if (cbFileSize != 0)
							{
								SetAvatarData(NULL, (WORD)(dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC), (char*)ppMap, cbFileSize);
							}

							if (ppMap != NULL) UnmapViewOfFile(ppMap);
							if (hMap  != NULL) CloseHandle(hMap);
							if (hFile != NULL) CloseHandle(hFile);
							SAFE_FREE((void**)&hash);
						}
						else
						{
							BYTE* pHash = (BYTE*)_alloca(0x14);

							NetLog_Server("Our file is different, set our new hash.");

							pHash[0] = 0;
							pHash[1] = dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC;
							pHash[2] = 1; // state of the hash
							pHash[3] = 0x10; // len of the hash
							memcpy(pHash + 4, hash, 0x10);
							updateServAvatarHash(pHash, 0x14);

							SAFE_FREE((void**)&hash);
						}

						SAFE_FREE((void**)&file);
						break;
				default:
					NetLog_Server("Received UNKNOWN Avatar Status.");
					}
				}
			}
			break;
		}

	case ICQ_ERROR:
		{ // Something went wrong, probably the request for avatar family failed
			WORD wError;

			if (wBufferLength >= 2)
				unpackWord(&pBuffer, &wError);
			else 
				wError = 0;

			LogFamilyError(ICQ_SERVICE_FAMILY, wError);
			break;
		}

		// Stuff we don't care about
	case ICQ_SERVER_MOTD:
#ifdef _DEBUG
		NetLog_Server("Server message of the day");
#endif
		break;

	default:
		NetLog_Server("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_SERVICE_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;

	}
}

#define MD5_BLOCK_SIZE 1024*1024 /* use 1MB blocks */

BYTE* calcMD5Hash(char* szFile)
{
	BYTE *res = NULL;

	if (szFile)
	{
		HANDLE hFile = NULL, hMap = NULL;
		BYTE* ppMap = NULL;

		if ((hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL )) != INVALID_HANDLE_VALUE)
			if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
			{
				long cbFileSize = GetFileSize( hFile, NULL );

				res = (BYTE*)SAFE_MALLOC(16*sizeof(char));
				if (cbFileSize != 0 && res)
				{
					mir_md5_state_t state;
					mir_md5_byte_t digest[16];
					int dwOffset = 0;

					mir_md5_init(&state);
					while (dwOffset < cbFileSize)
					{
						int dwBlockSize = min(MD5_BLOCK_SIZE, cbFileSize-dwOffset);

						if (!(ppMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, dwOffset, dwBlockSize)))
							break;
						mir_md5_append(&state, (const mir_md5_byte_t *)ppMap, dwBlockSize);
						UnmapViewOfFile(ppMap);
						dwOffset += dwBlockSize;
					}
					mir_md5_finish(&state, digest);
					memcpy(res, digest, 16);
				}
			}

			if (hMap  != NULL) CloseHandle(hMap);
			if (hFile != NULL) CloseHandle(hFile);
	}

	return res;
}

char* CIcqProto::buildUinList(int subtype, WORD wMaxLen, HANDLE* hContactResume)
{
	char* szList;
	HANDLE hContact;
	WORD wCurrentLen = 0;
	DWORD dwUIN;
	uid_str szUID;
	char szLen[2];
	int add;

	szList = (char*)SAFE_MALLOC(CallService(MS_DB_CONTACT_GETCOUNT, 0, 0) * UINMAXLEN);
	szLen[1] = '\0';

	if (*hContactResume)
		hContact = *hContactResume;
	else
		hContact = FindFirstContact();

	while (hContact != NULL)
	{
		if (!getContactUid(hContact, &dwUIN, &szUID))
		{
			szLen[0] = strlennull(strUID(dwUIN, szUID));

			switch (subtype)
			{

			case BUL_VISIBLE:
				add = ID_STATUS_ONLINE == getSettingWord(hContact, "ApparentMode", 0);
				break;

			case BUL_INVISIBLE:
				add = ID_STATUS_OFFLINE == getSettingWord(hContact, "ApparentMode", 0);
				break;

			case BUL_TEMPVISIBLE:
				add = getSettingByte(hContact, "TemporaryVisible", 0);
				// clear temporary flag
				// Here we assume that all temporary contacts will be in one packet
				setSettingByte(hContact, "TemporaryVisible", 0);
				break;

			default:
				add = 1;

				// If we are in SS mode, we only add those contacts that are
				// not in our SS list, or are awaiting authorization, to our
				// client side list
				if (m_bSsiEnabled && getSettingWord(hContact, "ServerId", 0) &&
					!getSettingByte(hContact, "Auth", 0))
					add = 0;

				// Never add hidden contacts to CS list
				if (DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
					add = 0;

				break;
			}

			if (add)
			{
				wCurrentLen += szLen[0] + 1;
				if (wCurrentLen > wMaxLen)
				{
					*hContactResume = hContact;
					return szList;
				}

				strcat(szList, szLen);
				strcat(szList, szUID);
			}
		}

		hContact = FindNextContact(hContact);
	}
	*hContactResume = NULL;

	return szList;
}

void CIcqProto::sendEntireListServ(WORD wFamily, WORD wSubtype, int listType)
{
	HANDLE hResumeContact;
	char* szList;
	int nListLen;
	icq_packet packet;


	hResumeContact = NULL;

	do
	{ // server doesn't seem to be able to cope with packets larger than 8k
		// send only about 100contacts per packet
		szList = buildUinList(listType, 0x3E8, &hResumeContact);
		nListLen = strlennull(szList);

		if (nListLen)
		{
			serverPacketInit(&packet, (WORD)(nListLen + 10));
			packFNACHeader(&packet, wFamily, wSubtype);
			packBuffer(&packet, (LPBYTE)szList, (WORD)nListLen);
			sendServPacket(&packet);
		}

		SAFE_FREE((void**)&szList);
	}
		while (hResumeContact);
}

static void packNewCap(icq_packet* packet, WORD wNewCap)
{ // pack standard capability
	DWORD dwQ1 = 0x09460000 | wNewCap;

	packDWord(packet, dwQ1); 
	packDWord(packet, 0x4c7f11d1);
	packDWord(packet, 0x82224445);
	packDWord(packet, 0x53540000);
}

// CLI_SETUSERINFO
void CIcqProto::setUserInfo()
{ 
	icq_packet packet;
	WORD wAdditionalData = 0;
	BYTE bXStatus = getContactXStatus(NULL);

	if (m_bAimEnabled)
		wAdditionalData += 16;
#ifdef DBG_CAPMTN
	wAdditionalData += 16;
#endif
	if (m_bUtfEnabled)
		wAdditionalData += 16;
#ifdef DBG_NEWCAPS
	wAdditionalData += 16;
#endif
#ifdef DBG_CAPXTRAZ
	wAdditionalData += 16;
#endif
#ifdef DBG_OSCARFT
	wAdditionalData += 16;
#endif
	if (m_bAvatarsEnabled)
		wAdditionalData += 16;
	if (bXStatus)
		wAdditionalData += 16;
#ifdef DBG_CAPHTML
	wAdditionalData += 16;
#endif
#ifdef DBG_AIMCONTACTSEND
	wAdditionalData += 16;
#endif

	serverPacketInit(&packet, (WORD)(62 + wAdditionalData));
	packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_SET_USER_INFO);

	/* TLV(5): capability data */
	packWord(&packet, 0x0005);
	packWord(&packet, (WORD)(48 + wAdditionalData));

#ifdef DBG_CAPMTN
	{
		packDWord(&packet, 0x563FC809); // CAP_TYPING
		packDWord(&packet, 0x0B6F41BD);
		packDWord(&packet, 0x9F794226);
		packDWord(&packet, 0x09DFA2F3);
	}
#endif
	{
		packNewCap(&packet, 0x1349);    // AIM_CAPS_ICQSERVERRELAY
	}
	if (m_bUtfEnabled)
	{
		packNewCap(&packet, 0x134E);    // CAP_UTF8MSGS
	} // Broadcasts the capability to receive UTF8 encoded messages
#ifdef DBG_NEWCAPS
	{
		packNewCap(&packet, 0x0000);    // CAP_NEWCAPS
	} // Tells server we understand to new format of caps
#endif
#ifdef DBG_CAPXTRAZ
	{
		packDWord(&packet, 0x1a093c6c); // CAP_XTRAZ
		packDWord(&packet, 0xd7fd4ec5); // Broadcasts the capability to handle
		packDWord(&packet, 0x9d51a647); // Xtraz
		packDWord(&packet, 0x4e34f5a0);
	}
#endif
	if (m_bAvatarsEnabled)
	{
		packNewCap(&packet, 0x134C);    // CAP_DEVILS
	}
#ifdef DBG_OSCARFT
	{
		packNewCap(&packet, 0x1343);    // CAP_AIM_FILE
	} // Broadcasts the capability to receive Oscar File Transfers
#endif
	if (m_bAimEnabled)
	{
		packNewCap(&packet, 0x134D);    // Tells the server we can speak to AIM
	}
#ifdef DBG_AIMCONTACTSEND
	{
		packNewCap(&packet, 0x134B);    // CAP_AIM_SENDBUDDYLIST
	}
#endif
	if (bXStatus)
	{
		packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
	}

	packNewCap(&packet, 0x1344);      // AIM_CAPS_ICQDIRECT

	/*packDWord(&packet, 0x178c2d9b); // Unknown cap
	packDWord(&packet, 0xdaa545bb);
	packDWord(&packet, 0x8ddbf3bd);
	packDWord(&packet, 0xbd53a10a);*/

#ifdef DBG_CAPHTML
	{
		packDWord(&packet, 0x0138ca7b); // CAP_HTMLMSGS
		packDWord(&packet, 0x769a4915); // Broadcasts the capability to receive
		packDWord(&packet, 0x88f213fc); // HTML messages
		packDWord(&packet, 0x00979ea8);
	}
#endif

	packDWord(&packet, 0x4D697261);   // Miranda Signature
	packDWord(&packet, 0x6E64614D);
	packDWord(&packet, MIRANDA_VERSION);
	packDWord(&packet, ICQ_PLUG_VERSION);

	sendServPacket(&packet);
}

void CIcqProto::handleServUINSettings(int nPort, serverthread_info *info)
{
	icq_packet packet;

	setUserInfo();

	/* SNAC 3,4: Tell server who's on our list */
	sendEntireListServ(ICQ_BUDDY_FAMILY, ICQ_USER_ADDTOLIST, BUL_ALLCONTACTS);

	if (m_iDesiredStatus == ID_STATUS_INVISIBLE)
	{
		/* Tell server who's on our visible list */
		if (!m_bSsiEnabled)
			sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDVISIBLE, BUL_VISIBLE);
		else
			updateServVisibilityCode(3);
	}

	if (m_iDesiredStatus != ID_STATUS_INVISIBLE)
	{
		/* Tell server who's on our invisible list */
		if (!m_bSsiEnabled)
			sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDINVISIBLE, BUL_INVISIBLE);
		else
			updateServVisibilityCode(4);
	}

	// SNAC 1,1E: Set status
	{
		WORD wStatus;
		DWORD dwDirectCookie = rand() ^ (rand() << 16);
		BYTE bXStatus = getContactXStatus(NULL);
		char szMoodId[32];
		WORD cbMoodId = 0;
		WORD cbMoodData = 0;

		// Get status
		wStatus = MirandaStatusToIcq(m_iDesiredStatus);

		if (bXStatus && moodXStatus[bXStatus-1] != -1)
		{ // prepare mood id
			null_snprintf(szMoodId, SIZEOF(szMoodId), "icqmood%d", moodXStatus[bXStatus-1]);
			cbMoodId = strlennull(szMoodId);
			cbMoodData = 8;
		}

		serverPacketInit(&packet, (WORD)(71 + cbMoodId + cbMoodData));
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
		packDWord(&packet, 0x00060004);             // TLV 6: Status mode and security flags
		packWord(&packet, GetMyStatusFlags());      // Status flags
		packWord(&packet, wStatus);                 // Status
		packTLVWord(&packet, 0x0008, 0x0000);       // TLV 8: Error code
		packDWord(&packet, 0x000c0025);             // TLV C: Direct connection info
		packDWord(&packet, getSettingDword(NULL, "RealIP", 0));
		packDWord(&packet, nPort);
		packByte(&packet, DC_TYPE);                 // TCP/FLAG firewall settings
		packWord(&packet, ICQ_VERSION);
		packDWord(&packet, dwDirectCookie);         // DC Cookie
		packDWord(&packet, WEBFRONTPORT);           // Web front port
		packDWord(&packet, CLIENTFEATURES);         // Client features
		packDWord(&packet, gbUnicodeCore ? 0x7fffffff : 0xffffffff); // Abused timestamp
		packDWord(&packet, ICQ_PLUG_VERSION);       // Abused timestamp
		if (ServiceExists("SecureIM/IsContactSecured"))
			packDWord(&packet, 0x5AFEC0DE);           // SecureIM Abuse
		else
			packDWord(&packet, 0x00000000);           // Timestamp
		packWord(&packet, 0x0000);                  // Unknown
		packTLVWord(&packet, 0x001F, 0x0000);

		if (cbMoodId)
		{ // Pack mood data
			packWord(&packet, 0x1D);              // TLV 1D
			packWord(&packet, (WORD)(cbMoodId + 4)); // TLV length
			packWord(&packet, 0x0E);              // Item Type
			packWord(&packet, cbMoodId);          // Flags + Item Length
			packBuffer(&packet, (LPBYTE)szMoodId, cbMoodId); // Mood
		}

		sendServPacket(&packet);
	}

	/* SNAC 1,11 */
	serverPacketInit(&packet, 14);
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_IDLE);
	packDWord(&packet, 0x00000000);

	sendServPacket(&packet);
	m_bIdleAllow = 0;

	// Change status
	SetCurrentStatus(m_iDesiredStatus);

	// Finish Login sequence
	serverPacketInit(&packet, 98);
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_READY);
	packDWord(&packet, 0x00220001); // imitate ICQ 6 behaviour
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00010004);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00130004);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00020001);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00030001);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00150001);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00040001);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00060001);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x00090001);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x000A0001);
	packDWord(&packet, 0x0110161b);
	packDWord(&packet, 0x000B0001);
	packDWord(&packet, 0x0110161b);

	sendServPacket(&packet);

	NetLog_Server(" *** Yeehah, login sequence complete");

	// login sequence is complete enter logged-in mode
	info->bLoggedIn = 1;

	// enable auto info-update routine
	icq_EnableUserLookup(TRUE);

	if (!info->isMigrating)
	{ /* Get Offline Messages Reqeust */
		offline_message_cookie *ack;

		ack = (offline_message_cookie*)SAFE_MALLOC(sizeof(offline_message_cookie));
		if (ack)
		{
			DWORD dwCookie = AllocateCookie(CKT_OFFLINEMESSAGE, ICQ_MSG_CLI_REQ_OFFLINE, 0, ack);

			serverPacketInit(&packet, 10);
			packFNACHeaderFull(&packet, ICQ_MSG_FAMILY, ICQ_MSG_CLI_REQ_OFFLINE, 0, dwCookie);

			sendServPacket(&packet);
		}
		else
			icq_LogMessage(LOG_WARNING, LPGEN("Failed to request offline messages. They may be received next time you log in."));

		// Update our information from the server
		sendOwnerInfoRequest();

		// Request info updates on all contacts
		icq_RescanInfoUpdate();

		// Start sending Keep-Alive packets
		StartKeepAlive(info);

		if (m_bAvatarsEnabled)
		{ // Send SNAC 1,4 - request avatar family 0x10 connection
			icq_requestnewfamily(ICQ_AVATAR_FAMILY, &CIcqProto::StartAvatarThread);

			m_pendingAvatarsStart = 1;
			NetLog_Server("Requesting Avatar family entry point.");
		}
	}
	info->isMigrating = 0;

	if (m_bAimEnabled)
	{
		char **szMsg = MirandaStatusToAwayMsg(m_iStatus);

		EnterCriticalSection(&m_modeMsgsMutex);
		if (szMsg)
			icq_sendSetAimAwayMsgServ(*szMsg);
		LeaveCriticalSection(&m_modeMsgsMutex);
	}
}

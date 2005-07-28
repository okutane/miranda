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
//  Global constants and default settings are defined here
//
// -----------------------------------------------------------------------------

// Most of the protocol constants follow the naming conventions of the
// Oscar documentation at http://iserverd.khstu.ru/oscar/index.html
// BIG THANKS to Alexandr for maintaining this site and to everyone
// in the ICQ devel community who have helped to collect the data.

#ifndef __ICQ_CONSTANTS_H
#define __ICQ_CONSTANTS_H



/* Some default settings */
#define DEFAULT_SERVER_PORT         5190
#define DEFAULT_SERVER_HOST         "login.icq.com"
#define DEFAULT_SS_ENABLED          1
#define DEFAULT_SS_ADDSERVER        1
#define DEFAULT_SS_LOAD             0
#define DEFAULT_SS_STORE            1
#define DEFAULT_SS_GROUP            "General"

#define DEFAULT_AIM_ENABLED         0
#define DEFAULT_UTF_ENABLED         2 // everything unicode is default
#define DEFAULT_ANSI_CODEPAGE       CP_ACP
#define DEFAULT_DCMSG_ENABLED       1 // passive dc messaging is default
#define DEFAULT_TEMPVIS_ENABLED     1 // temporary visible is enabled by default
#define DEFAULT_MTN_ENABLED         0
#define DEFAULT_CAPS                0
#define DEFAULT_AVATARS_ENABLED     1
#define DEFAULT_LOAD_AVATARS        1
#define DEFAULT_LINK_AVATARS        1
#define DEFAULT_XSTATUS_ENABLED     1
#define DEFAULT_XSTATUS_AUTO        1
#define DEFAULT_KILLSPAM_ENABLED    1

// Database setting names
#define DBSETTING_CAPABILITIES      "caps"


// Status FLAGS (used to determine status of other users)
#define ICQ_STATUSF_ONLINE      0x0000
#define ICQ_STATUSF_AWAY        0x0001
#define ICQ_STATUSF_DND         0x0002
#define ICQ_STATUSF_NA          0x0004
#define ICQ_STATUSF_OCCUPIED    0x0010
#define ICQ_STATUSF_FFC         0x0020
#define ICQ_STATUSF_INVISIBLE   0x0100

// Status values (used to set own status)
#define ICQ_STATUS_ONLINE       0x0000
#define ICQ_STATUS_AWAY         0x0001
#define ICQ_STATUS_NA           0x0005
#define ICQ_STATUS_OCCUPIED     0x0011
#define ICQ_STATUS_DND          0x0013
#define ICQ_STATUS_FFC          0x0020
#define ICQ_STATUS_INVISIBLE    0x0100

#define STATUS_WEBAWARE         0x0001 // Status webaware flag
#define STATUS_SHOWIP           0x0002 // Status show ip flag
#define STATUS_BIRTHDAY         0x0008 // User birthday flag
#define STATUS_WEBFRONT         0x0020 // User active webfront flag
#define STATUS_DCDISABLED       0x0100 // Direct connection not supported
#define STATUS_DCAUTH           0x1000 // Direct connection upon authorization
#define STATUS_DCCONT           0x2000 // DC only with contact users



// Typing notification statuses
#define MTN_FINISHED            0x0000
#define MTN_TYPED               0x0001
#define MTN_BEGUN               0x0002



// Ascii Capability IDs
#define CAP_RTFMSGS  	          "{97B12751-243C-4334-AD22-D6ABF73F1492}"
#define CAP_UTF8MSGS            "{0946134E-4C7F-11D1-8222-444553540000}"

// Binary Capability IDs
#define BINARY_CAP_SIZE         16
#define CAP_SRV_RELAY           0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define CAP_UTF                 0x09, 0x46, 0x13, 0x4e, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define CAP_TYPING              0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 0xbd, 0x9f, 0x79, 0x42, 0x26, 0x09, 0xdf, 0xa2, 0xf3

// Miranda IM Capability bitmask
#define CAPF_SRV_RELAY          0x00000001
#define CAPF_UTF                0x00000002
#define CAPF_TYPING             0x00000004


// Message Capability IDs
#define MCAP_TLV2711_FMT_s      0x09461349, 0x4c7f11d1, 0x82224445, 0x53540000
#define MCAP_REVERSE_REQ_s      0x09461344, 0x4c7f11d1, 0x82224445, 0x53540000

// Plugin Type GUIDs
#define PSIG_MESSAGE_s          0x00000000, 0x00000000, 0x00000000, 0x00000000
#define PSIG_INFO_PLUGIN_s      0xa0e93f37, 0x4fe9d311, 0xbcd20004, 0xac96dd96
#define PSIG_STATUS_PLUGIN_s    0x10cf40d1, 0x4fe9d311, 0xbcd20004, 0xac96dd96

// Plugin Message GUIDs
#define PMSG_QUERY_INFO_s       0xF002BF71, 0x4371D311, 0x8DD20010, 0x4B06462E
#define PMSG_QUERY_STATUS_s     0x10180670, 0x5471D311, 0x8DD20010, 0x4B06462E



// Message types
#define MTYPE_PLAIN             0x01 // Plain text (simple) message
#define MTYPE_CHAT              0x02 // Chat request message
#define MTYPE_FILEREQ           0x03 // File request / file ok message
#define MTYPE_URL               0x04 // URL message (0xFE formatted)
#define MTYPE_AUTHREQ           0x06 // Authorization request message (0xFE formatted)
#define MTYPE_AUTHDENY          0x07 // Authorization denied message (0xFE formatted)
#define MTYPE_AUTHOK            0x08 // Authorization given message (empty)
#define MTYPE_SERVER            0x09 // Message from OSCAR server (0xFE formatted)
#define MTYPE_ADDED             0x0C // "You-were-added" message (0xFE formatted)
#define MTYPE_WWP               0x0D // Web pager message (0xFE formatted)
#define MTYPE_EEXPRESS          0x0E // Email express message (0xFE formatted)
#define MTYPE_CONTACTS          0x13 // Contact list message
#define MTYPE_PLUGIN            0x1A // Plugin message described by text string
#define MTYPE_AUTOAWAY          0xE8 // Auto away message
#define MTYPE_AUTOBUSY          0xE9 // Auto occupied message
#define MTYPE_AUTONA            0xEA // Auto not available message
#define MTYPE_AUTODND           0xEB // Auto do not disturb message
#define MTYPE_AUTOFFC           0xEC // Auto free for chat message
// Internal Message types
#define MTYPE_UNKNOWN           0x00 // Unknown message

#define MTYPE_GREETINGCARD      0x101 // Request for Contacts
#define MTYPE_REQUESTCONTACTS   0x102 // Greeting Card
#define MTYPE_MESSAGE           0x103 // Message+
#define MTYPE_SCRIPT_INVITATION 0x201 // Xtraz Invitation
#define MTYPE_SCRIPT_DATA       0x202 // Xtraz Message
#define MTYPE_SCRIPT_NOTIFY     0x208 // Xtraz Response

// Message Plugin Type GUIDs
#define MGTYPE_MESSAGE_s        0xBE6B7305, 0x0FC2104F, 0xA6DE4DB1, 0xE3564B0E 
#define MGTYPE_FILE_s           0xF02D12D9, 0x3091D311, 0x8DD70010, 0x4B06462E
#define MGTYPE_WEBURL_s         0x371C5872, 0xE987D411, 0xA4C100D0, 0xB759B1D9
#define MGTYPE_CONTACTS_s       0x2A0E7D46, 0x7676D411, 0xBCE60004, 0xAC961EA6 
#define MGTYPE_GREETING_CARD_s  0x01E53B48, 0x2AE4D111, 0xB6790060, 0x97E1E294
#define MGTYPE_CHAT_s           0xBFF720B2, 0x378ED411, 0xBD280004, 0xAC96D905
#define MGTYPE_XTRAZ_SCRIPT_s   0x3b60b3ef, 0xd82a6c45, 0xa4e09c5a, 0x5e67e865

// Message Plugin Sub-Type IDs
#define MGTYPE_STANDARD_SEND      0x00
#define MGTYPE_CONTACTS_REQUEST   0x02
#define MGTYPE_SCRIPT_INVITATION  0x01
#define MGTYPE_SCRIPT_DATA        0x02
#define MGTYPE_SCRIPT_USER_REMOVE 0x04
#define MGTYPE_SCRIPT_NOTIFY      0x08



/* Channels */
#define	ICQ_LOGIN_CHAN			        0x01
#define ICQ_DATA_CHAN				        0x02
#define ICQ_ERROR_CHAN			        0x03
#define ICQ_CLOSE_CHAN			        0x04
#define ICQ_PING_CHAN				        0x05

/* Families */
#define ICQ_SERVICE_FAMILY          0x0001
#define ICQ_LOCATION_FAMILY         0x0002
#define ICQ_BUDDY_FAMILY            0x0003
#define ICQ_MSG_FAMILY              0x0004
#define ICQ_BOS_FAMILY              0x0009
#define ICQ_STATS_FAMILY            0x000b
#define ICQ_AVATAR_FAMILY           0x0010
#define ICQ_LISTS_FAMILY            0x0013
#define ICQ_EXTENSIONS_FAMILY       0x0015

/* Subtypes for Service Family 0x0001 */
#define ICQ_ERROR                   0x0001
#define ICQ_CLIENT_READY            0x0002
#define ICQ_SERVER_READY            0x0003
#define ICQ_CLIENT_NEW_SERVICE      0x0004
#define ICQ_SERVER_REDIRECT_SERVICE 0x0005
#define ICQ_CLIENT_REQ_RATE_INFO    0x0006
#define ICQ_SERVER_RATE_INFO        0x0007
#define ICQ_CLIENT_RATE_ACK         0x0008
#define ICQ_SERVER_RATE_CHANGE      0x000a
#define ICQ_SERVER_PAUSE            0x000b
#define ICQ_CLIENT_PAUSE_ACK        0x000c
#define ICQ_SERVER_RESUME           0x000d
#define ICQ_CLIENT_REQINFO          0x000e
#define ICQ_SERVER_NAME_INFO        0x000f
#define ICQ_CLIENT_SET_IDLE         0x0011
#define ICQ_SERVER_MIGRATIONREQ     0x0012
#define ICQ_SERVER_MOTD             0x0013
#define ICQ_CLIENT_FAMILIES         0x0017
#define ICQ_SERVER_FAMILIES2        0x0018
#define ICQ_CLIENT_SET_STATUS       0x001e
#define ICQ_SERVER_EXTSTATUS        0x0021

/* Subtypes for Location Family 0x0002 */
#define ICQ_LOCATION_CLI_REQ_RIGHTS 0x0002
#define ICQ_LOCATION_RIGHTS_REPLY   0x0003
#define ICQ_LOCATION_SET_USER_INFO  0x0004
#define ICQ_LOCATION_REQ_USER_INFO  0x0005
#define ICQ_LOCATION_USR_INFO_REPLY 0x0006

/* Subtypes for Buddy Family 0x0003 */
#define ICQ_USER_CLI_REQBUDDY       0x0002
#define ICQ_USER_SRV_REPLYBUDDY     0x0003
#define ICQ_USER_ADDTOLIST          0x0004
#define ICQ_USER_REMOVEFROMLIST     0x0005
#define ICQ_USER_NOTIFY_REJECTED    0x000a
#define ICQ_USER_ONLINE             0x000b
#define ICQ_USER_OFFLINE            0x000c

/* Subtypes for Message Family 0x0004 */
#define ICQ_MSG_SRV_ERROR           0x0001
#define ICQ_MSG_CLI_SETPARAMS       0x0002
#define ICQ_MSG_CLI_RESETPARAMS     0x0003
#define ICQ_MSG_CLI_REQICBM         0x0004
#define ICQ_MSG_SRV_REPLYICBM       0x0005
#define ICQ_MSG_SRV_SEND            0x0006
#define ICQ_MSG_SRV_RECV            0x0007
#define ICQ_MSG_SRV_MISSED_MESSAGE  0x000A
#define ICQ_MSG_RESPONSE            0x000B
#define ICQ_MSG_SRV_ACK             0x000C
#define ICQ_MSG_MTN                 0x0014

/* Subtypes for Privacy Family 0x0009 */
#define ICQ_PRIVACY_REQ_RIGHTS      0x0002
#define ICQ_PRIVACY_RIGHTS_REPLY    0x0003
#define ICQ_CLI_ADDVISIBLE          0x0005
#define ICQ_CLI_REMOVEVISIBLE       0x0006
#define ICQ_CLI_ADDINVISIBLE        0x0007
#define ICQ_CLI_REMOVEINVISIBLE     0x0008
#define ICQ_PRIVACY_SERVICE_ERROR   0x0009
#define ICQ_CLI_ADDTEMPVISIBLE      0x000A
#define ICQ_CLI_REMOVETEMPVISIBLE   0x000B

/* Subtypes for Stats Family 0x000b */
#define ICQ_STATS_MINREPORTINTERVAL 0x0002

/* Subtypes for Avatar Family 0x0010 */
#define ICQ_AVATAR_ERROR            0x0001
#define ICQ_AVATAR_UPLOAD_REQUEST   0x0002
#define ICQ_AVATAR_UPLOAD_ACK       0x0003
#define ICQ_AVATAR_GET_REQUEST	    0x0006
#define ICQ_AVATAR_GET_REPLY        0x0007

/* Subtypes for Server Lists Family 0x0013 */
#define ICQ_LISTS_ERROR             0x0001
#define ICQ_LISTS_CLI_REQLISTS      0x0002
#define ICQ_LISTS_SRV_REPLYLISTS    0x0003
#define ICQ_LISTS_CLI_REQUEST       0x0004
#define ICQ_LISTS_CLI_CHECK         0x0005
#define ICQ_LISTS_LIST              0x0006
#define ICQ_LISTS_GOTLIST           0x0007
#define ICQ_LISTS_ADDTOLIST         0x0008
#define ICQ_LISTS_UPDATEGROUP       0x0009
#define ICQ_LISTS_REMOVEFROMLIST    0x000A
#define ICQ_LISTS_ACK               0x000E
#define SRV_SSI_UPTODATE            0x000F
#define ICQ_LISTS_CLI_MODIFYSTART   0x0011
#define ICQ_LISTS_CLI_MODIFYEND     0x0012
#define ICQ_LISTS_GRANTAUTH         0x0014
#define ICQ_LISTS_AUTHGRANTED       0x0015
#define ICQ_LISTS_REMOVEYOURSELF    0x0016
#define ICQ_LISTS_REQUESTAUTH       0x0018
#define ICQ_LISTS_AUTHREQUEST       0x0019
#define ICQ_LISTS_CLI_AUTHRESPONSE  0x001A
#define ICQ_LISTS_SRV_AUTHRESPONSE  0x001B
#define ICQ_LISTS_YOUWEREADDED      0x001C

// SNACS for ICQ Extensions Family 0x0015
#define SRV_ICQEXT_ERROR            0x0001
#define CLI_META_REQ                0x0002
#define SRV_META_REPLY              0x0003

// Reply types for SNAC 15/02 & 15/03
#define CLI_OFFLINE_MESSAGE_REQ     0x003C
#define CLI_DELETE_OFFLINE_MSGS_REQ 0x003E
#define SRV_OFFLINE_MESSAGE         0x0041
#define SRV_END_OF_OFFLINE_MSGS     0x0042
#define CLI_META_INFO_REQ           0x07D0
#define SRV_META_INFO_REPLY         0x07DA

// Reply subtypes for SNAC 15/02 & 15/03
#define META_PROCESSING_ERROR       0x0001 // Meta processing error server reply
#define META_SET_HOMEINFO_ACK       0x0064 // Set user home info server ack
#define META_SET_WORKINFO_ACK       0x006E // Set user work info server ack
#define META_SET_MOREINFO_ACK       0x0078 // Set user more info server ack
#define META_SET_NOTES_ACK          0x0082 // Set user notes info server ack
#define META_SET_EMAILINFO_ACK      0x0087 // Set user email(s) info server ack
#define META_SET_INTINFO_ACK        0x008C // Set user interests info server ack
#define META_SET_AFFINFO_ACK        0x0096 // Set user affilations info server ack
#define META_SMS_DELIVERY_RECEIPT   0x0096 // Server SMS response (delivery receipt) NOTE: same as ID above
#define META_SET_PERMS_ACK          0x00A0 // Set user permissions server ack
#define META_SET_PASSWORD_ACK       0x00AA // Set user password server ack
#define META_UNREGISTER_ACK         0x00B4 // Unregister account server ack
#define META_SET_HPAGECAT_ACK       0x00BE // Set user homepage category server ack
#define META_BASIC_USERINFO         0x00C8 // User basic info reply
#define META_WORK_USERINFO          0x00D2 // User work info reply
#define META_MORE_USERINFO          0x00DC // User more info reply
#define META_NOTES_USERINFO         0x00E6 // User notes (about) info reply
#define META_EMAIL_USERINFO         0x00EB // User extended email info reply
#define META_INTERESTS_USERINFO     0x00F0 // User interests info reply
#define META_AFFILATIONS_USERINFO   0x00FA // User past/affilations info reply
#define META_SHORT_USERINFO         0x0104 // Short user information reply
#define META_HPAGECAT_USERINFO      0x010E // User homepage category information reply
#define SRV_USER_FOUND              0x01A4 // Search: user found reply
#define SRV_LAST_USER_FOUND         0x01AE // Search: last user found reply
#define META_REGISTRATION_STATS_ACK 0x0302 // Registration stats ack
#define SRV_RANDOM_FOUND            0x0366 // Random search server reply
#define META_REQUEST_FULL_INFO      0x04B2 // Request full user info
#define META_REQUEST_SHORT_INFO     0x04BA // Request short user info
#define META_REQUEST_SELF_INFO      0x04D0 // Request full self user info
#define META_SEARCH_GENERIC         0x055F // Search user by details (TLV)
#define META_SEARCH_UIN             0x0569 // Search user by UIN (TLV)
#define META_SEARCH_EMAIL           0x0573 // Search user by E-mail (TLV)

#define META_XML_INFO               0x08A2 // Server variable requested via xml
#define META_SET_FULLINFO_REQ       0x3A0C // Set full user info request
#define META_SET_FULLINFO_ACK       0x0C3F // Server ack for set fullinfo command
#define META_SPAM_REPORT_ACK        0x2012 // Server ack for user spam report

// TLV types

// SECURITY flags
#define TLV_AUTH        0x02F8  //   uint8      User authorization permissions
#define TLV_WEBAWARE    0x030C  //   uint8      User 'show web status' permissions


// SEARCH only TLVs
#define TLV_AGERANGE    0x0168  //   acombo     Age range to search
#define TLV_KEYWORDS    0x0226  //   sstring    Whitepages search keywords string
#define TLV_ONLINEONLY  0x0230  //   uint8      Search only online users flag
#define TLV_UIN         0x0136  //   uint32     User uin

// common
#define TLV_FIRSTNAME   0x0140  //   sstring    User firstname
#define TLV_LASTNAME    0x014A  //   sstring    User lastname
#define TLV_NICKNAME    0x0154  //   sstring    User nickname
#define TLV_EMAIL       0x015E  //   ecombo     User email
#define TLV_GENDER      0x017C  //   uint8      User gender
#define TLV_MARITAL     0x033E  //   uint8      User marital status
#define TLV_LANGUAGE    0x0186  //   uint16     User spoken language
#define TLV_CITY        0x0190  //   sstring    User home city name
#define TLV_STATE       0x019A  //   sstring    User home state abbr
#define TLV_COUNTRY     0x01A4  //   uint16     User home country code
#define TLV_COMPANY     0x01AE  //   sstring    User work company name
#define TLV_DEPARTMENT  0x01B8  //   sstring    User work department name
#define TLV_POSITION    0x01C2  //   sstring    User work position (title)
#define TLV_OCUPATION   0x01CC  //   uint16     User work ocupation code
#define TLV_PASTINFO    0x01D6  //   icombo     !! User affilations node
#define TLV_AFFILATIONS 0x01FE  //   icombo     !! User past info node
#define TLV_INTERESTS   0x01EA  //   icombo     User interests node
#define TLV_HOMEPAGE    0x0212  //   sstring    User homepage category/keywords

// changeinfo
#define TLV_AGE         0x0172  //   uint16     User age
#define TLV_URL         0x0213  //   sstring    User homepage url
#define TLV_BIRTH       0x023A  //   bcombo     User birthday info (year, month, day)
#define TLV_ABOUT       0x0258  //   sstring    User notes (about) text
#define TLV_STREET      0x0262  //   sstring    User home street address
#define TLV_ZIPCODE     0x026C  //   uint32     User home zip code
#define TLV_PHONE       0x0276  //   sstring    User home phone number
#define TLV_FAX         0x0280  //   sstring    User home fax number
#define TLV_MOBILE      0x028A  //   sstring    User home cellular phone number
#define TLV_WORKSTREET  0x0294  //   sstring    User work street address
#define TLV_WORKCITY    0x029E  //   sstring    User work city name
#define TLV_WORKSTATE   0x02A8  //   sstring    User work state name
#define TLV_WORKCOUNTRY 0x02B2  //   uint16     User work country code
#define TLV_WORKZIPCODE 0x02BC  //   uint32     User work zip code
#define TLV_WORKPHONE   0x02C6  //   sstring    User work phone number
#define TLV_WORKFAX     0x02D0  //   sstring    User work fax number
#define TLV_WORKURL     0x02DA  //   sstring    User work webpage url
#define TLV_TIMEZONE    0x0316  //   uint8      User GMT offset
#define TLV_ORGCITY	    0x0320  //   sstring    User originally from city
#define TLV_ORGSTATE    0x032A  //   sstring    User originally from state
#define TLV_ORGCOUNTRY  0x0334  //   uint16     User originally from country (code)


/* Direct packet types */
#define PEER_INIT                   0xFF
#define PEER_INIT_ACK               0x01
#define PEER_MSG_INIT               0x03
#define PEER_MSG                    0x02
#define PEER_FILE_INIT              0x00
#define PEER_FILE_INIT_ACK          0x01
#define PEER_FILE_NEXTFILE          0x02
#define PEER_FILE_RESUME            0x03
#define PEER_FILE_STOP              0x04
#define PEER_FILE_SPEED             0x05
#define PEER_FILE_DATA              0x06

/* Direct command types */
#define DIRECT_CANCEL               0x07D0    /* 2000 TCP cancel previous file/chat request */
#define DIRECT_ACK                  0x07DA    /* 2010 TCP acknowledge message packet */
#define DIRECT_MESSAGE              0x07EE    /* 2030 TCP message */

// DC types
#define DC_DISABLED                 0x0000 // Direct connection disabled / auth required
#define DC_HTTPS                    0x0001 // Direct connection thru firewall or https proxy
#define DC_SOCKS                    0x0002 // Direct connection thru socks4/5 proxy server
#define DC_NORMAL                   0x0004 // Normal direct connection (without proxy/firewall)
#define DC_WEB                      0x0006 // Web client - no direct connection

// Message flags
#define MFLAG_NORMAL                0x01 // Normal message
#define MFLAG_AUTO                  0x03 // Auto-message flag
#define MFLAG_MULTI                 0x80 // This is multiple recipients message

// Some SSI constants
#define SSI_ITEM_BUDDY              0x0000  // Buddy record (name: uin for ICQ and screenname for AIM)
#define SSI_ITEM_GROUP              0x0001  // Group record
#define SSI_ITEM_PERMIT             0x0002  // Permit record ("Allow" list in AIM, and "Visible" list in ICQ)
#define SSI_ITEM_DENY               0x0003  // Deny record ("Block" list in AIM, and "Invisible" list in ICQ)
#define SSI_ITEM_VISIBILITY         0x0004  // Permit/deny settings or/and bitmask of the AIM classes
#define SSI_ITEM_PRESENCE           0x0005  // Presence info (if others can see your idle status, etc)
#define SSI_ITEM_UNKNOWN1           0x0009  // Unknown. ICQ2k shortcut bar items ?
#define SSI_ITEM_IGNORE             0x000e  // Ignore list record.
#define SSI_ITEM_NONICQ             0x0010  // Non-ICQ contact (to send SMS). Name: 1#EXT, 2#EXT, etc
#define SSI_ITEM_UNKNOWN2           0x0011  // Unknown.
#define SSI_ITEM_IMPORTTIME         0x0013  // Item that contain roster import time (name: "Import time")
#define SSI_ITEM_BUDDYICON          0x0014  // Buddy icon info. (names: from "0" and incrementing by one)



// Internal Constants
#define ICQ_PLUG_VERSION            0x80030603
#define ICQ_VERSION                 8
#define DC_TYPE                     DC_NORMAL // Used for DC settings
#define MAX_NICK_SIZE               32
#define MAX_CONTACTSSEND            15
#define MAX_MESSAGESNACSIZE         8000
#define CLIENTRATELIMIT             0
#define UPDATE_THRESHOLD            1209600 // Two weeks
#define COOKIE_TIMEOUT              3600 // One hour
#define WEBFRONTPORT                0x50
#define CLIENTFEATURES              0x3
#define URL_FORGOT_PASSWORD         "https://www.icq.com/password/"
#define URL_REGISTER                "http://lite.icq.com/register"
#define FLAP_MARKER                 0x2a
#define CLIENT_ID_STRING            "ICQBasic"
#define UNIQUEIDSETTING             "UIN"
#define UINMAXLEN                   11 // DWORD string max len + 1

#endif /* __ICQ_CONSTANTS_H */

/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-08  George Hazan
Copyright ( C ) 2007-08  Maxim Mluhov
Copyright ( C ) 2007-08  Victor Pavlychko

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL: https://miranda.svn.sourceforge.net/svnroot/miranda/trunk/miranda/protocols/JabberG/ui_utils.h $
Revision       : $Revision: 8244 $
Last change on : $Date: 2008-08-17 15:09:42 +0300 (��, 17 C�� 2008) $
Last change by : $Author: vpavlychko $

*/

#ifndef __jabber_db_utils_h__
#define __jabber_db_utils_h__

template<typename Int> struct CMIntTraits { static __forceinline bool IsSigned() { return false; } };
template<> struct CMIntTraits<signed char> { static __forceinline bool IsSigned() { return true; } };
template<> struct CMIntTraits<signed short> { static __forceinline bool IsSigned() { return true; } };
template<> struct CMIntTraits<signed long> { static __forceinline bool IsSigned() { return true; } };

template<int Size>
struct CMDBTraits
{
};

template<>
struct CMDBTraits<1>
{
	typedef BYTE DBType;
	enum { DBTypeId = DBVT_BYTE };
	static __forceinline DBType Get(char *szModule, char *szSetting, DBType value)
	{
		return DBGetContactSettingByte(NULL, szModule, szSetting, value);
	}
	static __forceinline void Set(char *szModule, char *szSetting, DBType value)
	{
		DBWriteContactSettingByte(NULL, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<2>
{
	typedef WORD DBType;
	enum { DBTypeId = DBVT_WORD };
	static __forceinline DBType Get(char *szModule, char *szSetting, DBType value)
	{
		return DBGetContactSettingWord(NULL, szModule, szSetting, value);
	}
	static __forceinline void Set(char *szModule, char *szSetting, DBType value)
	{
		DBWriteContactSettingWord(NULL, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<4>
{
	typedef DWORD DBType;
	enum { DBTypeId = DBVT_DWORD };
	static __forceinline BYTE GetDBType()
	{
		return DBVT_DWORD;
	}
	static __forceinline DBType Get(char *szModule, char *szSetting, DBType value)
	{
		return DBGetContactSettingDword(NULL, szModule, szSetting, value);
	}
	static __forceinline void Set(char *szModule, char *szSetting, DBType value)
	{
		DBWriteContactSettingDword(NULL, szModule, szSetting, value);
	}
};

class CMOptionBase
{
public:
	BYTE GetDBType() { return m_dbType; }
	char *GetDBModuleName() { return m_proto->m_szModuleName; }
	char *GetDBSettingName() { return m_szSetting; }

protected:
	CMOptionBase(PROTO_INTERFACE *proto, char *szSetting, BYTE dbType): m_proto(proto), m_szSetting(szSetting), m_dbType(dbType) {}

	PROTO_INTERFACE *m_proto;
	char *m_szSetting;
	BYTE m_dbType;

private:
	CMOptionBase(const CMOptionBase &) {}
	void operator= (const CMOptionBase &) {}
};

template<class T>
class CMOption: public CMOptionBase
{
public:
	typedef T Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, char *szSetting, Type defValue):
		CMOptionBase(proto, szSetting, CMDBTraits<sizeof(T)>::DBTypeId), m_default(defValue) {}

	__forceinline operator Type()
	{
		return (Type)CMDBTraits<sizeof(Type)>::Get(m_proto->m_szModuleName, m_szSetting, m_default);
	}
	__forceinline Type operator= (Type value)
	{
		CMDBTraits<sizeof(Type)>::Set(m_proto->m_szModuleName, m_szSetting, (CMDBTraits<sizeof(Type)>::DBType)value);
		return value;
	}

private:
	Type m_default;

	CMOption(const CMOption &): CMOptionBase(NULL, NULL, DBVT_DELETED) {}
	void operator= (const CMOption &) {}
};

template<>
class CMOption<CMString>: public CMOptionBase
{
public:
	typedef const TCHAR *Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, char *szSetting, Type defValue, bool crypt=false):
		CMOptionBase(proto, szSetting, DBVT_TCHAR), m_default(defValue), m_crypt(crypt) {}

	__forceinline operator CMString()
	{
		CMString result;
		DBVARIANT dbv;
		if (!DBGetContactSettingTString(NULL, m_proto->m_szModuleName, m_szSetting, &dbv))
		{
			result = dbv.ptszVal;
			DBFreeVariant(&dbv);
		}
		return result;
	}
	__forceinline Type operator= (Type value)
	{
		DBWriteContactSettingTString(NULL, m_proto->m_szModuleName, m_szSetting, value);
		return value;
	}

private:
	Type m_default;
	bool m_crypt;

	CMOption(const CMOption &): CMOptionBase(NULL, NULL, DBVT_DELETED) {}
	void operator= (const CMOption &) {}
};

struct CJabberOptions
{
	CMOption<BYTE> AcceptHttpAuth;
	CMOption<BYTE> AddRoster2Bookmarks;
	CMOption<BYTE> AutoAcceptAuthorization;
	CMOption<BYTE> AutoAcceptMUC;
	CMOption<BYTE> AutoAdd;
	CMOption<BYTE> AutoJoinBookmarks;
	CMOption<BYTE> AutoJoinConferences;
	CMOption<BYTE> AutoJoinHidden;
	CMOption<BYTE> AvatarType;
	CMOption<BYTE> BsDirect;
	CMOption<BYTE> BsDirectManual;
	CMOption<BYTE> BsOnlyIBB;
	CMOption<BYTE> BsProxyManual;
	CMOption<BYTE> Disable3920auth;
	CMOption<BYTE> EnableAvatars;
	CMOption<BYTE> EnableRemoteControl;
	CMOption<BYTE> EnableServerXMPPPing;
	CMOption<BYTE> EnableUserActivity;
	CMOption<BYTE> EnableUserMood;
	CMOption<BYTE> EnableUserTune;
	CMOption<BYTE> EnableZlib;
	CMOption<BYTE> ExtendedSearch;
	CMOption<BYTE> FixIncorrectTimestamps;
	CMOption<BYTE> GcLogAffiliations;
	CMOption<BYTE> GcLogBans;
	CMOption<BYTE> GcLogConfig;
	CMOption<BYTE> GcLogRoles;
	CMOption<BYTE> GcLogStatuses;
	CMOption<BYTE> HostNameAsResource;
	CMOption<BYTE> IgnoreMUCInvites;
	CMOption<BYTE> KeepAlive;
	CMOption<BYTE> LogChatstates;
	CMOption<BYTE> LogPresence;
	CMOption<BYTE> LogPresenceErrors;
	CMOption<BYTE> ManualConnect;
	CMOption<BYTE> MsgAck;
	CMOption<BYTE> RosterSync;
	CMOption<BYTE> SavePassword;
	CMOption<BYTE> ShowForeignResourceInMirVer;
	CMOption<BYTE> ShowOSVersion;
	CMOption<BYTE> ShowTransport;
	CMOption<BYTE> UseSSL;
	CMOption<BYTE> UseTLS;

	CJabberOptions(PROTO_INTERFACE *proto):
		BsDirect(proto, "BsDirect", FALSE),
		AcceptHttpAuth(proto, "AcceptHttpAuth", TRUE),
		AddRoster2Bookmarks(proto, "AddRoster2Bookmarks", TRUE),
		AutoAcceptAuthorization(proto, "AutoAcceptAuthorization", FALSE),
		AutoAcceptMUC(proto, "AutoAcceptMUC", FALSE),
		AutoAdd(proto, "AutoAdd", TRUE),
		AutoJoinBookmarks(proto, "AutoJoinBookmarks", TRUE),
		AutoJoinConferences(proto, "AutoJoinConferences", 0),
		AutoJoinHidden(proto, "AutoJoinHidden", TRUE),
		AvatarType(proto, "AvatarType", PA_FORMAT_UNKNOWN),
		BsDirectManual(proto, "BsDirectManual", FALSE),
		BsOnlyIBB(proto, "BsOnlyIBB", FALSE),
		BsProxyManual(proto, "BsProxyManual", FALSE),
		Disable3920auth(proto, "Disable3920auth", 0),
		EnableAvatars(proto, "EnableAvatars", TRUE),
		EnableRemoteControl(proto, "EnableRemoteControl", FALSE),
		EnableServerXMPPPing(proto, "EnableServerXMPPPing", FALSE),
		EnableUserActivity(proto, "EnableUserActivity", TRUE),
		EnableUserMood(proto, "EnableUserMood", TRUE),
		EnableUserTune(proto, "EnableUserTune", FALSE),
		EnableZlib(proto, "EnableZlib", FALSE),
		ExtendedSearch(proto, "ExtendedSearch", TRUE),
		FixIncorrectTimestamps(proto, "FixIncorrectTimestamps", TRUE),
		GcLogAffiliations(proto, "GcLogAffiliations", FALSE),
		GcLogBans(proto, "GcLogBans", TRUE),
		GcLogConfig(proto, "GcLogConfig", FALSE),
		GcLogRoles(proto, "GcLogRoles", FALSE),
		GcLogStatuses(proto, "GcLogStatuses", FALSE),
		HostNameAsResource(proto, "HostNameAsResource", FALSE),
		IgnoreMUCInvites(proto, "IgnoreMUCInvites", FALSE),
		KeepAlive(proto, "KeepAlive", 1),
		LogChatstates(proto, "LogChatstates", FALSE),
		LogPresence(proto, "LogPresence", TRUE),
		LogPresenceErrors(proto, "LogPresenceErrors", TRUE),
		ManualConnect(proto, "ManualConnect", FALSE),
		MsgAck(proto, "MsgAck", FALSE),
		RosterSync(proto, "RosterSync", FALSE),
		SavePassword(proto, "SavePassword", TRUE),
		ShowForeignResourceInMirVer(proto, "ShowForeignResourceInMirVer", FALSE),
		ShowOSVersion(proto, "ShowOSVersion", TRUE),
		ShowTransport(proto, "ShowTransport", TRUE),
		UseSSL(proto, "UseSSL", FALSE),
		UseTLS(proto, "UseTLS", TRUE)
		{}
/*
JGetByte( hContact, "ChatRoom", 0 )
JGetByte( hContact, "AvatarType", 0 )
JGetByte( hContact, "AvatarXVcard", 0 )
JGetByte( hContact, "ChatRoom", 0)
JGetByte( hContact, "IsTransport", 0 )
JGetByte( hContact, "IsTransported", 0 )
JGetByte( hContact, "MsgAck", TRUE )
*/
};

#endif // __jabber_db_utils_h__

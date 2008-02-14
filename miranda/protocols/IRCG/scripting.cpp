/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-08 George Hazan

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

#include "irc.h"
#include <algorithm>

int __cdecl CIrcProto::Scripting_InsertRawIn(WPARAM wParam,LPARAM lParam)
{
	char* pszRaw = ( char* ) lParam;

	if ( bMbotInstalled && ScriptingEnabled && pszRaw && IsConnected() ) {
		TCHAR* p = mir_a2t( pszRaw );
		InsertIncomingEvent( p );
		mir_free( p );
		return 0;
	}

	return 1;
}
 
int __cdecl CIrcProto::Scripting_InsertRawOut( WPARAM wParam, LPARAM lParam )
{
	char* pszRaw = ( char* ) lParam;
	if ( bMbotInstalled && ScriptingEnabled && pszRaw && IsConnected() ) {	
		String S = pszRaw;
		ReplaceString( S, "%", "%%%%");
		NLSendNoScript((const unsigned char *)S.c_str(), lstrlenA(S.c_str()));
		return 0;
	}

	return 1;
}

int __cdecl CIrcProto::Scripting_InsertGuiIn(WPARAM wParam,LPARAM lParam)
{
	GCEVENT* gce = (GCEVENT *) lParam;
	WPARAM_GUI_IN * wgi = (WPARAM_GUI_IN *) wParam;


	if ( bMbotInstalled && ScriptingEnabled && gce ) {	
		TCHAR* p1 = NULL;
		TString S;
		if ( gce->pDest && gce->pDest->ptszID ) {
			p1 = gce->pDest->ptszID;
			S = MakeWndID(gce->pDest->ptszID);
			gce->pDest->ptszID = ( TCHAR* )S.c_str();
		}
		gce->cbSize = sizeof(GCEVENT);

		CallServiceSync( MS_GC_EVENT, wgi?wgi->wParam:0, (LPARAM)gce);

		if ( p1 )
			gce->pDest->ptszID = p1;
		return 0;
	}

	return 1;
}

//helper functions
static void __stdcall OnHook(void * pi)
{
	GCHOOK* gch = ( GCHOOK* )pi;

	//Service_GCEventHook(1, (LPARAM) gch);

	if(gch->pszUID)
		free(gch->pszUID);
	if(gch->pszText)
		free(gch->pszText);
	if(gch->pDest->ptszID)
		free(gch->pDest->ptszID);
	if(gch->pDest->pszModule)
		free(gch->pDest->pszModule);
	delete gch->pDest;
	delete gch;
}
static void __cdecl GuiOutThread(LPVOID di)
{	
	GCHOOK* gch = ( GCHOOK* )di;
	CallFunctionAsync( OnHook, ( void* )gch );
}

int __cdecl CIrcProto::Scripting_InsertGuiOut( WPARAM wParam,LPARAM lParam )
{
	GCHOOK* gch = ( GCHOOK* )lParam;

	if ( bMbotInstalled && ScriptingEnabled && gch ) {	
		GCHOOK* gchook = new GCHOOK;
		gchook->pDest = new GCDEST;

		gchook->dwData = gch->dwData;
		gchook->pDest->iType = gch->pDest->iType;
		if ( gch->ptszText )
			gchook->ptszText = _tcsdup( gch->ptszText );
		else gchook->pszText = NULL;

		if ( gch->ptszUID )
			gchook->ptszUID = _tcsdup( gch->ptszUID );
		else gchook->pszUID = NULL;

		if ( gch->pDest->ptszID ) {
			TString S = MakeWndID( gch->pDest->ptszID );
			gchook->pDest->ptszID = _tcsdup( S.c_str());
		}
		else gchook->pDest->ptszID = NULL;

		if ( gch->pDest->pszModule )
			gchook->pDest->pszModule = _strdup(gch->pDest->pszModule);
		else gchook->pDest->pszModule = NULL;

		mir_forkthread( GuiOutThread, gchook );
		return 0;
	}

	return 1;
}

BOOL CIrcProto::Scripting_TriggerMSPRawIn( char** pszRaw )
{
	int iVal = CallService( MS_MBOT_IRC_RAW_IN, (WPARAM)m_szModuleName, (LPARAM)pszRaw);
	if ( iVal == 0 )
		return TRUE;

	return iVal > 0 ? FALSE : TRUE;
}

BOOL CIrcProto::Scripting_TriggerMSPRawOut(char ** pszRaw)
{
	int iVal =  CallService( MS_MBOT_IRC_RAW_OUT, (WPARAM)m_szModuleName, (LPARAM)pszRaw);
	if ( iVal == 0 )
		return TRUE;

	return iVal > 0 ? FALSE : TRUE;
}

BOOL CIrcProto::Scripting_TriggerMSPGuiIn(WPARAM * wparam, GCEVENT * gce)
{
	WPARAM_GUI_IN wgi = {0};

	wgi.pszModule = m_szModuleName;
	wgi.wParam = *wparam;
	if (gce->time == 0)
		gce->time = time(0);

	int iVal =  CallService( MS_MBOT_IRC_GUI_IN, (WPARAM)&wgi, (LPARAM)gce);
	if ( iVal == 0 ) {
		*wparam = wgi.wParam;
		return TRUE;
	}

	return iVal > 0 ? FALSE : TRUE;
}

BOOL CIrcProto::Scripting_TriggerMSPGuiOut(GCHOOK* gch)
{
	int iVal =  CallService( MS_MBOT_IRC_GUI_OUT, (WPARAM)m_szModuleName, (LPARAM)gch);
	if ( iVal == 0 )
		return TRUE;

	return iVal > 0 ? FALSE : TRUE;
}

int __cdecl CIrcProto::Scripting_GetIrcData(WPARAM wparam, LPARAM lparam)
{
	if ( bMbotInstalled && ScriptingEnabled && lparam ) {
		String sString = ( char* ) lparam, sRequest;
		TString sOutput, sChannel; 

		int i = sString.find("|",0);
		if ( i != string::npos ) {
			sRequest = sString.substr(0, i);
			TCHAR* p = mir_a2t(( char* )sString.substr(i+1, sString.length()).c_str());
			sChannel = p;
			mir_free( p );
		}
		else sRequest = sString;

		transform (sRequest.begin(),sRequest.end(), sRequest.begin(), tolower);

		if (sRequest == "ownnick" && IsConnected())
			sOutput = GetInfo().sNick;

		else if (sRequest == "network" && IsConnected())
			sOutput = GetInfo().sNetwork;

		else if (sRequest == "primarynick")
			sOutput = Nick;

		else if (sRequest == "secondarynick")
			sOutput = AlternativeNick;

		else if (sRequest == "myip")
			return ( int )mir_strdup( ManualHost ? MySpecifiedHostIP : 
										( IPFromServer ) ? MyHost : MyLocalHost);

		else if (sRequest == "usercount" && !sChannel.empty()) {
			TString S = MakeWndID(sChannel.c_str());
			GC_INFO gci = {0};
			gci.Flags = BYID|COUNT;
			gci.pszModule = m_szModuleName;
			gci.pszID = (TCHAR*)S.c_str();
			if ( !CallServiceSync( MS_GC_GETINFO, 0, (LPARAM)&gci )) {
				TCHAR szTemp[40];
				_sntprintf( szTemp, 35, _T("%u"), gci.iCount);
				sOutput = szTemp;
			}
		}
		else if (sRequest == "userlist" && !sChannel.empty()) {
			TString S = MakeWndID(sChannel.c_str());
			GC_INFO gci = {0};
			gci.Flags = BYID|USERS;
			gci.pszModule = m_szModuleName;
			gci.pszID = ( TCHAR* )S.c_str();
			if ( !CallServiceSync( MS_GC_GETINFO, 0, (LPARAM)&gci ))
				return (int)mir_strdup( gci.pszUsers );
		}
		else if (sRequest == "channellist") {
			TString S = _T("");
			int i = CallServiceSync( MS_GC_GETSESSIONCOUNT, 0, (LPARAM)m_szModuleName);
			if ( i >= 0 ) {
				int j = 0;
				while (j < i) {
					GC_INFO gci = {0};
					gci.Flags = BYINDEX|ID;
					gci.pszModule = m_szModuleName;
					gci.iItem = j;
					if ( !CallServiceSync( MS_GC_GETINFO, 0, ( LPARAM )&gci )) {
						if ( lstrcmpi( gci.pszID, SERVERWINDOW)) {
							TString S1 = gci.pszID;
							int k = S1.find(_T(" "), 0);
							if ( k != string::npos )
								S1 = S1.substr(0, k);
							S += S1;
							S += _T(" ");
					}	}
					j++;
			}	}
			
			if ( !S.empty() )
				sOutput = ( TCHAR* )S.c_str();
		}
		// send it to mbot
		if ( !sOutput.empty())
			return ( int )mir_t2a( sOutput.c_str() );
	}
	return 0;
}

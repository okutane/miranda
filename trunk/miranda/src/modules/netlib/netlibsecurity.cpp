/*
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

#include "commonheaders.h"
#include "netlib.h"

#define SECURITY_WIN32
#include <security.h>
#include <rpcdce.h>

static HMODULE g_hSecurity = NULL;
static PSecurityFunctionTable g_pSSPI = NULL;

typedef struct
{
	CtxtHandle hClientContext;
	CredHandle hClientCredential;
	TCHAR* szProvider;
	TCHAR* szPrincipal;
	unsigned cbMaxToken;
	bool hasDomain;
}
	NtlmHandleType;

typedef struct
{
	WORD     len;
	WORD     allocedSpace;
	DWORD    offset;
}
	NTLM_String;

typedef struct
{
	char        sign[8];
	DWORD       type;   // == 2
	NTLM_String targetName;
	DWORD       flags;
	BYTE        challenge[8];
	BYTE        context[8];
	NTLM_String targetInfo;
}
	NtlmType2packet;

static unsigned secCnt = 0, ntlmCnt = 0;
static HANDLE hSecMutex;

static void ReportSecError(SECURITY_STATUS scRet, int line)
{
	char szMsgBuf[256];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, scRet, LANG_USER_DEFAULT, szMsgBuf, SIZEOF(szMsgBuf), NULL);

	char *p = strchr(szMsgBuf, 13); if (p) *p = 0;

	NetlibLogf(NULL, "Security error 0x%x on line %u (%s)", scRet, line, szMsgBuf);
}

static void LoadSecurityLibrary(void)
{
	INIT_SECURITY_INTERFACE pInitSecurityInterface;

	g_hSecurity = LoadLibraryA("secur32.dll");
	if (g_hSecurity == NULL)
		g_hSecurity = LoadLibraryA("security.dll");

	if (g_hSecurity == NULL)
		return;

	pInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress(g_hSecurity, SECURITY_ENTRYPOINT_ANSI);
	if (pInitSecurityInterface != NULL)
	{
		g_pSSPI = pInitSecurityInterface();
	}

	if (g_pSSPI == NULL) 
	{
		FreeLibrary(g_hSecurity);
		g_hSecurity = NULL;
	}
}

static void FreeSecurityLibrary(void)
{
	FreeLibrary(g_hSecurity);
	g_hSecurity = NULL;
	g_pSSPI = NULL;
}

HANDLE NetlibInitSecurityProvider(const TCHAR* szProvider, const TCHAR* szPrincipal)
{
	HANDLE hSecurity = NULL;

	if (_tcsicmp(szProvider, _T("Basic")) == 0)
	{
		NtlmHandleType* hNtlm = (NtlmHandleType*)mir_calloc(sizeof(NtlmHandleType));
		hNtlm->szProvider = mir_tstrdup(szProvider);
		SecInvalidateHandle(&hNtlm->hClientContext);
		SecInvalidateHandle(&hNtlm->hClientCredential);
		ntlmCnt++;

		return hNtlm;
	}

	WaitForSingleObject(hSecMutex, INFINITE);

	if (secCnt == 0 ) 
	{
		LoadSecurityLibrary();
		secCnt += g_hSecurity != NULL;
	}
	else secCnt++;

	if (g_pSSPI != NULL) 
	{
		PSecPkgInfo ntlmSecurityPackageInfo;
		SECURITY_STATUS sc = g_pSSPI->QuerySecurityPackageInfo((LPTSTR)szProvider, &ntlmSecurityPackageInfo);
		if (sc == SEC_E_OK)
		{
			NtlmHandleType* hNtlm;

			hSecurity = hNtlm = (NtlmHandleType*)mir_calloc(sizeof(NtlmHandleType));
			hNtlm->cbMaxToken = ntlmSecurityPackageInfo->cbMaxToken;
			g_pSSPI->FreeContextBuffer(ntlmSecurityPackageInfo);

			hNtlm->szProvider = mir_tstrdup(szProvider);
			hNtlm->szPrincipal = mir_tstrdup(szPrincipal ? szPrincipal : _T(""));
			SecInvalidateHandle(&hNtlm->hClientContext);
			SecInvalidateHandle(&hNtlm->hClientCredential);
			ntlmCnt++;
		}
	}

	ReleaseMutex(hSecMutex);
	return hSecurity;
}

#ifdef UNICODE
HANDLE NetlibInitSecurityProvider(const char* szProvider, const char* szPrincipal)
{
	return NetlibInitSecurityProvider(StrConvT(szProvider), StrConvT(szPrincipal));
}
#endif

void NetlibDestroySecurityProvider(HANDLE hSecurity)
{
	if (hSecurity == NULL) return;

	WaitForSingleObject(hSecMutex, INFINITE);

	if (ntlmCnt != 0) 
	{
		NtlmHandleType* hNtlm = (NtlmHandleType*)hSecurity;
		if (SecIsValidHandle(&hNtlm->hClientContext)) g_pSSPI->DeleteSecurityContext(&hNtlm->hClientContext);
		if (SecIsValidHandle(&hNtlm->hClientCredential)) g_pSSPI->FreeCredentialsHandle(&hNtlm->hClientCredential);
		mir_free(hNtlm->szProvider);
		mir_free(hNtlm->szPrincipal);

		--ntlmCnt;

		mir_free(hNtlm);
	}

	if (secCnt && --secCnt == 0)
		FreeSecurityLibrary();

	ReleaseMutex(hSecMutex);
}

char* NtlmCreateResponseFromChallenge(HANDLE hSecurity, const char *szChallenge, const TCHAR* login, const TCHAR* psw, 
									  bool http, unsigned& complete)
{
	SECURITY_STATUS sc;
	SecBufferDesc outputBufferDescriptor,inputBufferDescriptor;
	SecBuffer outputSecurityToken,inputSecurityToken;
	TimeStamp tokenExpiration;
	ULONG contextAttributes;
	NETLIBBASE64 nlb64 = { 0 };

	NtlmHandleType* hNtlm = (NtlmHandleType*)hSecurity;

	if (hSecurity == NULL || ntlmCnt == 0) return NULL;

 	if (_tcsicmp(hNtlm->szProvider, _T("Basic")))
	{
		bool isKerberos = _tcsicmp(hNtlm->szProvider, _T("Kerberos")) == 0;
		bool hasChallenge = szChallenge != NULL && szChallenge[0] != '\0';
		if (hasChallenge) 
		{
			nlb64.cchEncoded = lstrlenA(szChallenge);
			nlb64.pszEncoded = (char*)szChallenge;
			nlb64.cbDecoded = Netlib_GetBase64DecodedBufferSize(nlb64.cchEncoded);
			nlb64.pbDecoded = (PBYTE)alloca(nlb64.cbDecoded);
			if (!NetlibBase64Decode(0, (LPARAM)&nlb64)) return NULL;

			inputBufferDescriptor.cBuffers=1;
			inputBufferDescriptor.pBuffers=&inputSecurityToken;
			inputBufferDescriptor.ulVersion=SECBUFFER_VERSION;
			inputSecurityToken.BufferType=SECBUFFER_TOKEN;
			inputSecurityToken.cbBuffer=nlb64.cbDecoded;
			inputSecurityToken.pvBuffer=nlb64.pbDecoded;

			// try to decode the domain name from the NTLM challenge
			if (login != NULL && login[0] != '\0' && !hNtlm->hasDomain) 
			{
				NtlmType2packet* pkt = ( NtlmType2packet* )nlb64.pbDecoded;
				if (!strncmp(pkt->sign, "NTLMSSP", 8) && pkt->type == 2) 
				{
#ifdef UNICODE
					wchar_t* domainName = (wchar_t*)&nlb64.pbDecoded[pkt->targetName.offset];
					int domainLen = pkt->targetName.len;

					// Negotiate ANSI? if yes, convert the ANSI name to unicode
					if ((pkt->flags & 1) == 0) 
					{
						int bufsz = MultiByteToWideChar(CP_ACP, 0, (char*)domainName, domainLen, NULL, 0);
						wchar_t* buf = (wchar_t*)alloca(bufsz * sizeof(wchar_t));
						domainLen = MultiByteToWideChar(CP_ACP, 0, (char*)domainName, domainLen, buf, bufsz) - 1;
						domainName = buf;
					}
					else
						domainLen /= sizeof(wchar_t);
#else
					char* domainName = (char*)&nlb64.pbDecoded[pkt->targetName.offset];
					int domainLen = pkt->targetName.len;

					// Negotiate Unicode? if yes, convert the unicode name to ANSI
					if (pkt->flags & 1) 
					{
						int bufsz = WideCharToMultiByte(CP_ACP, 0, (WCHAR*)domainName, domainLen, NULL, 0, NULL, NULL);
						char* buf = (char*)alloca(bufsz);
						domainLen = WideCharToMultiByte(CP_ACP, 0, (WCHAR*)domainName, domainLen, buf, bufsz, NULL, NULL) - 1;
						domainName = buf;
					}
#endif

					if (domainLen) 
					{
						size_t newLoginLen = _tcslen(login) + domainLen + 1;
						TCHAR *newLogin = (TCHAR*)alloca(newLoginLen * sizeof(TCHAR));

						_tcsncpy(newLogin, domainName, domainLen);
						newLogin[domainLen] = '\\';
						_tcscpy(newLogin + domainLen + 1, login);

						char* szChl = NtlmCreateResponseFromChallenge(hSecurity, NULL, newLogin, psw, http, complete);
						mir_free(szChl);
					}
				}
			}
		}
		else 
		{
			if (SecIsValidHandle(&hNtlm->hClientContext)) g_pSSPI->DeleteSecurityContext(&hNtlm->hClientContext);
			if (SecIsValidHandle(&hNtlm->hClientCredential)) g_pSSPI->FreeCredentialsHandle(&hNtlm->hClientCredential);

			SEC_WINNT_AUTH_IDENTITY auth;

			if (login != NULL && login[0] != '\0') 
			{
				memset(&auth, 0, sizeof(auth));
#ifdef _UNICODE
				NetlibLogf(NULL, "Security login requested, user: %S pssw: %s", login, psw ? "(exist)" : "(no psw)");
#else
				NetlibLogf(NULL, "Security login requested, user: %s pssw: %s", login, psw ? "(exist)" : "(no psw)");
#endif

				const TCHAR* loginName = login;
				const TCHAR* domainName = _tcschr(login, '\\');
				int domainLen = 0;
				int loginLen = lstrlen(loginName);
				if (domainName != NULL) 
				{
					loginName = domainName + 1;
					loginLen = lstrlen(loginName);
					domainLen = domainName - login;
					domainName = login;
				}
				else if ((domainName = _tcschr(login, '@')) != NULL) 
				{
					loginName = login;
					loginLen = domainName - login;
					domainLen = lstrlen(++domainName);
				}

#ifdef UNICODE
				auth.User = (PWORD)loginName;
				auth.UserLength = loginLen;
				auth.Password = (PWORD)psw;
				auth.PasswordLength = lstrlen(psw);
				auth.Domain = (PWORD)domainName;
				auth.DomainLength = domainLen;
				auth.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
				auth.User = (PBYTE)loginName;
				auth.UserLength = loginLen;
				auth.Password = (PBYTE)psw;
				auth.PasswordLength = lstrlen(psw);
				auth.Domain = (PBYTE)domainName;
				auth.DomainLength = domainLen;
				auth.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif

				hNtlm->hasDomain = domainLen != 0;
			}

			sc = g_pSSPI->AcquireCredentialsHandle(NULL, hNtlm->szProvider, 
				SECPKG_CRED_OUTBOUND, NULL, hNtlm->hasDomain ? &auth : NULL, NULL, NULL, 
				&hNtlm->hClientCredential, &tokenExpiration);
			if (sc != SEC_E_OK) 
			{
				ReportSecError(sc, __LINE__);
				return NULL;
			}
		}

		outputBufferDescriptor.cBuffers = 1;
		outputBufferDescriptor.pBuffers = &outputSecurityToken;
		outputBufferDescriptor.ulVersion = SECBUFFER_VERSION;
		outputSecurityToken.BufferType = SECBUFFER_TOKEN;
		outputSecurityToken.cbBuffer = hNtlm->cbMaxToken;
		outputSecurityToken.pvBuffer = alloca(outputSecurityToken.cbBuffer);

		sc = g_pSSPI->InitializeSecurityContext(&hNtlm->hClientCredential,
			hasChallenge ? &hNtlm->hClientContext : NULL,
			hNtlm->szPrincipal, isKerberos ? ISC_REQ_MUTUAL_AUTH | ISC_REQ_STREAM : 0, 0, SECURITY_NATIVE_DREP,
			hasChallenge ? &inputBufferDescriptor : NULL, 0, &hNtlm->hClientContext,
			&outputBufferDescriptor, &contextAttributes, &tokenExpiration);

		complete = (sc != SEC_I_COMPLETE_AND_CONTINUE && sc != SEC_I_CONTINUE_NEEDED);

		if (sc == SEC_I_COMPLETE_NEEDED || sc == SEC_I_COMPLETE_AND_CONTINUE)
		{
			sc = g_pSSPI->CompleteAuthToken(&hNtlm->hClientContext, &outputBufferDescriptor);
		}

		if (sc != SEC_E_OK && sc != SEC_I_CONTINUE_NEEDED)
		{
			ReportSecError(sc, __LINE__);
			return NULL;
		}

		nlb64.cbDecoded = outputSecurityToken.cbBuffer;
		nlb64.pbDecoded = (PBYTE)outputSecurityToken.pvBuffer;
	}
	else
	{
		if (!login || !psw) return NULL;

		char *szLogin = mir_t2a(login);
		char *szPassw = mir_t2a(psw);

		size_t authLen = strlen(szLogin) + strlen(szPassw) + 5;
		char *szAuth = (char*)alloca(authLen);
		
		nlb64.cbDecoded = mir_snprintf(szAuth, authLen,"%s:%s", szLogin, szPassw);
		nlb64.pbDecoded=(PBYTE)szAuth;
		complete = true;

		mir_free(szPassw);
		mir_free(szLogin);
	}

	nlb64.cchEncoded = Netlib_GetBase64EncodedBufferSize(nlb64.cbDecoded);
	nlb64.pszEncoded = (char*)alloca(nlb64.cchEncoded);
	if (!NetlibBase64Encode(0,(LPARAM)&nlb64)) return NULL;

	char* result;
	if (http)
	{
		char* szProvider = mir_t2a(hNtlm->szProvider);
		nlb64.cchEncoded += (int)strlen(szProvider) + 10;
		result = (char*)mir_alloc(nlb64.cchEncoded);
		mir_snprintf(result, nlb64.cchEncoded, "%s %s", szProvider, nlb64.pszEncoded);
		mir_free(szProvider);
	}
	else
		result = mir_strdup(nlb64.pszEncoded);

	return result;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR InitSecurityProviderService(WPARAM, LPARAM lParam)
{
	HANDLE hSecurity = NetlibInitSecurityProvider((char*)lParam, NULL);
	return (INT_PTR)hSecurity;
}

static INT_PTR InitSecurityProviderService2(WPARAM, LPARAM lParam)
{
	NETLIBNTLMINIT2 *req = ( NETLIBNTLMINIT2* )lParam;
	if (req->cbSize < sizeof(*req)) return 0;

	HANDLE hSecurity;

#ifdef UNICODE
	if (req->flags & NNR_UNICODE)
		hSecurity = NetlibInitSecurityProvider(req->szProviderName, req->szPrincipal);
	else
#endif
		hSecurity = NetlibInitSecurityProvider((char*)req->szProviderName, (char*)req->szPrincipal);

	return (INT_PTR)hSecurity;
}

static INT_PTR DestroySecurityProviderService( WPARAM, LPARAM lParam )
{
	NetlibDestroySecurityProvider(( HANDLE )lParam );
	return 0;
}

static INT_PTR NtlmCreateResponseService( WPARAM wParam, LPARAM lParam )
{
	NETLIBNTLMREQUEST* req = ( NETLIBNTLMREQUEST* )lParam;
	unsigned complete;

	char* response = NtlmCreateResponseFromChallenge(( HANDLE )wParam, req->szChallenge, 
		StrConvT(req->userName), StrConvT(req->password), false, complete );

	return (INT_PTR)response;
}

static INT_PTR NtlmCreateResponseService2( WPARAM wParam, LPARAM lParam )
{
	NETLIBNTLMREQUEST2* req = ( NETLIBNTLMREQUEST2* )lParam;
	if (req->cbSize < sizeof(*req)) return 0;

	char* response;

#ifdef UNICODE
	if (req->flags & NNR_UNICODE)
	{
		response = NtlmCreateResponseFromChallenge(( HANDLE )wParam, req->szChallenge, 
			req->szUserName, req->szPassword, false, req->complete );
	}
	else
	{
		TCHAR *szLogin = mir_a2t((char*)req->szUserName);
		TCHAR *szPassw = mir_a2t((char*)req->szPassword);
		response = NtlmCreateResponseFromChallenge(( HANDLE )wParam, req->szChallenge, 
			szLogin, szPassw, false, req->complete );
		mir_free(szLogin);
		mir_free(szPassw);
	}
#else
	response = NtlmCreateResponseFromChallenge(( HANDLE )wParam, req->szChallenge, 
		req->szUserName, req->szPassword, false, req->complete );
#endif

	return (INT_PTR)response;
}

void NetlibSecurityInit(void)
{
	hSecMutex = CreateMutex(NULL, FALSE, NULL);

	CreateServiceFunction( MS_NETLIB_INITSECURITYPROVIDER, InitSecurityProviderService );
	CreateServiceFunction( MS_NETLIB_INITSECURITYPROVIDER2, InitSecurityProviderService2 );
	CreateServiceFunction( MS_NETLIB_DESTROYSECURITYPROVIDER, DestroySecurityProviderService );
	CreateServiceFunction( MS_NETLIB_NTLMCREATERESPONSE, NtlmCreateResponseService );
	CreateServiceFunction( MS_NETLIB_NTLMCREATERESPONSE2, NtlmCreateResponseService2 );
}

void NetlibSecurityDestroy(void)
{
	CloseHandle(hSecMutex);
}
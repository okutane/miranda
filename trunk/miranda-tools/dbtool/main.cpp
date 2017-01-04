/*
Miranda Database Tool
Copyright 2000-2017 Miranda IM project, 
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
#include "dbtool.h"

HINSTANCE hInst = NULL;

DbToolOptions opts = {0};

const static struct {
	HKEY hRoot;
	LPCTSTR szKey;
	LPCTSTR szName;
} MirandaPaths[] = {
		{ HKEY_LOCAL_MACHINE, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Miranda IM" ), _T( "InstallLocation" ) },
		{ HKEY_LOCAL_MACHINE, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\miranda64.exe" ), _T( "Path" ) },
		{ HKEY_LOCAL_MACHINE, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\miranda32.exe" ), _T( "Path" ) },
		{ NULL, NULL, NULL }
};

void AddPathList(PathList** ppList, LPCTSTR szPath)
{
	if ( ! ppList || ! szPath || ! *szPath )
		return;

	PathList* pLast = *ppList;
	while ( pLast ) {
		if ( _tcsicmp( pLast->szPath, szPath ) == 0 )
			return;
		if ( ! pLast->pNext )
			break;
		pLast = pLast->pNext;
	}

	PathList* pNew = new PathList;
	pNew->pNext = NULL;
	_tcscpy_s( pNew->szPath, szPath );

	size_t nLen = _tcslen( pNew->szPath );
	if ( pNew->szPath[ nLen - 1 ] == _T( '\\' ) )
		pNew->szPath[ --nLen ] = 0;

	if ( pLast ) {
		pLast->pNext = pNew;
		pLast = pNew;
	}
	else {
		*ppList = pNew;
	}
}

void FreePathList(PathList* pList)
{
	while ( pList ) {
		PathList* pFree = pList;
		pList = pList->pNext;
		delete pFree;
	}
}

bool IsMirandaPath(LPCTSTR szPath)
{
	if ( ! szPath || ! *szPath )
		return false;

	TCHAR szTest[ MAX_PATH ];
	_tcscpy_s( szTest, szPath );

	size_t nLen = _tcslen( szTest );
	if ( szTest[ nLen - 1 ] == _T('\\') )
		szTest[ --nLen ] = 0;

	_tcscpy_s( szTest + nLen, SIZEOF(szTest) - nLen, _T( "\\miranda64.exe" ) );
	if ( GetFileAttributes( szTest ) != INVALID_FILE_ATTRIBUTES )
		return true;

	_tcscpy_s( szTest + nLen, SIZEOF(szTest) - nLen, _T( "\\miranda32.exe" ) );
	if ( GetFileAttributes( szTest ) != INVALID_FILE_ATTRIBUTES )
		return true;

	return false;
}

PathList* GetMirandaPath()
{
	TCHAR szPath[ MAX_PATH ];
	PathList* pList = NULL;

	// Method 1 - current folder
	GetCurrentDirectory( SIZEOF( szPath ), szPath );
	if ( IsMirandaPath( szPath ) )
		AddPathList( &pList, szPath );

	// Method 2 - dbtool's folder
	GetModuleFileName( NULL, szPath, SIZEOF( szPath ) );
	TCHAR* str = _tcsrchr( szPath, _T('\\') );
	if ( str != NULL )
		*str = 0;
	if ( IsMirandaPath( szPath ) )
		AddPathList( &pList, szPath );

	// Method 3 - registry locations
	for ( int i = 0; MirandaPaths[ i ].szKey; ++i ) {
		for ( int bits = 0; bits < 2; ++bits ) {
			HKEY hKey;
			if ( RegOpenKeyEx( MirandaPaths[ i ].hRoot, MirandaPaths[ i ].szKey, 0, KEY_QUERY_VALUE | ( bits ? KEY_WOW64_32KEY : KEY_WOW64_64KEY ), &hKey ) == ERROR_SUCCESS ) {
				DWORD cbData = SIZEOF( szPath );
				if ( RegQueryValueEx( hKey, MirandaPaths[ i ].szName, NULL, NULL, (PBYTE)szPath, &cbData ) == ERROR_SUCCESS ) {
					if ( IsMirandaPath( szPath ) )
						AddPathList( &pList, szPath );
				}
				RegCloseKey( hKey );
			}
		}
	}

	// Method 4 - get well-known Miranda IM folder
	if ( SHGetSpecialFolderPath( NULL, szPath, CSIDL_PROGRAM_FILES, FALSE ) ) {
		_tcscat_s( szPath, _T( "\\Miranda IM" ) );
		if ( IsMirandaPath( szPath ) )
			AddPathList( &pList, szPath );
	}

	return pList;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	hInst=hInstance;
	LoadLangPackModule();
	InitCommonControls();
	DialogBox(hInst,MAKEINTRESOURCE(IDD_WIZARD),NULL,WizardDlgProc);
	return 0;
}

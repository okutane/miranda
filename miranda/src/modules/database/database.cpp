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
#include "profilemanager.h"
#include "../srfile/file.h"

// from the plugin loader, hate extern but the db frontend is pretty much tied
extern PLUGINLINK pluginCoreLink;
// contains the location of mirandaboot.ini
extern TCHAR mirandabootini[MAX_PATH];
bool dbCreated;
TCHAR g_profileDir[MAX_PATH];

const TCHAR tszMoveMsg[] =
	_T("Your database is located in Miranda root folder.\n")
	_T("Even though your profile folder is %s\n")
	_T("Profiles are not allowed in the Miranda root folder.\n")
	_T("Please move Miranda database into the current profile folder.");


bool fileExist(TCHAR* fname)
{
	if (fname[0] == 0) return false;

	FILE* fp = _tfopen(fname, _T("r+"));
	bool res = fp != NULL;
	if (res) fclose(fp);
	return res;
}

int validateProfileDir(TCHAR* profiledir)
{
	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof(dat);
	dat.dwFlags = RVF_TCHAR;

	TCHAR* pfd = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)_T("%miranda_path%\\"), (LPARAM)&dat);

	int res = lstrcmpi(profiledir, pfd) == 0;
	if (res)
	{
		MessageBox(NULL, 
			_T("Profile cannot be placed into Miranda root folder.\n")
			_T("Please move Miranda profile to some other location."),
			_T("Miranda IM"), MB_ICONERROR | MB_OK);
	}
	mir_free(pfd);

	return res;
}

// returns 1 if the profile path was returned, without trailing slash
int getProfilePath(TCHAR * buf, size_t cch)
{
	TCHAR profiledir[MAX_PATH];
	GetPrivateProfileString(_T("Database"), _T("ProfileDir"), _T(""), profiledir, SIZEOF(profiledir), mirandabootini);

	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof( dat );
	dat.dwFlags = RVF_TCHAR;

	if (profiledir[0] == 0)
		_tcscpy(profiledir, _T("%miranda_path%\\Profiles"));

	TCHAR* exprofiledir = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)profiledir, (LPARAM)&dat);
	size_t len = pathToAbsoluteT(exprofiledir, buf, NULL);
	mir_free(exprofiledir);

	if (len < (cch-1)) _tcscat(buf, _T("\\"));
	return 0;
}

// returns 1 if *.dat spec is matched
int isValidProfileName(const TCHAR *name)
{
	size_t len = _tcslen(name) - 4;
	return len > 0 && _tcsicmp(&name[len], _T(".dat")) == 0;
}

// returns 1 if the profile manager should be shown
static bool showProfileManager(void)
{
	TCHAR Mgr[32];
	// is control pressed?
	if (GetAsyncKeyState(VK_CONTROL)&0x8000)
		return 1;

	// wanna show it?
	GetPrivateProfileString(_T("Database"), _T("ShowProfileMgr"), _T("never"), Mgr, SIZEOF(Mgr), mirandabootini);
	return ( _tcsicmp(Mgr, _T("yes")) == 0 );
}

bool shouldAutoCreate(TCHAR *szProfile)
{
	if (szProfile[0] == 0)
		return false;

	TCHAR ac[32];
	GetPrivateProfileString(_T("Database"), _T("AutoCreate"), _T(""), ac, SIZEOF(ac), mirandabootini);
	return _tcsicmp(ac, _T("yes")) == 0;
}

static void getDefaultProfile(TCHAR * szProfile, size_t cch, TCHAR * profiledir)
{
	TCHAR defaultProfile[MAX_PATH];
	GetPrivateProfileString(_T("Database"), _T("DefaultProfile"), _T(""), defaultProfile, SIZEOF(defaultProfile), mirandabootini);

	if (defaultProfile[0] == 0)
		return;

	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof(dat);
	dat.dwFlags = RVF_TCHAR;

	TCHAR* res = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)defaultProfile, (LPARAM)&dat);
	if (res) {
		mir_sntprintf(szProfile, cch, _T("%s%s\\%s%s"), profiledir, res, res, isValidProfileName(res) ? _T("") : _T(".dat"));
		mir_free(res);
	}
	else szProfile[0] = 0;
}

// returns 1 if something that looks like a profile is there
static int getProfileCmdLineArgs(TCHAR * szProfile, size_t cch)
{
	TCHAR *szCmdLine=GetCommandLine();
	TCHAR *szEndOfParam;
	TCHAR szThisParam[1024];
	int firstParam=1;

	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof(dat);
	dat.dwFlags = RVF_TCHAR;

	while(szCmdLine[0]) {
		if(szCmdLine[0]=='"') {
			szEndOfParam=_tcschr(szCmdLine+1,'"');
			if(szEndOfParam==NULL) break;
			lstrcpyn(szThisParam,szCmdLine+1,min( SIZEOF(szThisParam),szEndOfParam-szCmdLine));
			szCmdLine=szEndOfParam+1;
		}
		else {
			szEndOfParam=szCmdLine+_tcscspn(szCmdLine,_T(" \t"));
			lstrcpyn(szThisParam,szCmdLine,min( SIZEOF(szThisParam),szEndOfParam-szCmdLine+1));
			szCmdLine=szEndOfParam;
		}
		while(*szCmdLine && *szCmdLine<=' ') szCmdLine++;
		if(firstParam) {firstParam=0; continue;}   //first param is executable name
		if(szThisParam[0]=='/' || szThisParam[0]=='-') continue;  //no switches supported

		TCHAR* res = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)szThisParam, (LPARAM)&dat);
		if (res == NULL) return 0;
		_tcsncpy(szProfile, res, cch); szProfile[cch-1] = 0;
		mir_free(res);
		return 1;
	}
	return 0;
}

void getProfileCmdLine(TCHAR * szProfile, size_t cch, TCHAR * profiledir)
{
	TCHAR buf[MAX_PATH];
	if (getProfileCmdLineArgs(buf, SIZEOF(buf))) {
		// have something that looks like a .dat, with or without .dat in the filename
		TCHAR newProfileDir[MAX_PATH];
		mir_sntprintf(newProfileDir, cch, _T("%s%s\\"), profiledir, buf);
		pathToAbsoluteT(buf, szProfile, newProfileDir); 
		if (!isValidProfileName(szProfile))
			_tcscat(szProfile, _T(".dat"));

		TCHAR *p = _tcsrchr(buf, '\\');
		if (p) {
			_tcscpy(profiledir, szProfile);
			p = _tcsrchr(profiledir, '\\');
			*(p+1) = 0;
		}	
	}
}

// move profile from profile subdir
static void moveProfileDirProfiles(TCHAR * profiledir, BOOL isRootDir = TRUE)
{
	TCHAR pfd[MAX_PATH];
	if (isRootDir) {
		REPLACEVARSDATA dat = {0};
		dat.cbSize = sizeof(dat);
		dat.dwFlags = RVF_TCHAR;
		TCHAR *path = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)_T("%miranda_path%\\*.dat"), (LPARAM)&dat);
		mir_sntprintf(pfd, SIZEOF(pfd), _T("%s"), path);
		mir_free(path);
	}
	else
		mir_sntprintf(pfd, SIZEOF(pfd), _T("%s\\*.dat"), profiledir);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(pfd, &ffd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			TCHAR path[MAX_PATH], path2[MAX_PATH];
			TCHAR* profile = mir_tstrdup(ffd.cFileName);
			TCHAR *c =_tcsrchr(profile, '.'); if (c) *c = 0;
			mir_sntprintf(path, SIZEOF(path), _T("%s\\%s"), pfd, ffd.cFileName); 
			mir_sntprintf(path2, SIZEOF(path2), _T("%s%s"), profiledir, profile);
			CreateDirectory(path2, NULL);
			mir_sntprintf(path2, SIZEOF(path2), _T("%s%s\\%s"), profiledir, profile, ffd.cFileName); 
			if (MoveFile(path, path2) == 0) {
				TCHAR buf[512];
				mir_sntprintf(buf, SIZEOF(buf), TranslateTS(tszMoveMsg), profiledir);
				MessageBox(NULL, buf, _T("Miranda IM"), MB_ICONERROR | MB_OK);
				break;
			}
			mir_free(profile);
		}
		while(FindNextFile(hFind, &ffd));
	}
	FindClose(hFind);
}

// returns 1 if a single profile (full path) is found within the profile dir
static int getProfile1(TCHAR * szProfile, size_t cch, TCHAR * profiledir, BOOL * noProfiles)
{
	unsigned int found = 0;

	moveProfileDirProfiles(profiledir);
	moveProfileDirProfiles(profiledir, FALSE);

	bool nodprof = szProfile[0] == 0;
	bool reqfd = !nodprof && (_taccess(szProfile, 0) == 0 || shouldAutoCreate(szProfile));
	bool shpm = showProfileManager();

	if (reqfd)
		found++;

	if (shpm || !reqfd) {
		TCHAR searchspec[MAX_PATH];
		mir_sntprintf(searchspec, SIZEOF(searchspec), _T("%s*.*"), profiledir);

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(searchspec, &ffd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				// make sure the first hit is actually a *.dat file
				if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && _tcscmp(ffd.cFileName, _T(".")) && _tcscmp(ffd.cFileName, _T("..")))  {
					TCHAR newProfile[MAX_PATH];
					mir_sntprintf(newProfile, MAX_PATH, _T("%s%s\\%s.dat"), profiledir, ffd.cFileName, ffd.cFileName);
					if (_taccess(newProfile, 0) == 0)
						if (++found == 1 && nodprof)
							_tcscpy(szProfile, newProfile);
				}
			}
				while (FindNextFile(hFind, &ffd));

			FindClose(hFind);
		}
		reqfd = !shpm && found == 1 && nodprof;
	}

	if ( noProfiles ) 
		*noProfiles = found == 0;

	if ( nodprof && !reqfd )
		szProfile[0] = 0;

	return reqfd;
}

// returns 1 if a default profile should be selected instead of showing the manager.
static int getProfileAutoRun(TCHAR * szProfile)
{
	TCHAR Mgr[32];
	GetPrivateProfileString(_T("Database"), _T("ShowProfileMgr"), _T(""), Mgr, SIZEOF(Mgr), mirandabootini);
	if (_tcsicmp(Mgr, _T("never")))
		return 0;		

	return fileExist(szProfile) || shouldAutoCreate(szProfile);
}

// returns 1 if a profile was selected
static int getProfile(TCHAR * szProfile, size_t cch)
{
	getProfilePath(g_profileDir, SIZEOF(g_profileDir));
	getDefaultProfile(szProfile, cch, g_profileDir);
	getProfileCmdLine(szProfile, cch, g_profileDir);
	if (validateProfileDir(g_profileDir))
		return 0;
	if (getProfileAutoRun(szProfile))
		return 1;

	PROFILEMANAGERDATA pd = {0};
	if (getProfile1(szProfile, cch, g_profileDir, &pd.noProfiles)) 
		return 1;

	pd.szProfile = szProfile;
	pd.szProfileDir = g_profileDir;
	return getProfileManager(&pd);
}

// carefully converts a file name from TCHAR* to char*
char* makeFileName( const TCHAR* tszOriginalName )
{
	char* szResult = NULL;
	char* szFileName = mir_t2a( tszOriginalName );
	TCHAR* tszFileName = mir_a2t( szFileName );
	if ( _tcscmp( tszOriginalName, tszFileName )) {
		TCHAR tszProfile[MAX_PATH];
		if ( GetShortPathName( tszOriginalName, tszProfile, MAX_PATH) != 0)
			szResult = mir_t2a( tszProfile );
	} 

	if ( !szResult )
		szResult = szFileName;
	else
		mir_free(szFileName);
	mir_free(tszFileName);

	return szResult;
}

// called by the UI, return 1 on success, use link to create profile, set error if any
int makeDatabase(TCHAR * profile, DATABASELINK * link, HWND hwndDlg)
{
	TCHAR buf[256];
	int err=0;	
	// check if the file already exists
	TCHAR * file = _tcsrchr(profile, '\\');
	if (file) file++;
	if (_taccess(profile, 0) == 0) {		
		// file already exists!
		mir_sntprintf(buf, SIZEOF(buf), TranslateTS( _T("The profile '%s' already exists. Do you want to move it to the ")
			_T("Recycle Bin? \n\nWARNING: The profile will be deleted if Recycle Bin is disabled.\n")
			_T("WARNING: A profile may contain confidential information and should be properly deleted.")), file );
		if ( MessageBox(hwndDlg, buf, TranslateT("The profile already exists"), MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) != IDYES )
			return 0;
		
		// move the file
		SHFILEOPSTRUCT sf = {0};
		sf.wFunc = FO_DELETE;
		sf.pFrom = buf;
		sf.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\0"), profile);
		if ( SHFileOperation(&sf) != 0 ) {
			mir_sntprintf(buf, SIZEOF(buf),TranslateT("Couldn't move '%s' to the Recycle Bin, Please select another profile name."),file);
			MessageBox(0,buf,TranslateT("Problem moving profile"),MB_ICONINFORMATION|MB_OK);
			return 0;
		}
		// now the file should be gone!
	}
	// ask the database to create the profile
	CreatePathToFileT(profile);
	char *prf = makeFileName(profile);
	if (link->makeDatabase(prf, &err)) { 
		mir_sntprintf(buf, SIZEOF(buf),TranslateT("Unable to create the profile '%s', the error was %x"),file, err);
		MessageBox(hwndDlg,buf,TranslateT("Problem creating profile"),MB_ICONERROR|MB_OK);
		mir_free(prf);
		return 0;
	}
	dbCreated = true;
	// the profile has been created! woot
	mir_free(prf);
	return 1;
}

// enumerate all plugins that had valid DatabasePluginInfo()
static int FindDbPluginForProfile(char*, DATABASELINK * dblink, LPARAM lParam)
{
	int res = DBPE_CONT;
	if ( dblink && dblink->cbSize == sizeof(DATABASELINK)) {
		char* szProfile = makeFileName((TCHAR*) lParam);
		// liked the profile?
		int err = 0;
		if (dblink->grokHeader(szProfile, &err) == 0) { 			
			// added APIs?
			res = dblink->Load(szProfile, &pluginCoreLink) ? DBPE_HALT : DBPE_DONE;
		}
		else {
			res = DBPE_HALT;
			switch ( err ) {				 
			case EGROKPRF_CANTREAD:
			case EGROKPRF_UNKHEADER:
				// just not supported.
				res = DBPE_CONT;

			case EGROKPRF_VERNEWER:
			case EGROKPRF_DAMAGED:
				break;
			}
		} //if
		mir_free(szProfile);
	}
	return res;
}

// enumerate all plugins that had valid DatabasePluginInfo()
static int FindDbPluginAutoCreate(char*, DATABASELINK * dblink, LPARAM lParam)
{
	int res = DBPE_CONT;
	if (dblink && dblink->cbSize == sizeof(DATABASELINK)) {
		TCHAR* tszProfile = (TCHAR*)lParam;
		CreatePathToFileT( tszProfile );

		int err;
		char *szProfile = makeFileName( tszProfile );
		if (dblink->makeDatabase(szProfile, &err) == 0) {
			dbCreated = true;
			res = dblink->Load(szProfile, &pluginCoreLink) ? DBPE_HALT : DBPE_DONE;
		}
		mir_free(szProfile);
	}
	return res;
}

typedef struct {
	TCHAR * profile;
	UINT msg;
	ATOM aPath;
	int found;
} ENUMMIRANDAWINDOW;

static BOOL CALLBACK EnumMirandaWindows(HWND hwnd, LPARAM lParam)
{
	TCHAR classname[256];
	ENUMMIRANDAWINDOW * x = (ENUMMIRANDAWINDOW *)lParam;
	DWORD_PTR res=0;
	if ( GetClassName(hwnd,classname,SIZEOF(classname)) && lstrcmp( _T("Miranda"),classname)==0 ) {		
		if ( SendMessageTimeout(hwnd, x->msg, (WPARAM)x->aPath, 0, SMTO_ABORTIFHUNG, 100, &res) && res ) {
			x->found++;
			return FALSE;
		}
	}
	return TRUE;
}

static int FindMirandaForProfile(TCHAR * szProfile)
{
	ENUMMIRANDAWINDOW x={0};
	x.profile=szProfile;
	x.msg=RegisterWindowMessage( _T( "Miranda::ProcessProfile" ));
	x.aPath=GlobalAddAtom(szProfile);
	EnumWindows(EnumMirandaWindows, (LPARAM)&x);
	GlobalDeleteAtom(x.aPath);
	return x.found;
}

int LoadDatabaseModule(void)
{
	TCHAR szProfile[MAX_PATH];
	pathToAbsoluteT(_T("."), szProfile, NULL);
	_tchdir(szProfile);
	szProfile[0] = 0;

	// load the older basic services of the db
	InitTime();
	InitUtils();

	// find out which profile to load
	if ( !getProfile( szProfile, SIZEOF( szProfile )))
		return 1;

	PLUGIN_DB_ENUM dbe;
	dbe.cbSize = sizeof(PLUGIN_DB_ENUM);
	dbe.lParam = (LPARAM)szProfile;

	if ( _taccess(szProfile, 0) && shouldAutoCreate( szProfile ))
		dbe.pfnEnumCallback=( int(*) (char*,void*,LPARAM) )FindDbPluginAutoCreate;
	else
		dbe.pfnEnumCallback=( int(*) (char*,void*,LPARAM) )FindDbPluginForProfile;

	// find a driver to support the given profile
	int rc = CallService(MS_PLUGINS_ENUMDBPLUGINS, 0, (LPARAM)&dbe);
	switch ( rc ) {
	case -1: {
		// no plugins at all
		TCHAR buf[256];
		TCHAR* p = _tcsrchr(szProfile,'\\');
		mir_sntprintf(buf,SIZEOF(buf),TranslateT("Miranda is unable to open '%s' because you do not have any profile plugins installed.\nYou need to install dbx_3x.dll or equivalent."), p ? ++p : szProfile );
		MessageBox(0,buf,TranslateT("No profile support installed!"),MB_OK | MB_ICONERROR);
		break;
	}
	case 1:
		// if there were drivers but they all failed cos the file is locked, try and find the miranda which locked it
		if (fileExist(szProfile)) {
			// file isn't locked, just no driver could open it.
			TCHAR buf[256];
			TCHAR* p = _tcsrchr(szProfile,'\\');
			mir_sntprintf(buf,SIZEOF(buf),TranslateT("Miranda was unable to open '%s', it's in an unknown format.\nThis profile might also be damaged, please run DB-tool which should be installed."), p ? ++p : szProfile);
			MessageBox(0,buf,TranslateT("Miranda can't understand that profile"),MB_OK | MB_ICONERROR);
		}
		else if (!FindMirandaForProfile(szProfile)) {
			TCHAR buf[256];
			TCHAR* p = _tcsrchr(szProfile,'\\');
			mir_sntprintf(buf,SIZEOF(buf),TranslateT("Miranda was unable to open '%s'\nIt's inaccessible or used by other application or Miranda instance"), p ? ++p : szProfile);
			MessageBox(0,buf,TranslateT("Miranda can't open that profile"),MB_OK | MB_ICONERROR);
		}
		break;
	}
	return (rc != 0);
}

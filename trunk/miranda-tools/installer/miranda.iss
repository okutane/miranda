[_ISTool]
UseAbsolutePaths=false

[Setup]
AppName=Miranda IM
AppVerName=Miranda IM 0.5 Preview Release 2
AppVersion=0.5
AppId=Miranda IM
LicenseFile=..\..\miranda\docs\license.txt
DefaultDirName={code:GetPath}
DefaultGroupName=Miranda IM
ShowLanguageDialog=auto
Compression=lzma
SolidCompression=true
AppPublisherURL=http://miranda-im.org/
AppSupportURL=http://miranda-im.org/support/
UninstallDisplayName=Miranda IM
UninstallFilesDir={app}\Uninstall
OutputBaseFilename=miranda-im-v0.5.0.0pr2-ansi
AppPublisher=Miranda IM Development Team
AppUpdatesURL=http://miranda-im.org/download/
OutputDir=.
DisableProgramGroupPage=true
FlatComponentsList=true
DisableReadyPage=false
ShowTasksTreeLines=true
VersionInfoVersion=0.5
VersionInfoCompany=Miranda IM
VersionInfoTextVersion=0.5 PR 2
MinVersion=4.0.950,4.0.1381
DirExistsWarning=no
LanguageDetectionMethod=uilanguage
UninstallDisplayIcon=
WizardImageFile=compiler:WizModernImage-IS.bmp
WizardSmallImageFile=compiler:WizModernSmallImage-IS.bmp

[Dirs]
Name: {app}\Plugins
Name: {app}\Uninstall
Name: {app}\Icons

[Files]
Source: ..\..\miranda\bin\release\miranda32.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\..\miranda\bin\release\dbtool.exe; DestDir: {app}; Components: Yahoo_Protocol; OnlyBelowVersion: 0,5.01.2600; Flags: ignoreversion
Source: ..\..\miranda\docs\mirandaboot.ini; DestDir: {app}; Flags: onlyifdoesntexist
Source: ..\..\miranda\docs\credits.txt; DestDir: {app}; DestName: Contributors.txt
Source: ..\..\miranda\docs\releasenotes.txt; DestDir: {app}; DestName: Readme.txt
Source: ..\..\miranda\docs\license.txt; DestDir: {app}; DestName: License.txt
Source: ..\..\miranda\bin\release\Plugins\clist_classic.dll; DestDir: {app}\Plugins; DestName: clist_classic.dll; Flags: ignoreversion
Source: ..\..\miranda\bin\release\Plugins\srmm.dll; DestDir: {app}\Plugins; DestName: SRMM.dll; Flags: ignoreversion
Source: ..\..\miranda\bin\release\plugins\png2dib.dll; DestDir: {app}\Plugins; DestName: png2dib.dll; Flags: ignoreversion
Source: ..\..\miranda\bin\release\Plugins\dbx_3x.dll; DestDir: {app}\Plugins; DestName: dbx_3x.dll; Flags: ignoreversion
Source: ..\..\miranda\bin\release\Plugins\chat.dll; DestDir: {app}\Plugins; Flags: ignoreversion; Components: IRC_Protocol MSN_Protocol Jabber_Protocol AIM_Protocol; DestName: Chat.dll
Source: ..\..\miranda\bin\release\plugins\import.dll; DestDir: {app}\Plugins; DestName: import.dll; Flags: ignoreversion
Source: ..\..\miranda\bin\release\Plugins\AIM.dll; DestDir: {app}\Plugins; Components: AIM_Protocol; Flags: ignoreversion
Source: icons\bin\hicolor\proto_AIM.dll; DestDir: {app}\Icons; Flags: ignoreversion; MinVersion: 0,5.01.2600; Components: AIM_Protocol
Source: icons\bin\locolor\proto_AIM.dll; DestDir: {app}\Icons; Flags: ignoreversion; OnlyBelowVersion: 0,5.01.2600; Components: AIM_Protocol
Source: ..\..\miranda\bin\release\Plugins\ICQ.dll; DestDir: {app}\Plugins; Components: ICQ_Protocol; Flags: ignoreversion
Source: icons\bin\locolor\proto_ICQ.dll; DestDir: {app}\Icons; Flags: ignoreversion; OnlyBelowVersion: 0,5.01.2600; Components: ICQ_Protocol
Source: icons\bin\hicolor\proto_ICQ.dll; DestDir: {app}\Icons; Flags: ignoreversion; MinVersion: 0,5.01.2600; Components: ICQ_Protocol
Source: ..\..\miranda\bin\release\Plugins\IRC.dll; DestDir: {app}\Plugins; Components: IRC_Protocol; Flags: ignoreversion
Source: IRC_servers.ini; DestDir: {app}\Plugins; Flags: onlyifdoesntexist
Source: icons\bin\hicolor\proto_IRC.dll; DestDir: {app}\Icons; Flags: ignoreversion; MinVersion: 0,5.01.2600; Components: IRC_Protocol
Source: icons\bin\locolor\proto_IRC.dll; DestDir: {app}\Icons; Flags: ignoreversion; Components: IRC_Protocol; OnlyBelowVersion: 0,5.01.2600
Source: ..\..\miranda\bin\release\Plugins\jabber.dll; DestDir: {app}\Plugins; Components: Jabber_Protocol; DestName: Jabber.dll; Flags: ignoreversion
Source: icons\bin\locolor\proto_JABBER.dll; DestDir: {app}\Icons; Flags: ignoreversion; OnlyBelowVersion: 0,5.01.2600; Components: Jabber_Protocol
Source: icons\bin\hicolor\proto_JABBER.dll; DestDir: {app}\Icons; Flags: ignoreversion; Components: Jabber_Protocol; MinVersion: 0,5.01.2600
Source: ..\..\miranda\bin\release\Plugins\msn.dll; DestDir: {app}\Plugins; Components: MSN_Protocol; DestName: MSN.dll; Flags: ignoreversion
Source: icons\bin\hicolor\proto_MSN.dll; DestDir: {app}\Icons; Flags: ignoreversion; MinVersion: 0,5.01.2600; Components: MSN_Protocol
Source: icons\bin\locolor\proto_MSN.dll; DestDir: {app}\Icons; Flags: ignoreversion; Components: MSN_Protocol; OnlyBelowVersion: 0,5.01.2600
Source: ..\..\miranda\bin\release\Plugins\Yahoo.dll; DestDir: {app}\Plugins; DestName: Yahoo.dll; Flags: ignoreversion; Components: Yahoo_Protocol
Source: icons\bin\hicolor\proto_YAHOO.dll; DestDir: {app}\Icons; Flags: ignoreversion; MinVersion: 0,5.01.2600; Components: Yahoo_Protocol
Source: icons\bin\locolor\proto_YAHOO.dll; DestDir: {app}\Icons; Flags: ignoreversion; Components: Yahoo_Protocol; OnlyBelowVersion: 0,5.01.2600

[Components]
Name: AIM_Protocol; Description: AIM; Types: custom compact full
Name: ICQ_Protocol; Description: ICQ; Types: custom compact full
Name: IRC_Protocol; Description: IRC; Types: custom compact full
Name: Jabber_Protocol; Description: Jabber; Types: custom compact full
Name: MSN_Protocol; Description: MSN; Types: custom compact full
Name: Yahoo_Protocol; Description: Yahoo!; Types: custom compact full

[Icons]
Name: {group}\Miranda IM; Filename: {app}\miranda32.exe; WorkingDir: {app}; IconIndex: 0; Tasks: Create_Startmenu_Shortcuts
Name: {group}\License; Filename: {app}\License.txt; Tasks: Create_Startmenu_Shortcuts; WorkingDir: {app}\Docs
Name: {userdesktop}\Miranda IM; Filename: {app}\miranda32.exe; WorkingDir: {app}; Tasks: Create_Desktop_Shortcuts; IconIndex: 0
Name: {group}\Uninstall; Filename: {uninstallexe}; WorkingDir: {app}\Uninstall; Tasks: Create_Startmenu_Shortcuts
Name: {userstartup}\Miranda IM; Filename: {app}\miranda32.exe; WorkingDir: {userstartup}; IconIndex: 0; Tasks: Startup_With_Windows

[Tasks]
Name: Create_Startmenu_Shortcuts; Description: Create Startmenu Shortcuts
Name: Create_Desktop_Shortcuts; Description: Create Desktop Shortcut
Name: Startup_With_Windows; Description: Start Miranda IM When Windows Starts; Flags: unchecked

[INI]

[UninstallDelete]

[InstallDelete]
Name: {app}\Docs\mirandaboot.ini; Type: files
Name: {app}\contributors.txt; Type: files
Name: {app}\gpl.txt; Type: files
Name: {app}\readme.txt; Type: files
Name: {app}\miranda32.exe.manifest; Type: files
Name: {app}\uninstall.exe; Type: files
Name: {userstartup}\Miranda IM.lnk; Type: files
Name: {app}\Docs\Contrib\*.txt; Type: files
Name: {app}\Docs\Contrib; Type: dirifempty

[Registry]
Root: HKLM; Subkey: Software\Microsoft\Windows\CurrentVersion\Uninstall\Miranda IM; Flags: deletekey
Root: HKCR; Subkey: SOFTWARE\Miranda; Flags: uninsdeletekey

[Languages]

[Code]
function GetAsyncKeyState(vKey: Integer): Word;
external 'GetAsyncKeyState@user32 stdcall';

function InitializeSetup(): Boolean;
begin
	Result := True;
	if (GetAsyncKeyState(17) and $8000) = 0 then begin
		while FindWindowByClassName('Miranda') > 0 do begin
			if MsgBox('Miranda IM is currently running.' #13#13 'Please exit Miranda IM so that the installation may continue.', mbConfirmation, MB_OKCANCEL) = idCancel then begin
				Result := False;
				Exit;
			end;
			Sleep(500);
		end;
	end;
end;

function GetPath(Default: string): string;
begin
	If RegValueExists(HKLM, 'SOFTWARE\Miranda', 'Install_Dir') then begin
		RegQueryStringValue(HKLM, 'SOFTWARE\Miranda', 'Install_Dir', Result);
	end
	else begin
		Result := ExpandConstant('{pf}') + '\Miranda IM';
	end;
end;
[Run]
Filename: {app}\miranda32.exe; Description: Start Miranda IM; Flags: nowait postinstall skipifsilent; WorkingDir: {app}

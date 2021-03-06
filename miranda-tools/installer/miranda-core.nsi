!include "MUI2.nsh"
!include "Sections.nsh"
!include "WinVer.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"

!define MIM_NAME                "Miranda IM"
!define MIM_URL                 "http://www.miranda-im.org/"
!define MIM_PUBLISHER           "${MIM_NAME} Project"
!define MIM_COPYRIGHT           "Copyright © 2000-2016 ${MIM_PUBLISHER}"

!define MIM_BUILD_ICONS_LOW     "icons\bin\locolor"
!define MIM_BUILD_ICONS_HI      "icons\bin\hicolor"
!define MIM_BUILD_OPTIONS_FILE  "miranda32.lst"
!define MIM_BUILD_OPTIONS_SECT  "InstalledSections"
!define MIM_BUILD_SUCCESS       "http://www.miranda-im.org/donate/"

!ifdef MIM_BUILD_UNICODE
!define MIM_BUILD_TYPE          "unicode"
!define MIM_BUILD_SUFFIX        "Release Unicode"
!else ifdef MIM_BUILD_X64
!define MIM_BUILD_TYPE          "x64"
!define MIM_BUILD_SUFFIX        "Release Unicode64"
!else
!define MIM_BUILD_TYPE          "ansi"
!define MIM_BUILD_SUFFIX        "Release"
!endif
!define MIM_BUILD_SRC           "..\..\miranda"

!ifdef MIM_BUILD_X64
!define MIM_BUILD_EXE           "miranda64.exe"
!else
!define MIM_BUILD_EXE           "miranda32.exe"
!endif

; Support /bin and /bin9 installer builds
!if      /FileExists "${MIM_BUILD_SRC}\bin9\${MIM_BUILD_SUFFIX}\${MIM_BUILD_EXE}"
  !define MIM_BIN               "bin9"
!else if /FileExists "${MIM_BUILD_SRC}\bin\${MIM_BUILD_SUFFIX}\${MIM_BUILD_EXE}"
  !define MIM_BIN               "bin"
!else
  !error "File '${MIM_BUILD_EXE}' not found!"
!endif

!define MIM_BUILD_DIR           "${MIM_BUILD_SRC}\${MIM_BIN}\${MIM_BUILD_SUFFIX}"
!define MIM_BUILD_DIRANSI       "${MIM_BUILD_SRC}\${MIM_BIN}\Release"

!ifdef MIM_BUILD_X64
!define MIM_BUILD_VCREDIST      "vcredist_x64.exe"
!else
!define MIM_BUILD_VCREDIST      "vcredist_x86.exe"
!endif
!define MIM_BUILD_VCREDIST_DIR  "${MIM_BUILD_SRC}\${MIM_BIN}\contrib"

!getdllversion                  "${MIM_BUILD_DIR}\${MIM_BUILD_EXE}" VER_
!define MIM_VERSION             "${VER_1}.${VER_2}.${VER_3}"

Name                            "${MIM_NAME} ${MIM_VERSION}"
OutFile                         "${MIM_BUILD_SRC}\bin\miranda-im-v${MIM_VERSION}-${MIM_BUILD_TYPE}.exe"

!ifdef MIM_BUILD_UNICODE || MIM_BUILD_X64
Unicode							true
!endif

!ifdef MIM_BUILD_X64
InstallDir                      "$PROGRAMFILES64\${MIM_NAME}"
!else
InstallDir                      "$PROGRAMFILES\${MIM_NAME}"
!endif
InstallDirRegKey                HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${MIM_BUILD_EXE}" "Path"
SetOverWrite                    on
BrandingText                    "miranda-im.org"

VIProductVersion                "${MIM_VERSION}.0"
VIAddVersionKey                 ProductName "${MIM_NAME}"
VIAddVersionKey                 ProductVersion "${MIM_VERSION}.0"
VIAddVersionKey                 FileDescription "${MIM_NAME}"
VIAddVersionKey                 FileVersion "${MIM_VERSION}.0"
VIAddVersionKey                 CompanyName "${MIM_PUBLISHER}"
VIAddVersionKey                 LegalCopyright "${MIM_COPYRIGHT}"

VAR INST_UPGRADE
VAR INST_SUCCESS
VAR INST_MODE
VAR INST_MODE_DEF
VAR INST_MODE_POR
VAR INST_DIR
VAR INST_WARN

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "Graphics\header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "Graphics\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "Graphics\welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "Graphics\welcome.bmp"
!define MUI_ICON "Graphics\install.ico"
!define MUI_UNICON "Graphics\uninstall.ico"
!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_LICENSEPAGE_BGCOLOR /grey
!define MUI_FINISHPAGE_RUN $INSTDIR\${MIM_BUILD_EXE}
!ifdef MIM_BUILD_X64
!define MUI_FINISHPAGE_RUN_TEXT "Start ${MIM_NAME} (x64)"
!else
!define MUI_FINISHPAGE_RUN_TEXT "Start ${MIM_NAME}"
!endif

!insertmacro MUI_PAGE_LICENSE "${MIM_BUILD_SRC}\docs\license.txt"
Page Custom CustomInstallPage CustomInstallPageLeave
!define MUI_DIRECTORYPAGE_VARIABLE $INST_DIR
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE VerifyInstallDir
!define MUI_PAGE_CUSTOMFUNCTION_PRE VerifyDirectoryDisplay
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_PAGE_CUSTOMFUNCTION_PRE VerifyComponentDisplay
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

LangString CLOSE_WARN ${LANG_ENGLISH}     "${MIM_NAME} is currently running. Please close ${MIM_NAME} to continue."

!macro CloseMiranda
  FindWindow $0 "Miranda"
  IsWindow $0 +3
  FindWindow $0 "Miranda IM"
  IsWindow $0 0 +6
  MessageBox MB_ICONEXCLAMATION|MB_ABORTRETRYIGNORE "$(CLOSE_WARN)" IDRETRY +2 IDIGNORE +5
  Quit
  SendMessage $0 16 0 0 /TIMEOUT=5000
  Sleep 5000
  Goto -8
!macroend

!macro PrintInstallerDetails Details
  SetDetailsPrint textonly
  DetailPrint "${Details}"
  SetDetailsPrint listonly
!macroend

!macro InstallMirandaProtoIcon IconFile
  SetOutPath "$INSTDIR\Icons"
  SetOverWrite off
  !ifdef MIM_BUILD_UNICODE || MIM_BUILD_X64
  ${If} ${AtLeastWinXP}
    File "${MIM_BUILD_ICONS_HI}\proto_${IconFile}.dll"
  ${Else}
    File "${MIM_BUILD_ICONS_LOW}\proto_${IconFile}.dll"
  ${EndIf}
  !else
  File "${MIM_BUILD_ICONS_LOW}\proto_${IconFile}.dll"
  !endif
  SetOverWrite on
!macroend

!macro InstallMirandaPlugin PluginFile
  SetOutPath "$INSTDIR\Plugins"
  !if /FileExists "${MIM_BUILD_DIR}\plugins\${PluginFile}"
    File "${MIM_BUILD_DIR}\plugins\${PluginFile}"
  !else if /FileExists "${MIM_BUILD_DIRANSI}\plugins\${PluginFile}"
    File "${MIM_BUILD_DIRANSI}\plugins\${PluginFile}"
  !else
    !error "File ${PluginFile} not found!"
  !endif
!macroend

!macro WriteInstallerOption IniOption IniValue
  ${If} $INST_MODE = 0
    SetOutPath "$INSTDIR"
    WriteINIStr "$INSTDIR\${MIM_BUILD_OPTIONS_FILE}" "${MIM_BUILD_OPTIONS_SECT}" "${IniValue}" "${IniOption}"
  ${EndIf}
!macroend

Section "${MIM_NAME}"
  SectionIn RO
  !insertmacro CloseMiranda
	
  !insertmacro PrintInstallerDetails "Installing ${MIM_NAME}..."
  
  !insertmacro WriteInstallerOption "0" "Import"
  !insertmacro WriteInstallerOption "0" "StartMenuShortCut"
  !insertmacro WriteInstallerOption "0" "DesktopShortCut"
  !insertmacro WriteInstallerOption "0" "QuickLaunchShortCut"

  SetOutPath "$INSTDIR"
  File "${MIM_BUILD_DIR}\${MIM_BUILD_EXE}"
  File "${MIM_BUILD_DIR}\dbtool.exe"
  File "${MIM_BUILD_DIR}\zlib.dll"
  File "${MIM_BUILD_SRC}\docs\contributors.txt"
  File "${MIM_BUILD_SRC}\docs\readme.txt"
  File "${MIM_BUILD_SRC}\docs\changelog.txt"
  File "${MIM_BUILD_SRC}\docs\license.txt"
  
  !ifdef MIM_BUILD_UNICODE || MIM_BUILD_X64
  ${If} $INST_MODE = 0
    ${If} $INST_UPGRADE = 1
      ${If} ${FileExists} "$INSTDIR\mirandaboot.ini"
        ReadINIStr $0 "$INSTDIR\mirandaboot.ini" "Database" "ProfileDir"
        ${If} $0 == ""
          StrCpy $INST_WARN 1
        ${Endif}
        ${If} $0 == ""
          StrCpy $INST_WARN 1
        ${Endif}
      ${EndIf}
    ${EndIf}
  ${EndIf}
  !endif
  ${If} $INST_UPGRADE = 0
    SetOverWrite off
    File "${MIM_BUILD_SRC}\docs\mirandaboot.ini"
    SetOverWrite on
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\mirandaboot.ini"
    ${If} $INST_UPGRADE = 0
      ${If} $INST_MODE = 0
	    !ifdef MIM_BUILD_UNICODE || MIM_BUILD_X64
          WriteINIStr "$INSTDIR\mirandaboot.ini" "Database" "ProfileDir" "%APPDATA%\Miranda"
		!endif
  	  ${ElseIf} $INST_MODE = 1
  	    CreateDirectory "$INSTDIR\Profiles"
	    WriteINIStr "$INSTDIR\mirandaboot.ini" "Database" "ProfileDir" "Profiles"
	  ${EndIf}
    ${EndIf}
  ${EndIf}
  
  !insertmacro InstallMirandaPlugin "clist_classic.dll"
  !insertmacro InstallMirandaPlugin "srmm.dll"
  !insertmacro InstallMirandaPlugin "avs.dll"
  !insertmacro InstallMirandaPlugin "advaimg.dll"
  !insertmacro InstallMirandaPlugin "dbx_sqlite.dll"
  !ifdef MIM_BUILD_UNICODE || MIM_BUILD_X64
  !insertmacro InstallMirandaPlugin "dbx_mmap.dll"
  Delete "$INSTDIR\Plugins\dbx_3x.dll"
  !else
  !insertmacro InstallMirandaPlugin "dbx_3x.dll"
  Delete "$INSTDIR\Plugins\dbx_mmap.dll"
  !endif
  !insertmacro InstallMirandaPlugin "chat.dll"
  ; Update Contrib plugins that exist even if options is not selected
  ${If} ${FileExists} "$INSTDIR\Plugins\clist_modern.dll"
    !insertmacro InstallMirandaPlugin "clist_modern.dll"
    File "${MIM_BUILD_DIRANSI}\Icons\toolbar_icons.dll"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\Plugins\clist_mw.dll"
    !insertmacro InstallMirandaPlugin "clist_mw.dll"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\Plugins\clist_nicer.dll"
    !insertmacro InstallMirandaPlugin "clist_nicer.dll"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\Plugins\scriver.dll"
    !insertmacro InstallMirandaPlugin "scriver.dll"
  ${EndIf}
  !ifdef MIM_BUILD_UNICODE || MIM_BUILD_X64
  ${If} ${FileExists} "$INSTDIR\Plugins\tabsrmm.dll"
    !insertmacro InstallMirandaPlugin "tabsrmm.dll"
    SetOutPath "$INSTDIR\Icons"
    File "${MIM_BUILD_DIRANSI}\Icons\tabsrmm_icons.dll"
  ${EndIf}
  !endif

  ${If} $INST_MODE = 0
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}" "DisplayName" "${MIM_NAME} ${MIM_VERSION}" 
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}" "InstallLocation" "$INSTDIR"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}" "DisplayIcon" "$INSTDIR\${MIM_BUILD_EXE}"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}" "DisplayVersion" "${MIM_VERSION}"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}" "URLInfoAbout" "${MIM_URL}"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}" "Publisher" "${MIM_PUBLISHER}"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${MIM_BUILD_EXE}" "" "$INSTDIR\${MIM_BUILD_EXE}"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${MIM_BUILD_EXE}" "Path" "$INSTDIR"
  ${EndIf}
  
  ; AIM
  !insertmacro PrintInstallerDetails "Installing AIM Protocol..."
  !insertmacro InstallMirandaPlugin "Aim.dll"
  !insertmacro InstallMirandaProtoIcon "AIM"
  
  ; Gadu-Gadu
  !insertmacro PrintInstallerDetails "Installing Gadu-Gadu Protocol..."
  !insertmacro InstallMirandaPlugin "GG.dll"
  !insertmacro InstallMirandaProtoIcon "GG"
  
  ; ICQ 
  !insertmacro PrintInstallerDetails "Installing ICQ Protocol..."
  !insertmacro InstallMirandaPlugin "icq.dll"
  SetOutPath "$INSTDIR\Icons"
  File "${MIM_BUILD_DIRANSI}\Icons\xstatus_ICQ.dll"
  !insertmacro InstallMirandaProtoIcon "ICQ"

  ; IRC
  !insertmacro PrintInstallerDetails "Installing IRC Protocol..."
  !insertmacro InstallMirandaPlugin "irc.dll"
  ${If} $INST_UPGRADE = 0
    SetOverWrite off
    File "${MIM_BUILD_SRC}\protocols\IRCG\Docs\irc_servers.ini"
    SetOverWrite on
  ${EndIf}
  !insertmacro InstallMirandaProtoIcon "IRC"

  ; Jabber
  !insertmacro PrintInstallerDetails "Installing Jabber Protocol..."
  !insertmacro InstallMirandaPlugin "jabber.dll"
  SetOutPath "$INSTDIR\Icons"
  File "${MIM_BUILD_DIRANSI}\Icons\xstatus_jabber.dll"
  !insertmacro InstallMirandaProtoIcon "Jabber"

  ; MSN
  !insertmacro PrintInstallerDetails "Installing MSN Protocol..."
  !insertmacro InstallMirandaPlugin "msn.dll"
  !insertmacro InstallMirandaProtoIcon "MSN"

  ; Yahoo
  !insertmacro PrintInstallerDetails "Installing Yahoo Protocol..."
  !insertmacro InstallMirandaPlugin "yahoo.dll"
  !insertmacro InstallMirandaProtoIcon "Yahoo"
  
  ; Import (installs automatically on new installs and if the file exists)
  ${If} ${FileExists} "$INSTDIR\plugins\import.dll"
    !insertmacro InstallMirandaPlugin "import.dll"
  ${ElseIf} $INST_UPGRADE = 0
    !insertmacro InstallMirandaPlugin "import.dll"
  ${EndIf}
  
  ; Run redistributable
  ${If} $INST_MODE = 0
    !insertmacro PrintInstallerDetails "Installing Microsoft Redistributable Package..."
    File "/oname=$TEMP\${MIM_BUILD_VCREDIST}" "${MIM_BUILD_VCREDIST_DIR}\${MIM_BUILD_VCREDIST}"
    ExecWait '"$TEMP\${MIM_BUILD_VCREDIST}" /q'
    Delete /REBOOTOK "$TEMP\${MIM_BUILD_VCREDIST}"
  ${Else}
    SetOutPath "$INSTDIR\Contrib"
	File "${MIM_BUILD_VCREDIST_DIR}\${MIM_BUILD_VCREDIST}"
  ${EndIf}
  
  ${If} $INST_MODE = 0
    WriteUninstaller "$INSTDIR\Uninstall.exe"
  ${EndIf}
SectionEnd
  
SubSection /e "Options" pOptions
  Section "Install Start Menu Shortcuts" pSCStartMenu
    !insertmacro PrintInstallerDetails "Configuring ${MIM_NAME}..."
    !insertmacro WriteInstallerOption "1" "StartMenuShortCut"
    SetOutPath "$INSTDIR"
    RMDir /r "$SMPROGRAMS\${MIM_NAME}"
    CreateDirectory "$SMPROGRAMS\${MIM_NAME}"
    CreateShortCut  "$SMPROGRAMS\${MIM_NAME}\${MIM_NAME}.lnk" "$INSTDIR\${MIM_BUILD_EXE}"
    WriteINIStr     "$SMPROGRAMS\${MIM_NAME}\Donate.url" "InternetShortcut" "URL" "http://www.miranda-im.org/donate/"
  SectionEnd

  Section "Install Desktop Shortcut" pSCDesktop
    !insertmacro PrintInstallerDetails "Configuring ${MIM_NAME}..."
    !insertmacro WriteInstallerOption "1" "DesktopShortCut"
    SetOutPath "$INSTDIR"
    CreateShortCut  "$DESKTOP\${MIM_NAME}.lnk" "$INSTDIR\${MIM_BUILD_EXE}"
  SectionEnd

  Section "Install Quicklaunch Shortcut" pSCQuickLaunch
    !insertmacro PrintInstallerDetails "Configuring ${MIM_NAME}..."
    !insertmacro WriteInstallerOption "1" "QuickLaunchShortCut"
	${If} ${AtMostWin8}
    SetOutPath "$INSTDIR"
    CreateShortCut  "$QUICKLAUNCH\${MIM_NAME}.lnk" "$INSTDIR\${MIM_BUILD_EXE}"
	${EndIf}
  SectionEnd
SubSectionEnd

Section Uninstall
  ; All users shortcuts
  SetShellVarContext "all"
  Delete "$SMPROGRAMS\${MIM_NAME}\${MIM_NAME}.lnk"
  Delete "$SMPROGRAMS\${MIM_NAME}\Homepage.url"
  Delete "$SMPROGRAMS\${MIM_NAME}\Get More Addons.url"
  RMDir "$SMPROGRAMS\${MIM_NAME}"
  Delete "$DESKTOP\${MIM_NAME}.lnk"
  Delete "$QUICKLAUNCH\${MIM_NAME}.lnk"

    ; Current user shortcuts
  SetShellVarContext "current"
  Delete "$SMPROGRAMS\${MIM_NAME}\${MIM_NAME}.lnk"
  Delete "$SMPROGRAMS\${MIM_NAME}\Donate.url"
  ; legacy begin
  Delete "$SMPROGRAMS\${MIM_NAME}\Homepage.url"
  Delete "$SMPROGRAMS\${MIM_NAME}\Get More Addons.url"
  ; legacy end
  RMDir "$SMPROGRAMS\${MIM_NAME}"
  Delete "$DESKTOP\${MIM_NAME}.lnk"
  Delete "$QUICKLAUNCH\${MIM_NAME}.lnk"
  
  ; Icons
  Delete "$INSTDIR\Icons\proto_AIM.dll"
  Delete "$INSTDIR\Icons\proto_GG.dll"
  Delete "$INSTDIR\Icons\proto_ICQ.dll"
  Delete "$INSTDIR\Icons\proto_IRC.dll"
  Delete "$INSTDIR\Icons\proto_Jabber.dll"
  Delete "$INSTDIR\Icons\proto_MSN.dll"
  Delete "$INSTDIR\Icons\proto_Yahoo.dll"
  Delete "$INSTDIR\Icons\tabsrmm_icons.dll"
  Delete "$INSTDIR\Icons\toolbar_icons.dll"
  Delete "$INSTDIR\Icons\xstatus_ICQ.dll"
  Delete "$INSTDIR\Icons\xstatus_jabber.dll"
  RMDir "$INSTDIR\Icons"

  ; Plugins
  Delete "$INSTDIR\Plugins\advaimg.dll"
  Delete "$INSTDIR\Plugins\aim.dll"
  Delete "$INSTDIR\Plugins\avs.dll"
  Delete "$INSTDIR\Plugins\chat.dll"
  Delete "$INSTDIR\Plugins\clist_classic.dll"
  Delete "$INSTDIR\Plugins\clist_modern.dll"
  Delete "$INSTDIR\Plugins\clist_mw.dll"
  Delete "$INSTDIR\Plugins\clist_nicer.dll"
  Delete "$INSTDIR\Plugins\dbx_3x.dll"
  Delete "$INSTDIR\Plugins\dbx_mmap.dll"
  Delete "$INSTDIR\Plugins\dbx_sqlite.dll"
  Delete "$INSTDIR\Plugins\gg.dll"
  Delete "$INSTDIR\Plugins\icq.dll"
  Delete "$INSTDIR\Plugins\import.dll"
  Delete "$INSTDIR\Plugins\irc.dll"
  Delete "$INSTDIR\Plugins\irc_servers.ini"
  Delete "$INSTDIR\Plugins\jabber.dll"
  Delete "$INSTDIR\Plugins\msn.dll"
  Delete "$INSTDIR\Plugins\scriver.dll"
  Delete "$INSTDIR\Plugins\srmm.dll"
  Delete "$INSTDIR\Plugins\tabsrmm.dll"
  Delete "$INSTDIR\Plugins\yahoo.dll"
  RMDir "$INSTDIR\Plugins"
 
  Delete "$INSTDIR\dbtool.exe"
  Delete "$INSTDIR\${MIM_BUILD_EXE}"
  Delete "$INSTDIR\zlib.dll"
  Delete "$INSTDIR\mirandaboot.ini"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\contributors.txt"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\changelog.txt"
  Delete "$INSTDIR\${MIM_BUILD_OPTIONS_FILE}"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"

  DeleteRegKey HKLM "SOFTWARE\Miranda"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MIM_NAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${MIM_BUILD_EXE}"
SectionEnd

Function .onInit
  SetShellVarContext "current"
  StrCpy $INST_SUCCESS 0
  StrCpy $INST_MODE 0
  StrCpy $INST_WARN 0
FunctionEnd

Function .onInstSuccess
  StrCpy $INST_SUCCESS 1
FunctionEnd

Function .onGUIEnd
  ${If} $INST_SUCCESS = 1
    ExecShell "open" "${MIM_BUILD_SUCCESS}"
  ${EndIf}
FunctionEnd

Function VerifyInstallDir
  StrCpy $INSTDIR $INST_DIR
  ${If} ${FileExists} "$INSTDIR\${MIM_BUILD_EXE}"
    StrCpy $INST_UPGRADE 1
  ${Else}
    StrCpy $INST_UPGRADE 0
  ${EndIf}
  ${If} $INST_MODE = 1
    !insertmacro ClearSectionFlag ${pSCStartMenu} ${SF_SELECTED}
    SectionSetText ${pSCStartMenu} ""
	!insertmacro ClearSectionFlag ${pSCDesktop} ${SF_SELECTED}
    SectionSetText ${pSCDesktop} ""
	!insertmacro ClearSectionFlag ${pSCQuickLaunch} ${SF_SELECTED}
    SectionSetText ${pSCQuickLaunch} ""
	!insertmacro ClearSectionFlag ${pOptions} ${SF_SELECTED}
    SectionSetText ${pOptions} ""
  ${Else}
    ReadINIStr $0 "$INSTDIR\${MIM_BUILD_OPTIONS_FILE}" ${MIM_BUILD_OPTIONS_SECT} "StartMenuShortCut"
    ${If} $0 == "0"
      !insertmacro ClearSectionFlag ${pSCStartMenu} ${SF_SELECTED}
    ${Else}
      !insertmacro SetSectionFlag ${pSCStartMenu} ${SF_SELECTED}
    ${EndIf}
    ReadINIStr $0 "$INSTDIR\${MIM_BUILD_OPTIONS_FILE}" ${MIM_BUILD_OPTIONS_SECT} "DesktopShortCut"
    ${If} $0 == "0"
      !insertmacro ClearSectionFlag ${pSCDesktop} ${SF_SELECTED}
    ${Else}
      !insertmacro SetSectionFlag ${pSCDesktop} ${SF_SELECTED}
    ${EndIf}
    ReadINIStr $0 "$INSTDIR\${MIM_BUILD_OPTIONS_FILE}" ${MIM_BUILD_OPTIONS_SECT} "QuickLaunchShortCut"
    ${If} $0 == "0"
      !insertmacro ClearSectionFlag ${pSCQuickLaunch} ${SF_SELECTED}
    ${Else}
      !insertmacro SetSectionFlag ${pSCQuickLaunch} ${SF_SELECTED}
    ${EndIf}
    ${If} ${AtLeastWin7}
      !insertmacro ClearSectionFlag ${pSCQuickLaunch} ${SF_SELECTED}
      SectionSetText ${pSCQuickLaunch} ""
    ${EndIf}
  ${Endif}
FunctionEnd

Function VerifyDirectoryDisplay
  ${If} $INST_MODE = 1
    GetDlgItem $1 $HWNDPARENT 1
    SendMessage $1 ${WM_SETTEXT} 0 "STR:$(^InstallBtn)"
  ${EndIf}
FunctionEnd

Function CustomInstallPage
  !insertmacro MUI_HEADER_TEXT "Installation Mode" "Select install type."
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}
  ${NSD_CreateLabel} 10 3 278 30 "Please select the type of installation you wish to peform."
  ${NSD_CreateRadioButton} 25 47 155 16 "Default Installation"
  Pop $INST_MODE_DEF
  ${NSD_CreateRadioButton} 25 70 155 16 "Portable Installation"
  Pop $INST_MODE_POR
  ${If} $INST_MODE = 0
    ${NSD_SetState} $INST_MODE_DEF ${BST_CHECKED}
	${NSD_SetState} $INST_MODE_POR ${BST_UNCHECKED}
  ${Else}
    ${NSD_SetState} $INST_MODE_DEF ${BST_UNCHECKED}
	${NSD_SetState} $INST_MODE_POR ${BST_CHECKED}
  ${EndIf}
  nsDialogs::Show
FunctionEnd

Function CustomInstallPageLeave
  ${NSD_GetState} $INST_MODE_DEF $0
  ${If} $0 = ${BST_CHECKED}
    StrCpy $INST_MODE 0
  ${Else}
    StrCpy $INST_MODE 1
  ${Endif}
  ${If} $INST_MODE = 1
	StrCpy $R0 $WINDIR 2
	StrCpy $INST_DIR "$R0\${MIM_NAME}"
  ${Else}
	ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${MIM_BUILD_EXE}" "Path"
	${If} $0 == ""
	  !ifdef MIM_BUILD_X64
        StrCpy $INST_DIR "$PROGRAMFILES64\${MIM_NAME}"
	  !else
	    StrCpy $INST_DIR "$PROGRAMFILES\${MIM_NAME}"
	  !endif
	${Else}
	  StrCpy $INST_DIR $0
	${EndIf}
  ${EndIf}
FunctionEnd


Function VerifyComponentDisplay
  ${If} $INST_MODE = 1
    Abort
  ${EndIf}
FunctionEnd
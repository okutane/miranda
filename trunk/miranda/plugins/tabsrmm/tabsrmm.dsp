# Microsoft Developer Studio Project File - Name="tabSRMM" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=tabSRMM - Win32 Debug
!MESSAGE Dies ist kein g�ltiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und f�hren Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "tabsrmm.mak".
!MESSAGE 
!MESSAGE Sie k�nnen beim Ausf�hren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "tabsrmm.mak" CFG="tabSRMM - Win32 Debug"
!MESSAGE 
!MESSAGE F�r die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "tabSRMM - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tabSRMM - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tabSRMM - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tabSRMM - Win32 Debug Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp".\Debug/srmm.pch" /YX /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp".\Debug/srmm.pch" /Yu"commonheaders.h" /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:".\Debug\srmm.pdb" /debug /machine:IX86 /out:"..\..\Bin\Debug\Plugins\tabsrmm.dll" /implib:".\Debug/srmm.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /subsystem:windows /dll /pdb:".\Debug\srmm.pdb" /debug /machine:IX86 /out:"..\..\Bin\Debug\Plugins\tabsrmm.dll" /implib:".\Debug/srmm.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release_Unicode"
# PROP BASE Intermediate_Dir ".\Release_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /D "_MBCS" /Fp".\Release_Unicode/srmm.pch" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp".\Release_Unicode/srmm.pch" /Yu"commonheaders.h" /GF /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "UNICODE"
# ADD RSC /l 0x809 /fo".\tabsrmm_private.res" /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /machine:IX86 /out:"..\..\Bin\Release\Plugins\tabsrmm_unicode.dll" /implib:".\Release_Unicode/srmm.lib" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /map /machine:IX86 /out:"..\..\Bin\Release Unicode\Plugins\tabsrmm.dll" /implib:".\Release_Unicode/srmm.lib" /opt:NOWIN98
# SUBTRACT LINK32 /pdb:none /incremental:yes /debug

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp".\Release/srmm.pch" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp".\Release/srmm.pch" /Yu"commonheaders.h" /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /machine:IX86 /out:"..\..\Bin\Release\Plugins\tabsrmm.dll" /implib:".\Release/srmm.lib" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /map /machine:IX86 /out:"..\..\Bin\Release\Plugins\tabsrmm.dll" /implib:".\Release/srmm.lib" /opt:NOWIN98
# SUBTRACT LINK32 /pdb:none /debug

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug_Unicode"
# PROP BASE Intermediate_Dir ".\Debug_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug_Unicode"
# PROP Intermediate_Dir ".\Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /D "_MBCS" /FR /Fp".\Debug_Unicode/srmm.pch" /YX /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "../" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /D "_MBCS" /FR /Fp".\Debug_Unicode/srmm.pch" /Yu"commonheaders.h" /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:".\Debug_Unicode\srmm.pdb" /debug /machine:IX86 /out:"..\..\Bin\Debug\Plugins\tabsrmm_unicode.dll" /implib:".\Debug_Unicode/srmm.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /subsystem:windows /dll /pdb:".\Debug_Unicode\srmm.pdb" /debug /machine:IX86 /out:"..\..\Bin\Debug Unicode\Plugins\tabsrmm.dll" /implib:".\Debug_Unicode/srmm.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "tabSRMM - Win32 Debug"
# Name "tabSRMM - Win32 Release Unicode"
# Name "tabSRMM - Win32 Release"
# Name "tabSRMM - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Chat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\chat\chat.h
# End Source File
# Begin Source File

SOURCE=.\chat\chat_resource.h
# End Source File
# Begin Source File

SOURCE=.\chat\chatprototypes.h
# End Source File
# Begin Source File

SOURCE=.\chat\clist.cpp
DEP_CPP_CLIST=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_CLIST=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\colorchooser.cpp
DEP_CPP_COLOR=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_COLOR=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\log.cpp
DEP_CPP_LOG_C=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_LOG_C=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\main.cpp
DEP_CPP_MAIN_=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MAIN_=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\manager.cpp
DEP_CPP_MANAG=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MANAG=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\message.cpp
DEP_CPP_MESSA=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MESSA=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\options.cpp
DEP_CPP_OPTIO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_OPTIO=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\services.cpp
DEP_CPP_SERVI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_SERVI=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\tools.cpp
DEP_CPP_TOOLS=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_TOOLS=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\window.cpp
DEP_CPP_WINDO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_WINDO=\
	".\om.h"\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# End Group
# Begin Group "API"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\API\m_cln_skinedit.h
# End Source File
# Begin Source File

SOURCE=.\API\m_fingerprint.h
# End Source File
# Begin Source File

SOURCE=.\API\m_flash.h
# End Source File
# Begin Source File

SOURCE=.\API\m_folders.h
# End Source File
# Begin Source File

SOURCE=.\API\m_historyevents.h
# End Source File
# Begin Source File

SOURCE=.\API\m_ieview.h
# End Source File
# Begin Source File

SOURCE=.\API\m_mathmodule.h
# End Source File
# Begin Source File

SOURCE=.\API\m_metacontacts.h
# End Source File
# Begin Source File

SOURCE=.\API\m_msg_buttonsbar.h
# End Source File
# Begin Source File

SOURCE=.\API\m_nudge.h
# End Source File
# Begin Source File

SOURCE=.\API\m_popup.h
# End Source File
# Begin Source File

SOURCE=.\API\m_smileyadd.h
# End Source File
# Begin Source File

SOURCE=.\API\m_spellchecker.h
# End Source File
# Begin Source File

SOURCE=.\API\m_toptoolbar.h
# End Source File
# Begin Source File

SOURCE=.\API\m_updater.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\buttonsbar.cpp
DEP_CPP_BUTTO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_BUTTO=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\container.cpp
DEP_CPP_CONTA=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_CONTA=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\containeroptions.cpp
DEP_CPP_CONTAI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_CONTAI=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\eventpopups.cpp
DEP_CPP_EVENT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_EVENT=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\formatting.cpp
DEP_CPP_FORMA=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_FORMA=\
	".\xtheme.h"\
	
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\src\generic_msghandlers.cpp
DEP_CPP_GENER=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_GENER=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\globals.cpp
DEP_CPP_GLOBA=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_GLOBA=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\hotkeyhandler.cpp
DEP_CPP_HOTKE=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_HOTKE=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\ImageDataObject.cpp
DEP_CPP_IMAGE=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\ImageDataObject.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_IMAGE=\
	".\xtheme.h"\
	
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\src\mim.cpp
DEP_CPP_MIM_C=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MIM_C=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tabmodplus\modplus.cpp
DEP_CPP_MODPL=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MODPL=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\src\msgdialog.cpp
DEP_CPP_MSGDI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MSGDI=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\msgdlgutils.cpp
DEP_CPP_MSGDL=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MSGDL=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\msglog.cpp
DEP_CPP_MSGLO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MSGLO=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\msgoptions.cpp
DEP_CPP_MSGOP=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_modernopt.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MSGOP=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tabmodplus\msgoptions_plus.cpp
DEP_CPP_MSGOPT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MSGOPT=\
	".\xtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\src\msgs.cpp
DEP_CPP_MSGS_=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_MSGS_=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\selectcontainer.cpp
DEP_CPP_SELEC=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_SELEC=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\sendqueue.cpp
DEP_CPP_SENDQ=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_SENDQ=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\srmm.cpp
DEP_CPP_SRMM_=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_SRMM_=\
	".\xtheme.h"\
	
# ADD CPP /Yc"commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\src\tabctrl.cpp
DEP_CPP_TABCT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_TABCT=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\templates.cpp
DEP_CPP_TEMPL=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_TEMPL=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\themes.cpp
DEP_CPP_THEME=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_THEME=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\trayicon.cpp
DEP_CPP_TRAYI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_TRAYI=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\TSButton.cpp
DEP_CPP_TSBUT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_TSBUT=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\typingnotify.cpp
DEP_CPP_TYPIN=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_TYPIN=\
	".\xtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\userprefs.cpp
DEP_CPP_USERP=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_toptoolbar.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\src\commonheaders.h"\
	".\src\functions.h"\
	".\src\generic_msghandlers.h"\
	".\src\globals.h"\
	".\src\mim.h"\
	".\src\msgdlgutils.h"\
	".\src\msgs.h"\
	".\src\nen.h"\
	".\src\sendqueue.h"\
	".\src\templates.h"\
	".\src\themes.h"\
	".\src\typingnotify.h"\
	
NODEP_CPP_USERP=\
	".\xtheme.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\src\functions.h
# End Source File
# Begin Source File

SOURCE=.\src\generic_msghandlers.h
# End Source File
# Begin Source File

SOURCE=.\src\globals.h
# End Source File
# Begin Source File

SOURCE=.\src\ImageDataObject.h
# End Source File
# Begin Source File

SOURCE=.\src\mim.h
# End Source File
# Begin Source File

SOURCE=.\src\msgdlgutils.h
# End Source File
# Begin Source File

SOURCE=.\src\msgs.h
# End Source File
# Begin Source File

SOURCE=.\src\nen.h
# End Source File
# Begin Source File

SOURCE=.\src\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\sendqueue.h
# End Source File
# Begin Source File

SOURCE=.\src\templates.h
# End Source File
# Begin Source File

SOURCE=.\src\themes.h
# End Source File
# Begin Source File

SOURCE=.\src\typingnotify.h
# End Source File
# Begin Source File

SOURCE=.\src\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\tabsrmm_private.rc
# End Source File
# End Group
# Begin Group "Misc Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\docs\changelog.txt
# End Source File
# Begin Source File

SOURCE=langpacks\langpack_tabsrmm_german.txt
# End Source File
# Begin Source File

SOURCE=.\Makefile.ansi
# End Source File
# Begin Source File

SOURCE=MAKEFILE.W32
# End Source File
# Begin Source File

SOURCE=.\docs\MetaContacts.TXT
# End Source File
# Begin Source File

SOURCE=.\docs\Popups.txt
# End Source File
# Begin Source File

SOURCE=.\docs\Readme.icons
# End Source File
# Begin Source File

SOURCE=.\docs\readme.txt
# End Source File
# End Group
# End Target
# End Project

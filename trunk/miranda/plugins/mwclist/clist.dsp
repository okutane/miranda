# Microsoft Developer Studio Project File - Name="clist" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=clist - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "clist.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "clist.mak" CFG="clist - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "clist - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "clist - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Miranda/miranda/plugins/mwclist", CLIAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "clist - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /machine:I386 /out:"../../bin/release/plugins/clist_mw.dll"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /YX"commonheaders.c" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /debug /machine:I386 /out:"../../bin/debug/plugins/clist_mw.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "clist - Win32 Release"
# Name "clist - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CLUIFrames"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CLUIFrames\cluiframes.h
# End Source File
# Begin Source File

SOURCE=.\CLUIFrames\extraimage.c
# End Source File
# Begin Source File

SOURCE=.\CLUIFrames\framesmenu.c
# End Source File
# Begin Source File

SOURCE=.\CLUIFrames\m_cluiframes.h
# End Source File
# Begin Source File

SOURCE=.\CLUIFrames\protocolorder.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\BkgrCfg.c
# End Source File
# Begin Source File

SOURCE=.\clc.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clcfiledrop.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clcidents.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clcitems.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clcmsgs.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clcopts.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clcpaint.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clcutils.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clistevents.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clistmenus.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clistmod.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clistopts.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clistsettings.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clisttray.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\clui.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CLUIFrames\cluiframes.c
# End Source File
# Begin Source File

SOURCE=.\cluiopts.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cluiservices.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\commonheaders.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yc"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\contact.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dblists.c
# End Source File
# Begin Source File

SOURCE=.\dblists.h
# End Source File
# Begin Source File

SOURCE=.\Docking.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\forkthread.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\genmenu.c
# End Source File
# Begin Source File

SOURCE=.\genmenuopt.c
# End Source File
# Begin Source File

SOURCE=.\groupmenu.c
# End Source File
# Begin Source File

SOURCE=.\groups.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\init.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\keyboard.c

!IF  "$(CFG)" == "clist - Win32 Release"

# ADD CPP /Yu"commonheaders.h"

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\movetogroup.c
# End Source File
# Begin Source File

SOURCE=.\CLUIFrames\statusbar.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BkgrCfg.h
# End Source File
# Begin Source File

SOURCE=.\clc.h
# End Source File
# Begin Source File

SOURCE=.\clist.h
# End Source File
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\forkthread.h
# End Source File
# Begin Source File

SOURCE=.\genmenu.h
# End Source File
# Begin Source File

SOURCE=.\genmenu_rc.h
# End Source File
# Begin Source File

SOURCE=.\IcoLib.h
# End Source File
# Begin Source File

SOURCE=.\m_clc.h
# End Source File
# Begin Source File

SOURCE=.\m_clist.h
# End Source File
# Begin Source File

SOURCE=.\m_clui.h
# End Source File
# Begin Source File

SOURCE=.\m_genmenu.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\statusmodes.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\res\&RQ.ico"
# End Source File
# Begin Source File

SOURCE=.\res\1.ico
# End Source File
# Begin Source File

SOURCE=.\res\2.ico
# End Source File
# Begin Source File

SOURCE=.\res\3.ico
# End Source File
# Begin Source File

SOURCE=.\res\4.ico
# End Source File
# Begin Source File

SOURCE=.\res\5.ico
# End Source File
# Begin Source File

SOURCE=.\res\6.ico
# End Source File
# Begin Source File

SOURCE=.\res\7.ico
# End Source File
# Begin Source File

SOURCE=.\res\8.ico
# End Source File
# Begin Source File

SOURCE=.\res\addcontact.ico
# End Source File
# Begin Source File

SOURCE=.\res\addgoupp.ico
# End Source File
# Begin Source File

SOURCE=.\res\away.ico
# End Source File
# Begin Source File

SOURCE=.\res\blank.ico
# End Source File
# Begin Source File

SOURCE=.\res\blank1.ico
# End Source File
# Begin Source File

SOURCE=.\res\changefont.ico
# End Source File
# Begin Source File

SOURCE=.\res\delete.ico
# End Source File
# Begin Source File

SOURCE=.\res\detailsl.ico
# End Source File
# Begin Source File

SOURCE=.\res\dnd.ico
# End Source File
# Begin Source File

SOURCE=.\res\downarrow.ico
# End Source File
# Begin Source File

SOURCE=.\res\dragcopy.cur
# End Source File
# Begin Source File

SOURCE=.\res\dropuser.cur
# End Source File
# Begin Source File

SOURCE=.\res\emptyblo.ico
# End Source File
# Begin Source File

SOURCE=.\res\file.ico
# End Source File
# Begin Source File

SOURCE=.\res\filledbl.ico
# End Source File
# Begin Source File

SOURCE=.\res\finduser.ico
# End Source File
# Begin Source File

SOURCE=.\res\freechat.ico
# End Source File
# Begin Source File

SOURCE=.\res\Gaim.ico
# End Source File
# Begin Source File

SOURCE=.\res\globus.ico
# End Source File
# Begin Source File

SOURCE=.\res\groupope.ico
# End Source File
# Begin Source File

SOURCE=.\res\groupshu.ico
# End Source File
# Begin Source File

SOURCE=.\res\help.ico
# End Source File
# Begin Source File

SOURCE=.\res\history.ico
# End Source File
# Begin Source File

SOURCE=.\res\hyperlin.cur
# End Source File
# Begin Source File

SOURCE=.\res\Icq2000.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2001.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2003.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2gof.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2goJ.ico
# End Source File
# Begin Source File

SOURCE=.\res\IcqL4.ico
# End Source File
# Begin Source File

SOURCE=.\res\IcqL5.ico
# End Source File
# Begin Source File

SOURCE=.\res\IcqLite.ico
# End Source File
# Begin Source File

SOURCE=.\res\IM2.ico
# End Source File
# Begin Source File

SOURCE=.\res\invisible.ico
# End Source File
# Begin Source File

SOURCE=.\res\Kopete.ico
# End Source File
# Begin Source File

SOURCE=.\res\Licq.ico
# End Source File
# Begin Source File

SOURCE=.\res\message.ico
# End Source File
# Begin Source File

SOURCE=.\res\miranda.ico
# End Source File
# Begin Source File

SOURCE=.\res\mirandaclassic.ico
# End Source File
# Begin Source File

SOURCE=.\res\mirandaw.ico
# End Source File
# Begin Source File

SOURCE=.\res\multisend.ico
# End Source File
# Begin Source File

SOURCE=.\res\na2.ico
# End Source File
# Begin Source File

SOURCE=.\res\notick.ico
# End Source File
# Begin Source File

SOURCE=.\res\notick1.ico
# End Source File
# Begin Source File

SOURCE=.\res\occupied.ico
# End Source File
# Begin Source File

SOURCE=.\res\offline2.ico
# End Source File
# Begin Source File

SOURCE=.\res\online2.ico
# End Source File
# Begin Source File

SOURCE=.\res\onthepho.ico
# End Source File
# Begin Source File

SOURCE=.\res\options.ico
# End Source File
# Begin Source File

SOURCE=.\res\outtolun.ico
# End Source File
# Begin Source File

SOURCE=.\res\QIP.ico
# End Source File
# Begin Source File

SOURCE=.\res\rename.ico
# End Source File
# Begin Source File

SOURCE=.\res\reply.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\res\searchal.ico
# End Source File
# Begin Source File

SOURCE=.\res\sendmail.ico
# End Source File
# Begin Source File

SOURCE=.\res\Sim.ico
# End Source File
# Begin Source File

SOURCE=.\res\smalldot.ico
# End Source File
# Begin Source File

SOURCE=.\res\sms.ico
# End Source File
# Begin Source File

SOURCE=.\res\sortcold.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sortcolu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\timestamp.ico
# End Source File
# Begin Source File

SOURCE=.\res\Trillian.ico
# End Source File
# Begin Source File

SOURCE=.\res\url.ico
# End Source File
# Begin Source File

SOURCE=.\res\useronli.ico
# End Source File
# Begin Source File

SOURCE=.\res\viewdetails.ico
# End Source File
# End Group
# End Target
# End Project

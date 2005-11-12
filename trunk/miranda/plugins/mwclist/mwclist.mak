# Microsoft Developer Studio Generated NMAKE File, Based on mwclist.dsp
!IF "$(CFG)" == ""
CFG=mwclist - Win32 Release
!MESSAGE No configuration specified. Defaulting to mwclist - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mwclist - Win32 Release" && "$(CFG)" != "mwclist - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mwclist.mak" CFG="mwclist - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mwclist - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mwclist - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "mwclist - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\clist_mw.dll"


CLEAN :
	-@erase "$(INTDIR)\BkgrCfg.obj"
	-@erase "$(INTDIR)\clc.obj"
	-@erase "$(INTDIR)\clcfiledrop.obj"
	-@erase "$(INTDIR)\clcidents.obj"
	-@erase "$(INTDIR)\clcitems.obj"
	-@erase "$(INTDIR)\clcmsgs.obj"
	-@erase "$(INTDIR)\clcopts.obj"
	-@erase "$(INTDIR)\clcpaint.obj"
	-@erase "$(INTDIR)\clcutils.obj"
	-@erase "$(INTDIR)\clistevents.obj"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistmod.obj"
	-@erase "$(INTDIR)\clistopts.obj"
	-@erase "$(INTDIR)\clistsettings.obj"
	-@erase "$(INTDIR)\clisttray.obj"
	-@erase "$(INTDIR)\clui.obj"
	-@erase "$(INTDIR)\cluiframes.obj"
	-@erase "$(INTDIR)\cluiopts.obj"
	-@erase "$(INTDIR)\cluiservices.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\dblists.obj"
	-@erase "$(INTDIR)\Docking.obj"
	-@erase "$(INTDIR)\extraimage.obj"
	-@erase "$(INTDIR)\forkthread.obj"
	-@erase "$(INTDIR)\framesmenu.obj"
	-@erase "$(INTDIR)\genmenu.obj"
	-@erase "$(INTDIR)\genmenuopt.obj"
	-@erase "$(INTDIR)\groupmenu.obj"
	-@erase "$(INTDIR)\groups.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\movetogroup.obj"
	-@erase "$(INTDIR)\mwclist.pch"
	-@erase "$(INTDIR)\protocolorder.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\statusbar.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\clist_mw.exp"
	-@erase "$(OUTDIR)\clist_mw.lib"
	-@erase "..\..\bin\release\plugins\clist_mw.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mwclist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /incremental:no /pdb:"$(OUTDIR)\clist_mw.pdb" /machine:I386 /out:"../../bin/release/plugins/clist_mw.dll" /implib:"$(OUTDIR)\clist_mw.lib" 
LINK32_OBJS= \
	"$(INTDIR)\cluiframes.obj" \
	"$(INTDIR)\extraimage.obj" \
	"$(INTDIR)\framesmenu.obj" \
	"$(INTDIR)\protocolorder.obj" \
	"$(INTDIR)\statusbar.obj" \
	"$(INTDIR)\BkgrCfg.obj" \
	"$(INTDIR)\clc.obj" \
	"$(INTDIR)\clcfiledrop.obj" \
	"$(INTDIR)\clcidents.obj" \
	"$(INTDIR)\clcitems.obj" \
	"$(INTDIR)\clcmsgs.obj" \
	"$(INTDIR)\clcopts.obj" \
	"$(INTDIR)\clcpaint.obj" \
	"$(INTDIR)\clcutils.obj" \
	"$(INTDIR)\clistevents.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistmod.obj" \
	"$(INTDIR)\clistopts.obj" \
	"$(INTDIR)\clistsettings.obj" \
	"$(INTDIR)\clisttray.obj" \
	"$(INTDIR)\clui.obj" \
	"$(INTDIR)\cluiopts.obj" \
	"$(INTDIR)\cluiservices.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\dblists.obj" \
	"$(INTDIR)\Docking.obj" \
	"$(INTDIR)\forkthread.obj" \
	"$(INTDIR)\genmenu.obj" \
	"$(INTDIR)\genmenuopt.obj" \
	"$(INTDIR)\groupmenu.obj" \
	"$(INTDIR)\groups.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\movetogroup.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\clist_mw.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\bin\debug\plugins\clist_mw.dll"


CLEAN :
	-@erase "$(INTDIR)\BkgrCfg.obj"
	-@erase "$(INTDIR)\clc.obj"
	-@erase "$(INTDIR)\clcfiledrop.obj"
	-@erase "$(INTDIR)\clcidents.obj"
	-@erase "$(INTDIR)\clcitems.obj"
	-@erase "$(INTDIR)\clcmsgs.obj"
	-@erase "$(INTDIR)\clcopts.obj"
	-@erase "$(INTDIR)\clcpaint.obj"
	-@erase "$(INTDIR)\clcutils.obj"
	-@erase "$(INTDIR)\clistevents.obj"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistmod.obj"
	-@erase "$(INTDIR)\clistopts.obj"
	-@erase "$(INTDIR)\clistsettings.obj"
	-@erase "$(INTDIR)\clisttray.obj"
	-@erase "$(INTDIR)\clui.obj"
	-@erase "$(INTDIR)\cluiframes.obj"
	-@erase "$(INTDIR)\cluiopts.obj"
	-@erase "$(INTDIR)\cluiservices.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\dblists.obj"
	-@erase "$(INTDIR)\Docking.obj"
	-@erase "$(INTDIR)\extraimage.obj"
	-@erase "$(INTDIR)\forkthread.obj"
	-@erase "$(INTDIR)\framesmenu.obj"
	-@erase "$(INTDIR)\genmenu.obj"
	-@erase "$(INTDIR)\genmenuopt.obj"
	-@erase "$(INTDIR)\groupmenu.obj"
	-@erase "$(INTDIR)\groups.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\movetogroup.obj"
	-@erase "$(INTDIR)\mwclist.pch"
	-@erase "$(INTDIR)\protocolorder.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\statusbar.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_mw.exp"
	-@erase "$(OUTDIR)\clist_mw.lib"
	-@erase "$(OUTDIR)\clist_mw.pdb"
	-@erase "..\..\bin\debug\plugins\clist_mw.dll"
	-@erase "..\..\bin\debug\plugins\clist_mw.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mwclist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /incremental:yes /pdb:"$(OUTDIR)\clist_mw.pdb" /debug /machine:I386 /out:"../../bin/debug/plugins/clist_mw.dll" /implib:"$(OUTDIR)\clist_mw.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cluiframes.obj" \
	"$(INTDIR)\extraimage.obj" \
	"$(INTDIR)\framesmenu.obj" \
	"$(INTDIR)\protocolorder.obj" \
	"$(INTDIR)\statusbar.obj" \
	"$(INTDIR)\BkgrCfg.obj" \
	"$(INTDIR)\clc.obj" \
	"$(INTDIR)\clcfiledrop.obj" \
	"$(INTDIR)\clcidents.obj" \
	"$(INTDIR)\clcitems.obj" \
	"$(INTDIR)\clcmsgs.obj" \
	"$(INTDIR)\clcopts.obj" \
	"$(INTDIR)\clcpaint.obj" \
	"$(INTDIR)\clcutils.obj" \
	"$(INTDIR)\clistevents.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistmod.obj" \
	"$(INTDIR)\clistopts.obj" \
	"$(INTDIR)\clistsettings.obj" \
	"$(INTDIR)\clisttray.obj" \
	"$(INTDIR)\clui.obj" \
	"$(INTDIR)\cluiopts.obj" \
	"$(INTDIR)\cluiservices.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\dblists.obj" \
	"$(INTDIR)\Docking.obj" \
	"$(INTDIR)\forkthread.obj" \
	"$(INTDIR)\genmenu.obj" \
	"$(INTDIR)\genmenuopt.obj" \
	"$(INTDIR)\groupmenu.obj" \
	"$(INTDIR)\groups.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\movetogroup.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\clist_mw.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("mwclist.dep")
!INCLUDE "mwclist.dep"
!ELSE 
!MESSAGE Warning: cannot find "mwclist.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "mwclist - Win32 Release" || "$(CFG)" == "mwclist - Win32 Debug"
SOURCE=.\CLUIFrames\cluiframes.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cluiframes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cluiframes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\CLUIFrames\extraimage.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\extraimage.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\extraimage.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\CLUIFrames\framesmenu.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\framesmenu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\framesmenu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\CLUIFrames\protocolorder.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\protocolorder.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\protocolorder.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\CLUIFrames\statusbar.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\statusbar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"../commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\statusbar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\BkgrCfg.c

"$(INTDIR)\BkgrCfg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"


SOURCE=.\clc.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcfiledrop.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcfiledrop.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcfiledrop.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcidents.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcidents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcidents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcitems.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcitems.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcitems.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcmsgs.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcmsgs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcmsgs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcopts.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcpaint.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcpaint.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcpaint.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcutils.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clistevents.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistevents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistevents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clistmenus.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistmenus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistmenus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clistmod.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistmod.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistmod.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clistopts.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clistsettings.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistsettings.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistsettings.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clisttray.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clisttray.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clisttray.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clui.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clui.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clui.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\cluiopts.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cluiopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cluiopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\cluiservices.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cluiservices.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cluiservices.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\commonheaders.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\mwclist.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\mwclist.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\contact.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\contact.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\contact.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\dblists.c

"$(INTDIR)\dblists.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"


SOURCE=.\Docking.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Docking.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Docking.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\forkthread.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\forkthread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\forkthread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\genmenu.c

"$(INTDIR)\genmenu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"


SOURCE=.\genmenuopt.c

"$(INTDIR)\genmenuopt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"


SOURCE=.\groupmenu.c

"$(INTDIR)\groupmenu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"


SOURCE=.\groups.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\groups.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\groups.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\init.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\keyboard.c

!IF  "$(CFG)" == "mwclist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\keyboard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mwclist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "mwclist_EXPORTS" /Fp"$(INTDIR)\mwclist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\keyboard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\movetogroup.c

"$(INTDIR)\movetogroup.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mwclist.pch"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 


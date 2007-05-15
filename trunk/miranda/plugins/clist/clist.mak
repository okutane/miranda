# Microsoft Developer Studio Generated NMAKE File, Based on clist.dsp
!IF "$(CFG)" == ""
CFG=clist - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to clist - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "clist - Win32 Release" && "$(CFG)" != "clist - Win32 Debug" && "$(CFG)" != "clist - Win32 Release Unicode" && "$(CFG)" != "clist - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "clist.mak" CFG="clist - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "clist - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "clist - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "clist - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "clist - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "clist - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\clist_classic.dll"


CLEAN :
	-@erase "$(INTDIR)\clcopts.obj"
	-@erase "$(INTDIR)\clcpaint.obj"
	-@erase "$(INTDIR)\clist.pch"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistopts.obj"
	-@erase "$(INTDIR)\cluiopts.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\forkthread.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_classic.exp"
	-@erase "$(OUTDIR)\clist_classic.map"
	-@erase "$(OUTDIR)\clist_classic.pdb"
	-@erase "..\..\bin\release\plugins\clist_classic.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include/" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /incremental:no /pdb:"$(OUTDIR)\clist_classic.pdb" /map:"$(INTDIR)\clist_classic.map" /debug /machine:I386 /out:"../../bin/release/plugins/clist_classic.dll" /implib:"$(OUTDIR)\clist_classic.lib" 
LINK32_OBJS= \
	"$(INTDIR)\clcopts.obj" \
	"$(INTDIR)\clcpaint.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistopts.obj" \
	"$(INTDIR)\cluiopts.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\forkthread.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\clist_classic.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\bin\debug\plugins\clist_classic.dll"


CLEAN :
	-@erase "$(INTDIR)\clcopts.obj"
	-@erase "$(INTDIR)\clcpaint.obj"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistopts.obj"
	-@erase "$(INTDIR)\cluiopts.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\forkthread.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_classic.exp"
	-@erase "$(OUTDIR)\clist_classic.pdb"
	-@erase "..\..\bin\debug\plugins\clist_classic.dll"
	-@erase "..\..\bin\debug\plugins\clist_classic.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include/" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /incremental:yes /pdb:"$(OUTDIR)\clist_classic.pdb" /debug /machine:I386 /out:"../../bin/debug/plugins/clist_classic.dll" /implib:"$(OUTDIR)\clist_classic.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clcopts.obj" \
	"$(INTDIR)\clcpaint.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistopts.obj" \
	"$(INTDIR)\cluiopts.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\forkthread.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\clist_classic.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\plugins\clist_classic.dll"


CLEAN :
	-@erase "$(INTDIR)\clcopts.obj"
	-@erase "$(INTDIR)\clcpaint.obj"
	-@erase "$(INTDIR)\clist.pch"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistopts.obj"
	-@erase "$(INTDIR)\cluiopts.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\forkthread.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_classic.exp"
	-@erase "$(OUTDIR)\clist_classic.map"
	-@erase "$(OUTDIR)\clist_classic.pdb"
	-@erase "..\..\bin\Release Unicode\plugins\clist_classic.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include/" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /incremental:no /pdb:"$(OUTDIR)\clist_classic.pdb" /map:"$(INTDIR)\clist_classic.map" /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/clist_classic.dll" /implib:"$(OUTDIR)\clist_classic.lib" 
LINK32_OBJS= \
	"$(INTDIR)\clcopts.obj" \
	"$(INTDIR)\clcpaint.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistopts.obj" \
	"$(INTDIR)\cluiopts.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\forkthread.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Release Unicode\plugins\clist_classic.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode

ALL : "..\..\bin\Debug Unicode\plugins\clist_classic.dll"


CLEAN :
	-@erase "$(INTDIR)\clcopts.obj"
	-@erase "$(INTDIR)\clcpaint.obj"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistopts.obj"
	-@erase "$(INTDIR)\cluiopts.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\forkthread.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_classic.exp"
	-@erase "$(OUTDIR)\clist_classic.pdb"
	-@erase "..\..\bin\Debug Unicode\plugins\clist_classic.dll"
	-@erase "..\..\bin\Debug Unicode\plugins\clist_classic.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include/" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /incremental:yes /pdb:"$(OUTDIR)\clist_classic.pdb" /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/clist_classic.dll" /implib:"$(OUTDIR)\clist_classic.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clcopts.obj" \
	"$(INTDIR)\clcpaint.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistopts.obj" \
	"$(INTDIR)\cluiopts.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\forkthread.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Debug Unicode\plugins\clist_classic.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("clist.dep")
!INCLUDE "clist.dep"
!ELSE 
!MESSAGE Warning: cannot find "clist.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "clist - Win32 Release" || "$(CFG)" == "clist - Win32 Debug" || "$(CFG)" == "clist - Win32 Release Unicode" || "$(CFG)" == "clist - Win32 Debug Unicode"
SOURCE=.\clcopts.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcopts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcopts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clcpaint.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcpaint.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcpaint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clcpaint.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clcpaint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clistmenus.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistmenus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistmenus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistmenus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistmenus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\clistopts.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistopts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\clistopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\clistopts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\cluiopts.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cluiopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cluiopts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cluiopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cluiopts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\commonheaders.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\clist.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\clist.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\forkthread.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\forkthread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\forkthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\forkthread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\forkthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\init.c

!IF  "$(CFG)" == "clist - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include/" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\clist.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "clist - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIST_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\clist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 


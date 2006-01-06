# Microsoft Developer Studio Generated NMAKE File, Based on dbtool.dsp
!IF "$(CFG)" == ""
CFG=dbtool - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dbtool - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dbtool - Win32 Release" && "$(CFG)" != "dbtool - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbtool.mak" CFG="dbtool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbtool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "dbtool - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "dbtool - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\miranda\bin\release\dbtool.exe"


CLEAN :
	-@erase "$(INTDIR)\aggressive.obj"
	-@erase "$(INTDIR)\cleaning.obj"
	-@erase "$(INTDIR)\contactchain.obj"
	-@erase "$(INTDIR)\disk.obj"
	-@erase "$(INTDIR)\eventchain.obj"
	-@erase "$(INTDIR)\fileaccess.obj"
	-@erase "$(INTDIR)\finaltasks.obj"
	-@erase "$(INTDIR)\finished.obj"
	-@erase "$(INTDIR)\initialchecks.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\modulechain.obj"
	-@erase "$(INTDIR)\openerror.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\selectdb.obj"
	-@erase "$(INTDIR)\settingschain.obj"
	-@erase "$(INTDIR)\user.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\welcome.obj"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\worker.obj"
	-@erase "$(OUTDIR)\dbtool.map"
	-@erase "$(OUTDIR)\dbtool.pdb"
	-@erase "..\..\miranda\bin\release\dbtool.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbtool.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dbtool.pdb" /map:"$(INTDIR)\dbtool.map" /debug /machine:I386 /out:"../../miranda/bin/release/dbtool.exe" /ALIGN:4096 /ALIGN:4096 
LINK32_OBJS= \
	"$(INTDIR)\aggressive.obj" \
	"$(INTDIR)\contactchain.obj" \
	"$(INTDIR)\eventchain.obj" \
	"$(INTDIR)\finaltasks.obj" \
	"$(INTDIR)\initialchecks.obj" \
	"$(INTDIR)\modulechain.obj" \
	"$(INTDIR)\settingschain.obj" \
	"$(INTDIR)\user.obj" \
	"$(INTDIR)\cleaning.obj" \
	"$(INTDIR)\fileaccess.obj" \
	"$(INTDIR)\finished.obj" \
	"$(INTDIR)\openerror.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\selectdb.obj" \
	"$(INTDIR)\welcome.obj" \
	"$(INTDIR)\disk.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\worker.obj" \
	"$(INTDIR)\resource.res"

"..\..\miranda\bin\release\dbtool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dbtool - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\miranda\bin\debug\dbtool.exe"


CLEAN :
	-@erase "$(INTDIR)\aggressive.obj"
	-@erase "$(INTDIR)\cleaning.obj"
	-@erase "$(INTDIR)\contactchain.obj"
	-@erase "$(INTDIR)\disk.obj"
	-@erase "$(INTDIR)\eventchain.obj"
	-@erase "$(INTDIR)\fileaccess.obj"
	-@erase "$(INTDIR)\finaltasks.obj"
	-@erase "$(INTDIR)\finished.obj"
	-@erase "$(INTDIR)\initialchecks.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\modulechain.obj"
	-@erase "$(INTDIR)\openerror.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\selectdb.obj"
	-@erase "$(INTDIR)\settingschain.obj"
	-@erase "$(INTDIR)\user.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\welcome.obj"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\worker.obj"
	-@erase "$(OUTDIR)\dbtool.pdb"
	-@erase "..\..\miranda\bin\debug\dbtool.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbtool.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dbtool.pdb" /debug /machine:I386 /out:"../../miranda/bin/debug/dbtool.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\aggressive.obj" \
	"$(INTDIR)\contactchain.obj" \
	"$(INTDIR)\eventchain.obj" \
	"$(INTDIR)\finaltasks.obj" \
	"$(INTDIR)\initialchecks.obj" \
	"$(INTDIR)\modulechain.obj" \
	"$(INTDIR)\settingschain.obj" \
	"$(INTDIR)\user.obj" \
	"$(INTDIR)\cleaning.obj" \
	"$(INTDIR)\fileaccess.obj" \
	"$(INTDIR)\finished.obj" \
	"$(INTDIR)\openerror.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\selectdb.obj" \
	"$(INTDIR)\welcome.obj" \
	"$(INTDIR)\disk.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\worker.obj" \
	"$(INTDIR)\resource.res"

"..\..\miranda\bin\debug\dbtool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("dbtool.dep")
!INCLUDE "dbtool.dep"
!ELSE 
!MESSAGE Warning: cannot find "dbtool.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dbtool - Win32 Release" || "$(CFG)" == "dbtool - Win32 Debug"
SOURCE=.\aggressive.cpp

"$(INTDIR)\aggressive.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\contactchain.cpp

"$(INTDIR)\contactchain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\eventchain.cpp

"$(INTDIR)\eventchain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\finaltasks.cpp

"$(INTDIR)\finaltasks.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\initialchecks.cpp

"$(INTDIR)\initialchecks.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\modulechain.cpp

"$(INTDIR)\modulechain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\settingschain.cpp

"$(INTDIR)\settingschain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\user.cpp

"$(INTDIR)\user.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cleaning.cpp

"$(INTDIR)\cleaning.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fileaccess.cpp

"$(INTDIR)\fileaccess.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\finished.cpp

"$(INTDIR)\finished.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\openerror.cpp

"$(INTDIR)\openerror.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\progress.cpp

"$(INTDIR)\progress.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\selectdb.cpp

"$(INTDIR)\selectdb.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\welcome.cpp

"$(INTDIR)\welcome.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\disk.cpp

"$(INTDIR)\disk.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\wizard.cpp

"$(INTDIR)\wizard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\worker.cpp

"$(INTDIR)\worker.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 


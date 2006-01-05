# Microsoft Developer Studio Generated NMAKE File, Based on Yahoo.dsp
!IF "$(CFG)" == ""
CFG=Yahoo - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Yahoo - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Yahoo - Win32 Release" && "$(CFG)" != "Yahoo - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Yahoo.mak" CFG="Yahoo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Yahoo - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Yahoo - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "Yahoo - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\Bin\Release\Plugins\Yahoo.dll"


CLEAN :
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\libyahoo2.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\utf8.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yahoo.obj"
	-@erase "$(INTDIR)\Yahoo.res"
	-@erase "$(INTDIR)\yahoo_fn.obj"
	-@erase "$(INTDIR)\yahoo_httplib.obj"
	-@erase "$(INTDIR)\yahoo_list.obj"
	-@erase "$(INTDIR)\yahoo_util.obj"
	-@erase "$(OUTDIR)\Yahoo.exp"
	-@erase "$(OUTDIR)\Yahoo.lib"
	-@erase "$(OUTDIR)\Yahoo.map"
	-@erase "$(OUTDIR)\Yahoo.pdb"
	-@erase "..\..\Bin\Release\Plugins\Yahoo.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\Yahoo.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Yahoo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Yahoo.pdb" /map:"$(INTDIR)\Yahoo.map" /debug /machine:I386 /out:"../../Bin/Release/Plugins/Yahoo.dll" /implib:"$(OUTDIR)\Yahoo.lib" 
LINK32_OBJS= \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\libyahoo2.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\yahoo_fn.obj" \
	"$(INTDIR)\yahoo_httplib.obj" \
	"$(INTDIR)\yahoo_list.obj" \
	"$(INTDIR)\yahoo_util.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\utf8.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\yahoo.obj" \
	"$(INTDIR)\Yahoo.res"

"..\..\Bin\Release\Plugins\Yahoo.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\Bin\Debug\Plugins\Yahoo.dll"


CLEAN :
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\libyahoo2.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\utf8.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yahoo.obj"
	-@erase "$(INTDIR)\Yahoo.res"
	-@erase "$(INTDIR)\yahoo_fn.obj"
	-@erase "$(INTDIR)\yahoo_httplib.obj"
	-@erase "$(INTDIR)\yahoo_list.obj"
	-@erase "$(INTDIR)\yahoo_util.obj"
	-@erase "$(OUTDIR)\Yahoo.exp"
	-@erase "$(OUTDIR)\Yahoo.lib"
	-@erase "$(OUTDIR)\Yahoo.pdb"
	-@erase "..\..\Bin\Debug\Plugins\Yahoo.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\Yahoo.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Yahoo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Yahoo.pdb" /debug /machine:I386 /out:"../../Bin/Debug/Plugins/Yahoo.dll" /implib:"$(OUTDIR)\Yahoo.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\libyahoo2.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\yahoo_fn.obj" \
	"$(INTDIR)\yahoo_httplib.obj" \
	"$(INTDIR)\yahoo_list.obj" \
	"$(INTDIR)\yahoo_util.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\utf8.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\yahoo.obj" \
	"$(INTDIR)\Yahoo.res"

"..\..\Bin\Debug\Plugins\Yahoo.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("Yahoo.dep")
!INCLUDE "Yahoo.dep"
!ELSE 
!MESSAGE Warning: cannot find "Yahoo.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Yahoo - Win32 Release" || "$(CFG)" == "Yahoo - Win32 Debug"
SOURCE=.\libyahoo2\crypt.c

"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\libyahoo2\libyahoo2.c

"$(INTDIR)\libyahoo2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\libyahoo2\md5.c

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\libyahoo2\sha.c

"$(INTDIR)\sha.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\libyahoo2\yahoo_fn.c

"$(INTDIR)\yahoo_fn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\libyahoo2\yahoo_httplib.c

"$(INTDIR)\yahoo_httplib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\libyahoo2\yahoo_list.c

"$(INTDIR)\yahoo_list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\libyahoo2\yahoo_util.c

"$(INTDIR)\yahoo_util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\options.c

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pthread.c

"$(INTDIR)\pthread.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\server.c

"$(INTDIR)\server.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\services.c

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\utf8.c

"$(INTDIR)\utf8.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\util.c

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\yahoo.c

"$(INTDIR)\yahoo.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Yahoo.rc

"$(INTDIR)\Yahoo.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 


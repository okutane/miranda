# Microsoft Developer Studio Project File - Name="FreeImage" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=FreeImage - Win32 Debug
!MESSAGE Dies ist kein g�ltiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und f�hren Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "FreeImage.mak".
!MESSAGE 
!MESSAGE Sie k�nnen beim Ausf�hren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "FreeImage.mak" CFG="FreeImage - Win32 Debug"
!MESSAGE 
!MESSAGE F�r die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "FreeImage - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FreeImage - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FreeImage - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FreeImage - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "Source" /I "Source\ZLib" /I "Source\DeprecationManager" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ..\zlib\Release\zlib.lib msimg32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x5130000" /dll /machine:I386 /out:"..\..\bin\Release\Plugins\advaimg.dll" /opt:NOWIN98
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\FreeImage.dll Dist	copy Release\FreeImage.lib Dist	copy Source\FreeImage.h Dist
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "Source" /I "Source\ZLib" /I "Source\DeprecationManager" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Debug/FreeImaged.dll" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\FreeImaged.dll Dist	copy Debug\FreeImaged.lib Dist	copy Source\FreeImage.h Dist
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FreeImage - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "FreeImage___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "FreeImage___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "FreeImage___Win32_Release_Unicode"
# PROP Intermediate_Dir "FreeImage___Win32_Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /I "Source" /I "Source\ZLib" /I "Source\DeprecationManager" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O1 /I "Source" /I "Source\ZLib" /I "Source\DeprecationManager" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_UNICODE" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 msimg32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x5130000" /dll /machine:I386 /opt:NOWIN98
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ..\zlib\Release\zlib.lib msimg32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x5130000" /dll /machine:I386 /out:"..\..\bin\Release Unicode\Plugins\advaimg.dll" /opt:NOWIN98
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "FreeImage - Win32 Release"
# Name "FreeImage - Win32 Debug"
# Name "FreeImage - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\FreeImage\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\PluginBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\PluginCUT.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\PluginGIF.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\PluginICO.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\PluginJPEG.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\PluginPNG.cpp
# End Source File
# End Group
# Begin Group "Conversion"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\FreeImage\Conversion.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\Conversion16_555.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\Conversion16_565.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\Conversion24.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\Conversion32.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\Conversion4.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\Conversion8.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\ConversionRGBF.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\ConversionType.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\Halftoning.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\tmoColorConvert.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\tmoDrago03.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\tmoReinhard05.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\ToneMapping.cpp
# End Source File
# End Group
# Begin Group "Quantizers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\FreeImage\NNQuantizer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\WuQuantizer.cpp
# End Source File
# End Group
# Begin Group "DeprecationMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\DeprecationManager\DeprecationMgr.cpp
# End Source File
# End Group
# Begin Group "MultiPaging"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\FreeImage\CacheFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\MultiPage.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\ZLibInterface.cpp
# End Source File
# End Group
# Begin Group "Metadata"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Metadata\Exif.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Metadata\FIRational.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Metadata\FreeImageTag.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Metadata\IPTC.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Metadata\TagConversion.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Metadata\TagLib.cpp
# End Source File
# End Group
# Begin Group "Miranda"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Miranda\main.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\FreeImage\BitmapAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\ColorLookup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\FreeImage.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeImage.rc
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\FreeImageC.c
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\FreeImageIO.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\GetType.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\MemoryIO.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImage\PixelAccess.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Source\CacheFile.h
# End Source File
# Begin Source File

SOURCE=.\Source\DeprecationManager\DeprecationMgr.h
# End Source File
# Begin Source File

SOURCE=.\Source\Metadata\FIRational.h
# End Source File
# Begin Source File

SOURCE=.\SOURCE\FreeImage.h
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageIO.h
# End Source File
# Begin Source File

SOURCE=.\Source\Metadata\FreeImageTag.h
# End Source File
# Begin Source File

SOURCE=.\Miranda\include\m_freeimage.h
# End Source File
# Begin Source File

SOURCE=.\Miranda\include\m_imgsrvc.h
# End Source File
# Begin Source File

SOURCE=.\Source\Plugin.h
# End Source File
# Begin Source File

SOURCE=.\Source\Quantizers.h
# End Source File
# Begin Source File

SOURCE=.\Source\ToneMapping.h
# End Source File
# Begin Source File

SOURCE=.\Source\Utilities.h
# End Source File
# End Group
# Begin Group "Toolkit Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\BSplineRotate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\Channels.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\ClassicRotate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\Colors.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\CopyPaste.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\Flip.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\JPEGTransform.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\Rescale.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\Resize.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FreeImageToolkit\Resize.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Todo.txt
# End Source File
# Begin Source File

SOURCE=.\Whatsnew.txt
# End Source File
# End Target
# End Project

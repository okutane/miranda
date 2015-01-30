set Version=0.10.29

set PDBVer=miranda-im-v%Version%-ansi-pdb
set FileVer=miranda-im-v%Version%-ansi
set ContribVer=miranda-im-v%Version%-ansi-contrib
rd /Q /S %Temp%\pdba >nul
md %Temp%\pdba
md %Temp%\pdba\plugins

copy ..\bin9\Release\miranda32.pdb                  %Temp%\pdba
copy ..\bin9\Release\dbtool.pdb	                  	%Temp%\pdba
copy ..\bin9\Release\zlib.pdb                  		%Temp%\pdba
rem  Protocols
copy ..\bin9\Release\Plugins\Aim.pdb          		%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\ICQ.pdb         		%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\IRC.pdb              	%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\jabber.pdb        		%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\MSN.pdb               	%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\Yahoo.pdb         	  	%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\GG.pdb          		%Temp%\pdba\plugins
rem  Plugins
copy ..\bin9\Release\Plugins\avs.pdb                %Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\chat.pdb               %Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\clist_classic.pdb     	%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\dbx_3x.pdb             %Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\import.pdb           	%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\srmm.pdb               %Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\clist_nicer.pdb 		%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\clist_modern.pdb    	%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\clist_mw.pdb        	%Temp%\pdba\plugins
copy ..\bin9\Release\Plugins\scriver.pdb         	%Temp%\pdba\plugins

del /Q /F "%PDBVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%PDBVer%.7z" %Temp%\pdba/*
rd /Q /S %Temp%\pdba

rd /Q /S %Temp%\miransi >nul
md %Temp%\miransi
md %Temp%\miransi\Plugins
md %Temp%\miransi\Icons
md %Temp%\miransi\Contrib

copy ..\bin9\Release\miranda32.exe              %Temp%\miransi
copy ..\bin9\Release\dbtool.exe                 %Temp%\miransi
rem  Protocols
copy ..\bin9\Release\plugins\aim.dll            %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\ICQ.dll            %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\IRC.dll            %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\jabber.dll         %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\MSN.dll            %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\Yahoo.dll          %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\GG.dll             %Temp%\miransi\Plugins
rem  Plugins
copy ..\bin9\Release\plugins\avs.dll            %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\chat.dll           %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\clist_classic.dll  %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\dbx_3x.dll         %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\advaimg.dll        %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\import.dll         %Temp%\miransi\Plugins
copy ..\bin9\Release\plugins\srmm.dll           %Temp%\miransi\Plugins
rem misc
copy ..\bin9\Release\zlib.dll                             %Temp%\miransi
copy ..\docs\Readme.txt                           %Temp%\miransi
copy ..\docs\Changelog.txt                           %Temp%\miransi
copy ..\docs\License.txt                          %Temp%\miransi
copy ..\docs\Contributors.txt                     %Temp%\miransi
copy ..\docs\mirandaboot.ini                     %Temp%\miransi\mirandaboot-example.ini

copy "..\bin9\Release\Icons\xstatus_ICQ.dll"                                    %Temp%\miransi\Icons
copy "..\bin9\Release\Icons\xstatus_jabber.dll"                                 %Temp%\miransi\Icons
copy ..\..\miranda-tools\installer\icons\bin\locolor\proto_AIM.dll      %Temp%\miransi\Icons
copy ..\..\miranda-tools\installer\icons\bin\locolor\proto_GG.dll       %Temp%\miransi\Icons
copy ..\..\miranda-tools\installer\icons\bin\locolor\proto_ICQ.dll      %Temp%\miransi\Icons
copy ..\..\miranda-tools\installer\icons\bin\locolor\proto_IRC.dll      %Temp%\miransi\Icons
copy ..\..\miranda-tools\installer\icons\bin\locolor\proto_JABBER.dll   %Temp%\miransi\Icons
copy ..\..\miranda-tools\installer\icons\bin\locolor\proto_MSN.dll      %Temp%\miransi\Icons
copy ..\..\miranda-tools\installer\icons\bin\locolor\proto_YAHOO.dll    %Temp%\miransi\Icons

rem vc
copy ..\bin9\contrib\vcredist_x86.exe %Temp%\miransi\Contrib

del /Q /F "%FileVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%FileVer%.7z" %Temp%\miransi/*
rd /Q /S %Temp%\miransi

rd /Q /S %Temp%\miransic >nul
md %Temp%\miransic
md %Temp%\miransic\Plugins
md %Temp%\miransic\Icons

copy ..\bin9\Release\Icons\tabsrmm_icons.dll             %Temp%\miransic\Icons
copy ..\bin9\Release\Icons\toolbar_icons.dll             %Temp%\miransic\Icons
copy ..\bin9\Release\plugins\clist_modern.dll            %Temp%\miransic\Plugins
copy ..\bin9\Release\plugins\clist_mw.dll                %Temp%\miransic\Plugins
copy ..\bin9\Release\plugins\clist_nicer.dll             %Temp%\miransic\Plugins
copy ..\bin9\Release\plugins\scriver.dll                 %Temp%\miransic\Plugins
copy ..\bin9\Release\plugins\tabsrmm.dll                 %Temp%\miransic\Plugins

del /Q /F "%ContribVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%ContribVer%.7z" %Temp%\miransic/*
rd /Q /S %Temp%\miransic

set PDBVer=miranda-im-v%Version%-unicode-pdb
set FileVer=miranda-im-v%Version%-unicode
set ContribVer=miranda-im-v%Version%-unicode-contrib
rd /Q /S %Temp%\pdbw >nul
md %Temp%\pdbw
md %Temp%\pdbw\plugins

copy "..\bin9\Release Unicode\miranda32.pdb"                   %Temp%\pdbw
copy "..\bin9\Release Unicode\dbtool.pdb"                   %Temp%\pdbw
copy "..\bin9\Release Unicode\zlib.pdb"                   %Temp%\pdbw
rem  Protocols
copy "..\bin9\Release Unicode\Plugins\Aim.pdb"                  %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\ICQ.pdb"                 %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\IRC.pdb"              %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\jabber.pdb"        %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\MSN.pdb"               %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\Yahoo.pdb"                   %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\GG.pdb"                  %Temp%\pdbw\plugins
rem  Unicode plugins
copy "..\bin9\Release Unicode\Plugins\avs.pdb"                 %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\chat.pdb"               %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\clist_classic.pdb"     %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\dbx_mmap.pdb"              %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\srmm.pdb"               %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\import.pdb"           %Temp%\pdbw\plugins

copy "..\bin9\Release Unicode\Plugins\clist_nicer.pdb" %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\clist_modern.pdb"    %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\clist_mw.pdb"        %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\scriver.pdb"         %Temp%\pdbw\plugins
copy "..\bin9\Release Unicode\Plugins\tabsrmm.pdb"         %Temp%\pdbw\plugins

del /Q /F "%PDBVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%PDBVer%.7z" %Temp%\pdbw/*
rd /Q /S %Temp%\pdbw

rd /Q /S %Temp%\miransiw >nul
md %Temp%\miransiw
md %Temp%\miransiw\Plugins
md %Temp%\miransiw\Icons
md %Temp%\miransiw\Contrib

copy "..\bin9\Release Unicode\miranda32.exe"              %Temp%\miransiw
copy "..\bin9\Release Unicode\dbtool.exe"                 %Temp%\miransiw
rem  Protocols
copy "..\bin9\Release Unicode\plugins\aim.dll"                    %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\ICQ.dll"                    %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\IRC.dll"            %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\jabber.dll"         %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\MSN.dll"            %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\Yahoo.dll"                  %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\GG.dll"                     %Temp%\miransiw\Plugins
rem  Plugins
copy "..\bin9\Release Unicode\plugins\avs.dll"            %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\chat.dll"           %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\clist_classic.dll"  %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\dbx_mmap.dll"               %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\advaimg.dll"                %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\import.dll"         %Temp%\miransiw\Plugins
copy "..\bin9\Release Unicode\plugins\srmm.dll"           %Temp%\miransiw\Plugins
rem misc
copy "..\bin9\Release Unicode\zlib.dll"                   %Temp%\miransiw
copy ..\docs\Readme.txt                           %Temp%\miransiw
copy ..\docs\Changelog.txt                           %Temp%\miransiw
copy ..\docs\License.txt                          %Temp%\miransiw
copy ..\docs\Contributors.txt                     %Temp%\miransiw
copy ..\docs\mirandaboot.ini                     %Temp%\miransiw\mirandaboot-example.ini

copy "..\bin9\Release\Icons\xstatus_ICQ.dll"                                    %Temp%\miransiw\Icons
copy "..\bin9\Release\Icons\xstatus_jabber.dll"                                 %Temp%\miransiw\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_AIM.dll      %Temp%\miransiw\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_GG.dll       %Temp%\miransiw\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_ICQ.dll      %Temp%\miransiw\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_IRC.dll      %Temp%\miransiw\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_JABBER.dll   %Temp%\miransiw\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_MSN.dll      %Temp%\miransiw\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_YAHOO.dll    %Temp%\miransiw\Icons

rem vc
copy ..\bin9\contrib\vcredist_x86.exe %Temp%\miransiw\Contrib

del /Q /F "%FileVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%FileVer%.7z" %Temp%\miransiw/*
rd /Q /S %Temp%\miransiw

rd /Q /S %Temp%\miransicw >nul
md %Temp%\miransicw
md %Temp%\miransicw\Plugins
md %Temp%\miransicw\Icons

copy "..\bin9\Release\Icons\tabsrmm_icons.dll"             %Temp%\miransicw\Icons
copy "..\bin9\Release\Icons\toolbar_icons.dll"                       %Temp%\miransicw\Icons
copy "..\bin9\Release Unicode\plugins\clist_modern.dll"            %Temp%\miransicw\Plugins
copy "..\bin9\Release Unicode\plugins\clist_mw.dll"                %Temp%\miransicw\Plugins
copy "..\bin9\Release Unicode\plugins\clist_nicer.dll"             %Temp%\miransicw\Plugins
copy "..\bin9\Release Unicode\plugins\scriver.dll"                 %Temp%\miransicw\Plugins
copy "..\bin9\Release Unicode\plugins\tabsrmm.dll"                 %Temp%\miransicw\Plugins

del /Q /F "%ContribVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%ContribVer%.7z" %Temp%\miransicw/*
rd /Q /S %Temp%\miransicw

set PDBVer=miranda-im-v%Version%-x64-pdb
set FileVer=miranda-im-v%Version%-x64
set ContribVer=miranda-im-v%Version%-x64-contrib
rd /Q /S %Temp%\pdbx >nul
md %Temp%\pdbx
md %Temp%\pdbx\plugins

copy "..\bin9\Release Unicode64\miranda64.pdb"                   %Temp%\pdbx
rem  Protocols
copy "..\bin9\Release Unicode64\Plugins\Aim.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\ICQ.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\IRC.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\jabber.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\MSN.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\Yahoo.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\GG.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\avs.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\chat.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\clist_classic.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\dbx_mmap.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\srmm.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\import.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\clist_nicer.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\clist_modern.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\clist_mw.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\scriver.pdb"                   %Temp%\pdbx\plugins
copy "..\bin9\Release Unicode64\Plugins\tabsrmm.pdb"                   %Temp%\pdbx\plugins

del /Q /F "%PDBVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%PDBVer%.7z" %Temp%\pdbx/*
rd /Q /S %Temp%\pdbx

rd /Q /S %Temp%\mirx64 >nul
md %Temp%\mirx64
md %Temp%\mirx64\Plugins
md %Temp%\mirx64\Icons
md %Temp%\mirx64\Contrib

copy "..\bin9\Release Unicode64\miranda64.exe"              %Temp%\mirx64
copy "..\bin9\Release Unicode64\dbtool.exe"              %Temp%\mirx64
rem  Protocols
copy "..\bin9\Release Unicode64\Plugins\aim.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\ICQ.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\IRC.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\jabber.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\MSN.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\Yahoo.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\GG.dll"              %Temp%\mirx64\Plugins
rem  Plugins
copy "..\bin9\Release Unicode64\Plugins\avs.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\chat.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\clist_classic.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\dbx_mmap.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\advaimg.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\import.dll"              %Temp%\mirx64\Plugins
copy "..\bin9\Release Unicode64\Plugins\srmm.dll"              %Temp%\mirx64\Plugins
rem misc
copy "..\bin9\Release Unicode64\zlib.dll"              %Temp%\mirx64
copy ..\docs\Readme.txt                           %Temp%\mirx64
copy ..\docs\Changelog.txt                           %Temp%\mirx64
copy ..\docs\License.txt                          %Temp%\mirx64
copy ..\docs\Contributors.txt                     %Temp%\mirx64
copy ..\docs\mirandaboot.ini                     %Temp%\mirx64\mirandaboot-example.ini

copy "..\bin9\Release\Icons\xstatus_ICQ.dll"                                    %Temp%\mirx64\Icons
copy "..\bin9\Release\Icons\xstatus_jabber.dll"                                 %Temp%\mirx64\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_AIM.dll      %Temp%\mirx64\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_GG.dll       %Temp%\mirx64\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_ICQ.dll      %Temp%\mirx64\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_IRC.dll      %Temp%\mirx64\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_JABBER.dll   %Temp%\mirx64\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_MSN.dll      %Temp%\mirx64\Icons
copy ..\..\miranda-tools\installer\icons\bin\hicolor\proto_YAHOO.dll    %Temp%\mirx64\Icons

rem vc
copy ..\bin9\contrib\vcredist_x64.exe %Temp%\mirx64\Contrib

del /Q /F "%FileVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%FileVer%.7z" %Temp%\mirx64/*
rd /Q /S %Temp%\mirx64

rd /Q /S %Temp%\mirx64c >nul
md %Temp%\mirx64c
md %Temp%\mirx64c\Plugins
md %Temp%\mirx64c\Icons

copy "..\bin9\Release\Icons\tabsrmm_icons.dll"                       %Temp%\mirx64c\Icons
copy "..\bin9\Release\Icons\toolbar_icons.dll"                       %Temp%\mirx64c\Icons
copy "..\bin9\Release Unicode64\Plugins\clist_mw.dll"              %Temp%\mirx64c\Plugins
copy "..\bin9\Release Unicode64\Plugins\clist_nicer.dll"              %Temp%\mirx64c\Plugins
copy "..\bin9\Release Unicode64\Plugins\clist_modern.dll"              %Temp%\mirx64c\Plugins
copy "..\bin9\Release Unicode64\Plugins\scriver.dll"              %Temp%\mirx64c\Plugins
copy "..\bin9\Release Unicode64\Plugins\tabsrmm.dll"              %Temp%\mirx64c\Plugins

del /Q /F "%ContribVer%.7z"
"C:\Program Files\7-Zip\7z.exe" a -t7z -r -mx=9 "%ContribVer%.7z" %Temp%\mirx64c/*
rd /Q /S %Temp%\mirx64c

"C:\Program Files (x86)\NSIS\makensis.exe" ../../miranda-tools/installer/miranda-install-ansi.nsi
"C:\Program Files (x86)\NSIS\makensis.exe" ../../miranda-tools/installer/miranda-install-unicode.nsi
"C:\Program Files (x86)\NSIS\makensis.exe" ../../miranda-tools/installer/miranda-install-x64.nsi


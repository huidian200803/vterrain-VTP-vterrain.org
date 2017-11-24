; Setup file for vtocx ActiveX plugin

[Setup]
AppName=vtocx
AppVerName=vtocx 071120
DefaultDirName={pf}\vtocx
DefaultGroupName=vtocx
UninstallDisplayIcon={app}\vtocx.ocx
OutputDir=data
OutputBaseFilename=vtocx

[Dirs]
Name: "{app}\bin"
Name: "{app}\data"

[Files]
Source: "..\OpenThreads\bin\Win32\OpenThreadsWin32.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osg.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgDB.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgUtil.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgText.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgSim.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgdb_osg.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgdb_ive.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgdb_gz.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\osgdb_freetype.dll"; DestDir: "{app}\bin"
Source: "C:\WINDOWS\SYSTEM32\MFC70.dll"; DestDir: "{app}\bin"; Flags: allowunsafefiles
Source: "..\OpenSceneGraph\bin\MSVCP70.dll"; DestDir: "{app}\bin"
Source: "..\OpenSceneGraph\bin\MSVCR70.dll"; DestDir: "{app}\bin"
Source: "Release\vtocx.ocx"; DestDir: "{app}\bin"; Flags: regserver

;[Run]
;Filename: "regsvr32.exe"; Parameters: "/s {app}\bin\vtocx.ocx"

;[UninstallRun]
;Filename: "regsvr32.exe"; Parameters: "/s /u {app}\bin\vtocx.ocx"

[Registry]
Root: HKCR; Subkey: "vtocx.vtocxCtrl.1\shell\open\command"; ValueType: string; ValueData: "{reg:HKCR\htmlfile\shell\open\command,|{pf}\INTERN~1\iexplorer.exe}"; Flags: uninsdeletekey
Root: HKCR; Subkey: "vtocx.vtocxCtrl.1\shell\open\ddeexec"; ValueType: string; ValueData: "{%22}file:%1{%22},,-1,,,,,"; Flags: uninsdeletekey
Root: HKCR; Subkey: "vtocx.vtocxCtrl.1\shell\open\ddeexec\Application"; ValueType: string; ValueData: "IExplore"; Flags: uninsdeletekey
Root: HKCR; Subkey: "vtocx.vtocxCtrl.1\shell\open\ddeexec\Topic"; ValueType: string; ValueData: "WWW_OpenURL"; Flags: uninsdeletekey

Root: HKCR; Subkey: "MIME\Database\Content Type\application/ivefile"; ValueType: string; ValueName: "CLSID"; ValueData: ".ive"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MIME\Database\Content Type\application/ivefile"; ValueType: string; ValueName: "Extension"; ValueData: "{{246039DB-3D65-47EC-B929-C6A040981483}"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MIME\Database\Content Type\application/ivzfile"; ValueType: string; ValueName: "CLSID"; ValueData: ".ivz"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MIME\Database\Content Type\application/ivzfile"; ValueType: string; ValueName: "Extension"; ValueData: "{{246039DB-3D65-47EC-B929-C6A040981483}"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MIME\Database\Content Type\application/osgfile"; ValueType: string; ValueName: "CLSID"; ValueData: ".osg"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MIME\Database\Content Type\application/osgfile"; ValueType: string; ValueName: "Extension"; ValueData: "{{246039DB-3D65-47EC-B929-C6A040981483}"; Flags: uninsdeletekey


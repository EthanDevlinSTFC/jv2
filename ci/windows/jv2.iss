; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "jv2"
#define MyAppVersion "0.1"
#define MyAppPublisher "Ethan Devlin & Tristan Youngs"
#define MyAppURL "https://www.projectaten.com/"
#define MyAppExeName "jv2.exe"

; Locations of bin directories of Dissolve, Qt, GnuWin, MinGW etc.
#define Jv2Dir GetEnv('JV2_DIR')
#define QtDir GetEnv('Qt6_DIR')
#define MinGWLibDir GetEnv('MINGW_LIB_DIR')
#define BackendDir GetEnv('BACKEND_DIR')

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{EB65EDA8-F371-4934-962B-8F79CF9D7980}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\jv2
DefaultGroupName={#MyAppName}
LicenseFile=..\..\LICENSE.txt
OutputDir=..\..\
OutputBaseFilename=jv2-{#MyAppVersion}-Win64
SetupIconFile=JournalViewer2.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#Jv2Dir}\jv2.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "JournalViewer2.ico"; DestDir: "{app}\bin"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: "{#QtDir}\bin\Qt6Gui.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6Core.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6OpenGL.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6OpenGLWidgets.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6Svg.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6Widgets.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6Network.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6PrintSupport.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6Charts.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt6Xml.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#QtDir}\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\bin\iconengines"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\bin\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\*.dll"; DestDir: "{app}\bin\imageformats"; Flags: ignoreversion
Source: "{#BackendDir}\launch\*"; DestDir: "{app}\bin\launch"; Flags: ignoreversion
Source: "{#BackendDir}\launch\isisInternal\*"; DestDir: "{app}\bin\launch\isisInternal"; Flags: ignoreversion
Source: "{#BackendDir}\launch\isisInternal\lxml\*"; DestDir: "{app}\bin\launch\isisInternal\lxml"; Flags: ignoreversion
Source: "{#BackendDir}\launch\isisInternal\h5py\*"; DestDir: "{app}\bin\launch\isisInternal\h5py"; Flags: ignoreversion
Source: "{#BackendDir}\launch\isisInternal\numpy\core\*"; DestDir: "{app}\bin\launch\isisInternal\numpy\core"; Flags: ignoreversion
Source: "{#BackendDir}\launch\isisInternal\numpy\fft\*"; DestDir: "{app}\bin\launch\isisInternal\numpy\fft"; Flags: ignoreversion
Source: "{#BackendDir}\launch\isisInternal\numpy\linalg\*"; DestDir: "{app}\bin\launch\isisInternal\numpy\linalg"; Flags: ignoreversion
Source: "{#BackendDir}\launch\isisInternal\numpy\random\*"; DestDir: "{app}\bin\launch\isisInternal\numpy\random"; Flags: ignoreversion
Source: "{#Jv2Dir}\..\..\extra\*"; DestDir: "{app}\bin\extra"; Flags: ignoreversion
; Windows 7
;Source: "C:\Windows\System32\D3DCompiler_43.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Windows 10
Source: "C:\Windows\System32\D3DCompiler_47.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; IconFilename: "{app}\bin\JournalViewer2.ico"; Filename: "{app}\bin\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{commondesktop}\{#MyAppName}"; IconFilename: "{app}\bin\JournalViewer2.ico"; Filename: "{app}\bin\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
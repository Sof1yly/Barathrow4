; Inno Setup 6 script for UCGEngine
; Compile with: ISCC.exe UCGEngine_installer.iss

#define AppName      "UCGEngine"
#define AppVersion   "1.0"
#define AppPublisher "DT170"
#define AppExeName   "UCG.exe"

[Setup]
AppId={{8A2F3C1D-4B6E-4F2A-9D3C-7E8F1A2B3C4D}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL=
AppSupportURL=
AppUpdatesURL=
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
OutputDir=.
OutputBaseFilename=UCGEngine_Setup
SetupIconFile=
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\{#AppExeName}
DisableProgramGroupPage=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional icons:"

[Files]
Source: "UCG.exe";              DestDir: "{app}"; Flags: ignoreversion
Source: "glew32.dll";           DestDir: "{app}"; Flags: ignoreversion
Source: "SDL2.dll";             DestDir: "{app}"; Flags: ignoreversion
Source: "SDL2_image.dll";       DestDir: "{app}"; Flags: ignoreversion
Source: "SDL2_ttf.dll";         DestDir: "{app}"; Flags: ignoreversion
Source: "SDL2_mixer.dll";       DestDir: "{app}"; Flags: ignoreversion
Source: "libjpeg-9.dll";        DestDir: "{app}"; Flags: ignoreversion
Source: "libpng16-16.dll";      DestDir: "{app}"; Flags: ignoreversion
Source: "libtiff-5.dll";        DestDir: "{app}"; Flags: ignoreversion
Source: "libwebp-7.dll";        DestDir: "{app}"; Flags: ignoreversion
Source: "zlib1.dll";            DestDir: "{app}"; Flags: ignoreversion
Source: "libfreetype-6.dll";    DestDir: "{app}"; Flags: ignoreversion
Source: "libFLAC-8.dll";        DestDir: "{app}"; Flags: ignoreversion
Source: "libmodplug-1.dll";     DestDir: "{app}"; Flags: ignoreversion
Source: "libmpg123-0.dll";      DestDir: "{app}"; Flags: ignoreversion
Source: "libogg-0.dll";         DestDir: "{app}"; Flags: ignoreversion
Source: "libvorbis-0.dll";      DestDir: "{app}"; Flags: ignoreversion
Source: "libvorbisfile-3.dll";  DestDir: "{app}"; Flags: ignoreversion
Source: "Resource\*"; DestDir: "{app}\Resource"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#AppName}";           Filename: "{app}\{#AppExeName}"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#AppName}";   Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Launch {#AppName}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\Resource\GameData\Save"

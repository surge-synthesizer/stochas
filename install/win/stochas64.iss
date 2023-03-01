#define MyAppPublisher "Surge Synth Team"
#define MyAppURL "https://stochas.org/"
#define MyAppName "Stochas"
#define MyAppNameCondensed "Stochas"
#define MyID "1858F586-B034-4272-B9F9-2239A8063E7A"

#ifndef MyAppVersion
#define MyAppVersion "0.0.0"
#endif

;uncomment these two lines if building the installer locally!
;#define STOCHAS_SRC "..\..\"
;#define STOCHAS_BIN "..\..\build\"

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppId={#MyID}
AppName={#MyAppName}
AppVerName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppPublisher}\{#MyAppName}\
DefaultGroupName={#MyAppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
AlwaysShowDirOnReadyPage=yes
LicenseFile={#STOCHAS_SRC}\COPYING
OutputBaseFilename={#MyAppName}-{#MyAppVersion}-Windows-64bit-setup
SetupIconFile={#STOCHAS_SRC}\install\win\stochas.ico
UninstallDisplayIcon={uninstallexe}
UsePreviousAppDir=yes
Compression=lzma
SolidCompression=yes
UninstallFilesDir={autoappdata}\{#MyAppName}\uninstall
CloseApplicationsFilter=*.exe,*.vst3
WizardStyle=modern
WizardSizePercent=100
WizardImageFile={#STOCHAS_SRC}\install\win\empty.bmp
WizardSmallImageFile={#STOCHAS_SRC}\install\win\empty.bmp
WizardImageAlphaFormat=defined

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Full installation"
;Name: "clap"; Description: "CLAP installation"
Name: "vst3"; Description: "VST3 installation"
Name: "standalone"; Description: "Standalone installation"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
;Name: "CLAP"; Description: "{#MyAppName} CLAP (64-bit)"; Types: full clap custom
Name: "VST3"; Description: "{#MyAppName} VST3 (64-bit)"; Types: full vst3 custom
Name: "SA"; Description: "{#MyAppName} Standalone (64-bit)"; Types: full standalone custom

[Files]
;Source: "{#STOCHAS_BIN}\stochas_artefacts\release\CLAP\{#MyAppName}.clap"; DestDir: "{autocf}\CLAP\{#MyAppPublisher}\"; Components: CLAP; Flags: ignoreversion
Source: "{#STOCHAS_BIN}\stochas_artefacts\release\VST3\{#MyAppName}.vst3\*"; DestDir: "{autocf}\VST3\{#MyAppPublisher}\{#MyAppName}.vst3\"; Components: VST3; Flags: ignoreversion recursesubdirs
Source: "{#STOCHAS_BIN}\stochas_artefacts\release\Standalone\{#MyAppName}.exe"; DestDir: "{app}"; Components: SA; Flags: ignoreversion
Source: "{#STOCHAS_SRC}\install\win\uninstall.ps1"; DestDir: "{tmp}"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppName}.exe"; Flags: createonlyiffileexists

[Run]
Filename: "powershell.exe"; \
  Parameters: "-ExecutionPolicy Bypass -File ""{tmp}\uninstall.ps1"""; \
  Flags: runhidden

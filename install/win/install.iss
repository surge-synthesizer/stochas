; 
; Source directories
; stochas_version must be defined externally
; these paths are relative to the .iss file
#define vst64 "..\..\build\stochas_artefacts\Release"
#define vst32 "..\..\build32\stochas_artefacts\Release"
[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{18D03379-6956-4365-BDA3-33D3CBBA991E}
AppName=Stochas
; needs to be passed in when running from command line
AppVersion={#stochas_version}
;AppVerName=Stochas 1.0
AppPublisher=Open Source Software
AppPublisherURL=https://github.com/surge-synthesizer/stochas
AppSupportURL=https://github.com/surge-synthesizer/stochas
AppUpdatesURL=https://github.com/surge-synthesizer/stochas
; complains if we don't have this
DefaultDirName=stochas
;DefaultGroupName=Stochas
DisableDirPage=yes
DisableProgramGroupPage=yes
LicenseFile=license.txt
OutputBaseFilename=stochas_setup
Compression=lzma
SolidCompression=yes
UninstallFilesDir={code:install_dir}\stochas_uninst
UsePreviousAppDir=no
ShowComponentSizes=no
DirExistsWarning=no
;EnableDirDoesntExistWarning=yes
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
[Files]
Source: "{#vst32}\VST3\Stochas.vst3\Contents\x86-win\Stochas.vst3"; DestDir: "{code:install_dir}"; Flags: ignoreversion; Check: check_inst(0)
Source: "{#vst64}\VST3\Stochas.vst3\Contents\x86_64-win\Stochas.vst3"; DestDir: "{code:install_dir}"; Flags: ignoreversion; Check: check_inst(1)

; NOTE: Don't use "Flags: ignoreversion" on any shared system files
[Code]
var
  PluginTypePage: TInputOptionWizardPage;
  vstpath: String;
procedure InitializeWizard;
begin
  { Create the pages }
  
  PluginTypePage := CreateInputOptionPage(wpLicense,
    'Plugin Type', 'Select the type of plugin to install.',
    'Please specify the version of the Stochas plugin that is applicable to your needs, then click Next.',
    True, False);
  PluginTypePage.Add('VST3 32 bit');
  PluginTypePage.Add('VST3 64 bit');
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
    if (CurPageID = PluginTypePage.ID) then
    begin
      if (PluginTypePage.SelectedValueIndex = 0) then {vst3 32 }
        vstpath:=ExpandConstant('{pf32}') + '\Common Files\VST3\'
      else if (PluginTypePage.SelectedValueIndex = 1) then  {vst3 64 }
        vstpath:=ExpandConstant('{pf64}') + '\Common Files\VST3\'
    end;
    Result := True;
end;

function install_dir(Param: String): String;
begin
  { Return the selected DataDir }
  Result := vstpath;
end;

{should the file be installed}
function check_inst(Param: Integer): Boolean;
begin
  Result := (PluginTypePage.SelectedValueIndex = Param);
end;
  

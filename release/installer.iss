; N Language Installer Script (Inno Setup)
#define MyAppName "N Language"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Squ4sh000"
#define MyAppExeName "n.exe"

[Setup]
AppId={{N-LANGUAGE-SQU4SH000}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\NLanguage
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=.
OutputBaseFilename=NLanguageSetup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ChangesEnvironment=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Files]
Source: "d:\Projects\N\n.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "d:\Projects\N\lib\*"; DestDir: "{app}\lib"; Flags: ignoreversion recursesubdirs
Source: "d:\Projects\N\src\*"; DestDir: "{app}\src"; Flags: ignoreversion recursesubdirs
Source: "d:\Projects\N\build.ps1"; DestDir: "{app}"; Flags: ignoreversion
Source: "d:\Projects\N\README_N.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "d:\Projects\N\vscode-n-lang\images\logo.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Registry]
; File Association for .ns
Root: HKCR; Subkey: ".ns"; ValueType: string; ValueName: ""; ValueData: "NLanguageScript"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "NLanguageScript"; ValueType: string; ValueName: ""; ValueData: "N Script File"; Flags: uninsdeletekey
Root: HKCR; Subkey: "NLanguageScript\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\logo.ico"
Root: HKCR; Subkey: "NLanguageScript\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Code]
function NeedRestart(): Boolean;
begin
  Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  OldPath: String;
  NewPath: String;
begin
  if CurStep = ssPostInstall then
  begin
    if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', OldPath) then
    begin
      NewPath := ExpandConstant('{app}');
      if Pos(NewPath, OldPath) = 0 then
      begin
        NewPath := OldPath + ';' + NewPath;
        RegWriteStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', NewPath);
      end;
    end;
  end;
end;

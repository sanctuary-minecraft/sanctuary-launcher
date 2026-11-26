; SPDX-License-Identifier: GPL-3.0-only
Unicode true
RequestExecutionLevel user
!include "MUI2.nsh"
!include "x64.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"

Name "Sanctuary Launcher"
OutFile "${OUTPUT}"
InstallDir "$LOCALAPPDATA\Programs\SanctuaryLauncher"
InstallDirRegKey HKCU "Software\SanctuaryLauncher" "InstallDir"
!define MUI_ABORTWARNING
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\SanctuaryLauncher"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "English"

Function .onInit
  ${IfNot} ${RunningX64}
    Abort
  ${EndIf}
FunctionEnd

Section
  SetShellVarContext current
  SetOutPath "$INSTDIR\versions\${VERSION}"
  ClearErrors
  File /r /x "sanctuary-launcher-start.exe" "${PAYLOAD}\*"
  ${If} ${Errors}
    Abort
  ${EndIf}
  SetOutPath "$INSTDIR"
  ClearErrors
  File /oname=sanctuary-launcher.next "${PAYLOAD}\sanctuary-launcher-start.exe"
  WriteINIStr "$INSTDIR\current.next.ini" "Launcher" "Version" "${VERSION}"
  ${If} ${Errors}
    Abort
  ${EndIf}
  System::Call 'kernel32::MoveFileExW(w "$INSTDIR\sanctuary-launcher.next", w "$INSTDIR\sanctuary-launcher.exe", i 9) i .r1'
  ${If} $1 == 0
    Abort
  ${EndIf}
  System::Call 'kernel32::MoveFileExW(w "$INSTDIR\current.next.ini", w "$INSTDIR\current.ini", i 9) i .r1'
  ${If} $1 == 0
    Abort
  ${EndIf}
  WriteUninstaller "$INSTDIR\uninstall.exe"
  CreateShortcut "$SMPROGRAMS\Sanctuary Launcher.lnk" "$INSTDIR\sanctuary-launcher.exe" "" "$INSTDIR\sanctuary-launcher.exe" 0
  WriteRegStr HKCU "Software\SanctuaryLauncher" "InstallDir" "$INSTDIR"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayName" "Sanctuary Launcher"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "Publisher" "Sanctuary"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  ${GetParameters} $0
  ${GetOptions} $0 "/Update" $1
  ${If} ${Errors}
    Exec '"$INSTDIR\sanctuary-launcher.exe"'
  ${EndIf}
SectionEnd

Section "Uninstall"
  SetShellVarContext current
  Delete "$SMPROGRAMS\Sanctuary Launcher.lnk"
  RMDir /r "$INSTDIR\versions"
  Delete "$INSTDIR\sanctuary-launcher.exe"
  Delete "$INSTDIR\current.ini"
  Delete "$INSTDIR\sanctuary-launcher.next"
  Delete "$INSTDIR\current.next.ini"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"
  DeleteRegKey HKCU "${UNINSTALL_KEY}"
  DeleteRegValue HKCU "Software\SanctuaryLauncher" "InstallDir"
SectionEnd

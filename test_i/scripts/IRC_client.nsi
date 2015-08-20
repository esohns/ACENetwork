; IRCclient.nsi
;--------------------------------

; Includes
;!include "FileFunc.nsh"
;--------------------------------

; Metadata
!searchparse /file "..\IRC_client_gui_defines.h" `#define IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY  "` CONFIGURATION_SUBDIR `"`
!searchparse /file "..\..\configure.ac" `m4_define([M4_LIBACENETWORK_VERSION_MAJOR], [` VER_MAJOR `])`
!searchparse /file "..\..\configure.ac" `m4_define([M4_LIBACENETWORK_VERSION_MINOR], [` VER_MINOR `])`
!searchparse /file "..\..\configure.ac" `m4_define([M4_LIBACENETWORK_VERSION_MACRO], [` VER_MICRO `])`
!searchparse /file "..\..\configure.ac" `m4_define([M4_LIBACENETWORK_VERSION_DEVEL], [` DEVEL `])`
!define PROGRAM "IRC_client"
!define SCRIPTS_SUBDIR "scripts"

; Languages
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

; The name of the installer
Name ${PROGRAM}

; The file to write
!searchparse /file "..\..\configure.ac" `m4_define([M4_LIBACENETWORK_VERSION_MAJOR], [` VER_MAJOR `])`
!searchparse /file "..\..\configure.ac" `m4_define([M4_LIBACENETWORK_VERSION_MINOR], [` VER_MINOR `])`
!searchparse /file "..\..\configure.ac" `m4_define([M4_LIBACENETWORK_VERSION_MICRO], [` VER_MICRO `])`
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" ${PROGRAM}
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "a IRC client"
;VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" ""
!define /date CURRENT_YEAR "%Y"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© ${CURRENT_YEAR}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${PROGRAM} installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VER_MAJOR}.${VER_MINOR}.${VER_MICRO}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VER_MAJOR}.${VER_MINOR}.${VER_MICRO}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "PrivateBuild" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "SpecialBuild" ""
; *TODO*: improve output file naming
;!if ${DEVEL} == "devel"
;  !define VER_DEVEL 0
;!else
;  !define VER_DEVEL 1
;!endif
!define VER_DEVEL 0
VIProductVersion "${VER_MAJOR}.${VER_MINOR}.${VER_MICRO}.${VER_DEVEL}"

; The installer filename
;Var /GLOBAL VER_STRING
;Var /GLOBAL VER_DEVEL
;Function setupVersionString
;StrCpy $VER_STRING "${PROGRAM}-${VER_MAJOR}.${VER_MINOR}.${VER_MICRO}"
;StrCpy $VER_DEVEL "${DEVEL}"
;StrCmp $VER_DEVEL "" 0 +3
;  Goto +2
;  StrCpy $VER_STRING "$VER_STRING-$VER_DEVEL"
;Var /GLOBAL TARGET_FILE
;StrCpy $TARGET_FILE "$VER_STRING.exe"
;FunctionEnd
;Call setupVersionString
;OutFile $TARGET_FILE
OutFile "${PROGRAM}-${VER_MAJOR}.${VER_MINOR}.${VER_MICRO}.exe"

; The default installation directory
;InstallDir $DESKTOP\${PROGRAM}
InstallDir "$PROGRAMFILES\${PROGRAM}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\${PROGRAM}" "Install_Dir"

; Request application privileges (for Windows Vista and above)
; *TODO*
RequestExecutionLevel admin

; License
LicenseData "..\..\LICENSE"

; Other options
AutoCloseWindow true
;Icon "..\..\graphics\data\images\image_icon.ico"
;XPStyle on

;--------------------------------

; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The files to install
Section "${PROGRAM} (required)"

SectionIn RO

; set output path to the installation directory
SetOutPath $INSTDIR

; put files there (3rd party)
!if ${release} == "Debug"
; *TODO*: remove this ASAP, it violates Microsoft's EULA (debug version only)
;File "D:\software\Develop\msvcr100d.dll"
File "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\Debug_NonRedist\x86\Microsoft.VC120.DebugCRT\msvcr120d.dll"
File "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\Debug_NonRedist\x86\Microsoft.VC120.DebugCRT\msvcp120d.dll"
File "D:\projects\ATCD\ACE\lib\ACEd.dll"
!else
File "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\msvcr120.dll"
File "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\msvcp120.dll"
File "D:\projects\ATCD\ACE\lib\ACE.dll"
!endif

;File "D:\projects\SDL-1.2.15\VisualC\SDL\${release}\SDL.dll"
;File "D:\projects\SDL_mixer-1.2.12\lib\x86\libogg-0.dll"
;File "D:\projects\SDL_mixer-1.2.12\lib\x86\libvorbis-0.dll"
;File "D:\projects\SDL_mixer-1.2.12\lib\x86\libvorbisfile-3.dll"
;File "D:\projects\SDL_mixer-1.2.12\lib\x86\libmikmod-2.dll"
;File "D:\projects\SDL_mixer-1.2.12\lib\x86\SDL_mixer.dll"
;File "D:\projects\SDL_ttf-2.0.11\lib\x86\libfreetype-6.dll"
;File "D:\projects\SDL_ttf-2.0.11\lib\x86\SDL_ttf.dll"

;File "D:\projects\lpng167\projects\vstudio\${release}\libpng16.dll"

File "D:\projects\gtk\bin\freetype6.dll"
File "D:\projects\gtk\bin\intl.dll"
File "D:\projects\gtk\bin\libatk-1.0-0.dll"
File "D:\projects\gtk\bin\libcairo-2.dll"
File "D:\projects\gtk\bin\libexpat-1.dll"
File "D:\projects\gtk\bin\libfontconfig-1.dll"
File "D:\projects\gtk\bin\libgdk_pixbuf-2.0-0.dll"
File "D:\projects\gtk\bin\libgdk-win32-2.0-0.dll"
File "D:\projects\gtk\bin\libgio-2.0-0.dll"
File "D:\projects\gtk\bin\libglib-2.0-0.dll"
File "D:\projects\gtk\bin\libgmodule-2.0-0.dll"
File "D:\projects\gtk\bin\libgobject-2.0-0.dll"
File "D:\projects\gtk\bin\libgthread-2.0-0.dll"
File "D:\projects\gtk\bin\libgtk-win32-2.0-0.dll"
File "D:\projects\gtk\bin\libpango-1.0-0.dll"
File "D:\projects\gtk\bin\libpangocairo-1.0-0.dll"
File "D:\projects\gtk\bin\libpangoft2-1.0-0.dll"
File "D:\projects\gtk\bin\libpangowin32-1.0-0.dll"
File "D:\projects\gtk\bin\libpng14-14.dll"
File "D:\projects\gtk\bin\zlib1.dll"

File "D:\projects\libglade\bin\libglade-2.0-0.dll"

File "G:\software\Development\libiconv-2.dll"
File "G:\software\Development\libxml2-2.dll"

File "..\..\prj\msvc\${release}\libCommon.dll"
File "..\..\prj\msvc\${release}\libCommon_UI.dll"
File "..\..\prj\msvc\${release}\libACEStream.dll"
File "..\..\prj\msvc\${release}\libACENetwork.dll"
;File "..\..\prj\msvc\${release}\libACENetwork_Client.dll"

File "..\..\prj\msvc\${release}\test_i_IRC_client_UI.exe"

; Config
; set output path to the configuration directory
SetOutPath "$INSTDIR\${CONFIGURATION_SUBDIR}"

; Config - glade
File "..\..\test_i\etc\IRC_client_channel.glade"
File "..\..\test_i\etc\IRC_client_connection.glade"
File "..\..\test_i\etc\IRC_client_main.glade"

; Config - ini/rc
; set output path to the scripts directory
;SetOutPath "$APPDATA\${PROGRAM}"
;SetOutPath "$INSTDIR\${SCRIPTS_SUBDIR}"
File "..\..\test_i\etc\IRC_client.ini"
File "..\..\test_i\etc\IRC_client_phonebook.ini"
File "..\..\test_i\etc\IRC_client.rc"

; Write the installation path into the registry
WriteRegStr HKLM "SOFTWARE\${PROGRAM}" "Install_Dir" "$INSTDIR"

; Write the uninstall keys for Windows
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM}" "DisplayName" "${PROGRAM}"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM}" "UninstallString" '"$INSTDIR\uninstall.exe"'
WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM}" "NoModify" 1
WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM}" "NoRepair" 1
WriteUninstaller "uninstall.exe"

SectionEnd

;--------------------------------

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

CreateDirectory "$SMPROGRAMS\${PROGRAM}"
SetOutPath "$INSTDIR"
CreateShortCut "$SMPROGRAMS\${PROGRAM}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
CreateShortCut "$SMPROGRAMS\${PROGRAM}\${PROGRAM}.lnk" "$INSTDIR\test_i_IRC_client_UI.exe" "" "$INSTDIR\test_i_IRC_client_UI.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

; Auto-Close
SetAutoClose true

; Remove registry keys
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM}"
DeleteRegKey HKLM "SOFTWARE\${PROGRAM}"

; Remove files AND uninstaller (yes this works !!!)
;Delete "$INSTDIR\*.*"
; Ask before removing player profiles ? *TODO*
RMDir /r "$APPDATA\${PROGRAM}"

; Remove shortcuts, if any
Delete "$SMPROGRAMS\${PROGRAM}\*.*"

; Remove directories used
RMDir /r "$INSTDIR"
RMDir "$SMPROGRAMS\${PROGRAM}"

SectionEnd

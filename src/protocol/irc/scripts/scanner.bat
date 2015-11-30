@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem #// File Name: scanner.bat
@rem #//
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------------
@rem #// 20/02/06 | soh | Creation.
@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1

@rem set FlexEXE="%ProgramFiles(x86)%\GnuWin32\bin\flex.exe"
set FlexEXE="C:\cygwin64\bin\flex.exe"
if exist %FlexEXE% goto Next
echo invalid file ^(was: "%FlexEXE%"^)^, exiting
goto Failed

:Next
%FlexEXE% bisector.l 2>scanner_report_bisector.txt
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate scanner from bisector.l^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

%FlexEXE% scanner.l 2>scanner_report_scanner.txt
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate scanner from scanner.l^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@move /Y irc_bisector.cpp .. >NUL
@move /Y irc_bisector.h .. >NUL
@move /Y irc_scanner.cpp .. >NUL
@move /Y irc_scanner.h .. >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to move scanner file^(s^)^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

goto Clean_Up

:Failed
echo processing scanner...FAILED

:Clean_Up
popd
::endlocal & set RC=%ERRORLEVEL%
endlocal & set RC=%RC%
goto Error_Level

:Exit_Code
:: echo %ERRORLEVEL% %1 *WORKAROUND*
exit /b %1

:Error_Level
call :Exit_Code %RC%


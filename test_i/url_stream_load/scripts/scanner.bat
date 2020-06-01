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
@rem set LEX_FILE=bisector.l
@rem if NOT exist "%LEX_FILE%" (
@rem  echo invalid lex file ^(was: "%LEX_FILE%"^)^, exiting
@rem  goto Failed
@rem )
@rem %FlexEXE% %LEX_FILE% 2>scanner_report_bisector.txt
@rem if %ERRORLEVEL% NEQ 0 (
@rem  echo failed to generate scanner from %LEX_FILE%^, exiting
@rem  set RC=%ERRORLEVEL%
@rem  goto Failed
@rem )

set LEX_FILE=scanner.l
if NOT exist "%LEX_FILE%" (
 echo invalid lex file ^(was: "%LEX_FILE%"^)^, exiting
 goto Failed
)
%FlexEXE% --noline %LEX_FILE% 2>scanner_report_scanner.txt
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate scanner from %LEX_FILE%^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@rem @move /Y http_bisector.cpp .. >NUL
@rem @move /Y http_bisector.h .. >NUL
@move /Y bittorrent_scanner.cpp .. >NUL
@move /Y bittorrent_scanner.h .. >NUL
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

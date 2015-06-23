@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem #// File Name: make_installer.bat
@rem #//
@rem #// arguments: Debug | Release
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------------
@rem #// 20/02/06 | soh | Creation.
@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////

@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1
goto Begin

:Print_Usage
echo usage: %~n0 ^[Debug ^| Release^]
goto Clean_Up

:Begin
if "%1."=="." (
 echo invalid argument^, exiting
 goto Print_Usage
)
if NOT "%1"=="Debug" if NOT "%1"=="Release" (
 echo invalid argument ^(was: "%1"^)^, exiting
 goto Print_Usage
)

set NSISEXE="C:\Program Files (x86)\NSIS\makensis.exe"
rem if NOT exist "%NSISEXE%" (
rem echo invalid file ^(was: "%NSISEXE%"^)^, exiting
rem  goto Failed
rem )

set SOURCE_FILE=.\IRCclient.nsi
if NOT exist "%SOURCE_FILE%" (
 echo invalid file ^(was: "%SOURCE_FILE%"^)^, exiting
 goto Failed
)
%NSISEXE% /V4 /Oinstaller.log /Drelease=%1 %SOURCE_FILE%
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate installer^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@rem move generated file into the release directory
set TARGET_DIRECTORY=.\..\releases
if NOT exist "%TARGET_DIRECTORY%" (
 echo invalid directory ^(was: "%TARGET_DIRECTORY%"^)^, exiting
 goto Failed
)
for /R . %FILE in (*.exe) do (
  move /Y %FILE %TARGET_DIRECTORY% >NUL
  if %ERRORLEVEL% NEQ 0 (
    echo failed to move file^(s^)^, exiting
    set RC=%ERRORLEVEL%
    goto Failed
  )
  echo moved "%~nFILE"...
)
goto Clean_Up

:Failed
echo processing installer...FAILED

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

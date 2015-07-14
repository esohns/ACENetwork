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

set SOURCE_FILE="%~dp0\IRC_client.nsi"
if NOT exist "%SOURCE_FILE%" (
 echo invalid file ^(was: "%SOURCE_FILE%"^)^, exiting
 goto Failed
)
echo generating installer...
%NSISEXE% /V4 /Oinstaller.log /Drelease=%1 %SOURCE_FILE%
rem %NSISEXE% /V4 /Drelease=%1 %SOURCE_FILE%
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate installer^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo generating installer...DONE

@rem move generated file into the releases directory
set TARGET_DIRECTORY="%~dp0\..\releases"
if NOT exist "%TARGET_DIRECTORY%" (
 echo invalid directory ^(was: "%TARGET_DIRECTORY%"^)^, exiting
 goto Failed
)
for /R "%~dp0" %%f in (*.exe) do (
  move /Y "%%f" %TARGET_DIRECTORY% >NUL
  if %ERRORLEVEL% NEQ 0 (
    echo failed to move file^(s^)^, exiting
    set RC=%ERRORLEVEL%
    goto Failed
  )
  echo moved "%%f"...
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

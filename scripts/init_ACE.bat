@rem //%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem // File Name: init_ACE.bat
@rem //
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------------
@rem #// 20/02/06 | soh | Creation.
@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1

@rem copy config.h
set SourceDirectory=%~dp0\..\3rd_party\ACE_wrappers\ace
if NOT exist "%SourceDirectory%" (
 echo invalid directory ^(was: "%SourceDirectory%"^)^, exiting
 goto Failed
)
set ACE_ROOT=D:\projects\ATCD\ACE
if NOT exist "%ACE_ROOT%" (
 echo invalid directory ^(was: "%ACE_ROOT%"^)^, exiting
 goto Failed
)
set TargetDirectory=%ACE_ROOT%\ace
if NOT exist "%TargetDirectory%" (
 echo invalid directory ^(was: "%TargetDirectory%"^)^, exiting
 goto Failed
)

set OriginalConfigFile=%SourceDirectory%\config-win32.h
if NOT exist "%OriginalConfigFile%" (
 echo invalid file ^(was: "%OriginalConfigFile%"^)^, exiting
 goto Failed
)
set ConfigFile=config.h
@copy /Y %OriginalConfigFile% %SourceDirectory%\%ConfigFile% >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to copy %ConfigFile% file, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@move /Y %SourceDirectory%\%ConfigFile% %TargetDirectory%\%ConfigFile% >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to move %ConfigFile% file, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

echo processing %ConfigFile%...DONE
timeout /T 2 /NOBREAK >NUL

goto Clean_Up

:Failed
echo processing %ConfigFile%...FAILED

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


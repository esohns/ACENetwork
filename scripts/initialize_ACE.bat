@rem //%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem // File Name: init_ACE.bat
@rem //
@rem #// arguments: linux | solaris | win32
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
echo usage: %~n0 ^[linux ^| solaris ^| win32^]
goto Clean_Up

:Begin
if "%1."=="." (
 echo invalid argument^, exiting
 goto Print_Usage
)
if NOT "%1"=="linux" if NOT "%1"=="solaris" if NOT "%1"=="win32" (
 echo invalid argument ^(was: "%1"^)^, exiting
 goto Print_Usage
)

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

set OriginalConfigFile=%SourceDirectory%\config-%1.h
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

@rem copy platform_macros.GNU
set SourceDirectory=%~dp0\..\3rd_party\ACE_wrappers\include\makeinclude
if NOT exist "%SourceDirectory%" (
 echo invalid directory ^(was: "%SourceDirectory%"^)^, exiting
 goto Failed
)
set TargetDirectory=%ACE_ROOT%\include\makeinclude
if NOT exist "%TargetDirectory%" (
 echo invalid directory ^(was: "%TargetDirectory%"^)^, exiting
 goto Failed
)

set OriginalMacrosFile=%SourceDirectory%\platform_macros_%1.GNU
if "%1"=="win32" (
 set OriginalMacrosFile=%SourceDirectory%\platform_macros_%1_msvc.GNU
)
if NOT exist "%OriginalMacrosFile%" (
 echo invalid file ^(was: "%OriginalMacrosFile%"^)^, exiting
 goto Failed
)
set MacrosFile=platform_macros.GNU
@copy /Y %OriginalMacrosFile% %SourceDirectory%\%MacrosFile% >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to copy %MacrosFile% file, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@move /Y %SourceDirectory%\%MacrosFile% %TargetDirectory%\%MacrosFile% >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to move %MacrosFile% file, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

echo processing %MacrosFile%...DONE

timeout /T 2 /NOBREAK >NUL

goto Clean_Up

:Failed
echo processing...FAILED

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


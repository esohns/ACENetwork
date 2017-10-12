@rem //%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem // File Name: configure_libACENetwork.bat
@rem //
@rem #// arguments: linux | solaris | win32
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------------
@rem #// 20/02/06 | soh | Creation.
@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@echo off
set ResultCode=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1
goto Begin

:Print_Usage
echo usage: %~n0 ^[linux ^| solaris ^| win32^]
goto Clean_Up

:Begin
@rem if "%1."=="." (
@rem  echo invalid argument^, exiting
@rem goto Print_Usage
@rem )
@rem if NOT "%1"=="linux" if NOT "%1"=="solaris" if NOT "%1"=="win32" (
@rem  echo invalid argument ^(was: "%1"^)^, exiting
@rem  goto Print_Usage
@rem )

set DefaultProjectsDirectory=%~dp0..\..
set ProjectsDirectory=%DefaultProjectsDirectory%
if NOT exist "%ProjectsDirectory%" (
 echo invalid projects directory ^(was: "%ProjectsDirectory%"^)^, exiting
 goto Failed
)
@rem echo set projects directory: %ProjectsDirectory%

@rem for /F %%i in ("%ProjectsDirectory%") do @set ProjectDrive=%%~di
set OpenSSLDefaultDirectory=%ProjectsDirectory%\openssl
set OpenSSLDirectory=%OpenSSLDefaultDirectory%
if NOT exist "%OpenSSLDirectory%" (
 echo invalid OpenSSL directory ^(was: "%OpenSSLDirectory%"^)^, exiting
 goto Failed
)
@rem echo set OpenSSL directory: "%OpenSSLDirectory%"
setx /M SSL_ROOT %OpenSSLDirectory% >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to setx SSL_ROOT^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo set SSL_ROOT environment variable: "%OpenSSLDirectory%"

goto Clean_Up

:Failed
echo processing...FAILED

:Clean_Up
popd
::endlocal & set ResultCode=%ERRORLEVEL%
endlocal & set ResultCode=%ResultCode%
goto Error_Level

:Exit
:: echo %ERRORLEVEL% %1 *WORKAROUND*
exit /b %1

:Error_Level
call :Exit %ResultCode%

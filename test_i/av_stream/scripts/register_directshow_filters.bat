@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem #// File Name: register_directshow_filters.bat
@rem #//
@rem #// arguments: N/A
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
echo usage: %~n0
goto Clean_Up

:Begin
@rem if "%1."=="." (
@rem  echo invalid argument^, exiting
@rem  goto Print_Usage
@rem )
@rem if NOT "%1"=="Debug" if NOT "%1"=="Release" (
@rem  echo invalid argument ^(was: "%1"^)^, exiting
@rem  goto Print_Usage
@rem )

@rem set REGSVREXE="C:\Windows\SysWOW64\regsvr32.exe"
set REGSVREXE="C:\Windows\System32\regsvr32.exe"
if NOT exist "%REGSVREXE%" (
 echo invalid file ^(was: "%REGSVREXE%"^)^, exiting
 goto Failed
)

set FILTER_DIRECTORY="%~dp0..\..\..\cmake\test_i\camstream\Debug"
if NOT exist "%FILTER_DIRECTORY%" (
 echo invalid directory ^(was: "%FILTER_DIRECTORY%"^)^, exiting
 goto Failed
)
set FILTER_DLL="%FILTER_DIRECTORY%\camtarget_source.dll"
if NOT exist "%FILTER_DLL%" (
 echo invalid file ^(was: "%FILTER_DLL%"^)^, exiting
 goto Failed
)

echo assembling dependencies...
set ACE_DLL="%~dp0..\..\..\..\ATCD\ACE\lib\ACEd.dll"
if NOT exist "%ACE_DLL%" (
 echo invalid file ^(was: "%ACE_DLL%"^)^, exiting
 goto Failed
)
copy %ACE_DLL% %FILTER_DIRECTORY% >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
 echo failed to copy %ACE_DLL%^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo copied %ACE_DLL%...

set AVUTIL_DLL="%~dp0..\..\..\..\ffmpeg\avutil-55.dll"
if NOT exist "%AVUTIL_DLL%" (
 echo invalid file ^(was: "%AVUTIL_DLL%"^)^, exiting
 goto Failed
)
copy %AVUTIL_DLL% %FILTER_DIRECTORY% >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
 echo failed to copy %AVUTIL_DLL%^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo copied %AVUTIL_DLL%...
set SWSCALE_DLL="%~dp0..\..\..\..\ffmpeg\swscale-4.dll"
if NOT exist "%SWSCALE_DLL%" (
 echo invalid file ^(was: "%SWSCALE_DLL%"^)^, exiting
 goto Failed
)
copy %SWSCALE_DLL% %FILTER_DIRECTORY% >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
 echo failed to copy %SWSCALE_DLL%^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo copied %SWSCALE_DLL%...
echo assembling dependencies...DONE

echo registering DLL...
%REGSVREXE% %FILTER_DLL%
if %ERRORLEVEL% NEQ 0 (
 echo failed to register DLL^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo registering DLL...DONE

goto Clean_Up

:Failed
echo registering DLL...FAILED

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


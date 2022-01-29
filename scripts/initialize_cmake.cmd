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

@rem set CC="%programfiles(x86)%\Microsoft Visual Studio 14.0\VC\bin\cl.exe"
@rem set CXX="%programfiles(x86)%\Microsoft Visual Studio 14.0\VC\bin\cl.exe"
@rem if NOT exist "!CXX!" (
@rem  echo compiler not found ^(was: "!CXX!"^)^, exiting
@rem  goto Failed
@rem )

@rem set CMakeParameters=-DCMAKE_SYSTEM_VERSION=10.0.10586.0 -G "Visual Studio 14 2015" -T v140 -Wdev
set CMakeParameters=-G "Visual Studio 14 2015" -Wdev
set Projects=libCommon libACEStream libACENetwork
for %%a in (%Projects%) do (
 set ProjectPath=%ProjectsDirectory%\%%a
 if NOT exist "!ProjectPath!" (
  echo invalid project directory ^(was: "!ProjectPath!"^)^, exiting
  goto Failed
 )

 set BuildPath=!ProjectPath!\cmake
 if NOT exist "!BuildPath!" (
  echo invalid build directory ^(was: "!BuildPath!"^)^, exiting
  goto Failed
 )
 cd "!BuildPath!"
 if %ERRORLEVEL% NEQ 0 (
  echo failed to cd to "!BuildPath!"^, exiting
  set RC=%ERRORLEVEL%
  goto Failed
 )

 set CMakeCacheFile=!BuildPath!\CMakeCache.txt
 if exist "!CMakeCacheFile!" (
  @del /Q "!CMakeCacheFile!" >NUL
  if %ERRORLEVEL% NEQ 0 (
   echo failed to del "!CMakeCacheFile!" file, exiting
   set RC=%ERRORLEVEL%
   goto Failed
  )
  echo processing %%a...deleted cache
 )
 
 set CMakeSourcePath=!ProjectPath!
 cmake.exe !CMakeParameters! !CMakeSourcePath!
 cmake.exe !CMakeParameters! !CMakeSourcePath!
 if %ERRORLEVEL% NEQ 0 (
  echo failed to cmake ^(source path was: "!CMakeSourcePath!"^)^, exiting
  set RC=%ERRORLEVEL%
  goto Failed
 )

 echo processing %%a...DONE
)

timeout /T 2 /NOBREAK >NUL

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


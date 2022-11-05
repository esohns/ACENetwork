@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem #// File Name: start_soffice.bat
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

title soffice.exe

set SOfficeIMAGE=soffice.exe
@rem set SOfficeEXE="%ProgramFiles(x86)%\LibreOffice\program\%SOfficeIMAGE%"
set SOfficeEXE="%ProgramFiles%\LibreOffice\program\%SOfficeIMAGE%"
if exist %SOfficeEXE% goto Next
echo invalid file ^(was: "%SOfficeEXE%"^)^, exiting
goto Failed

:Next
set TaskKillEXE=taskkill.exe
%TaskKillEXE% /F /IM %SOfficeIMAGE% /T >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
 echo killed office server %SOfficeIMAGE%^, continuing
)

:Next_2
start "" %SOfficeEXE% --accept="socket,host=localhost,port=2083,tcpNoDelay=1;urp;StarOffice.ServiceManager" --nofirststartwizard --nologo --headless --norestore --invisible >>%TEMP%\ACEStream\libreoffice.log
::if %ERRORLEVEL% NEQ 0 (
:: echo failed to start office server %SOfficeEXE%^, exiting
:: set RC=%ERRORLEVEL%
:: goto Failed
::)
for /F "tokens=2" %%a in ('tasklist /NH /FI "imagename eq %SOfficeIMAGE%"') do (
 set /A PID=%%a
)
if %PID% EQU 0 (
 echo failed to start office server %SOfficeEXE%^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo started office server ^(PID: %PID%^)

goto Clean_Up

:Failed

:Clean_Up
popd
::endlocal & set RC=%ERRORLEVEL%
endlocal & set RC=%RC%
goto Error_Level

:Exit_Code
::echo %ERRORLEVEL% %1 *WORKAROUND*
exit /b %1

:Error_Level
call :Exit_Code %RC%


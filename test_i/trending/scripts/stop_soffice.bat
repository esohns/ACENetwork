@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem #// File Name: stop_soffice.bat
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

set SOfficeIMAGE=soffice.exe
set TaskKillEXE=taskkill.exe
%TaskKillEXE% /F /IM %SOfficeIMAGE% /T >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
 echo killed office server %SOfficeIMAGE%^, continuing
)

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

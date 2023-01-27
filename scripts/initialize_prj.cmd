@rem #//%%%FILE%%%//////////////////////////////////////////////////////////////
@rem #// File Name: initialize_prj.cmd
@rem #//
@rem #// arguments:
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------
@rem #// 12/07/16 | soh | Creation.
@rem #//%%%FILE%%%//////////////////////////////////////////////////////////////

@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1
goto Begin

:Print_Usage
@rem echo usage: %~n0
echo usage: %~n0
goto Clean_Up

:Begin
@rem step1: initialize ACE
@rem set SCRIPT_FILE="%~dp0\initialize_ACE.cmd"
@rem if NOT exist "%SCRIPT_FILE%" (
@rem  echo invalid script file ^(was: "%SCRIPT_FILE%"^)^, exiting
@rem  goto Failed
@rem )
@rem "%SCRIPT_FILE%" windows
@rem if %ERRORLEVEL% NEQ 0 (
@rem  echo failed to run script "%SCRIPT_FILE%"^, exiting
@rem  set RC=%ERRORLEVEL%
@rem  goto Failed
@rem )
@rem echo "%SCRIPT_FILE%"...DONE

goto Clean_Up

:Failed
echo "%SCRIPT_FILE%"...FAILED

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


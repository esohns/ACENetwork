@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1
goto Begin

:Begin
set LOCATION=%LIB_ROOT%\\antlr\\tools\\antlr-4.13.2-complete.jar
::java -jar %LOCATION% -Dlanguage=Cpp -listener -visitor -o generated/ -package antlrcpptest antlr_http_scanner.g4 antlr_http_parser.g4
java -jar %LOCATION% -Dlanguage=Cpp -o generated/ antlr_http_scanner.g4 antlr_http_parser.g4
if %ERRORLEVEL% NEQ 0 (
  echo failed to generate scanner/parser file^(s^)^, exiting
  set RC=%ERRORLEVEL%
  goto Failed
)

for /R "%~dp0\generated" %%f in (*.h) do (
  copy /Y "%%f" ".\.." >NUL
  if %ERRORLEVEL% NEQ 0 (
    echo failed to cp file^(s^)^, exiting
    set RC=%ERRORLEVEL%
    goto Failed
  )
  echo copied "%%f"...
)
for /R "%~dp0\generated" %%f in (*.cpp) do (
  copy /Y "%%f" ".\.." >NUL
  if %ERRORLEVEL% NEQ 0 (
    echo failed to cp file^(s^)^, exiting
    set RC=%ERRORLEVEL%
    goto Failed
  )
  echo copied "%%f"...
)
goto Clean_Up

:Failed
echo processing parser...FAILED

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


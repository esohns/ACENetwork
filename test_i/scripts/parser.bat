@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem #// File Name: parser.bat
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

@rem set BisonEXE=C:\cygwin\bin\bison.exe
set BisonEXE=C:\MinGW\msys\1.0\bin\bison.exe
if NOT exist "%BisonEXE%" (
 echo invalid file ^(was: "%BisonEXE%"^)^, exiting
 goto Failed
)

set SOURCE_FILE=.\IRCparser.y
if NOT exist "%SOURCE_FILE%" (
 echo invalid file ^(was: "%SOURCE_FILE%"^)^, exiting
 goto Failed
)
%BisonEXE% --verbose --graph=parser_graph.txt --xml=parser_graph.xml %SOURCE_FILE% --report=all --report-file=parser_report.txt --warnings=all
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate parser^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@rem move generated files into the project directory
@rem @move /Y IRC_client_IRCparser.h .\..
@rem *NOTE*: need to add a specific method to the parser class
@rem --> copy a patched version back into the project directory
@rem *IMPORTANT NOTE*: needs to be updated after every change
@copy /Y IRC_client_IRCparser_patched.h .\..\IRC_client_IRCparser.h >NUL
@del /F /Q IRC_client_IRCparser.h >NUL

@move /Y IRC_client_IRCparser.cpp .\.. >NUL
@rem @move /Y location.hh .\.. >NUL
@rem @move /Y position.hh .\.. >NUL
@rem @move /Y stack.hh .\.. >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to move parser file^(s^)^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
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

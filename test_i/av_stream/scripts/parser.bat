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

@rem set BisonEXE="%ProgramFiles(x86)%\GnuWin32\bin\bison.exe"
set BisonEXE="H:\bin\flex_bison\win_bison.exe"
if exist %BisonEXE% goto Next
set BisonEXE="C:\mingw\msys\1.0\bin\bison.exe"
if exist %BisonEXE% goto Next
echo invalid file ^(was: "%BisonEXE%"^)^, exiting
goto Failed

:Next
@rem set SOURCE_FILE=parser.y
@rem if NOT exist "%SOURCE_FILE%" (
@rem  echo invalid file ^(was: "%SOURCE_FILE%"^)^, exiting
@rem  goto Failed
@rem )
%BisonEXE% --verbose --graph=parser_graph.txt --xml=parser_graph.xml parser.y --report=all --report-file=parser_report.txt --warnings=all
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate parser from parser.y^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@rem move generated files into the project directory
@rem @move /Y irc_parser.h ..
@rem *NOTE*: need to add specific method declarations to the parser class
@rem         --> copy a patched version back into the project directory
@rem *IMPORTANT NOTE*: needs to be updated after every change
@copy /A /V /Y av_stream_parser_patched.h .\..\av_stream_parser.h >NUL
@del /F /Q av_stream_parser.h >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to copy parser file^(s^)^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@move /Y av_stream_parser.cpp .\.. >NUL
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

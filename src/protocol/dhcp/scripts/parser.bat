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
set BisonEXE="C:\cygwin64\bin\bison.exe"
if exist %BisonEXE% goto Next
echo invalid file ^(was: "%BisonEXE%"^)^, exiting
goto Failed

:Next
set YACC_FILE=parser.y
if NOT exist "%YACC_FILE%" (
 echo invalid yacc file ^(was: "%YACC_FILE%"^)^, exiting
 goto Failed
)
%BisonEXE% --feature=caret --graph --report=all --report-file=parser_report.txt --xml --warnings=all %YACC_FILE%
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate parser from %YACC_FILE%^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@rem move generated files into the project directory
@rem @move /Y http_parser.h ..
@rem *NOTE*: need to add specific method declarations to the parser class
@rem         --> copy a patched version back into the project directory
@rem *IMPORTANT NOTE*: needs to be updated after every change
@rem @copy /A /V /Y http_parser_patched.h .\..\http_parser.h >NUL
@rem @del /F /Q http_parser.h >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to copy parser file^(s^)^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

@move /Y http_parser.cpp .\.. >NUL
@move /Y http_parser.h .\.. >NUL
@rem @del /F /Q location.hh >NUL
@rem @del /F /Q position.hh >NUL
@rem @del /F /Q stack.hh >NUL
if %ERRORLEVEL% NEQ 0 (
 echo failed to delete parser file^(s^)^, exiting
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

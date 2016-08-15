@ECHO OFF
SETLOCAL
IF NOT DEFINED cdb (
  CALL :SET_CDB_%PROCESSOR_ARCHITECTURE%
  IF NOT DEFINED cdb (
      ECHO - Cannot find cdb.exe, please set the "cdb" environment variable to the correct path.
      EXIT /B 1
  )
) ELSE (
  :: Make sure cdb is quoted
  SET cdb="%cdb:"=%"
)
IF NOT EXIST %cdb% (
  ECHO - Cannot find cdb.exe at %cdb%, please set the "cdb" environment variable to the correct path.
  EXIT /B 1
)

IF NOT DEFINED EdgeDbg (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET EdgeDbg="%~dp0bin\EdgeDbg_x64.exe"
  ) ELSE (
    SET EdgeDbg="%~dp0bin\EdgeDbg_x86.exe"
  )
) ELSE (
  SET EdgeDbg="%EdgeDbg:"=%"
)
IF NOT EXIST %EdgeDbg% (
  ECHO - Cannot find EdgeDbg at %EdgeDbg%, please set the "EdgeDbg" environment variable to the correct path.
  EXIT /B 1
)

IF NOT DEFINED Kill (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET Kill="%~dp0modules\Kill\bin\Kill_x64.exe"
  ) ELSE (
    SET Kill="%~dp0modules\Kill\bin\Kill_x86.exe"
  )
) ELSE (
  SET Kill="%Kill:"=%"
)
IF NOT EXIST %Kill% (
  ECHO - Cannot find Kill at %Kill%, please set the "Kill" environment variable to the correct path.
  EXIT /B 1
)

IF NOT DEFINED BugId (
  SET BugId="%~dp0..\BugId\BugId.py"
) ELSE (
  SET BugId="%BugId:"=%"
)
IF NOT EXIST %BugId% (
  ECHO - Cannot find BugId at %BugId%, please set the "BugId" environment variable to the correct path.
  EXIT /B 1
)

IF NOT DEFINED PYTHON (
  SET PYTHON="%SystemDrive%:\Python27\python.exe"
) ELSE (
  SET PYTHON="%PYTHON:"=%"
)
IF NOT EXIST %PYTHON% (
  ECHO - Cannot find Python at %PYTHON%, please set the "PYTHON" environment variable to the correct path.
  EXIT /B 1
)

If "%~1" == "" (
  SET URL="http://%COMPUTERNAME%:28876/"
  SET BugIdArguments=
) ELSE (
  SET URL="%~1"
  SET BugIdArguments=%2 %3 %4 %5 %6 %7 %8 %9
)

ECHO * Terminating all running processes associated with Edge...
%Kill% ApplicationFrameHost.exe browser_broker.exe MicrosoftEdge.exe MicrosoftEdgeCP.exe RuntimeBroker.exe 
IF ERRORLEVEL 1 (
  ECHO - Cannot terminate all running processes associated with Edge.
  EXIT /B 1
)

IF EXIST "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" (
  ECHO * Deleting crash recovery data...
  DEL "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" /Q >nul
)

ECHO * Starting Edge in BugId...
ECHO + URL: %URL%
%EdgeDbg% %URL% %PYTHON% %BugId% @edge --pids=@ProcessIds@ %BugIdArguments%
EXIT /B %ERRORLEVEL%

:SET_CDB_AMD64
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles%\Windows Kits\10\Debuggers\x64\cdb.exe"
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles%\Windows Kits\8.1\Debuggers\x64\cdb.exe"
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles%\Windows Kits\8.0\Debuggers\x64\cdb.exe"
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles(x86)%\Windows Kits\10\Debuggers\x64\cdb.exe"
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles(x86)%\Windows Kits\8.1\Debuggers\x64\cdb.exe"
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles(x86)%\Windows Kits\8.0\Debuggers\x64\cdb.exe"
  EXIT /B 0

:SET_CDB_x86
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles%\Windows Kits\10\Debuggers\x86\cdb.exe"
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles%\Windows Kits\8.1\Debuggers\x86\cdb.exe"
  CALL :SET_CDB_IF_EXISTS "%ProgramFiles%\Windows Kits\8.0\Debuggers\x86\cdb.exe"
  EXIT /B 0

:SET_CDB_IF_EXISTS
  IF NOT DEFINED cdb (
    IF EXIST "%~1" (
      SET cdb="%~1"
    )
  )
  EXIT /B 0

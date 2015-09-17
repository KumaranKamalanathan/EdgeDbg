@ECHO OFF
SETLOCAL
IF NOT "%cdb:~0,0%" == "" (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET cdb=C:\Program Files\Windows Kits\8.1\Debuggers\x64\cdb.exe
  ) ELSE (
    SET cdb=C:\Program Files\Windows Kits\8.1\Debuggers\x86\cdb.exe
  )
)
SET cdb=%cdb:"=%
IF NOT EXIST "%cdb%" (
  ECHO - Cannot find cdb at "%cdb%", please set the "cdb" environment variable to the correct path.
  EXIT /B 1
)

IF NOT "%EdgeDbg:~0,0%" == "" (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET EdgeDbg=%~dp0EdgeDbg_x64.exe
  ) ELSE (
    SET EdgeDbg=%~dp0EdgeDbg_x86.exe
  )
)
SET EdgeDbg=%EdgeDbg:"=%
IF NOT EXIST "%EdgeDbg%" (
  ECHO - Cannot find EdgeDbg at "%EdgeDbg%", please set the "EdgeDbg" environment variable to the correct path.
  EXIT /B 1
)

IF NOT "%BugId:~0,0%" == "" (
  SET BugId=%~dp0..\BugId\BugId.py
)
SET BugId=%BugId:"=%
IF NOT EXIST "%BugId%" (
  ECHO - Cannot find BugId at "%BugId%", please set the "BugId" environment variable to the correct path.
  EXIT /B 1
)

IF NOT "%PYTHON:~0,0%" == "" (
  SET PYTHON=C:\Python27\python.exe
)
SET PYTHON=%PYTHON:"=%
IF NOT EXIST "%PYTHON%" (
  ECHO - Cannot find Python at "%PYTHON%", please set the "PYTHON" environment variable to the correct path.
  EXIT /B 1
)

If "%~1" == "" (
  SET URL=http://%COMPUTERNAME%:28876/
  SET BugIdArguments=
) ELSE (
  SET URL=%1
  SET BugIdArguments=%2 %3 %4 %5 %6 %7 %8 %9
)

ECHO * Terminating any running instances of Microsoft Edge...
TASKKILL /F /IM MicrosoftEdge.exe 2>nul >nul
TASKKILL /F /IM browser_broker.exe 2>nul >nul
TASKKILL /F /IM RuntimeBroker.exe 2>nul >nul
TASKKILL /F /IM MicrosoftEdgeCP.exe 2>nul >nul

IF EXIST "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" (
  ECHO * Deleting crash recovery data...
  DEL "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" /Q >nul
)

ECHO * Starting Edge in BugId...
ECHO + URL: %URL%
"%EdgeDbg%" "%URL%" "%PYTHON%" "%BugId%" --pids=@ProcessIds@ %BugIdArguments%

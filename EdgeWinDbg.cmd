@ECHO OFF
SETLOCAL
IF NOT "%WinDbg:~0,0%" == "" (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET WinDbg=C:\Program Files\Windows Kits\8.1\Debuggers\x64\windbg.exe
  ) ELSE (
    SET WinDbg=C:\Program Files\Windows Kits\8.1\Debuggers\x86\windbg.exe
  )
)
SET WinDbg=%WinDbg:"=%
IF NOT EXIST "%WinDbg%" (
  ECHO - Cannot find windbg at "%WinDbg%", please set the "WinDbg" environment variable to the correct path.
  EXIT /B 1
)

IF NOT "%EdgeDbg:~0,0%" == "" (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET EdgeDbg=%~dp0bin\EdgeDbg_x64.exe
  ) ELSE (
    SET EdgeDbg=%~dp0bin\EdgeDbg_x86.exe
  )
)
SET EdgeDbg=%EdgeDbg:"=%
IF NOT EXIST "%EdgeDbg%" (
  ECHO - Cannot find EdgeDbg at "%EdgeDbg%", please set the "EdgeDbg" environment variable to the correct path.
  EXIT /B 1
)

IF NOT "%Kill:~0,0%" == "" (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET Kill=%~dp0modules\Kill\bin\Kill_x64.exe
  ) ELSE (
    SET Kill=%~dp0modules\Kill\bin\Kill_x86.exe
  )
)
SET Kill=%Kill:"=%
IF NOT EXIST "%Kill%" (
  ECHO - Cannot find Kill at "%Kill%", please set the "Kill" environment variable to the correct path.
  EXIT /B 1
)

IF "%~1" == "" (
  SET URL=http://%COMPUTERNAME%:28876/
  SET WinDbgArguments=
) ELSE (
  SET URL=%~1
  SET WinDbgArguments=%2 %3 %4 %5 %6 %7 %8 %9
)

ECHO * Terminating all running processes associated with Edge...
"%Kill%" MicrosoftEdge.exe browser_broker.exe RuntimeBroker.exe MicrosoftEdgeCP.exe ApplicationFrameHost.exe
IF ERRORLEVEL 1 (
  ECHO - Cannot terminate all running processes associated with Edge.
  EXIT /B 1
)

IF EXIST "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" (
  ECHO * Deleting crash recovery data...
  DEL "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" /Q >nul
)

:: 1) Attach to first process through "-p" command line argument
:: 2) Attach to remaining three processes using ".attach" & "g"
:: We cannot resume the processes yet, as attaching requires "g", which will cause the processes to run, which may
:: trigger an exception in one of the processes, which will then interfere with the remaining commands we are going to
:: execute and everything will be one big mess.
:: 3) Resume threads in current process (last attached process) and enable child debugging
:: 4) And resume first three processes (set current and resume) and enable child debugging
:: 5) Continue the processes ("g")
:: This way all processes are resumed at the same time and all child processes are debugged.
ECHO * Starting Edge in WinDbg...
ECHO + URL: %URL%

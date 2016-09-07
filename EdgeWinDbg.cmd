@ECHO OFF
SETLOCAL
IF "%PROCESSOR_ARCHITEW6432%" == "AMD64" (
  SET OSISA=x64
) ELSE IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
  SET OSISA=x64
) ELSE (
  SET OSISA=x86
)

IF NOT DEFINED WinDbg (
  CALL :SET_WINDBG_IF_EXISTS "%ProgramFiles%\Windows Kits\10\Debuggers\%OSISA%\WinDbg.exe"
  CALL :SET_WINDBG_IF_EXISTS "%ProgramFiles%\Windows Kits\8.1\Debuggers\%OSISA%\WinDbg.exe"
  CALL :SET_WINDBG_IF_EXISTS "%ProgramFiles%\Windows Kits\8.0\Debuggers\%OSISA%\WinDbg.exe"
  IF EXIST "%ProgramFiles(x86)%" (
    CALL :SET_WINDBG_IF_EXISTS "%ProgramFiles(x86)%\Windows Kits\10\Debuggers\%OSISA%\WinDbg.exe"
    CALL :SET_WINDBG_IF_EXISTS "%ProgramFiles(x86)%\Windows Kits\8.1\Debuggers\%OSISA%\WinDbg.exe"
    CALL :SET_WINDBG_IF_EXISTS "%ProgramFiles(x86)%\Windows Kits\8.0\Debuggers\%OSISA%\WinDbg.exe"
  )
  IF NOT DEFINED WinDbg (
      ECHO - Cannot find WinDbg.exe, please set the "WinDbg" environment variable to the correct path.
      EXIT /B 1
  )
) ELSE (
  SET WinDbg="%WinDbg:"=%"
)
IF NOT EXIST %WinDbg% (
  ECHO - Cannot find WinDbg.exe at %WinDbg%, please set the "WinDbg" environment variable to the correct path.
  EXIT /B 1
)

IF NOT DEFINED EdgeDbg (
  SET EdgeDbg="%~dp0bin\EdgeDbg_%OSISA%.exe"
) ELSE (
  SET EdgeDbg="%EdgeDbg:"=%"
)
IF NOT EXIST %EdgeDbg% (
  ECHO - Cannot find EdgeDbg at %EdgeDbg%, please set the "EdgeDbg" environment variable to the correct path.
  EXIT /B 1
)

IF NOT DEFINED Kill (
  SET Kill="%~dp0modules\Kill\bin\Kill_%OSISA%.exe"
) ELSE (
  SET Kill="%Kill:"=%"
)
IF NOT EXIST %Kill% (
  ECHO - Cannot find Kill at %Kill%, please set the "Kill" environment variable to the correct path.
  EXIT /B 1
)

IF "%~1" == "" (
  SET URL="http://%COMPUTERNAME%:28876/"
  SET WinDbgArguments=
) ELSE (
  SET URL="%~1"
  SET WinDbgArguments=%2 %3 %4 %5 %6 %7 %8 %9
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
%EdgeDbg% %URL% %WinDbg% -o -p @MicrosoftEdge@ -c ".attach 0n@MicrosoftEdgeCP@;.attach 0n@browser_broker@;.attach 0n@RuntimeBroker@;.attach 0n@ApplicationFrameHost@;$<%~dpn0.script" %WinDbgArguments%
EXIT /B %ERRORLEVEL%

:SET_WINDBG_IF_EXISTS
  IF NOT DEFINED WinDbg (
    IF EXIST "%~1" (
      SET WinDbg="%~1"
    )
  )
  EXIT /B 0

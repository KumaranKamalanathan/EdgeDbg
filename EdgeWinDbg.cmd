@ECHO OFF
IF "%WinDbg%" == "" (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET WinDbg=C:\Program Files\Windows Kits\8.1\Debuggers\x64\windbg.exe
  ) ELSE (
    SET WinDbg=C:\Program Files\Windows Kits\8.1\Debuggers\x86\windbg.exe
  )
)
IF NOT EXIST "%WinDbg%" (
  ECHO - Cannot find WinDbg at %WinDbg%
  EXIT /B 1
)

IF "%EdgeDbg%" == "" (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET EdgeDbg=%~dp0Build\EdgeDbg_x64.exe
  ) ELSE (
    SET EdgeDbg=%~dp0Build\EdgeDbg_x86.exe
  )
)
IF NOT EXIST "%EdgeDbg%" (
  ECHO - Cannot find EdgeDbg at %EdgeDbg%
  EXIT /B 1
)

IF "%~1" == "" (
  SET URL=http://%COMPUTERNAME%:28876/
) ELSE (
  SET URL=%1
)

ECHO * Terminating any running Edge processes...
TASKKILL /F /IM MicrosoftEdge.exe 2>nul >nul
TASKKILL /F /IM browser_broker.exe 2>nul >nul
TASKKILL /F /IM RuntimeBroker.exe 2>nul >nul
TASKKILL /F /IM MicrosoftEdgeCP.exe 2>nul >nul

IF EXIST "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" (
  ECHO * Deleting crash recovery data...
  DEL "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" /Q >nul
)
ECHO * Starting Edge in WinDbg...
ECHO + URL: %URL%
:: 1) Attach to first process through "-p" command line argument
:: 2) Attach to remaining three processes using ".attach" & "g"
:: We cannot resume the processes yet, as attaching requires "g", which will cause the processes to run, which may
:: trigger an exception in one of the processes, which will then interfere with the remaining commands we are going to
:: execute and everything will be one big mess.
:: 3) Resume threads in current process (last attached process) and enable child debugging
:: 4) And resume first three processes (set current and resume) and enable child debugging
:: 5) Continue the processes ("g")
:: This way all processes are resumed at the same time and all child processes are debugged.
"%EdgeDbg%" "%URL%" "%WinDbg%" -o -p @MicrosoftEdge@ -c ".attach 0n@MicrosoftEdgeCP@;g;.attach 0n@browser_broker@;g;.attach 0n@RuntimeBroker@;g;~*m;.childdbg 1;|0s;~*m;.childdbg 1;|1s;~*m;.childdbg 1;|2s;~*m;.childdbg 1;g" %2 %3 %4 %5 %6 %7 %8 %9

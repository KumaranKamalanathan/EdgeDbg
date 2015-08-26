@ECHO OFF
IF "%~1" == "" (
  ECHO Usage:
  ECHO   %~n0 path\to\windbg.exe htp://url.to/open [optional windbg arguments]
  EXIT /B 0
)
IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
  SET EdgeDbg=%~dp0Build\EdgeDbg_x64.exe
) ELSE (
  SET EdgeDbg=%~dp0Build\EdgeDbg_x86.exe
)

ECHO * Terminating any running instancess of Microsoft Edge...
TASKKILL /F /IM MicrosoftEdge.exe 2>nul >nul
TASKKILL /F /IM browser_broker.exe 2>nul >nul
TASKKILL /F /IM RuntimeBroker.exe 2>nul >nul
TASKKILL /F /IM MicrosoftEdgeCP.exe 2>nul >nul

ECHO * Deleting crash recovery data...
DEL "%LOCALAPPDATA%\Packages\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\AC\MicrosoftEdge\User\Default\Recovery\Active\*.*" /Q >nul

ECHO * Starting Edge in Windbg...
:: 1) Attach to first process through "-p" command line argument
:: 2) Attach to remaining three processes using ".attach" & "g"
:: We cannot resume the processes yet, as attaching requires "g", which will cause the processes to run, which may
:: trigger an exception in one of the processes, which will then interfere with the remaining commands we are going to
:: execute and everything will be one big mess.
:: 3) Resume threads in current process (last attached process)
:: 4) And resume first three processes (set current and resume)
:: 5) Continue the processes ("g")
:: This way all processes are resumed at the same time.
"%EdgeDbg%" %2 %1 -o -p @MicrosoftEdge@ -c ".attach 0n@MicrosoftEdgeCP@;g;.attach 0n@browser_broker@;g;.attach 0n@RuntimeBroker@;g;~*m;|0s;~*m;|1s;~*m;|2s;~*m;g" %3 %4 %5 %6 %7 %8 %9

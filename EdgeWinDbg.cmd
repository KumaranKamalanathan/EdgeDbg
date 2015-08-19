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
"%EdgeDbg%" %2 %1 -o -p @MicrosoftEdge@ -c ".attach 0n@MicrosoftEdgeCP@;~*m;g;.attach 0n@browser_broker@;~*m;g;.attach 0n@RuntimeBroker@;~*m;g;~*m;g" %3 %4 %5 %6 %7 %8 %9

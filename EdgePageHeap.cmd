@ECHO OFF
SETLOCAL
NET SESSION >nul 2>&1
IF ERRORLEVEL 1 (
  ECHO - Must be run as administrator.
  EXIT /B 1
)
IF NOT DEFINED GFlags (
  IF "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET GFlags=C:\Program Files\Windows Kits\8.1\Debuggers\x64\gflags.exe
  ) ELSE (
    SET GFlags=C:\Program Files\Windows Kits\8.1\Debuggers\x86\gflags.exe
  )
)
SET GFlags=%GFlags:"=%
IF NOT EXIST "%GFlags%" (
  ECHO - Cannot find Global Flags at "%GFlags%", please set the "GFlags" environment variable to the correct path.
  EXIT /B 1
)
IF "%~1" == "OFF" (
  ECHO * Disabling page heap for Microsoft Edge processes...
  ECHO   * MicrosoftEdge.exe
  "%GFlags%" -i MicrosoftEdge.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  ECHO   * MicrosoftEdgeCP.exe
  "%GFlags%" -i MicrosoftEdgeCP.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  ECHO   * browser_broker.exe
  "%GFlags%" -i browser_broker.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  ECHO   * RuntimeBroker.exe
  "%GFlags%" -i RuntimeBroker.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  "%GFlags%" -i ApplicationFrameHost.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
) ELSE (
  ECHO * Enabling page heap for Microsoft Edge processes...
  ECHO   * MicrosoftEdge.exe
  "%GFlags%" -i MicrosoftEdge.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  "%GFlags%" -i MicrosoftEdge.exe +02109870 >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  ECHO   * MicrosoftEdgeCP.exe
  "%GFlags%" -i MicrosoftEdgeCP.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  "%GFlags%" -i MicrosoftEdgeCP.exe +02109870 >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  ECHO   * browser_broker.exe
  "%GFlags%" -i browser_broker.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  "%GFlags%" -i browser_broker.exe +02109870 >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  ECHO   * RuntimeBroker.exe
  "%GFlags%" -i RuntimeBroker.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  "%GFlags%" -i RuntimeBroker.exe +02109870 >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  ECHO   * ApplicationFrameHost.exe
  "%GFlags%" -i ApplicationFrameHost.exe -FFFFFFFF >nul
  IF ERRORLEVEL 1 GOTO :ERROR
  "%GFlags%" -i ApplicationFrameHost.exe +02109870 >nul
  IF ERRORLEVEL 1 GOTO :ERROR
)
EXIT /B 0

:ERROR
  ECHO - Error code %ERRORLEVEL%.
  EXIT /B %ERRORLEVEL%
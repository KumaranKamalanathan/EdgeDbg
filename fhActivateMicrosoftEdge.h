HRESULT fhActivateMicrosoftEdge(IApplicationActivationManager* pAAM, _TCHAR* sURL, BOOL bSuspendThreads,
    UINT uDebuggerCommandLineComponentsCount, _TCHAR** asDebuggerCommandLine) {
  DWORD dwMicrosoftEdgeProcessId;
  _tprintf(_T("* Activating Microsoft Edge and opening %s...\r\n"), sURL);
  HRESULT hResult = pAAM->ActivateApplication(sAUMID, sURL, AO_NONE, &dwMicrosoftEdgeProcessId);
  if (!SUCCEEDED(hResult)) {
    // I've seen a number of errors that appear to be temporary; trying again will work.
    hResult = pAAM->ActivateApplication(sAUMID, sURL, AO_NONE, &dwMicrosoftEdgeProcessId);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Failed to launch Microsoft Edge (HRESULT %08X, error %08X).\r\n"), hResult, GetLastError());
      return hResult;
    }
  }
  _tprintf(_T("+ %s process id = %d\r\n"), sMicrosoftEdgeExecutable, dwMicrosoftEdgeProcessId);
  // Wait for the MicrosoftEdgeCP.exe process to be launched, which happens last.
  DWORD dwMicrosoftEdgeCPProcessId;
  hResult = fhWaitAndGetProcessIdForExecutableName(sMicrosoftEdgeCPExecutable, dwMicrosoftEdgeCPProcessId);
  if (!SUCCEEDED(hResult)) return hResult;
  _tprintf(_T("+ %s process id = %d\r\n"), sMicrosoftEdgeCPExecutable, dwMicrosoftEdgeCPProcessId);
  if (bSuspendThreads) {
    // We can now suspend the MicrosoftEdge.exe process and the MicrosoftEdgeCP.exe process
    hResult = fhSuspendThreadsInProcessById(dwMicrosoftEdgeProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
    hResult = fhSuspendThreadsInProcessById(dwMicrosoftEdgeCPProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  // Get and suspend the runtime broker process
  DWORD dwRuntimeBrokerProcessId;
  BOOL bProcessFound;
  hResult = fhGetProcessIdForExecutableName(sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId, bProcessFound);
  if (!SUCCEEDED(hResult)) return hResult;
  if (!bProcessFound) {
    _tprintf(_T("- %s process not found.\r\n"), sRuntimeBrokerExecutable);
    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); // similar to cdb behavior
  }
  _tprintf(_T("+ %s process id = %d\r\n"), sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId);
  if (bSuspendThreads) {
    hResult = fhSuspendThreadsInProcessById(dwRuntimeBrokerProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  // Get and suspend the browser broker process
  DWORD dwBrowserBrokerProcessId;
  hResult = fhGetProcessIdForExecutableName(sBrowserBrokerExecutable, dwBrowserBrokerProcessId, bProcessFound);
  if (!SUCCEEDED(hResult)) return hResult;
  if (!bProcessFound) {
    _tprintf(_T("- %s process not found.\r\n"), sBrowserBrokerExecutable);
    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); // similar to cdb behavior
  }
  _tprintf(_T("+ %s process id = %d\r\n"), sBrowserBrokerExecutable, dwBrowserBrokerProcessId);
  if (bSuspendThreads) {
    hResult = fhSuspendThreadsInProcessById(dwBrowserBrokerProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  if (uDebuggerCommandLineComponentsCount > 0) {
    hResult = fhRunDebugger(
      dwMicrosoftEdgeProcessId, dwBrowserBrokerProcessId, dwRuntimeBrokerProcessId, dwMicrosoftEdgeCPProcessId, 
      uDebuggerCommandLineComponentsCount, asDebuggerCommandLine
    );
  }
  return hResult;
}

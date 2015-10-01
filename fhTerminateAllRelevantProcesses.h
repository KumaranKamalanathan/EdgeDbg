HRESULT fhTerminateAllRelevantProcesses() {
  _tprintf(_T("* Terminating any running instances of Microsoft Edge...\r\n"));
  HRESULT hResult = fTerminateAllProcessesForExecutableName(sMicrosoftEdgeExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fTerminateAllProcessesForExecutableName(sBrowserBrokerExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fTerminateAllProcessesForExecutableName(sRuntimeBrokerExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fTerminateAllProcessesForExecutableName(sMicrosoftEdgeCPExecutable);
  return hResult;
}

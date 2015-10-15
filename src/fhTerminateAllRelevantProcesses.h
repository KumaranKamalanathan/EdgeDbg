HRESULT fhTerminateAllRelevantProcesses() {
  _tprintf(_T("* Terminating any running instances of Microsoft Edge...\r\n"));
  HRESULT hResult = fhTerminateAllProcessesForExecutableName(sMicrosoftEdgeExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fhTerminateAllProcessesForExecutableName(sBrowserBrokerExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fhTerminateAllProcessesForExecutableName(sRuntimeBrokerExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fhTerminateAllProcessesForExecutableName(sMicrosoftEdgeCPExecutable);
  return hResult;
}

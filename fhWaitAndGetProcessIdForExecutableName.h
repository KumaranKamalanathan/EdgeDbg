HRESULT fhWaitAndGetProcessIdForExecutableName(const _TCHAR* sExecutableName, DWORD &dwProcessId) {
  BOOL bProcessFound = FALSE;
  HRESULT hResult;
  _tprintf(_T("* Waiting for %s process to start...\r\n"), sExecutableName);
  while (!bProcessFound) {
    hResult = fhGetProcessIdForExecutableName(sExecutableName, dwProcessId, bProcessFound);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Cannot wait for %s process to start (HRESULT %08X, error %08x).\r\n"), sExecutableName, hResult, GetLastError());
      return hResult;
    }
  }
  return hResult;
}

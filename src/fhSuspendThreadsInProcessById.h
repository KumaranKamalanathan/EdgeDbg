HRESULT fhSuspendThreadsInProcessById(DWORD dwProcessId) {
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
  HRESULT hResult;
  if (hProcess == NULL) {
    _tprintf(_T("- Cannot open process %d (error %08X).\r\n"), dwProcessId, GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    hResult = _NtSuspendProcess(hProcess);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Cannot suspend process %d (HRESULT %08X, error %08X).\r\n"), dwProcessId, hResult, GetLastError());
    } else {
      _tprintf(_T("* Suspended process %d.\r\n"), dwProcessId);
    }
    if (!fbCloseHandleAndUpdateResult(hProcess, hResult)) {
      _tprintf(_T("- Cannot close process %d (error %d).\r\n"), dwProcessId, GetLastError());
    }
  }
  return hResult;
}

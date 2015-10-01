HRESULT fhTerminateAllProcessesForExecutableName(const _TCHAR* sExecutableName) {
  HRESULT hResult;
  DWORD dwProcessId;
  BOOL bProcessFound, bProcessesKilled = FALSE;
  do {
    hResult = fhGetProcessIdForExecutableName(sExecutableName, dwProcessId, bProcessFound);
    if (!SUCCEEDED(hResult)) return hResult;
    if (bProcessFound) {
      HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
      if (!hProcess) {
        _tprintf(_T("- Cannot open process %d (error %08X).\r\n"), dwProcessId, GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
      }
      if (!TerminateProcess(hProcess, 0)) {
        _tprintf(_T("- Cannot terminate process %d (error %08X).\r\n"), dwProcessId, GetLastError());
        hResult = HRESULT_FROM_WIN32(GetLastError());
      } else if (WaitForSingleObject(hProcess, 0) == WAIT_TIMEOUT) {
        _tprintf(_T("- Cannot wait for termination of process %d (error %08X).\r\n"), dwProcessId, GetLastError());
        hResult = HRESULT_FROM_WIN32(GetLastError());
      } else {
        _tprintf(_T("* Terminated %s process %d.\r\n"), sExecutableName, dwProcessId);
        bProcessesKilled = TRUE;
      }
      if (!fbCloseHandleAndUpdateResult(hProcess, hResult)) {
        _tprintf(_T("- Cannot close process %d (error %08X).\r\n"), dwProcessId, GetLastError());
	    }
	    if (!SUCCEEDED(hResult)) return hResult;
    }
  } while (bProcessFound);
  return hResult;
}

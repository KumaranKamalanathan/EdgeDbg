HRESULT fhShowProcessIdsAndSuspendThreadsForExecutableName(const _TCHAR* sExecutableName, BOOL bSuspendThreads) {
  HANDLE hProcessesSnapshot;
  HRESULT hResult = fhGetSnapshot(TH32CS_SNAPPROCESS, 0, hProcessesSnapshot);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("- Cannot create processes snapshot (HRESULT %08X, error %08X).\r\n"), hResult, GetLastError());
  } else {
    PROCESSENTRY32 oProcessEntry32;
    oProcessEntry32.dwSize = sizeof(oProcessEntry32);
    if (!Process32First(hProcessesSnapshot, &oProcessEntry32)) {
      _tprintf(_T("- Cannot get first process from snapshot (error %08X).\r\n"), GetLastError());
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else do {
      HANDLE hModulesSnapshot;
      HRESULT hResult = fhGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
      if (SUCCEEDED(hResult)) { // ignore errors: we do not have access to all processes on the system.
        MODULEENTRY32 oModuleEntry32;
        oModuleEntry32.dwSize = sizeof(oModuleEntry32);
        if (!Module32First(hModulesSnapshot, &oModuleEntry32)) {
          _tprintf(_T("- Cannot get first module from snapshot (error %08X).\r\n"), GetLastError());
          hResult = HRESULT_FROM_WIN32(GetLastError());
        } else do {
          if (_tcscmp(oModuleEntry32.szModule, sExecutableName) == 0) {
            _tprintf(_T("+ %s process id = %d\r\n"), sExecutableName, oModuleEntry32.th32ProcessID);
            if (bSuspendThreads) {
              hResult = fhSuspendThreadsInProcessById(oModuleEntry32.th32ProcessID);
            }
          }
        } while (SUCCEEDED(hResult) && Module32Next(hModulesSnapshot, &oModuleEntry32));
        if (!fbCloseHandleAndUpdateResult(hModulesSnapshot, hResult)) {
          _tprintf(_T("- Cannot close modules snapshot (error %08X).\r\n"), GetLastError());
        }
      }
    } while (SUCCEEDED(hResult) && Process32Next(hProcessesSnapshot, &oProcessEntry32));
    if (!fbCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
      _tprintf(_T("- Cannot close processes snapshot (error %08X).\r\n"), GetLastError());
    }
  }
  return hResult;
}

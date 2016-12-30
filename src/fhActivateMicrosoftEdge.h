HRESULT fhActivateMicrosoftEdge(
  IApplicationActivationManager* pAAM,
  _TCHAR* sURL,
  BOOL bSuspendThreads,
  UINT uDebuggerCommandLineComponentsCount,
  _TCHAR** asDebuggerCommandLine
) {
  DWORD dwMicrosoftEdgeProcessId;
  _tprintf(_T("* Activating Microsoft Edge and opening %s...\r\n"), sURL);
  HRESULT hResult = pAAM->ActivateApplication(sAUMID, sURL, AO_NONE, &dwMicrosoftEdgeProcessId);
  if (!SUCCEEDED(hResult)) {
    // I've seen a number of errors that appear to be temporary; trying again will work.
    hResult = pAAM->ActivateApplication(sAUMID, sURL, AO_NONE, &dwMicrosoftEdgeProcessId);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Failed to launch Microsoft Edge (HRESULT %08X, error %08X).\r\n"), hResult, GetLastError());
      return hResult;
    };
  };
  _tprintf(_T("+ %s process id = %d\r\n"), sMicrosoftEdgeExecutable, dwMicrosoftEdgeProcessId);
  // MicrosoftEdge.exe is now running
  
  // Find all running processes associated with Microsoft Edge, suspend them and get their process ids:
  // Wait until all processes except MicrosoftEdgeCP.exe are started (the later does not need to be started).
  _tprintf(_T("* Waiting for relevant processes to start...\r\n"));
  DWORD dwApplicationFrameHostProcessId = 0,
        dwBrowserBrokerProcessId = 0,
        dwRuntimeBrokerProcessId = 0,
        dwMicrosoftEdgeCPProcessId = 0;
  BOOL bOneAdditionalRoundExecuted;
  do {
    bOneAdditionalRoundExecuted = dwApplicationFrameHostProcessId != 0 && dwBrowserBrokerProcessId != 0 && dwRuntimeBrokerProcessId != 0;
    HANDLE hProcessesSnapshot;
    hResult = fhGetSnapshot(TH32CS_SNAPPROCESS, 0, hProcessesSnapshot);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Cannot create processes snapshot (HERSULT %08X, error %08X).\r\n"), hResult, GetLastError());
      return hResult;
    };
    PROCESSENTRY32 oProcessEntry32;
    oProcessEntry32.dwSize = sizeof(oProcessEntry32);
    if (!Process32First(hProcessesSnapshot, &oProcessEntry32)) {
      hResult = HRESULT_FROM_WIN32(GetLastError());
      _tprintf(_T("- Cannot get first process from snapshot (HERSULT %08X, error %08X).\r\n"), hResult, GetLastError());
      return hResult;
    };
    do {
      // Get a module snapshot of the process. This may fail, as access may be denied. This is ignored.
      HANDLE hModulesSnapshot;
      HRESULT hSnapshotResult = fhGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
      if (!SUCCEEDED(hSnapshotResult)) continue;
      // We seem to have access to the module list, check if we can get the first module.
      MODULEENTRY32 oModuleEntry32;
      oModuleEntry32.dwSize = sizeof(oModuleEntry32);
      if (!Module32First(hModulesSnapshot, &oModuleEntry32)) {
        if (GetLastError() != ERROR_NO_MORE_FILES) {
          // No: if this is not a common (temporary) error, return an error
          hResult = HRESULT_FROM_WIN32(GetLastError());
          _tprintf(_T("- Cannot get first module from snapshot (HERSULT %08X, error %08X).\r\n"), hResult, GetLastError());
          if (!CloseHandle(hModulesSnapshot)) {
            _tprintf(_T("- Cannot close modules snapshot (HERSULT %08X, error %08X).\r\n"), HRESULT_FROM_WIN32(GetLastError()), GetLastError());
          };
          if (!CloseHandle(hProcessesSnapshot)) {
            _tprintf(_T("- Cannot close processes snapshot (HERSULT %08X, error %08X).\r\n"), HRESULT_FROM_WIN32(GetLastError()), GetLastError());
          };
          return hResult;
        };
      } else {
        // If we can access the first module, scan the module list for the desired executable names.
        do {
          if (dwApplicationFrameHostProcessId == 0 && _tcsicmp(oModuleEntry32.szModule, sApplicationFrameHostExecutable) == 0) {
            dwApplicationFrameHostProcessId = oModuleEntry32.th32ProcessID;
            _tprintf(_T("+ %s process id = %d\r\n"), sApplicationFrameHostExecutable, dwApplicationFrameHostProcessId);
            if (bSuspendThreads) {
              hResult = fhSuspendThreadsInProcessById(dwApplicationFrameHostProcessId);
              if (!SUCCEEDED(hResult)) {
                _tprintf(_T("- Cannot suspend %s process %d (HERSULT %08X, error %08X).\r\n"),
                    sApplicationFrameHostExecutable, dwApplicationFrameHostProcessId, hResult, GetLastError());
              };
            }
          } else if (dwBrowserBrokerProcessId == 0 && _tcsicmp(oModuleEntry32.szModule, sBrowserBrokerExecutable) == 0) {
            dwBrowserBrokerProcessId = oModuleEntry32.th32ProcessID;
            _tprintf(_T("+ %s process id = %d\r\n"), sBrowserBrokerExecutable, dwBrowserBrokerProcessId);
            if (bSuspendThreads) {
              hResult = fhSuspendThreadsInProcessById(dwBrowserBrokerProcessId);
              if (!SUCCEEDED(hResult)) {
                _tprintf(_T("- Cannot suspend %s process %d (HERSULT %08X, error %08X).\r\n"),
                    sBrowserBrokerExecutable, dwBrowserBrokerProcessId, hResult, GetLastError());
              };
            }
          } else if (dwRuntimeBrokerProcessId == 0 && _tcsicmp(oModuleEntry32.szModule, sRuntimeBrokerExecutable) == 0) {
            dwRuntimeBrokerProcessId = oModuleEntry32.th32ProcessID;
            _tprintf(_T("+ %s process id = %d\r\n"), sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId);
            if (bSuspendThreads) {
              hResult = fhSuspendThreadsInProcessById(dwRuntimeBrokerProcessId);
              if (!SUCCEEDED(hResult)) {
                _tprintf(_T("- Cannot suspend %s process %d (HERSULT %08X, error %08X).\r\n"),
                    sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId, hResult, GetLastError());
              };
            }
          } else if (dwMicrosoftEdgeCPProcessId == 0 && _tcsicmp(oModuleEntry32.szModule, sMicrosoftEdgeCPExecutable) == 0) {
            dwMicrosoftEdgeCPProcessId = oModuleEntry32.th32ProcessID;
            _tprintf(_T("+ %s process id = %d\r\n"), sMicrosoftEdgeCPExecutable, dwMicrosoftEdgeCPProcessId);
            if (bSuspendThreads) {
              hResult = fhSuspendThreadsInProcessById(dwMicrosoftEdgeCPProcessId);
              if (!SUCCEEDED(hResult)) {
                _tprintf(_T("- Cannot suspend %s process %d (HERSULT %08X, error %08X).\r\n"),
                    sMicrosoftEdgeCPExecutable, dwMicrosoftEdgeCPProcessId, hResult, GetLastError());
              };
            }
          };
          if (!SUCCEEDED(hResult)) {
            // Failed to suspend a newly discovered process
            if (!CloseHandle(hModulesSnapshot)) {
              _tprintf(_T("- Cannot close modules snapshot (HERSULT %08X, error %08X).\r\n"), HRESULT_FROM_WIN32(GetLastError()), GetLastError());
            };
            if (!CloseHandle(hProcessesSnapshot)) {
              _tprintf(_T("- Cannot close processes snapshot (HERSULT %08X, error %08X).\r\n"), HRESULT_FROM_WIN32(GetLastError()), GetLastError());
            };
            return hResult;
          };
        } while (
          // as long as we haven't found all of them yet and there are more modules to process.
          (dwApplicationFrameHostProcessId == 0 || dwBrowserBrokerProcessId == 0 || dwRuntimeBrokerProcessId == 0 || dwMicrosoftEdgeCPProcessId == 0)
          && Module32Next(hModulesSnapshot, &oModuleEntry32)
        );
      };
      if (!fbCloseHandleAndUpdateResult(hModulesSnapshot, hResult)) {
        _tprintf(_T("- Cannot close modules snapshot (HERSULT %08X, error %08X).\r\n"), hResult, GetLastError());
        if (!CloseHandle(hProcessesSnapshot)) {
          _tprintf(_T("- Cannot close processes snapshot (HERSULT %08X, error %08X).\r\n"), HRESULT_FROM_WIN32(GetLastError()), GetLastError());
        };
        return hResult;
      };
    } while (
      (dwApplicationFrameHostProcessId == 0 || dwBrowserBrokerProcessId == 0 || dwRuntimeBrokerProcessId == 0 || dwMicrosoftEdgeCPProcessId == 0)
      && Process32Next(hProcessesSnapshot, &oProcessEntry32)
    );
    if (!fbCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
      _tprintf(_T("- Cannot close processes snapshot (HERSULT %08X, error %08X).\r\n"), hResult, GetLastError());
      return hResult;
    };
  } while (
    (dwApplicationFrameHostProcessId == 0 || dwBrowserBrokerProcessId == 0 || dwRuntimeBrokerProcessId == 0)
    && !bOneAdditionalRoundExecuted
  );
  // Run the debugger
  if (uDebuggerCommandLineComponentsCount > 0) {
    hResult = fhRunDebugger(
      dwMicrosoftEdgeProcessId, dwBrowserBrokerProcessId, dwRuntimeBrokerProcessId, dwMicrosoftEdgeCPProcessId, 
      dwApplicationFrameHostProcessId, uDebuggerCommandLineComponentsCount, asDebuggerCommandLine
    );
  }
  return hResult;
}

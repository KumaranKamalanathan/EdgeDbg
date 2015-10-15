UINT fuEdgeDbg(UINT uArgumentsCount, _TCHAR* asArguments[]) {
  HRESULT hResult;
  _IsWow64Process = (tIsWow64Process)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");
  _NtSuspendProcess = (tNtSuspendProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtSuspendProcess");
  
  BOOL bIsWow64 = FALSE;
  if (_IsWow64Process && !_IsWow64Process(GetCurrentProcess(), &bIsWow64)) {
    _tprintf(_T("- Cannot determine if this is a 64-bit version of Windows (error %08X).\r\n"), GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else if (bIsWow64) {
    _tprintf(_T("- The 32-bit build of EdgeDbg does not work on a 64-bit build of Windows.\r\n"));
    hResult = E_NOTIMPL;
  } else if (!_NtSuspendProcess) {
    _tprintf(_T("- Cannot find ntdll!NtSuspendProcess.\r\n"));
    hResult = E_NOTIMPL;
  } else if (uArgumentsCount < 2) {
    _tprintf(_T("Usage:\r\n"));
    _tprintf(_T("  EdgeDbg <url>\r\n"));
    _tprintf(_T("  EdgeDbg <url> --suspend\r\n"));
    _tprintf(_T("  EdgeDbg <url> <debugger command line>\r\n"));
    _tprintf(_T("See documentation at https://github.com/SkyLined/EdgeDbg/ for more details.\r\n"));
    hResult = E_INVALIDARG;
  } else  {
    hResult = CoInitialize(NULL);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Failed to initialize (HRESULT %08X, error %08X).\r\n"), hResult, GetLastError());
    } else {
      IApplicationActivationManager* pAAM;
      hResult = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pAAM));
      if (!SUCCEEDED(hResult)) {
        _tprintf(_T("- Failed to create application activation manager (HRESULT %08X, error %08X).\r\n"), hResult, GetLastError());
      } else {
        hResult = fhTerminateAllRelevantProcesses();
        if (SUCCEEDED(hResult)) {
          BOOL bSuspendThreads = uArgumentsCount > 2;
          // We start a process (the debugger we presume) using everything starting at our second argument as the
          // command line:
          UINT uDebuggerArgumentCount = uArgumentsCount - 2;
          if (uArgumentsCount == 3 && _tcscmp(asArguments[2], _T("--suspend")) == 0) {
            // Unless the second argument is the only one and it is "--suspend": then we just suspend the processes.
            uDebuggerArgumentCount = 0;
          };
          hResult = fhActivateMicrosoftEdge(pAAM, asArguments[1], bSuspendThreads, uDebuggerArgumentCount, asArguments + 2);
        }
        pAAM->Release();
      }
    }
    CoUninitialize();
  }
  return hResult;
}

UINT fuKill(UINT uArgumentsCount, _TCHAR* asArguments[]) {
  HRESULT hResult;
  _IsWow64Process = (tIsWow64Process)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");
//  _NtSuspendProcess = (tNtSuspendProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtSuspendProcess");
  
  BOOL bIsWow64 = FALSE;
  if (_IsWow64Process && !_IsWow64Process(GetCurrentProcess(), &bIsWow64)) {
    _tprintf(_T("- Cannot determine if this is a 64-bit version of Windows (error %08X).\r\n"), GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else if (bIsWow64) {
    _tprintf(_T("- The 32-bit build of EdgeDbg does not work on a 64-bit build of Windows.\r\n"));
    hResult = E_NOTIMPL;
//  } else if (!_NtSuspendProcess) {
//    _tprintf(_T("- Cannot find ntdll!NtSuspendProcess.\r\n"));
//    hResult = E_NOTIMPL;
  } else if (uArgumentsCount < 2) {
    _tprintf(_T("Usage:\r\n"));
    _tprintf(_T("  Kill binary_name.exe [another_binary_name.exe [...]]\r\n"));
    _tprintf(_T("See documentation at https://github.com/SkyLined/EdgeDbg/ for more details.\r\n"));
    hResult = E_INVALIDARG;
  } else  {
    for (UINT uArgumentIndex = 1; uArgumentIndex < uArgumentsCount; uArgumentIndex++) {
      _tprintf(_T("* Killing %s\r\n"), asArguments[uArgumentIndex]);
      hResult = fhTerminateAllProcessesForExecutableName(asArguments[uArgumentIndex]);
      if (!SUCCEEDED(hResult)) break;
    }
  }
  return hResult;
}

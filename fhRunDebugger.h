HRESULT fhRunDebugger(
    DWORD dwMicrosoftEdgeProcessId, DWORD dwBrowserBrokerProcessId, DWORD dwRuntimeBrokerProcessId,
    DWORD dwMicrosoftEdgeCPProcessId, UINT uCommandLineCount, _TCHAR* asCommandLine[]
) {
  std::basic_string<TCHAR> sCommandLine = _T("");
  #ifdef UNICODE
    std::basic_string<TCHAR> sMicrosoftEdgeProcessId = std::to_wstring(dwMicrosoftEdgeProcessId);
    std::basic_string<TCHAR> sBrowserBrokerProcessId = std::to_wstring(dwBrowserBrokerProcessId);
    std::basic_string<TCHAR> sRuntimeBrokerProcessId = std::to_wstring(dwRuntimeBrokerProcessId);
    std::basic_string<TCHAR> sMicrosoftEdgeCPProcessId = std::to_wstring(dwMicrosoftEdgeCPProcessId);
  #else
    std::basic_string<TCHAR> sMicrosoftEdgeProcessId = std::to_string(dwMicrosoftEdgeProcessId);
    std::basic_string<TCHAR> sBrowserBrokerProcessId = std::to_string(dwBrowserBrokerProcessId);
    std::basic_string<TCHAR> sRuntimeBrokerProcessId = std::to_string(dwRuntimeBrokerProcessId);
    std::basic_string<TCHAR> sMicrosoftEdgeCPProcessId = std::to_string(dwMicrosoftEdgeCPProcessId);
  #endif
  std::basic_string<TCHAR> sAllProcessIds = sRuntimeBrokerProcessId + _T(",") + sBrowserBrokerProcessId +  _T(",") +
                                            sMicrosoftEdgeProcessId + _T(",") + sMicrosoftEdgeCPProcessId;
  for (UINT uIndex = 0; uIndex < uCommandLineCount; uIndex++) {
    if (uIndex > 0) sCommandLine += _T(" ");
    std::basic_string<TCHAR> sArgument = asCommandLine[uIndex];
    fReplaceAll(sArgument, _T("@MicrosoftEdge@"), sMicrosoftEdgeProcessId);
    fReplaceAll(sArgument, _T("@browser_broker@"), sBrowserBrokerProcessId);
    fReplaceAll(sArgument, _T("@RuntimeBroker@"), sRuntimeBrokerProcessId);
    fReplaceAll(sArgument, _T("@MicrosoftEdgeCP@"), sMicrosoftEdgeCPProcessId);
    fReplaceAll(sArgument, _T("@ProcessIds@"), sAllProcessIds);
    if (sArgument.find(_T(" ")) != std::basic_string<TCHAR>::npos) { // If the argument contains spaces, quotes are needed
      fReplaceAll(sArgument, _T("\\"), _T("\\\\")); // escape all existing escapes.
      fReplaceAll(sArgument, _T("\""), _T("\\\"")); // escape all quotes
      sCommandLine += _T("\"") + sArgument + _T("\""); // add quoted argument
    } else {
      sCommandLine += sArgument;
    }
  }
  _tprintf(_T("* Starting %s\r\n"), sCommandLine.c_str());
  HRESULT hResult;
  STARTUPINFO oStartupInfo = {};
  oStartupInfo.cb = sizeof(oStartupInfo);
  oStartupInfo.dwFlags = STARTF_USESTDHANDLES;
  oStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  oStartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  oStartupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  PROCESS_INFORMATION oProcessInformation = {};
  if (!CreateProcess(NULL, (LPWSTR)sCommandLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &oStartupInfo, &oProcessInformation)) {
    _tprintf(_T("- Cannot start debugger (error %d).\r\n"), GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    if (WaitForSingleObject( oProcessInformation.hProcess, INFINITE ) != WAIT_OBJECT_0) {
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else {
      hResult = S_OK;
    }
    if (!fbCloseHandleAndUpdateResult(oProcessInformation.hProcess, hResult)) {
      _tprintf(_T("- Cannot close debugger process %d (error %08X).\r\n"), oProcessInformation.dwProcessId, GetLastError());
    }
    if (!fbCloseHandleAndUpdateResult(oProcessInformation.hThread, hResult)) {
      _tprintf(_T("- Cannot close debugger thread %d (error %08X).\r\n"), oProcessInformation.dwThreadId, GetLastError());
    }
  }
  return hResult;
}
HRESULT fTerminateAllProcessesForExecutableName(const _TCHAR* sExecutableName) {
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
      if (!TerminateProcess(hProcess, 0) && WaitForSingleObject(hProcess, 0) == WAIT_TIMEOUT) {
        _tprintf(_T("- Cannot terminate process %d (error %08X).\r\n"), dwProcessId, GetLastError());
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

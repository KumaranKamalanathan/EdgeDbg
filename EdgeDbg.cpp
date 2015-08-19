#include "stdafx.h"
typedef HRESULT(NTAPI *tNtSuspendProcess)(IN HANDLE);
tNtSuspendProcess _NtSuspendProcess;
typedef BOOL (WINAPI *tIsWow64Process)(HANDLE, PBOOL);
tIsWow64Process _IsWow64Process;

const _TCHAR* sAUMID = _T("Microsoft.MicrosoftEdge_8wekyb3d8bbwe!MicrosoftEdge");
const _TCHAR* sMicrosoftEdgeExecutable = _T("MicrosoftEdge.exe");
const _TCHAR* sBrowserBrokerExecutable = _T("browser_broker.exe");
const _TCHAR* sRuntimeBrokerExecutable = _T("RuntimeBroker.exe");
const _TCHAR* sMicrosoftEdgeCPExecutable = _T("MicrosoftEdgeCP.exe");

HRESULT fGetSnapshot(DWORD dwFlags, DWORD dwProcessId, HANDLE &hSnapshot) {
  HRESULT hResult;
  hSnapshot = CreateToolhelp32Snapshot(dwFlags, dwProcessId);
  if (hSnapshot == INVALID_HANDLE_VALUE) {
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    hResult = S_OK;
  }
  return hResult;
}
BOOL fCloseHandleAndUpdateResult(HANDLE hHandle, HRESULT &hResult) {
  // Close the handle, if this fails and the hResult is not an error, update hResult.
  // Return TRUE if the handle was successfully closed.
  if (!CloseHandle(hHandle)) {
    if (SUCCEEDED(hResult)) {
      hResult = HRESULT_FROM_WIN32(GetLastError());
    }
    return FALSE;
  }
  return TRUE;
}
HRESULT fGetProcessIdForExecutableName(const _TCHAR* sExecutableName, DWORD &dwProcessId, BOOL &bProcessFound) {
  bProcessFound = FALSE;
  HANDLE hProcessesSnapshot;
  HRESULT hResult = fGetSnapshot(TH32CS_SNAPPROCESS, 0, hProcessesSnapshot);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("- Cannot create processes snapshot (HERSULT %08X, error %08X).\r\n"), hResult, GetLastError());
    return hResult;
  }
  PROCESSENTRY32 oProcessEntry32;
  oProcessEntry32.dwSize = sizeof(oProcessEntry32);
  if (!Process32First(hProcessesSnapshot, &oProcessEntry32)) {
    _tprintf(_T("- Cannot get first process from snapshot (error %08X).\r\n"), GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else do {
    // Get a module snapshot of the process. This may fail, as access may be denied. This is ignored.
    HANDLE hModulesSnapshot;
    HRESULT hSnapshotResult = fGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
    if (SUCCEEDED(hSnapshotResult)) {
      // We have access to the module list, check if it is the requested process.
      MODULEENTRY32 oModuleEntry32;
      oModuleEntry32.dwSize = sizeof(oModuleEntry32);
      if (!Module32First(hModulesSnapshot, &oModuleEntry32)) {
        _tprintf(_T("- Cannot get first module from snapshot (error %08X).\r\n"), GetLastError());
        hResult = HRESULT_FROM_WIN32(GetLastError());
      } else do {
        if (_tcsicmp(oModuleEntry32.szModule, sExecutableName) == 0) {
          dwProcessId = oModuleEntry32.th32ProcessID;
          bProcessFound = TRUE;
        }
      } while (SUCCEEDED(hResult) && !bProcessFound && Module32Next(hModulesSnapshot, &oModuleEntry32));
      if (!fCloseHandleAndUpdateResult(hModulesSnapshot, hResult)) {
        _tprintf(_T("- Cannot close modules snapshot (error %08X).\r\n"), GetLastError());
      }
    }
  } while (SUCCEEDED(hResult) && !bProcessFound && Process32Next(hProcessesSnapshot, &oProcessEntry32));
  if (!fCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
    _tprintf(_T("- Cannot close processes snapshot (error %08X).\r\n"), GetLastError());
  }
  return hResult;
}
HRESULT fWaitAndGetProcessIdForExecutableName(const _TCHAR* sExecutableName, DWORD &dwProcessId) {
  BOOL bProcessFound = FALSE;
  HRESULT hResult;
  _tprintf(_T("* Waiting for %s process to start...\r\n"), sExecutableName);
  while (!bProcessFound) {
    hResult = fGetProcessIdForExecutableName(sExecutableName, dwProcessId, bProcessFound);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Cannot wait for %s process to start (HRESULT %08X, error %08x).\r\n"), sExecutableName, hResult, GetLastError());
      return hResult;
    }
  }
  return hResult;
}
HRESULT fSuspendThreadsInProcessById(DWORD dwProcessId) {
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
  HRESULT hResult;
  if (hProcess == NULL) {
    _tprintf(_T("- Cannot open process %d (error %08X).\r\n"), dwProcessId, GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    hResult = _NtSuspendProcess(hProcess);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("- Cannot suspend process %d (HRESULT %08X, error %08X).\r\n"), dwProcessId, hResult, GetLastError());
    }
    if (!fCloseHandleAndUpdateResult(hProcess, hResult)) {
      _tprintf(_T("- Cannot close process %d (error %d).\r\n"), dwProcessId, GetLastError());
    }
  }
  return hResult;
}
HRESULT fShowProcessIdsAndSuspendThreadsForExecutableName(const _TCHAR* sExecutableName, BOOL bSuspendThreads) {
  HANDLE hProcessesSnapshot;
  HRESULT hResult = fGetSnapshot(TH32CS_SNAPPROCESS, 0, hProcessesSnapshot);
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
      HRESULT hResult = fGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
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
              hResult = fSuspendThreadsInProcessById(oModuleEntry32.th32ProcessID);
            }
          }
        } while (SUCCEEDED(hResult) && Module32Next(hModulesSnapshot, &oModuleEntry32));
        if (!fCloseHandleAndUpdateResult(hModulesSnapshot, hResult)) {
          _tprintf(_T("- Cannot close modules snapshot (error %08X).\r\n"), GetLastError());
        }
      }
    } while (SUCCEEDED(hResult) && Process32Next(hProcessesSnapshot, &oProcessEntry32));
    if (!fCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
      _tprintf(_T("- Cannot close processes snapshot (error %08X).\r\n"), GetLastError());
    }
  }
  return hResult;
}
VOID fReplaceAll(std::basic_string<TCHAR> &sHayStack, std::basic_string<TCHAR> sNeedle, std::basic_string<TCHAR> sReplacement) {
  size_t uIndex = 0;
  while (uIndex < sNeedle.length()) {
    uIndex = sHayStack.find(sNeedle, uIndex);
    if (uIndex == std::basic_string<TCHAR>::npos) {
      break;
    }
    sHayStack.replace(uIndex, sNeedle.length(), sReplacement);
    uIndex += sNeedle.length(); 
  }
}
HRESULT fRunDebugger(
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
  std::basic_string<TCHAR> sAllProcessIds = sMicrosoftEdgeProcessId + _T(",") + sBrowserBrokerProcessId + _T(",") + 
                                            sRuntimeBrokerProcessId + _T(",") + sMicrosoftEdgeCPProcessId;
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
    if (!fCloseHandleAndUpdateResult(oProcessInformation.hProcess, hResult)) {
      _tprintf(_T("- Cannot close debugger process %d (error %08X).\r\n"), oProcessInformation.dwProcessId, GetLastError());
    }
    if (!fCloseHandleAndUpdateResult(oProcessInformation.hThread, hResult)) {
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
    hResult = fGetProcessIdForExecutableName(sExecutableName, dwProcessId, bProcessFound);
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
      if (!fCloseHandleAndUpdateResult(hProcess, hResult)) {
        _tprintf(_T("- Cannot close process %d (error %08X).\r\n"), dwProcessId, GetLastError());
	    }
	    if (!SUCCEEDED(hResult)) return hResult;
    }
  } while (bProcessFound);
  return hResult;
}
HRESULT fTerminateAllRelevantProcesses() {
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
HRESULT fActivateMicrosoftEdge(IApplicationActivationManager* pAAM, _TCHAR* sURL, BOOL bSuspendThreads,
    UINT uDebuggerCommandLineComponentsCount, _TCHAR** asDebuggerCommandLine) {
  DWORD dwMicrosoftEdgeProcessId;
  _tprintf(_T("* Activating Microsoft Edge application...\r\n"));
  HRESULT hResult = pAAM->ActivateApplication(sAUMID, sURL, AO_NONE, &dwMicrosoftEdgeProcessId);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("- Failed to launch Microsoft Edge (HRESULT %08X, error %08X).\r\n"), hResult, GetLastError());
    return hResult;
  }
  _tprintf(_T("+ %s process id = %d\r\n"), sMicrosoftEdgeExecutable, dwMicrosoftEdgeProcessId);
  // Wait for the MicrosoftEdgeCP.exe process to be launched, which happens last.
  DWORD dwMicrosoftEdgeCPProcessId;
  hResult = fWaitAndGetProcessIdForExecutableName(sMicrosoftEdgeCPExecutable, dwMicrosoftEdgeCPProcessId);
  if (!SUCCEEDED(hResult)) return hResult;
  _tprintf(_T("+ %s process id = %d\r\n"), sMicrosoftEdgeCPExecutable, dwMicrosoftEdgeCPProcessId);
  if (bSuspendThreads) {
    // We can now suspend the MicrosoftEdge.exe process and the MicrosoftEdgeCP.exe process
    hResult = fSuspendThreadsInProcessById(dwMicrosoftEdgeProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
    hResult = fSuspendThreadsInProcessById(dwMicrosoftEdgeCPProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  // Get and suspend the runtime broker process
  DWORD dwRuntimeBrokerProcessId;
  BOOL bProcessFound;
  hResult = fGetProcessIdForExecutableName(sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId, bProcessFound);
  if (!SUCCEEDED(hResult)) return hResult;
  if (!bProcessFound) {
    _tprintf(_T("- %s process not found.\r\n"), sRuntimeBrokerExecutable);
    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); // similar to cdb behavior
  }
  _tprintf(_T("+ %s process id = %d\r\n"), sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId);
  if (bSuspendThreads) {
    hResult = fSuspendThreadsInProcessById(dwRuntimeBrokerProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  // Get and suspend the browser broker process
  DWORD dwBrowserBrokerProcessId;
  hResult = fGetProcessIdForExecutableName(sBrowserBrokerExecutable, dwBrowserBrokerProcessId, bProcessFound);
  if (!SUCCEEDED(hResult)) return hResult;
  if (!bProcessFound) {
    _tprintf(_T("- %s process not found.\r\n"), sBrowserBrokerExecutable);
    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); // similar to cdb behavior
  }
  _tprintf(_T("+ %s process id = %d\r\n"), sBrowserBrokerExecutable, dwBrowserBrokerProcessId);
  if (bSuspendThreads) {
    hResult = fSuspendThreadsInProcessById(dwBrowserBrokerProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  if (uDebuggerCommandLineComponentsCount > 0) {
    hResult = fRunDebugger(
      dwMicrosoftEdgeProcessId, dwBrowserBrokerProcessId, dwRuntimeBrokerProcessId, dwMicrosoftEdgeCPProcessId, 
      uDebuggerCommandLineComponentsCount, asDebuggerCommandLine
    );
  }
  return hResult;
}
int _tmain(UINT uArgumentsCount, _TCHAR* asArguments[]) {
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
        hResult = fTerminateAllRelevantProcesses();
        if (SUCCEEDED(hResult)) {
          BOOL bSuspendThreads = uArgumentsCount > 2;
          // We start a process (the debugger we presume) using everything starting at our second argument as the
          // command line:
          UINT uDebuggerArgumentCount = uArgumentsCount - 2;
          if (uArgumentsCount == 3 && _tcscmp(asArguments[2], _T("--suspend")) == 0) {
            // Unless the second argument is the only one and it is "--suspend": then we just suspend the processes.
            uDebuggerArgumentCount = 0;
          };
          hResult = fActivateMicrosoftEdge(pAAM, asArguments[1], bSuspendThreads, uDebuggerArgumentCount, asArguments + 2);
        }
        pAAM->Release();
      }
    }
    CoUninitialize();
  }
  return hResult;
}

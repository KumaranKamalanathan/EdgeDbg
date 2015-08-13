#include "stdafx.h"
typedef HRESULT(NTAPI *tNtSuspendProcess)(IN HANDLE);
tNtSuspendProcess NtSuspendProcess;

const _TCHAR* sAUMID = _T("Microsoft.MicrosoftEdge_8wekyb3d8bbwe!MicrosoftEdge");
const _TCHAR* sMainExecutable = _T("MicrosoftEdge.exe");
const _TCHAR* sBrowserBrokerExecutable = _T("browser_broker.exe");
const _TCHAR* sRuntimeBrokerExecutable = _T("RuntimeBroker.exe");
const _TCHAR* sContentExecutable = _T("MicrosoftEdgeCP.exe");

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
    _tprintf(_T("Cannot create processes snapshot.\r\n"));
    return hResult;
  }
  PROCESSENTRY32 oProcessEntry32;
  oProcessEntry32.dwSize = sizeof(oProcessEntry32);
  if (!Process32First(hProcessesSnapshot, &oProcessEntry32)) {
    _tprintf(_T("Cannot get first process from snapshot (error %08x).\r\n"), GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else do {
    // Get a module snapshot of the process. This may fail, as access may be denied. This is ignored.
    HANDLE hModulesSnapshot;
    HRESULT hSnapshotResult = fGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
    if (SUCCEEDED(hSnapshotResult)) { // if it did not fail, check if it is the requested process.
      MODULEENTRY32 oModuleEntry32;
      oModuleEntry32.dwSize = sizeof(oModuleEntry32);
      if (!Module32First(hModulesSnapshot, &oModuleEntry32)) {
        _tprintf(_T("Cannot get first module from snapshot (error %08X).\r\n"), GetLastError());
        hResult = HRESULT_FROM_WIN32(GetLastError());
      } else do {
        if (_tcsicmp(oModuleEntry32.szModule, sExecutableName) == 0) {
          dwProcessId = oModuleEntry32.th32ProcessID;
          bProcessFound = TRUE;
        }
      } while (SUCCEEDED(hResult) && !bProcessFound && Module32Next(hModulesSnapshot, &oModuleEntry32));
      if (!fCloseHandleAndUpdateResult(hModulesSnapshot, hResult)) {
        _tprintf(_T("Cannot close modules snapshot.\r\n"));
      }
    }
  } while (SUCCEEDED(hResult) && !bProcessFound && Process32Next(hProcessesSnapshot, &oProcessEntry32));
  if (!fCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
    _tprintf(_T("Cannot close processes snapshot.\r\n"));
  }
  return hResult;
}
HRESULT fWaitAndGetProcessIdForExecutableName(const _TCHAR* sExecutableName, DWORD &dwProcessId) {
  BOOL bProcessFound = FALSE;
  HRESULT hResult;
  _tprintf(_T("// Waiting for %s process to start...\r\n"), sExecutableName);
  while (!bProcessFound) {
    hResult = fGetProcessIdForExecutableName(sExecutableName, dwProcessId, bProcessFound);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("Cannot wait for %s process to start.\r\n"), sExecutableName);
      return hResult;
    }
  }
  return hResult;
}
HRESULT fSuspendThreadsInProcessById(DWORD dwProcessId) {
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
  HRESULT hResult;
  if (hProcess == NULL) {
    _tprintf(_T("Cannot open process %d.\r\n"), dwProcessId);
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    hResult = NtSuspendProcess(hProcess);
    if (!SUCCEEDED(hResult)) {
      _tprintf(_T("Cannot suspend process %d.\r\n"), dwProcessId);
    }
    if (!fCloseHandleAndUpdateResult(hProcess, hResult)) {
      _tprintf(_T("Cannot close modules snapshot.\r\n"));
    }
  }
  return hResult;
}
HRESULT fShowProcessIdsAndSuspendThreadsForExecutableName(const _TCHAR* sExecutableName, BOOL bSuspendThreads) {
  HANDLE hProcessesSnapshot;
  HRESULT hResult = fGetSnapshot(TH32CS_SNAPPROCESS, 0, hProcessesSnapshot);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("Cannot create processes snapshot.\r\n"));
  } else {
    PROCESSENTRY32 oProcessEntry32;
    oProcessEntry32.dwSize = sizeof(oProcessEntry32);
    if (!Process32First(hProcessesSnapshot, &oProcessEntry32)) {
      _tprintf(_T("Cannot get first process from snapshot (error %08x).\r\n"), GetLastError());
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else do {
      HANDLE hModulesSnapshot;
      HRESULT hResult = fGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
      if (SUCCEEDED(hResult)) { // ignore errors: we do not have access to all processes on the system.
        MODULEENTRY32 oModuleEntry32;
        oModuleEntry32.dwSize = sizeof(oModuleEntry32);
        if (!Module32First(hModulesSnapshot, &oModuleEntry32)) {
          _tprintf(_T("Cannot get first module from snapshot (error %08X).\r\n"), GetLastError());
          hResult = HRESULT_FROM_WIN32(GetLastError());
        } else do {
          if (_tcscmp(oModuleEntry32.szModule, sExecutableName) == 0) {
            _tprintf(_T("%s process id = %d\r\n"), sExecutableName, oModuleEntry32.th32ProcessID);
            if (bSuspendThreads) {
              hResult = fSuspendThreadsInProcessById(oModuleEntry32.th32ProcessID);
            }
          }
        } while (SUCCEEDED(hResult) && Module32Next(hModulesSnapshot, &oModuleEntry32));
        if (!fCloseHandleAndUpdateResult(hModulesSnapshot, hResult)) {
          _tprintf(_T("Cannot close modules snapshot.\r\n"));
        }
      }
    } while (SUCCEEDED(hResult) && Process32Next(hProcessesSnapshot, &oProcessEntry32));
    if (!fCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
      _tprintf(_T("Cannot close processes snapshot.\r\n"));
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
    DWORD dwMainProcessId, DWORD dwBrowserBrokerProcessId, DWORD dwRuntimeBrokerProcessId, DWORD dwContentProcessId,
    UINT uCommandLineCount, _TCHAR* asCommandLine[]
) {
  std::basic_string<TCHAR> sCommandLine = _T("");
  #ifdef UNICODE
    std::basic_string<TCHAR> sMainProcessId = std::to_wstring(dwMainProcessId);
    std::basic_string<TCHAR> sBrowserBrokerProcessId = std::to_wstring(dwBrowserBrokerProcessId);
    std::basic_string<TCHAR> sRuntimeBrokerProcessId = std::to_wstring(dwRuntimeBrokerProcessId);
    std::basic_string<TCHAR> sContentProcessId = std::to_wstring(dwContentProcessId);
  #else
    std::basic_string<TCHAR> sMainProcessId = std::to_string(dwMainProcessId);
    std::basic_string<TCHAR> sBrowserBrokerProcessId = std::to_string(dwBrowserBrokerProcessId);
    std::basic_string<TCHAR> sRuntimeBrokerProcessId = std::to_string(dwRuntimeBrokerProcessId);
    std::basic_string<TCHAR> sContentProcessId = std::to_string(dwContentProcessId);
  #endif
  for (UINT uIndex = 0; uIndex < uCommandLineCount; uIndex++) {
    if (uIndex > 0) sCommandLine += _T(" ");
    std::basic_string<TCHAR> sArgument = asCommandLine[uIndex];
    fReplaceAll(sArgument, _T("@main_pid@"), sMainProcessId);
    fReplaceAll(sArgument, _T("@broker_pid@"), sBrowserBrokerProcessId);
    fReplaceAll(sArgument, _T("@runtime_pid@"), sRuntimeBrokerProcessId);
    fReplaceAll(sArgument, _T("@content_pid@"), sContentProcessId);
    if (sArgument.find(_T(" ")) != std::basic_string<TCHAR>::npos) { // If the argument contains spaces, quotes are needed
      fReplaceAll(sArgument, _T("\\"), _T("\\\\")); // escape all existing escapes.
      fReplaceAll(sArgument, _T("\""), _T("\\\"")); // escape all quotes
      sCommandLine += _T("\"") + sArgument + _T("\""); // add quoted argument
    } else {
      sCommandLine += sArgument;
    }
  }
  _tprintf(_T("Starting %s\r\n"), sCommandLine.c_str());
  HRESULT hResult;
  STARTUPINFO oStartupInfo = {};
  oStartupInfo.cb = sizeof(oStartupInfo);
  oStartupInfo.dwFlags = STARTF_USESTDHANDLES;
  oStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  oStartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  oStartupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  PROCESS_INFORMATION oProcessInformation = {};
  if (!CreateProcess(NULL, (LPWSTR)sCommandLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &oStartupInfo, &oProcessInformation)) {
    _tprintf(_T("Cannot start debugger (error %d).\r\n"), GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    if (WaitForSingleObject( oProcessInformation.hProcess, INFINITE ) != WAIT_OBJECT_0) {
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else {
      hResult = S_OK;
    }
    if (!fCloseHandleAndUpdateResult(oProcessInformation.hProcess, hResult)) {
      _tprintf(_T("Cannot close debugger process %d (error %08X).\r\n"), oProcessInformation.dwProcessId, GetLastError());
    }
    if (!fCloseHandleAndUpdateResult(oProcessInformation.hThread, hResult)) {
      _tprintf(_T("Cannot close debugger thread %d (error %08X).\r\n"), oProcessInformation.dwThreadId, GetLastError());
    }
  }
  return hResult;
}
HRESULT fKillProcessById(DWORD dwProcessId) {
  HRESULT hResult;
  HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
  if (!hProcess) {
    _tprintf(_T("Cannot open process %d (error %08X).\r\n"), dwProcessId, GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    if (!TerminateProcess(hProcess, 0) && WaitForSingleObject(hProcess, 0) == WAIT_TIMEOUT) {
      _tprintf(_T("Cannot terminate process %d (error %08X).\r\n"), dwProcessId, GetLastError());
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else {
      hResult = S_OK;
    }
    if (!fCloseHandleAndUpdateResult(hProcess, hResult)) {
      _tprintf(_T("Cannot close process %d.\r\n")), dwProcessId;
    }
  }
  return hResult;
}
HRESULT fTerminateProcessesForExecutableName(const _TCHAR* sExecutableName) {
  HRESULT hResult;
  DWORD dwProcessId;
  BOOL bProcessFound;
  do {
    hResult = fGetProcessIdForExecutableName(sExecutableName, dwProcessId, bProcessFound);
    if (!SUCCEEDED(hResult)) return hResult;
    if (bProcessFound) {
      hResult = fKillProcessById(dwProcessId);
      if (!SUCCEEDED(hResult)) return hResult;
      _tprintf(_T("// Terminated %s process %d.\r\n"), sExecutableName, dwProcessId);
    }
  } while (bProcessFound);
  return hResult;
}
HRESULT fTerminateAllRelevantProcesses() {
  HRESULT hResult = fTerminateProcessesForExecutableName(sMainExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fTerminateProcessesForExecutableName(sBrowserBrokerExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fTerminateProcessesForExecutableName(sRuntimeBrokerExecutable);
  if (!SUCCEEDED(hResult)) return hResult;
  hResult = fTerminateProcessesForExecutableName(sContentExecutable);
  return hResult;
}
HRESULT fActivateMicrosoftEdge(IApplicationActivationManager* pAAM, _TCHAR* sURL, BOOL bSuspendThreads,
    UINT uDebuggerCommandLineComponentsCount, _TCHAR** asDebuggerCommandLine) {
  DWORD dwMainProcessId;
  HRESULT hResult = pAAM->ActivateApplication(sAUMID, sURL, AO_NONE, &dwMainProcessId);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("Failed to launch Microsoft Edge.\r\n"));
    return hResult;
  }
  _tprintf(_T("%s process id = %d\r\n"), sMainExecutable, dwMainProcessId);
  // Wait for the content process to be launched, which happens last.
  DWORD dwContentProcessId;
  hResult = fWaitAndGetProcessIdForExecutableName(sContentExecutable, dwContentProcessId);
  if (!SUCCEEDED(hResult)) return hResult;
  _tprintf(_T("%s process id = %d\r\n"), sContentExecutable, dwContentProcessId);
  if (bSuspendThreads) {
    // We can now suspend the main process and the content process
    hResult = fSuspendThreadsInProcessById(dwMainProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
    hResult = fSuspendThreadsInProcessById(dwContentProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  // Get and suspend the runtime broker process
  DWORD dwRuntimeBrokerProcessId;
  BOOL bProcessFound;
  hResult = fGetProcessIdForExecutableName(sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId, bProcessFound);
  if (!SUCCEEDED(hResult)) return hResult;
  if (!bProcessFound) {
    _tprintf(_T("%s process not found\r\n"), sRuntimeBrokerExecutable);
    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); // similar to cdb behavior
  }
  _tprintf(_T("%s process id = %d\r\n"), sRuntimeBrokerExecutable, dwRuntimeBrokerProcessId);
  if (bSuspendThreads) {
    hResult = fSuspendThreadsInProcessById(dwRuntimeBrokerProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  // Get and suspend the browser broker process
  DWORD dwBrowserBrokerProcessId;
  hResult = fGetProcessIdForExecutableName(sBrowserBrokerExecutable, dwBrowserBrokerProcessId, bProcessFound);
  if (SUCCEEDED(hResult)) hResult;
  if (!bProcessFound) {
    _tprintf(_T("%s process not found\r\n"), sBrowserBrokerExecutable);
    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); // similar to cdb behavior
  }
  _tprintf(_T("%s process id = %d\r\n"), sBrowserBrokerExecutable, dwBrowserBrokerProcessId);
  if (bSuspendThreads) {
    hResult = fSuspendThreadsInProcessById(dwBrowserBrokerProcessId);
    if (!SUCCEEDED(hResult)) return hResult;
  }
  if (uDebuggerCommandLineComponentsCount > 0) {
    hResult = fRunDebugger(
      dwMainProcessId, dwBrowserBrokerProcessId, dwRuntimeBrokerProcessId, dwContentProcessId, 
      uDebuggerCommandLineComponentsCount, asDebuggerCommandLine
    );
  }
}
int _tmain(UINT uArgumentsCount, _TCHAR* asArguments[]) {
  HRESULT hResult;
  NtSuspendProcess = (tNtSuspendProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtSuspendProcess");
  if (!NtSuspendProcess) {
    _tprintf(_T("Cannot find NtSuspendProcess.\r\n"));
    hResult = E_NOTIMPL;
  } else if (uArgumentsCount < 2) {
    _tprintf(_T("Usage: EdgeDbg <url> <debugger command line>\r\n"));
    hResult = E_INVALIDARG;
  } else  {
    hResult = CoInitialize(NULL);
    if (!SUCCEEDED(hResult)) {
      _tprintf(L"Failed to initialize.\r\n");
    } else {
      IApplicationActivationManager* pAAM;
      hResult = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pAAM));
      if (!SUCCEEDED(hResult)) {
        _tprintf(_T("Failed to create application activation manager.\r\n"));
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

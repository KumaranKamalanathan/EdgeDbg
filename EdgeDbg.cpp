#include "stdafx.h"
typedef HRESULT(NTAPI *tNtSuspendProcess)(IN HANDLE);
tNtSuspendProcess NtSuspendProcess;

const _TCHAR* sAUMID = _T("Microsoft.MicrosoftEdge_8wekyb3d8bbwe!MicrosoftEdge");
const _TCHAR* sMainExecutable = _T("MicrosoftEdge.exe");
const _TCHAR* sBrokerExecutable = _T("browser_broker.exe");
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
HRESULT fGetProcessIdForExecutableName(const _TCHAR* sExecutableName, DWORD &dwProcessId) {
  BOOL bProcessFound = FALSE;
  HANDLE hProcessesSnapshot;
  HRESULT hResult = fGetSnapshot(TH32CS_SNAPPROCESS, 0, hProcessesSnapshot);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("Cannot create processes snapshot\r\n"));
  } else {
    PROCESSENTRY32 oProcessEntry32;
    oProcessEntry32.dwSize = sizeof(oProcessEntry32);
    if (!Process32First(hProcessesSnapshot, &oProcessEntry32)) {
      _tprintf(_T("Cannot get first process from snapshot (error %08x)\r\n"), GetLastError());
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else do {
      HANDLE hModulesSnapshot;
      HRESULT hResult = fGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
      if (SUCCEEDED(hResult)) { // ignore errors: we do not have access to all processes on the system.
        MODULEENTRY32 oModuleEntry32;
        oModuleEntry32.dwSize = sizeof(oModuleEntry32);
        if (!Module32First(hModulesSnapshot, &oModuleEntry32)) {
          _tprintf(_T("Cannot get first module from snapshot (error %08X)\r\n"), GetLastError());
          hResult = HRESULT_FROM_WIN32(GetLastError());
        } else do {
          if (_tcscmp(oModuleEntry32.szModule, sExecutableName) == 0) {
            dwProcessId = oModuleEntry32.th32ProcessID;
            bProcessFound = TRUE;
          }
        } while (SUCCEEDED(hResult) && !bProcessFound && Module32Next(hModulesSnapshot, &oModuleEntry32));
        if (!fCloseHandleAndUpdateResult(hModulesSnapshot, hResult)) {
          _tprintf(_T("Cannot close modules snapshot\r\n"));
        }
      }
    } while (SUCCEEDED(hResult) && !bProcessFound && Process32Next(hProcessesSnapshot, &oProcessEntry32));
    if (!bProcessFound) {
      dwProcessId = 0;
    }
    if (!fCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
      _tprintf(_T("Cannot close processes snapshot\r\n"));
    }
  }
  return hResult;
}
HRESULT fSuspendThreadsInProcessById(DWORD dwProcessId) {
  HANDLE hThreadSnapshot;
  HRESULT hResult = fGetSnapshot(TH32CS_SNAPTHREAD, dwProcessId, hThreadSnapshot);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("Cannot create threads snapshot for process %d\r\n"), dwProcessId);
  } else {
    THREADENTRY32 oThreadEntry32;
    oThreadEntry32.dwSize = sizeof(oThreadEntry32);
    if (!Thread32First(hThreadSnapshot, &oThreadEntry32)) {
      _tprintf(_T("Cannot get first thread from snapshot (error %08X)\r\n"), GetLastError());
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else do {
      if (oThreadEntry32.th32OwnerProcessID == dwProcessId) {
        HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, oThreadEntry32.th32ThreadID);
        if (!hThread) {
          _tprintf(_T("Cannot open thread %d of process %d (error %08X)\r\n"), oThreadEntry32.th32ThreadID, oThreadEntry32.th32OwnerProcessID, GetLastError());
          hResult = HRESULT_FROM_WIN32(GetLastError());
        } else {
          if (SuspendThread(hThread) == -1) {
            _tprintf(_T("Cannot suspend thread %d of process %d (error %08X)\r\n"), oThreadEntry32.th32ThreadID, oThreadEntry32.th32OwnerProcessID, GetLastError());
            hResult = HRESULT_FROM_WIN32(GetLastError());
          }
          if (!fCloseHandleAndUpdateResult(hThread, hResult)) {
            _tprintf(_T("Cannot close thread %d of process %d\r\n"), oThreadEntry32.th32ThreadID, oThreadEntry32.th32OwnerProcessID);
          }
          _tprintf(L"// Suspended thread %d.\r\n", oThreadEntry32.th32ThreadID);
        }
      }
    } while (SUCCEEDED(hResult) && Thread32Next(hThreadSnapshot, &oThreadEntry32));
    if (!fCloseHandleAndUpdateResult(hThreadSnapshot, hResult)) {
      _tprintf(_T("Cannot close threads snapshot\r\n"));
    }
  }
  return hResult;
}
HRESULT fShowProcessIdsAndSuspendThreadsForExecutableName(const _TCHAR* sExecutableName, BOOL bSuspendThreads) {
  HANDLE hProcessesSnapshot;
  HRESULT hResult = fGetSnapshot(TH32CS_SNAPPROCESS, 0, hProcessesSnapshot);
  if (!SUCCEEDED(hResult)) {
    _tprintf(_T("Cannot create processes snapshot\r\n"));
  } else {
    PROCESSENTRY32 oProcessEntry32;
    oProcessEntry32.dwSize = sizeof(oProcessEntry32);
    if (!Process32First(hProcessesSnapshot, &oProcessEntry32)) {
      _tprintf(_T("Cannot get first process from snapshot (error %08x)\r\n"), GetLastError());
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else do {
      HANDLE hModulesSnapshot;
      HRESULT hResult = fGetSnapshot(TH32CS_SNAPMODULE, oProcessEntry32.th32ProcessID, hModulesSnapshot);
      if (SUCCEEDED(hResult)) { // ignore errors: we do not have access to all processes on the system.
        MODULEENTRY32 oModuleEntry32;
        oModuleEntry32.dwSize = sizeof(oModuleEntry32);
        if (!Module32First(hModulesSnapshot, &oModuleEntry32)) {
          _tprintf(_T("Cannot get first module from snapshot (error %08X)\r\n"), GetLastError());
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
          _tprintf(_T("Cannot close modules snapshot\r\n"));
        }
      }
    } while (SUCCEEDED(hResult) && Process32Next(hProcessesSnapshot, &oProcessEntry32));
    if (!fCloseHandleAndUpdateResult(hProcessesSnapshot, hResult)) {
      _tprintf(_T("Cannot close processes snapshot\r\n"));
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
HRESULT fRunDebugger(DWORD dwMainProcessId, DWORD dwBrokerProcessId, UINT uCommandLineCount, _TCHAR* asCommandLine[]) {
  std::basic_string<TCHAR> sCommandLine = _T("");
  #ifdef UNICODE
    std::basic_string<TCHAR> sMainProcessId = std::to_wstring(dwMainProcessId);
    std::basic_string<TCHAR> sBrowserBrokerProcessId = std::to_wstring(dwBrokerProcessId);
  #else
    std::basic_string<TCHAR> sMainProcessId = std::to_string(dwMainProcessId);
    std::basic_string<TCHAR> sBrowserBrokerProcessId = std::to_string(dwBrokerProcessId);
  #endif
  for (UINT uIndex = 0; uIndex < uCommandLineCount; uIndex++) {
    if (uIndex > 0) sCommandLine += _T(" ");
    std::basic_string<TCHAR> sArgument = asCommandLine[uIndex];
    fReplaceAll(sArgument, _T("@main_pid@"), sMainProcessId);
    fReplaceAll(sArgument, _T("@broker_pid@"), sBrowserBrokerProcessId);
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
    _tprintf(_T("Cannot start debugger (error %d)\r\n"), GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    if (WaitForSingleObject( oProcessInformation.hProcess, INFINITE ) != WAIT_OBJECT_0) {
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else {
      hResult = S_OK;
    }
    if (!fCloseHandleAndUpdateResult(oProcessInformation.hProcess, hResult)) {
      _tprintf(_T("Cannot close debugger process %d (error %08X)\r\n"), oProcessInformation.dwProcessId, GetLastError());
    }
    if (!fCloseHandleAndUpdateResult(oProcessInformation.hThread, hResult)) {
      _tprintf(_T("Cannot close debugger thread %d (error %08X)\r\n"), oProcessInformation.dwThreadId, GetLastError());
    }
  }
  return hResult;
}
HRESULT fKillProcessById(DWORD dwProcessId) {
  HRESULT hResult;
  HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
  if (!hProcess) {
    _tprintf(_T("Cannot open process %d (error %08X)\r\n"), dwProcessId, GetLastError());
    hResult = HRESULT_FROM_WIN32(GetLastError());
  } else {
    if (!TerminateProcess(hProcess, 0) && WaitForSingleObject(hProcess, 0) == WAIT_TIMEOUT) {
      _tprintf(_T("Cannot terminate process %d (error %08X)\r\n"), dwProcessId, GetLastError());
      hResult = HRESULT_FROM_WIN32(GetLastError());
    } else {
      hResult = S_OK;
    }
    if (!fCloseHandleAndUpdateResult(hProcess, hResult)) {
      _tprintf(_T("Cannot close process %d\r\n")), dwProcessId;
    }
  }
  return hResult;
}
HRESULT fTerminateProcessesForExecutableName(const _TCHAR* sExecutableName) {
  HRESULT hResult;
  DWORD dwProcessId;
  do {
    hResult = fGetProcessIdForExecutableName(sExecutableName, dwProcessId);
    if (SUCCEEDED(hResult) && dwProcessId) {
      hResult = fKillProcessById(dwProcessId);
      if (SUCCEEDED(hResult)) {
        _tprintf(_T("// Terminated %s process %d\r\n"), sExecutableName, dwProcessId);
      }
    }
  } while (SUCCEEDED(hResult) && dwProcessId);
  return hResult;
}
int _tmain(UINT uArgumentsCount, _TCHAR* asArguments[]) {
  HRESULT hResult;
  NtSuspendProcess = (tNtSuspendProcess)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtSuspendProcess");
  if (!NtSuspendProcess) {
    _tprintf(_T("Cannot open process\r\n"));
    hResult = E_NOTIMPL;
  } else if (uArgumentsCount < 2) {
    _tprintf(_T("Usage: EdgeDbg <url> <debugger command line>\r\n"));
    hResult = E_INVALIDARG;
  } else  {
    hResult = CoInitialize(NULL);
    if (!SUCCEEDED(hResult)) {
      _tprintf(L"Failed to initialize\r\n");
    } else {
      IApplicationActivationManager* pAAM;
      hResult = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pAAM));
      if (!SUCCEEDED(hResult)) {
        _tprintf(_T("Failed to create Application Activation Manager\r\n"));
      } else {
        hResult = fTerminateProcessesForExecutableName(sMainExecutable);
        if (SUCCEEDED(hResult)) {
          hResult = fTerminateProcessesForExecutableName(sBrokerExecutable);
        }
        if (SUCCEEDED(hResult)) {
          hResult = fTerminateProcessesForExecutableName(sContentExecutable);
        }
        if (SUCCEEDED(hResult)) {
          DWORD dwMainProcessId;
          hResult = pAAM->ActivateApplication(sAUMID, asArguments[1], AO_NONE, &dwMainProcessId);
          if (!SUCCEEDED(hResult)) {
            _tprintf(_T("Failed to launch Microsoft Edge\r\n"));
          } else {
            _tprintf(_T("%s process id = %d\r\n"), sMainExecutable, dwMainProcessId);
            BOOL bSuspendThreads = uArgumentsCount > 2;
            if (bSuspendThreads) {
              hResult = fSuspendThreadsInProcessById(dwMainProcessId);
            }
            if (SUCCEEDED(hResult)) {
              DWORD dwBrokerProcessId;
              hResult = fGetProcessIdForExecutableName(_T("browser_broker.exe"), dwBrokerProcessId);
              if (SUCCEEDED(hResult)) {
                if (!dwBrokerProcessId) {
                  _tprintf(_T("Browser broker process not found!\r\n"));
                  hResult = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); // similar to cdb behavior
                } else {
                  _tprintf(_T("%s process id = %d\r\n"), sBrokerExecutable, dwBrokerProcessId);
                  if (bSuspendThreads) {
                    hResult = fSuspendThreadsInProcessById(dwBrokerProcessId);
                  }
                  if (SUCCEEDED(hResult)) {
                    BOOL bRunDebugger = uArgumentsCount > 3 || (uArgumentsCount == 3 && _tcscmp(asArguments[2], _T("--suspend")) != 0);
                     // The browser process was hopefully suspended before it started any content processes...
                    if (bRunDebugger) {
                      // ... but if this is not the case, and a debugger should be started, they will be killed now:
                      // The browser should recreate them automagically later and debugging can happen as if they were
                      // never created in the first place.
                      hResult = fTerminateProcessesForExecutableName(sContentExecutable);
                      if (SUCCEEDED(hResult)) {
                        hResult = fRunDebugger(dwMainProcessId, dwBrokerProcessId, uArgumentsCount - 2, asArguments + 2);
                      }
                    } else if (bSuspendThreads) {
                      // ... but if this is not the case, and if all threads should be suspended, make sure to include
                      // those in any content processes.
                      hResult = fShowProcessIdsAndSuspendThreadsForExecutableName(sContentExecutable, bSuspendThreads);
                    }
                  }
                }
              }
            }
          }
        }
      }
      pAAM->Release();
    }
    CoUninitialize();
  }
  return hResult;
}

#include <windows.h>
#include <tchar.h>
#include <Shobjidl.h>
#include <tlhelp32.h>
#include <string>

typedef HRESULT(NTAPI *tNtSuspendProcess)(IN HANDLE);
tNtSuspendProcess _NtSuspendProcess;
typedef BOOL (WINAPI *tIsWow64Process)(HANDLE, PBOOL);
tIsWow64Process _IsWow64Process;

const _TCHAR* sAUMID = _T("Microsoft.MicrosoftEdge_8wekyb3d8bbwe!MicrosoftEdge");
const _TCHAR* sMicrosoftEdgeExecutable = _T("MicrosoftEdge.exe");
const _TCHAR* sBrowserBrokerExecutable = _T("browser_broker.exe");
const _TCHAR* sRuntimeBrokerExecutable = _T("RuntimeBroker.exe");
const _TCHAR* sMicrosoftEdgeCPExecutable = _T("MicrosoftEdgeCP.exe");
const _TCHAR* sApplicationFrameHostExecutable = _T("ApplicationFrameHost.exe");

#include "Kill\src\fhGetSnapshot.h"
#include "Kill\src\fbCloseHandleAndUpdateResult.h"
#include "Kill\src\fhGetProcessIdForExecutableName.h"
#include "fhSuspendThreadsInProcessById.h"
#include "fReplaceAll.h"
#include "fhRunDebugger.h"
#include "Kill\src\fhProcessExistsForId.h"
#include "Kill\src\fhTerminateProcessForId.h"
#include "Kill\src\fhTerminateAllProcessesForExecutableName.h"
#include "fhTerminateAllRelevantProcesses.h"
#include "fhActivateMicrosoftEdge.h"
#include "fuEdgeDbg.h"

UINT _tmain(UINT uArgumentsCount, _TCHAR* asArguments[]) {
  return fuEdgeDbg(uArgumentsCount, asArguments);
}
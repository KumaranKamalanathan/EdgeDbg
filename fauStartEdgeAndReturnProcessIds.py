import os, re, subprocess, time;
# The CWD may not be this script's folder; make sure it looks there for modules first:
sBaseFolderPath = os.path.dirname(__file__);
for sPath in [sBaseFolderPath] + [os.path.join(sBaseFolderPath, x) for x in ["modules"]]:
  if sPath not in sys.path:
    sys.path.insert(0, sPath);

from FileSystem import FileSystem;
from dxEdgeDbgConfig import dxEdgeDbgConfig;
from fDeleteRecoveryData import fDeleteRecoveryData;
sOSISA = {"AMD64": "x64", "x86": "x86"}[os.getenv("PROCESSOR_ARCHITEW6432") or os.getenv("PROCESSOR_ARCHITECTURE")];

def fauStartEdgeAndReturnProcessIds(sURL = None, bDeleteRecoveryData = True):
    # This does not always work as expected, try again and again if possible.
  for x in xrange(60):
    if bDeleteRecoveryData:
      fDeleteRecoveryData();
    sEdgDbgBinaryPath = dxEdgeDbgConfig["sEdgeDbgBinaryPath_%s" % sOSISA];
    assert sEdgDbgBinaryPath and FileSystem.fbIsFile(sEdgDbgBinaryPath), \
        "No %s EdgDbg binary found at %s" % (sOSISA, sEdgDbgBinaryPath);
    asEdgeDbgCommand = [sEdgDbgBinaryPath] + (sURL is not None and [sURL] or []) + ["--suspend"];
    oEdgeDbgProcess = subprocess.Popen(asEdgeDbgCommand, stdout = subprocess.PIPE, stderr = subprocess.PIPE);
    (sStdOut, sStdErr) = oEdgeDbgProcess.communicate();
    oEdgeDbgProcess.stdout.close();
    oEdgeDbgProcess.stderr.close();
    oEdgeDbgProcess.wait();
    assert not sStdErr, "Error running EdgeDbg:\r\n%s" % sStdErr;
    auProcessIds = [];
    asErrors = [];
    for sLine in sStdOut.split("\n"):
      oProcessIdOrErrorMatch = re.match(r"^(?:%s)\r*$" % "|".join([
        r"\+ (?:.+) process id = (\d+)",
        r"\- (.+)",
      ]), sLine);
      if oProcessIdOrErrorMatch:
        sProcessId, sError = oProcessIdOrErrorMatch.groups();
        if sError:
          asErrors.append(sError);
        else:
          auProcessIds.append(long(sProcessId));
    if len(asErrors) == 0:
      assert len(auProcessIds) > 0, \
          "Could not detect process id's in EdgeDbg output:\r\n%s" % sStdOut;
      return auProcessIds;
    time.sleep(1);
  raise AssertionError("Error starting Edge:\r\n%s" % sStdOut);

from fauStartEdgeAndReturnProcessIds import fauStartEdgeAndReturnProcessIds;
from Kill import fKillProcessesUntilTheyAreDead;

print "* Killing any running instances of Edge...";
fKillProcessesUntilTheyAreDead(asBinaryNames = [
  "ApplicationFrameHost.exe", "browser_broker.exe", "MicrosoftEdge.exe", "MicrosoftEdgeCP.exe", "RuntimeBroker.exe",
]);
print "* Starting Edge and returning process ids...";
auProcessIds = fauStartEdgeAndReturnProcessIds();
print repr(auProcessIds);
print "* Kill all Edge process ids...";
fKillProcessesUntilTheyAreDead(auProcessIds = auProcessIds);
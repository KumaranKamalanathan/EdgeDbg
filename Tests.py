from fauStartEdgeAndReturnProcessIds import fauStartEdgeAndReturnProcessIds;
from Kill import fKillProcessesUntilTheyAreDead;

print "* Starting Edge and returning process ids...";
auProcessIds = fauStartEdgeAndReturnProcessIds();
print repr(auProcessIds);
print "* Kill all Edge process ids...";
fKillProcessesUntilTheyAreDead(auProcessIds = auProcessIds);
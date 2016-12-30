2016-12-30
==========
+ Attempt to reduce chances of hitting a race condition: When Edge is started,
  it can run freely until EdgeDbg suspends it's processes. EdgeDbg attempts to
  reduce this time period to a minimum, as Edge could load a web-page that
  causes an exceptions, which would go undetected by the debugger because it
  has not yet attached. This leads to Edge being killed by Windows and all
  kinds of errors. The new code attempts to suspend all relevant processes as
  soon they are detected, rather than try to detect them in sequence. I am
  hoping this reduces the risk of hitting this race condition, but I have no
  guarantees.

2016-09-26
==========
+ Rename `EdgePageHeap.cmd` to `PageHeap-Edge.cmd` as similar scripts in the
  BugId project follow that naming pattern.

2016-09-12
==========
+ Change BugId application keyword from `@edge` to `edge` and move it after the
  "--pids" argument to comply with newer BugId versions.

2016-09-08
==========
+ Fix bugs in EdgeWinDbg.cmd caused by superfluous "%"-s around %OSISA%.
+ Fix bugs in EdgeBugId.cmd caused by superfluous ":" after %SystemdDrive%.

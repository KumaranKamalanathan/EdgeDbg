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

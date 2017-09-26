2017-09-26
==========
Deprecation
-----------
Starting with Windows 10.0.15063 (Creators Edition), Edge has become a full
Universal Windows Platform (UWP) App. This means that the main Edge process no
longer spawns any of the sandboxed child processes directly, but that all
processes are spawned by the UWP framework. As a result, EdgeDbg can no longer
be used to debug all Edge processes using BugId or WinDbg.
+ If you want to continue to debug Edge using BugId, you can simply use BugId
  directly: recent versions are able to debug USP Apps, and edge is a known,
  pre-configured applications in BugId.
+ If you want to continue to debug Edge using WinDbg, you are encouraged to
  look at the Microsoft website that explains how you can do this. It is
  unfortunately a bit more involved than using EdgeWinDbg, and I may in the
  future revive EdgeDbg in order to automate most of the process of starting
  Edge in WinDbg. For now, I have no immediate need for that myself, so I can
  offer no guarantees when and if it will ever be implemented.
  https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debugging-a-uwp-app-using-windbg

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

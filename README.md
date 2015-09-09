EdgeDbg
==================

A simple command line exe to start and debug the Microsoft Edge browser.

Usage
-----
There are three ways to use the application. First, you can simply start
Microsoft Edge:

    EdgeDbg.exe <url>

Or you can start Microsoft Edge with all threads in all its processes suspended:

    EdgeDbg.exe <url> --suspend

Finally, you can start Microsoft Edge and attach a debugger to the all
associated processes.

    EdgeDbg.exe <url> path\to\debugger.exe [debugger arguments]

Note: You can use `@MicrosoftEdge@`, `@MicrosoftEdgeCP@`, `@browser_broker@`,
and `@RuntimeBroker@` in the debugger arguments; they will be replaced by the
decimal process id of these processes. You can also use `@ProcessIds@`, which
will be replaced by the process ids of all four of these processes, separated
by commas.

EdgeDbg will terminate any running instances of Microsoft Edge prior to starting
the application. This means terminating all accessible processes that have loaded
the `MicrosoftEdge.exe`, `MicrosoftEdgeCP.exe`, `browser_broker.exe`, or
`RuntimeBroker.exe` executables.

Example
-------
Start Microsoft Edge and open "http://example.com":

    H:\dev\C\EdgeDbg>Build\EdgeDbg_x86.exe http://example.com
    * Terminating any running instances of Microsoft Edge...
    * Activating Microsoft Edge and opening http://example.com...
    + MicrosoftEdge.exe process id = 2744
    * Waiting for MicrosoftEdgeCP.exe process to start...
    + MicrosoftEdgeCP.exe process id = 2748
    + RuntimeBroker.exe process id = 2936
    + browser_broker.exe process id = 3088
    
    H:\dev\C\EdgeDbg>

EdgeWinDbg.cmd
--------------
This script demonstrates how to start Microsoft Edge and attach a debugger
(WinDbg) before loading a page. It also cleans up the crash recovery data before
launching Microsoft Edge to prevent it from reloading tabs that were open the
last time you ran it.

    H:\dev\C\EdgeDbg>Set WinDbg=path\to\windbg.exe
    H:\dev\C\EdgeDbg>EdgeWinDbg
    * Terminating any running Edge processes...
    * Deleting crash recovery data...
    * Starting Edge in WinDbg...
    + URL: http://W10-IE11:28876/
    * Terminating any running instances of Microsoft Edge...
    * Activating Microsoft Edge and opening http://W10-IE11:28876/...
    + MicrosoftEdge.exe process id = 5388
    * Waiting for MicrosoftEdgeCP.exe process to start...
    + MicrosoftEdgeCP.exe process id = 3852
    * Suspended process 5388.
    * Suspended process 3852.
    + RuntimeBroker.exe process id = 1476
    * Suspended process 1476.
    + browser_broker.exe process id = 1836
    * Suspended process 1836.
    * Starting "path\to\windbg.exe" -o -p 5388 -c ".attach 0n3852;g;.attach 0n1836;g;.attach 0n1476;g;~*m;.childdbg 1;|0s;~*m;.childdbg 1;|1s;~*m;.childdbg 1;|2s;~*m;.childdbg 1;g"

Parsing output
--------------
If you want to use EdgeDbg with your own debugger, but do not want EdgeDbg to
start it, you can use the --suspend switch and parse the EdgeDbg output to
determine which processes to attach to: lines starting with `-` indicate a
fatal error, lines starting with `*` are progress notifications and lines
starting with `+` contain process ids.

Download
--------
If you do not want to build EdgeDbg from source, you can download pre-build
executables for the latest release:

&nbsp;&nbsp;&nbsp;&nbsp;[EdgeDbg.zip]
    (https://github.com/SkyLined/EdgeDbg/releases/download/1.1b/EdgeDbg.zip)

You can also download the most up-to-date pre-build executables, including
.pdb files for debugging, from the [repository]
(https://github.com/SkyLined/EdgeDbg/tree/master/Build) itself. Please note
that these later binaries may have newer features, but are also more likely
to have bugs in them.
I'd advise you to use the release executables unless they do not work for
you for some reason.

Please note that the 64-bit executable will (obviously) not run on 32-bit
windows builds, and that the 32-bit executable will not work on 64-bit windows
builds.

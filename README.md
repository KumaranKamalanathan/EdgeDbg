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

    EdgeDbg.exe <url> path\to\debugger.exe [debugger options]

Note: You can use `@MicrosoftEdge@`, `@MicrosoftEdgeCP@`, `@browser_broker@`,
and `@RuntimeBroker@` in the debugger options to have the decimal process id of
these processes inserted.

In all three options, EdgeDbg will terminate any running instances of Microsoft
Edge prior to starting the application. This means terminating all processes
that have loaded the `MicrosoftEdge.exe`, `MicrosoftEdgeCP.exe`,
`browser_broker.exe`, or `RuntimeBroker.exe` executables.

Example
-------
Start Microsoft Edge and open "http://example.com":

    H:\dev\C\EdgeDbg>x64\Release\EdgeDbg.exe http://example.com
    * Terminating any running instances of Microsoft Edge...
    * Activating Microsoft Edge application...
    + MicrosoftEdge.exe process id = 904
    * Waiting for MicrosoftEdgeCP.exe process...
    * Waiting for MicrosoftEdgeCP.exe process to start...
    + MicrosoftEdgeCP.exe process id = 6956
    + RuntimeBroker.exe process id = 6316
    + browser_broker.exe process id = 6980
    
    H:\dev\C\EdgeDbg>

EdgeWinDbg.cmd
--------------
This script demonstrates how to start Microsoft Edge and attach a debugger
(WinDbg) before loading a page. It also cleans up the crash recovery data before
launching Microsoft Edge to prevent it from reloading tabs that were open the
last time you ran it.

    H:\dev\C\EdgeDbg>EdgeWinDbg.cmd "path\to\windbg.exe" http://example.com
    * Killing any running instancess of Microsoft Edge...
    * Deleting crash recovery data...
    * Starting Edge in Windbg...
    * Terminating any running instances of Microsoft Edge...
    * Activating Microsoft Edge application...
    + MicrosoftEdge.exe process id = 6604
    * Waiting for MicrosoftEdgeCP.exe process to start...
    + MicrosoftEdgeCP.exe process id = 3372
    + RuntimeBroker.exe process id = 5104
    + browser_broker.exe process id = 4124
    * Starting "path\to\windbg.exe" -o -p 6604 -c ".attach 0n3372;~*m;g;.attach 0n4124;~*m;g;.attach 0n5104;~*m;g;~*m;g"
    
    H:\dev\C\EdgeDbg>

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
executables:

&nbsp;&nbsp;&nbsp;&nbsp;[EdgeDbg.zip]
    (https://github.com/SkyLined/EdgeDbg/releases/download/1.0/EdgeDbg.zip)

Please note that the 64-bit executable will (obviously) not run on 32-bit
windows builds, and that the 32-bit executable will not work on 64-bit windows
builds.

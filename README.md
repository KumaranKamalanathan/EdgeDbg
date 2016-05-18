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
, `@RuntimeBroker@`, and `@ApplicationFrameHost@` in the debugger arguments;
these will be replaced by the decimal process id of these processes. You can
also use `@ProcessIds@`, which will be replaced by the process ids of all four
of these processes, separated by commas.

EdgeDbg will terminate any running instances of Microsoft Edge prior to
starting the application. This means terminating all accessible processes that
have loaded the `MicrosoftEdge.exe`, `MicrosoftEdgeCP.exe`,
`browser_broker.exe`, `RuntimeBroker.exe`, or `ApplicationFrameHost.exe`
executables.

Example
-------
Start Microsoft Edge and open "http://example.com":

    H:\dev\C\EdgeDbg>EdgeDbg_x86.exe http://example.com
    * Terminating any running instances of Microsoft Edge...
    * Activating Microsoft Edge and opening http://example.com...
    + MicrosoftEdge.exe process id = 2744
    * Waiting for MicrosoftEdgeCP.exe process to start...
    + MicrosoftEdgeCP.exe process id = 2748
    + RuntimeBroker.exe process id = 2936
    + browser_broker.exe process id = 3088
    + ApplicationFrameHost.exe process id = 1234
    
    H:\dev\C\EdgeDbg>

EdgeWinDbg.cmd
--------------
This script demonstrates how to start Microsoft Edge and attach a debugger
(WinDbg) before loading a page. It also cleans up the crash recovery data before
launching Microsoft Edge to prevent it from reloading tabs that were open the
last time you ran it.

Syntax:

    EdgeWinDbg.cmd <url> <additional windbg arguments>

This script requires windbg.exe from Microsoft's Debugging Tools for Windows,
and the environment variable `WinDbg` should be set to the path of the
`windbg.exe` binary before running the script. If this environment variable is
not defined, the script attempts to locate windbg.exe automatically.

Example:

    H:\dev\C\EdgeDbg>Set WinDbg=path\to\windbg.exe
    
    H:\dev\C\EdgeDbg>EdgeWinDbg.cmd
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
    + ApplicationFrameHost.exe process id = 1234
    * Suspended process 1234.
    * Starting "path\to\windbg.exe" -o -p 5388 -c ".attach 0n3852;g;.attach 0n1836;g;.attach 0n1476;g;.attach 0n1234;g;~*m;.childdbg 1;|0s;~*m;.childdbg 1;|1s;~*m;.childdbg 1;|2s;~*m;.childdbg 1;|3s;~*m;.childdbg 1;g"

EdgePageHeap.cmd
----------------
You may want to enable page heap for Microsoft Edge to make detection and
analysis of bugs more reliable and detailed. This can be done using the
gflags.exe application that is distributed with Microsoft's Debugging Tools for
Windows. The preferred flags to use are +02109870, see the page heap
documentation for an explanation of the switches in use. The command-line
utility EdgePageHeap.cmd is provided to facilitate enabling/disabling page heap
for all processes associated with Microsoft Edge. It can be used in the
following way:

    EdgePageHeap.cmd [ON|OFF]

This will enable (ON, default) or disable (OFF) page heap. Note that enabling
page heap is highly recommended when using BugId to detect crashes (more on
that below).


EdgeBugId.cmd
-------------
This script facilitates use of [BugId](https://github.com/SkyLined/BugId) with
Microsoft Edge. BugId is a set of python scripts that allow automatic bug
detection, analysis and identification. For more information on BugId see the
project's [README](https://github.com/SkyLined/BugId/blob/master/README.md)
file. The script also clean up the crash recovery data before launching
Microsoft Edge to prevent it from reloading tabs that were open the last time
you ran it.

Syntax:

    EdgeBugId.cmd <url> <additional BugId arguments>

This script requires BugId, and the environment variable `BugId` should be
set to the path of the `BugId.py` script before running the script.
BugId requires cdb.exe from Microsoft's Debugging Tools for Windows, and the
environment variable `cdb` should be set to the path of the `cdb.exe` binary
before running the script.
If either environment variable is not defined, the script attempts to locate
the required component automatically.

Example:

    X:\path\to\EdgeDbg>Set cdb=\path\to\cdb.exe
    
    X:\path\to\EdgeDbg>Set BugId=\path\to\BugId.py
    
    X:\path\to\EdgeDbg>EdgeBugId.cmd
    * Terminating any running instances of Microsoft Edge...
    * Deleting crash recovery data...
    * Starting Edge in BugId...
    + URL: http://PC:28876/
    * Terminating any running instances of Microsoft Edge...
    * Activating Microsoft Edge and opening http://PC:28876/...
    + MicrosoftEdge.exe process id = 4792
    * Waiting for MicrosoftEdgeCP.exe process to start...
    + MicrosoftEdgeCP.exe process id = 3928
    * Suspended process 4792.
    * Suspended process 3928.
    + RuntimeBroker.exe process id = 2104
    * Suspended process 2104.
    + browser_broker.exe process id = 2096
    * Suspended process 2096.
    + ApplicationFrameHost.exe process id = 1234
    * Suspended process 1234.
    * Starting \path\to\python.exe \path\to\BugId.py --pids=2104,2096,4792,3928,1234
    * The debugger is attaching to the application...
    * Attached to process 2104.
    * The application was resumed successfully and is running...
    * Attached to process 2096.
    * Attached to process 4792.
    * Attached to process 3928.
    * Attached to process 1234.
    * Exception code 0xC0000005 (Access violation) was detected and is being analyzed...
    * A bug was detected in the application.
    
    Id:               E39B AVR:NULL microsoftedgecp.exe!EDGEHTML.dll!CDocument::getElementsByTagNameNSInternal
    Description:      Access violation while reading memory at 0x0 using a NULL ptr
    Process binary:   microsoftedgecp.exe
    Location:         EDGEHTML.dll!CDocument::getElementsByTagNameNSInternal + 0x36
    Security impact:  None
    Error report:     E39B AVR.NULL microsoftedgecp.exe!EDGEHTML.dll!CDocument..getElementsByTagNameNSInternal.html (66859 bytes)
    
    X:\path\to\EdgeDbg>

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

License
-------
This project was originally forked from [Microsoft Edge Launcher]
(https://github.com/MicrosoftEdge/MicrosoftEdgeLauncher) but the program has
been completely rewritten and expanded. It is therefore no longer a (modified)
copy of that code and the MIT licence that covers Microsoft Edge Launcher no
longer applies to this code.

This work is licensed under a [Creative Commons Attribution-NonCommercial 4.0
International License](http://creativecommons.org/licenses/by-nc/4.0/).

![Creative Commons License](https://i.creativecommons.org/l/by-nc/4.0/88x31.png
"Creative Commons License")
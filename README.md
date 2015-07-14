Edge Dbg
==================

A simple command line exe to start and debug the Microsoft Edge browser.

Usage
-----
There are three ways to use the application. First, you can simply start
Microsoft Edge:

    EdgeDbg.exe <url>

Or you can start Microsoft Edge with all threads in all its processes suspended:

    EdgeDbg.exe <url> --suspend

Finally, you can start Microsoft Edge, terminate all `MicrosoftEdgeCP.exe`
processes and attach a debugger to the `MicrosoftEdge.exe` and `browser_broker.exe`
processes.

    EdgeDbg.exe <url> path\to\debugger.exe [debugger options]

Note: You can use `@main_pid@` and `@broker_pid@` in the debugger options
to have the decimal process id of these two processes inserted.

In all three options, this application will terminate any running instances of
Microsoft Edge prior to starting the application. This means terminating all
processes running the `MicrosoftEdge.exe`, `MicrosoftEdgeCP.exe` or `browser_broker.exe`
executables.

Download
--------
If you do not want to build Edge Dbg from source, you can download a pre-build
executable:
    [EdgeDbg.zip]
    (https://github.com/SkyLined/EdgeDbg/releases/download/0.6/EdgeDbg.zip)

Examples
--------
Start project spartan:

    H:\dev\C\EdgeDbg>Release\EdgeDbg.exe http://example.com
    // Terminated MicrosoftEdge.exe process 444
    MicrosoftEdge.exe process id = 4712
    browser_broker.exe process id = 4492
    H:\dev\C\EdgeDbg>
The output shows Edge Dbg closed a running instance of Microsoft Edge by
terminating the main process and then started Microsoft Edge, which
resulted in two new processes.

Start project spartan suspended:

    H:\dev\C\EdgeDbg>Release\EdgeDbg.exe http://example.com --suspend
    // Terminated MicrosoftEdge.exe process 4944
    // Terminated browser_broker.exe process 4544
    MicrosoftEdge.exe process id = 4184
    // Suspended thread 5760.
    // Suspended thread 3444.
    // Suspended thread 4700.
    // Suspended thread 672.
    // Suspended thread 5796.
    // Suspended thread 5852.
    // Suspended thread 2612.
    // Suspended thread 4448.
    // Suspended thread 4276.
    // Suspended thread 3004.
    // Suspended thread 5564.
    // Suspended thread 5828.
    // Suspended thread 5188.
    // Suspended thread 4916.
    // Suspended thread 6056.
    // Suspended thread 4884.
    // Suspended thread 3360.
    // Suspended thread 5484.
    // Suspended thread 5288.
    // Suspended thread 2024.
    // Suspended thread 180.
    // Suspended thread 548.
    // Suspended thread 4060.
    // Suspended thread 728.
    // Suspended thread 5308.
    // Suspended thread 2752.
    // Suspended thread 4132.
    // Suspended thread 4512.
    // Suspended thread 1612.
    // Suspended thread 4848.
    browser_broker.exe process id = 4436
    // Suspended thread 696.
    // Suspended thread 4352.
    // Suspended thread 4744.
    // Suspended thread 5788.
    // Suspended thread 4364.
    // Suspended thread 5680.
    // Suspended thread 2988.
    // Suspended thread 5576.
    // Suspended thread 3064.
    // Suspended thread 5772.
    H:\dev\C\EdgeDbg>
The output is the same, but this time `example.com` is not loaded as all threads
in the processes that were started have been suspended. You will need to attach
a debugger to these (two) processes and resume all threads in them for
Microsoft Edge to continue and open the page.

Start Microsoft Edge in cdb.exe:

    H:\dev\C\EdgeDbg>Release\EdgeDbg.exe http://example.com path\to\cdb.exe -o
        -p @main_pid@ -c ".attach 0n@broker_pid@;~*m;g;~*m;g"
    MicrosoftEdge.exe process id = 3856
    <<<snip>>>
    browser_broker.exe process id = 2576
    <<<snip>>>
    Starting path\to\cdb.exe -o -p 3856 -c ".attach 0n2576;~*m;g;~*m;g"
    Microsoft (R) Windows Debugger Version 6.3.9600.16384 X86
    Copyright (c) Microsoft Corporation. All rights reserved.
    <<<snip>>>

At this point Microsoft Edge is running and has opened the requested URL. As you
can see, the `@main_pid@` and `@broker_pid@` tokens provided in the command
line have been replaced with the process id of the `MicrosoftEdge.exe` and
`browser_broker.exe` processes respectively before executing cdb.
Also, all `MicrosoftEdgeCP.exe` processes started by Microsoft Edge prior to
attaching the debugger will have been terminated. This is done because there may
be any number of these, making attaching the debugger to them through the command
line not practical. Microsoft Edge will simply recreate these processes as
needed, so this should not affect the result.

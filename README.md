Edge Dbg
==================

A simple command line exe to start and debug the Microsoft Edge browser.

Usage
-----
There are three ways to use the application. First, you can simply start
Microsoft Edge:

    `EdgeDbg.exe <url>`

Or you can start Microsoft Edge with all threads in all its processes suspended:

    `EdgeDbg.exe <url> --suspend`

Finally, you can start Microsoft Edge, terminate all `spartan_edge.exe`
processes and attach a debugger to the `spartan.exe` and `browser_broker.exe`
processes.

    `EdgeDbg.exe <url> path\to\debugger.exe [debugger options]`

Note: You can use `@spartan_pid@` and `@broker_pid@` in the debugger options
to have the decimal process id of these two processes inserted.

In all three options, this application will terminate any running instances of
Microsoft Edge prior to starting the application. This means terminating all
processes running the `spartan.exe`, `spartan_edge.exe` or `browser_broker.exe`
executables.

Download
--------
If you do not want to build Edge Dbg from source, you can download a pre-build
executable for version v0.5 here:
    [EdgeDbg.0.5.zip]
    (https://github.com/SkyLined/EdgeDbg/releases/download/0.5/EdgeDbg.0.5.zip)

Examples
--------
Start project spartan:

  ```
  H:\dev\C\EdgeDbg>Release\EdgeDbg.exe http://example.com
  Terminated spartan.exe process 4668
  Terminated browser_broker.exe process 3148
  Terminated spartan_edge.exe process 3576
  spartan.exe process id = 4148
  browser_broker.exe process id = 4492
  spartan_edge.exe process id = 2004
  H:\dev\C\EdgeDbg>
  ```
The output shows Edge Dbg closed a running instance of Microsoft Edge by
terminating a number of processes and then started Microsoft Edge, which
resulted in three new processes.

Start project spartan suspended:

  ```
  H:\dev\C\EdgeDbg>Release\EdgeDbg.exe http://example.com --suspend
  Terminated spartan.exe process 4148
  spartan.exe process id = 5780
  browser_broker.exe process id = 5404
  spartan_edge.exe process id = 2340
  H:\dev\C\EdgeDbg>
  ```
The output is the same, but this time `example.com` is not loaded as all threads
in the processes that were started have been suspended. You will need to attach
a debugger to these (three) processes and resume all threads in them for
Microsoft Edge to continue and open the page.

Start project spartan in cdb.exe:

  ```
  H:\dev\C\EdgeDbg>Release\EdgeDbg.exe http://example.com path\to\cdb.exe -o -p @spartan_pid@ -c ".attach 0n@broker_pid@;~*m;g;~*m;g"
  Terminated spartan.exe process 5780
  Terminated browser_broker.exe process 5404
  Terminated spartan_edge.exe process 2340
  spartan.exe process id = 3856
  browser_broker.exe process id = 2576
  Terminated spartan_edge.exe process 3952
  Starting path\to\cdb.exe -o -p 3856 -c ".attach 0n2576;~*m;g;~*m;g"
  Microsoft (R) Windows Debugger Version 6.3.9600.16384 X86
  Copyright (c) Microsoft Corporation. All rights reserved.
  
  *** wait with pending attach
  <<<snip>>>
  ModLoad: 00ba0000 00e97000   C:\Windows\SystemApps\Spartan_cw5n1h2txyewy\spartan.exe
  <<<snip>>>
  (f10.16dc): Break instruction exception - code 80000003 (first chance)
  <<<snip>>>
  ntdll!DbgBreakPoint:
  77cd3060 cc              int     3
  0:028> cdb: Reading initial command '.attach 0n2576;~*m;g;~*m;g'
  Attach will occur on next execution
  *** wait with pending attach
  <<<snip>>>
  ModLoad: 01200000 0120d000   C:\WINDOWS\system32\browser_broker.exe
  <<<snip>>>
  ```
At this point Microsoft Edge is running and has opened the requested URL. As you
can see, the `@spartan_pid@` and `@broker_pid@` tokens provided in the command
line have been replaced with the process id of the `spartan.exe` and
`browser_broker.exe` processes respectively before executing cdb.
Also, all `spartan_edge.exe` processes started by Microsoft Edge prior to
attaching the debugger have been terminated. This is done because there may be
any number of these, making attaching the debugger to them through the command
line not practical. Microsoft Edge will simply recreate these processes as
needed, so this should not affect the result.

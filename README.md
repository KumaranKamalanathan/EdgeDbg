# Project Spartan Dbg
A simple command line exe to launch Project Spartan and immediately suspend all
threads in the `spartan.exe` and `browser_broker.exe` processes. Then kill any
existing `spartan_edge.exe` process.
After all this it can optionally start an application such as a debugger and
provide the process id for the `spartan.exe` and `browser_broker.exe` processes
through the command-line using `@spartan_pid@` and `@broker_pid@`.
Finally, a url can be provided that Project Spartan will open as soon as all
threads in the `spartan.exe` and `browser_broker.exe` processes are resumed.

#Usage
1. Build the solution or [download v0.4]
  (https://github.com/SkyLined/ProjectSpartanDbg/releases/download/0.4/ProjectSpartanDbg.0.4.zip)
2. Start Project Spartan and attach a debugger, e.g.
  
  ```
  C:\path> ProjectSpartanDbg.exe <url> cdb.exe -o -p @spartan_pid@ -c ".attach 0n@broker_pid@;~*m;g;~*m;g"
  Project Spartan process id = 2372
  Browser broker process id = 1304
  Starting cdb.exe -o -p 2372 -c ".attach 0n1304;~*m;g;~*m;g"
  Microsoft (R) Windows Debugger Version 6.3.9600.16384 X86
  Copyright (c) Microsoft Corporation. All rights reserved.
  
  <<<snip>>>
  ModLoad: 008c0000 00bb7000   C:\Windows\SystemApps\Spartan_cw5n1h2txyewy\spartan.exe
  <<<snip>>>
  (944.cec): Break instruction exception - code 80000003 (first chance)
  <<<snip>>>
  ntdll!DbgBreakPoint:
  77cd3060 cc              int     3
  0:021> cdb: Reading initial command '.attach 0n1304;~*m;g;~*m;g'
  Attach will occur on next execution
  <<<snip>>>
  ModLoad: 00ea0000 00ead000   C:\WINDOWS\system32\browser_broker.exe
  <<<snip>>>
  ```
Notes:
  * `-p @spartan_pid@` tells cdb to attach to the `spartan.exe` process.
  * `attach 0n@broker_pid@;` tells cdb to attach to the `browser_broker.exe`
    process after it has attached to the `spartan.exe` process.
  * the first `~*m;g;` tells cdb to resume all threads in the current process
    (`spartan.exe`) and run it. The debugger will then attach to the
    `browser_broker.exe` process and break again.
  * the second `~*m;g` tells cdb to resume all threads in the current process
    (now `browser_broker.exe`) and run it as well. Project Spartan is now
    running in your debugger.

    

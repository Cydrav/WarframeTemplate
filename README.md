# WarframeTemplate
### KEEP IN MIND THIS DOES NOT BYPASS WARFRAME'S OTHER ANTICHEAT CHECKS
This is a template people can look at for my Warframe hack which is a kernel driver that communicates with a usermode application.
The purpose of this is so that the hack opens absolutely NO handles to Warframe. Warframe has an invasive usermode anti-cheat.

### Stated in DE's EULA
> As stated in Digital Extreme's End User License Agreement: 
> the software may monitor your computer’s Random Access Memory (RAM) for unauthorized third-party programs running concurrently 
> with the Software. An “Unauthorized Third-Party Program” as used herein shall be defined as any third-party software prohibited 
> by Section 2. In the event that the Software detects an Unauthorized Third-Party Program, the Software may communicate 
> information back to Digital Extremes

### Important

Warframe monitors your computer for a variety of third party software, checking if they're opening Warframe's process handle, etc... Some of these functions include, but are not limited to:

```
KERNELBASE.GetSystemInfo

KERNELBASE.K32GetPerformanceInfo2

KERNELBASE.IsDebuggerPresent

KERNEL32.VirtualQueryEx

KERNEL32.OpenProcess

KERNEL32.GetProcessHandleCount

KERNEL32.GetProcessImageFileName

KERNEL32.Process32NextW

KERNEL32.Process32FirstW

PSAPI.EnumProcesses

PSAPI.EnumProcessModules

USER32.EnumWindows
```

Regardless of the EnumProcesses function being present, Process32Next in conjunction with Process32First is being used to enumerate through the system's usermode processes as well. You might be asking why the anticheat's privileges can't be exploited like one of the bypasses for XIGNCODE which forced the process to run without admin privileges using: 

``` set __COMPAT_LAYER=RUNASINVOKER && start ```

Well it's simple, Warframe crashes without administrative privileges. However, the system process handle(s) cannot be opened from usermode. The kernel driver also isn't opening a handle to the process, it's attaching a thread to Warframe's address space to inject code.

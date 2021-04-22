# AuroraCrashPatcher
Quick and dirty patch to prevent fatal crashing when downloading title assets (boxart, etc) through FSD or Aurora. As of v0.2-beta, this patch should work for everyone, regardless of geographic location (both in and outside of the US).

If you are still crashing while attempting to update your title assets in Aurora/FSD with this patch loaded, switch to the `AuroraCrashPatcher-Debug.xex`, and have XboxWatson or some attached debugger running on your PC to display the debug prints. Open an issue for me here, and include the console output.

## Install/Usage
- Download the latest compiled xex from the `Releases` page (direct download: https://github.com/StelioKontosXBL/AuroraCrashPatcher/releases/download/v1.0/AuroraCrashPatcher.xex)
- Copy the xex to your RGH's Hdd, as you would any menu or plugin xex
- Set as a launch plugin (use Dashlaunch, or edit your launch.ini/JRPC.ini manually)
- Reboot, and you're done

## Whuddaydo-wid-dis...
Note that **this is not an Aurora plugin** - meaning you use it by adding it to your plugins through DashLaunch (or editing `launch.ini` directly) - not by attempting to run it like an Aurora script/plugin/whatever.

If you don't use Aurora/FSD, don't bother. If you do, download the compiled xex and add to your plugins to prevent the crashing. If you do use this crap, be sure to remove from plugins once the devs release an official patch.

## WTH?
Ye, I know. Hacky af, but it works. It's a quick fix, nothing more, nothing less. Not to be used as a guide to best coding standards; threw it together in a couple hours, and hopefully the Aurora/FSD devs will be able to release an official patch soon, deprecating this. Use at your own risk.

## Note to Aurora/FSD devs
If you want to automatically disable this patch on users' consoles with your next update, simply add a call to XAM ordinal 0x43 with "aurora.crash.patched" as the first parameter, ignoring the result (see DllMain.cpp:31). If the patch is running on the user's console, that will trigger a self-unload and wipe this patch from the user's console. As of v0.3-beta, AuroraCrashPatcher will automatically disable itself for the duration of that session until the next title reboot if an updated version of Aurora or FSD is detected (see DllMain.cpp:46)

## Disclaimer
Neither this repo, nor myself, are directly affiliated with Aurora or FSD, or their development teams.

# AuroraCrashPatcher
Quick patch to prevent fatal crashing when downloading title assets (boxart, etc) through FSD or Aurora. As of v0.2-beta, this patch should work for everyone, regardless of geographic location (both in and outside of the US).

## Install/Usage
- Download the [latest release](https://github.com/StelioKontosXBL/AuroraCrashPatcher/releases/latest/). Direct download link for the compiled xex (`AuroraCrashPatcher.xex`) will always be:
https://github.com/StelioKontosXBL/AuroraCrashPatcher/releases/latest/download/AuroraCrashPatcher.xex
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
Neither this repo, ~~nor myself~~, are directly affiliated with Aurora or FSD, or their development teams.

## NOTE:
This patch can still be used for FSD. Following the release of this patch, I was brought on to the Aurora team (Phoenix), and consequently the latest release of Aurora has been updated to fix this bug, making this patch no longer necessary.

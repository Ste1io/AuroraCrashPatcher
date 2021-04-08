# AuroraCrashPatcher
Quick and dirty patch to prevent fatal crashing when downloading title assets (boxart, etc) through FSD or Aurora. Currently this patch only works for users inside the US.

## Install/Usage
- Download the compiled xex (direct link: https://github.com/StelioKontosXBL/AuroraCrashPatcher/blob/c32cd3e0c9e02dc59fc31de2f293994d788fed01/AuroraCrashPatcher.xex)
- Copy the xex to your RGH's Hdd, as you would any menu or plugin xex
- Set as a plugin (either in your launch.ini, or in JRPC.ini if you use XDRPC)
- Reboot, and you're done

## Whuddaydo-wid-dis...
If you don't use Aurora/FSD, don't bother. If you do, download the compiled xex and add to your plugins to prevent the crashing. If you do use this crap, be sure to remove from plugins once the devs release an official patch.

## WTH?
Ye, I know. Hacky af, but it works. It's a quick fix, nothing more, nothing less. Not to be used as a guide to best coding standards; threw it together in a couple hours, and hopefully the Aurora/FSD devs will be able to release an official patch soon, deprecating this. Use at your own risk.

## Note to Aurora/FSD devs
When an official patch is released, launch Aurora with this module loaded, and check DbgOut for the killswitch address in mem. Writing the value `0xDEADC0DE` at this address will unload and wipe this patch from the user's console.

## Disclaimer
Neither this repo, nor myself, are directly affiliated with Aurora or FSD, or their development teams.

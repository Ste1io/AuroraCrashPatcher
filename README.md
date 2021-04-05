# AuroraCrashPatcher
Quick and dirty patch to prevent fatal crashing when downloading title assets (boxart, etc) through FSD or Aurora.

Download the compiled xex and add to your plugins to prevent the crashing.

# WTH?
Ye, I know. Hacky af, but it works. It's a quick fix, nothing more, nothing less. Not to be used as a guide to best coding standards; threw it together in a couple hours, and hopefully the Aurora/FSD devs will be able to release an official patch soon, deprecating this. Use at your own risk.

# Note to Aurora/FSD devs
When an official patch is released, launch Aurora with this module loaded, and check DbgOut for the killswitch address in mem. Writing the value `0xDEADC0DE` at this address will unload and wipe this patch from the user's console.

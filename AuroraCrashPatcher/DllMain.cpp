/*
 * Name:	AuroraCrashPatcher
 * Author:	Stelio Kontos
 * Desc:	Quick and dirty patch to prevent fatal crashing when
 *			downloading title assets (boxart, etc) through FSD and Aurora.
 *
 * Changelog:
 * v0.1-beta: 04/02/2021
 *		-	Initial commit.
 * v0.2-beta: 04/20/2021
 *		-	Changed base address to avoid conflict with nomni.
 *		-	Replaced hook method to more accurately identify and intercept the
 *			api calls causing the crash for everyone regardless of geographic locale.
 * v0.3-beta: 04/21/2021
 *		-	Added check to disable patching if Aurora/FSD update is detected.
 * v0.4-beta: 04/21/2021
 *		-	Changed killswitch method for Aurora devs to use once official patch is released.
 * v1.0: 04/22/2021
 * v1.1: 04/25/2021
 *		-	Fixed bug causing patch to not load for users not using a stealth server
 *			(props to rubensyama for helping track this down).
 * v1.2: 04/28/2021
 *		-	Add tray open check for when playing OG games.
 *		-	Add file logs
 *		-	Add compatibility for old kernel builds < 17559
 */

#include "stdafx.h"
#include "Detour.h"

uint32_t g_flag = 0;
HANDLE g_hModule, g_hThread = 0;
Detour<INT> origHook;

INT HookProc(INT x, PCHAR h, HANDLE e, XNDNS **s) {
	if (!strcmp(h, "download.xbox.com")) {
		skDbgPrint("[sk] Blocked DNS Lookup for \"%s\"\n", h);
		strncpy(h, "stelio.kontos.nop", strlen(h));
	} else if (!strcmp(h, "aurora.crash.patched")) {
		g_flag = 0xDEADC0DE; //killswitch
	} else {
		skDbgPrint("[sk] Detected DNS Lookup for \"%s\"\n", h);
	}
	return ((INT(*)(INT, PCHAR, HANDLE, XNDNS **))origHook.SaveStub)(x, h, e, s);
}

DWORD WINAPI MainThread(LPVOID lpParameter) {
	auto Run = [] (uint32_t t) -> bool
	{
		static uint32_t p = 0;
		if (g_flag < 2) {
			if (t != p) {
				if ((!t || t == 0xFFFE07D1 || t == 0xF5D20000) && MmIsAddressValid((PVOID)0x82000000)) {
					if (*(uint16_t*)0x82000000 != 0x4D5A)
						return true;
					g_flag = ByteSwap(*(uint32_t*)(0x82000008 + ByteSwap(*(uint32_t*)0x8200003C))) > 0x607F951E;
					if (!g_flag && !origHook.Addr) {
						if (origHook.SetupDetour(ResolveFunction("xam.xex", 0x43), HookProc)) { //0x81741150
							skDbgLog(TRUE, "AuroraCrashPatcher v" SK_VERSION ": ENABLED [flag: 0x%X]", &g_flag);
						}
					}
				} else if (!p || p == 0xFFFE07D1 || p == 0xF5D20000) {
					if (origHook.Addr) {
						origHook.TakeDownDetour();
						skDbgLog(TRUE, "AuroraCrashPatcher v" SK_VERSION ": DISABLED");
						g_flag = 0;
					}
				}
				if (!t && !MmIsAddressValid((PVOID)0x82000000))
					return true;
				p = t;
			}
			return true;
		} else {
			origHook.TakeDownDetour();
			if (g_flag == 0xDEADC0DE)
				SelfDestruct(g_hModule);
			return false;
		}
	};

	while (Run(((uint32_t(*)())0x816E03B8)())) {
		Sleep(100);
	}

	*(uint16_t *)((uint8_t *)g_hModule + 0x40) = 1;
	((void(*)(HANDLE, HANDLE))ResolveFunction("xboxkrnl.exe", 0x1A2))(g_hModule, g_hThread);

	return 0;
}

BOOL Init() {
	skDbgLog(TRUE, "AuroraCrashPatcher initializing");

	if (!MountSysDrives()) {
		DbgPrint("[sk] Failed to mount system drives\n");
	} else { skDbgLog(TRUE, "System drives mounted"); }

	if (XboxKrnlVersion->Build < 0x4497) {
		DbgLog(TRUE, "Kernel build %i detected...you really should update", XboxKrnlVersion->Build);
	} else { skDbgLog(TRUE, "Kernel build %i detected", XboxKrnlVersion->Build); }

	if (TrayOpen()) {
		DbgLog(TRUE, "Tray open...AuroraCrashPatcher aborting");
		return FALSE;
	} else { skDbgLog(TRUE, "Tray closed"); }

	skDbgLog(TRUE, "AuroraCrashPatcher init success");
	return TRUE;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		skDbgLog(TRUE, "AuroraCrashPatcher v" SK_VERSION ": LOADING");
		skDbgLog(TRUE, "- Author: Stelio Kontos");
		skDbgLog(TRUE, "- Github: https://github.com/StelioKontosXBL/AuroraCrashPatcher");
		if (Init()) {
			g_hModule = hModule;
			ExCreateThread(&g_hThread, 0, 0, (PVOID)XapiThreadStartup, (LPTHREAD_START_ROUTINE)MainThread, 0, 0x2 | 0x1);
			XSetThreadProcessor(g_hThread, 0x4);
			ResumeThread(g_hThread);
		}
	} else if (dwReason == DLL_PROCESS_DETACH) {
		skDbgLog(TRUE, "AuroraCrashPatcher v" SK_VERSION ": UNLOADING");
	}

	return TRUE;
}

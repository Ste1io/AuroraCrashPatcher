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
 */

#include "stdafx.h"
#include "Detour.h"

uint32_t g_flag = 0;
HANDLE g_hModule, g_hThread = 0;
Detour<INT> origHook;

INT HookProc(INT x, PCHAR h, HANDLE e, XNDNS **s) {
	if (!strcmp(h, "download.xbox.com")) {
		skDbgPrint("[sk] Intercepted DNS Lookup for \"%s\"" skNewLn, h);
		strncpy(h, "stelio.kontos.nop", strlen(h));
	}
	return ((INT(*)(INT, PCHAR, HANDLE, XNDNS **))origHook.SaveStub)(x, h, e, s);
}

DWORD WINAPI MainThread(LPVOID lpParameter) {
	auto Run = [] (uint32_t title) -> bool
	{
		static uint32_t last = 0;
		if (!g_flag) {
			if (title != last) {
				if (title == 0xFFFE07D1 && ((uint32_t(*)(PVOID))0x800819D0)((PVOID)0x82000000)) {
					DbgPrint("[sk] AuroraCrashPatcher by Stelio Kontos: %s. Control flag @ 0x%X" skNewLn, !g_flag ? "ENABLED" : "DISABLED", &g_flag);
					DbgPrint("[sk] Flag options: default=0x0, unload=0x1, destroy=0xDEADC0DE" skNewLn);
					origHook.SetupDetour(0x81741150, HookProc);
				} else if (last == 0xFFFE07D1) {
					origHook.TakeDownDetour();
				}
				last = title;
			}
			return true;
		} else {
			origHook.TakeDownDetour();
			if (g_flag == 0xDEADC0DE)
				SelfDestruct(g_hModule);
			return false;
		}
	};

	while (Run(((uint32_t(*)())0x816E03B8)()))
		Sleep(100);

	*(uint16_t *)((uint8_t *)g_hModule + 0x40) = 1;
	((void(*)(HANDLE, HANDLE))0x8007D190)(g_hModule, g_hThread);

	return 0;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		skDbgPrint("++++++++++ sk ~ DLL_PROCESS_ATTACH ++++++++++" skNewLn);
		g_hModule = hModule;
		ExCreateThread(&g_hThread, 0, 0, (PVOID)XapiThreadStartup, (LPTHREAD_START_ROUTINE)MainThread, 0, 0x2 | 0x1);
		XSetThreadProcessor(g_hThread, 0x4);
		ResumeThread(g_hThread);
	} else if (dwReason == DLL_PROCESS_DETACH) {
		skDbgPrint("========== sk ~ DLL_PROCESS_DETACH ==========" skNewLn);
	}

	return TRUE;
}

//
// By Stelio Kontos
// April 2, 2021
//
// Quick and dirty patch to prevent fatal crashing when 
// downloading title assets (boxart, etc) through FSD and Aurora.
// 

#include "stdafx.h"
#include "Detour.h"

uint32_t g_flag = 0;
HANDLE g_hModule, g_hThread = 0;
Detour<int32_t> origHook;

int32_t HookProc(uint32_t xnc, SOCKET sock, const sockaddr *name, int namelen) {
	int32_t result = ((int32_t(*)(uint32_t, SOCKET, const sockaddr *, int))origHook.SaveStub)(xnc, sock, name, namelen);

	if (*(uint16_t *)name->sa_data != 0x50)
		return result;

	#if VERBOSE_DBG_PRINT
	sockaddr_in *addr = (sockaddr_in *)name;
	skDbgPrint("[sk] - connect(xnc=%i, s=0x%X, f=%i, p=%i, a=0x%08X) ==  %i; //%i.%i.%i.%i:%i" skNewLn,
		xnc,
		sock,
		addr->sin_family,
		addr->sin_port,
		addr->sin_addr.S_un.S_addr,
		result,
		addr->sin_addr.S_un.S_un_b.s_b1,
		addr->sin_addr.S_un.S_un_b.s_b2,
		addr->sin_addr.S_un.S_un_b.s_b3,
		addr->sin_addr.S_un.S_un_b.s_b4,
		addr->sin_port
	);
	#endif

	switch ((*((uint32_t *)name + 1) >> 0x18) & 0xFF) {
		case 0x17: //akamai
		case 0x08: //level3
		case 0x43:
			result = SOCKET_ERROR;
			WSASetLastError(WSAEADDRNOTAVAIL);
			skDbgPrint("[sk] Blocked connection attempt to (IP=0x%08X):" skNewLn, *((uint32_t *)name + 1));
			break;
		default:
			break;
	}

	return result;
}

DWORD WINAPI MainThread(LPVOID lpParameter) {
	auto Run = [](uint32_t title) -> bool {
		static uint32_t last = 0;
		if (!g_flag) {
			if (title != last) {
				if (title == 0xFFFE07D1 && ((uint32_t(*)(PVOID))0x800819D0)((PVOID)0x82000000)) {
					DbgPrint("[sk] AuroraCrashPatcher %s. Control flag @ 0x%X" skNewLn, !g_flag ? "ENABLED" : "DISABLED", &g_flag);
					DbgPrint("[sk] Flag options: default=0x0, unload=0x1, destroy=0xDEADC0DE" skNewLn);
					origHook.SetupDetour(0x81741BF8, HookProc);
				} else if (last == 0xFFFE07D1) {
					origHook.TakeDownDetour();
				}
				last = title;
			}
			return true;
		} else {
			origHook.TakeDownDetour();
			if (g_flag == 0xDEADC0DE) {
				SelfDestruct(g_hModule);
			}
			return false;
		}
	};

	while (Run(((uint32_t(*)())0x816E03B8)()))
		Sleep(16);

	*(uint16_t *)((uint8_t *)g_hModule + 0x40) = 1;
	((void(*)(...))0x8007D190)(g_hModule, g_hThread);

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

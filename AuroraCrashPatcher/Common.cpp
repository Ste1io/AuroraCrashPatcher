#include "stdafx.h"
#include "Common.h"

namespace {
	BOOL CreateSymbolicLink(PCHAR szDrive, PCHAR szDeviceName, BOOL System) {
		CHAR szDestinationDrive[MAX_PATH];
		sprintf_s(szDestinationDrive, MAX_PATH, System ? "\\System??\\%s" : "\\??\\%s", szDrive);
		STRING linkname, devicename;
		RtlInitAnsiString(&linkname, szDestinationDrive);
		RtlInitAnsiString(&devicename, szDeviceName);
		if (FileExists(szDrive)) return TRUE;
		LONG status = ObCreateSymbolicLink(&linkname, &devicename);
		return (status >= 0) ? TRUE : FALSE;
	}

	BOOL DeleteSymbolicLink(PCHAR szDrive, BOOL System) {
		CHAR szDestinationDrive[MAX_PATH];
		sprintf_s(szDestinationDrive, MAX_PATH, System ? "\\System??\\%s" : "\\??\\%s", szDrive);
		STRING linkname;
		RtlInitAnsiString(&linkname, szDestinationDrive);
		LONG status = ObDeleteSymbolicLink(&linkname);
		return (status >= 0) ? TRUE : FALSE;
	}
} //namespace

BOOL MountSysDrives() {
	if ((XboxHardwareInfo->Flags & 0x20) == 0x20)
		return CreateSymbolicLink(SKMOUNT, HDD, TRUE);
	return CreateSymbolicLink(SKMOUNT, USB, TRUE);
}

BOOL FileExists(const char * filename) {
	if (GetFileAttributesA(filename) == -1) {
		CONST DWORD lastError = GetLastError();
		if (lastError == 2L || lastError == 3L)
			return FALSE;
	}
	return TRUE;
}

BOOL TrayOpen() {
	uint8_t msg[0x10], resp[0x10];
	memset(msg, 0x0, 0x10);
	memcpy(resp, msg, 0x10);
	msg[0] = 0xA;
	///((void(*)(LPVOID, LPVOID))ResolveFunction("xboxkrnl.xex", 0x29))(msg, resp); //0x80067F48
	HalSendSMCMessage(msg, resp);
	if (resp[1] == 0x60)
		return TRUE;
	return FALSE;
}

BOOL critSecInit = FALSE;
CRITICAL_SECTION writeLock;

void DbgLog(BOOL printToConsole, const char * fmt, ...) {
	CHAR buf[0x512];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, 0x512, fmt, va);
	va_end(va);

	if (printToConsole) {
		DbgPrint("[sk] %i: %s\n", GetTickCount(), buf);
	}

	#ifdef SK_LOGPATH

	if (!critSecInit) {
		InitializeCriticalSection(&writeLock);
		critSecInit = TRUE;
	}

	EnterCriticalSection(&writeLock);

	FILE *f = fopen(SK_LOGPATH, "a");

	if (f) {
		fprintf(f, "%10i: %s\n", GetTickCount(), buf);
		fclose(f);
	}

	LeaveCriticalSection(&writeLock);

	#endif
}

BOOL SelfDestruct(HANDLE hModule) {
	char chName[260];
	wchar_t *wchName = (*(UNICODE_STRING*)((uint8_t *)hModule + 0x24)).Buffer;
	auto len = wcstombs(chName, wchName, 260);
	chName[len] = '\0';

	skDbgPrint("[sk] Attempting to delete file: %s\n", chName);

	char chPath[260];
	int i = strlen(chName) - 1;
	for (; i > 0 && chName[i] != '\\'; i--);
	strcpy(chPath, SKMOUNT);
	strcat(chPath, &chName[i]);

	if (!FileExists(chPath)) {
		skDbgPrint("[sk] Error finding file: %s\n", chPath);
		return FALSE;
	}

	if (!DeleteFileA(chPath)) {
		skDbgPrint("[sk] Error deleting file: %s\n", chPath);
		return FALSE;
	}

	skDbgPrint("[sk] Successfully deleted file: %s\n", chName);
	return TRUE;
}

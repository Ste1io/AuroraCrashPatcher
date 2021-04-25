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

	DWORD WINAPI XNotifyProc(LPVOID lpParam) {
		((void(*)(DWORD, DWORD, DWORD, LPCWSTR, ULONGLONG))0x816AAC08)(34, 0, 2, (PWCHAR)lpParam, 0);
		return 0;
	}
} //namespace

VOID XNotify(PWCHAR chText) {
	if (((uint8_t(*)())0x80071A68)() != 1) {
		CreateThread(0, 0, XNotifyProc, chText, 0, 0);
	} else { XNotifyProc(chText); }
}

BOOL MountSysDrives() {
	if ((XboxHardwareInfo->Flags & 0x20) == 0x20)
		return CreateSymbolicLink(SKMOUNT, HDD, TRUE);
	return CreateSymbolicLink(SKMOUNT, USB, TRUE);
}

BOOL FileExists(LPCSTR fileName) {
	if (GetFileAttributesA(fileName) == -1) {
		CONST DWORD lastError = GetLastError();
		if (lastError == 2L || lastError == 3L)
			return FALSE;
	}
	return TRUE;
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

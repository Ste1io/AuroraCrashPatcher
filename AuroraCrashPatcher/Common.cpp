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

bool FileExists(LPCSTR lpFileName) {
	if (GetFileAttributesA(lpFileName) == -1) {
		CONST DWORD lastError = GetLastError();
		if (lastError == 2L || lastError == 3L)
			return false;
	}
	return true;
}

void SelfDestruct(HANDLE hModule) {
	char chName[260];
	wchar_t *wchName = (*(UNICODE_STRING*)((uint8_t *)hModule + 0x24)).Buffer;
	auto len = wcstombs(chName, wchName, 260);
	chName[len] = '\0';
	skDbgPrint("Deleting file: %s" skNewLn, chName);

	if (!MountSysDrives()) {
		skDbgPrint("Failed to mount system drives" skNewLn);
	} else {
		char name[260];
		int nameIdx;
		for (nameIdx = strlen(chName) - 1; nameIdx > 0 && chName[nameIdx] != '\\'; nameIdx--);
		strcpy(name, SKMOUNT);
		strcat(name, &chName[nameIdx]);

		if (!FileExists(name)) {
			skDbgPrint("Error finding file: %s" skNewLn, name);
		} else {
			if (!DeleteFileA(name)) {
				skDbgPrint("Error deleting file: %s" skNewLn, name);
			}
		}
	}
}
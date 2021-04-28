#pragma once

typedef struct _STRING {
	USHORT Length;
	USHORT MaximumLength;
	PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _CSTRING {
	USHORT Length;
	USHORT MaximumLength;
	CONST char *Buffer;
} CSTRING, *PCSTRING;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _XBOX_KRNL_VERSION {
	WORD Major;
	WORD Minor;
	WORD Build;
	WORD Qfe;
} XBOX_KRNL_VERSION, *PXBOX_KRNL_VERSION;

typedef struct _XBOX_HARDWARE_INFO {
	DWORD Flags;
	unsigned char NumberOfProcessors;
	unsigned char PCIBridgeRevisionID;
	unsigned char Reserved[6];
	unsigned short BldrMagic;
	unsigned short BldrFlags;
} XBOX_HARDWARE_INFO, *PXBOX_HARDWARE_INFO;

typedef struct _XEX_EXECUTION_ID {
	DWORD MediaID;
	DWORD Version;
	DWORD BaseVersion;
	union {
		DWORD TitleID;
		struct {
			WORD PublisherID;
			WORD GameID;
		};
	};
	BYTE Platform;
	BYTE ExecutableType;
	BYTE DiscNum;
	BYTE DiscsInSet;
	DWORD SaveGameID;
} XEX_EXECUTION_ID, *PXEX_EXECUTION_ID;

typedef struct _XEX_IMPORT_DESCRIPTOR {
	DWORD Size;
	DWORD NameTableSize;
	DWORD ModuleCount;
} XEX_IMPORT_DESCRIPTOR, *PXEX_IMPORT_DESCRIPTOR;

typedef struct _HV_IMAGE_IMPORT_TABLE {
	BYTE NextImportDigest[0x14];
	DWORD ModuleNumber;
	DWORD Version[2];
	BYTE Unused;
	BYTE ModuleIndex;
	WORD ImportCount;
} HV_IMAGE_IMPORT_TABLE, *PHV_IMAGE_IMPORT_TABLE;

typedef struct _XEX_IMPORT_TABLE {
	DWORD                 TableSize;
	HV_IMAGE_IMPORT_TABLE ImportTable;
} XEX_IMPORT_TABLE, *PXEX_IMPORT_TABLE;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY     InLoadOrderLinks;
	LIST_ENTRY     InClosureOrderLinks;
	LIST_ENTRY     InInitializationOrderLinks;
	VOID*          NtHeadersBase;
	VOID*          ImageBase;
	DWORD          SizeOfNtImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	DWORD          Flags;
	DWORD          SizeOfFullImage;
	VOID*          EntryPoint;
	WORD           LoadCount;
	WORD           ModuleIndex;
	VOID*          DllBaseOriginal;
	DWORD          CheckSum;
	DWORD          ModuleLoadFlags;
	DWORD          TimeDateStamp;
	VOID*          LoadedImports;
	VOID*          XexHeaderBase;
	union {
		STRING               LoadFileName;
		struct {
			_LDR_DATA_TABLE_ENTRY* ClosureRoot;
			_LDR_DATA_TABLE_ENTRY* TraversalParent;
		} asEntry;
	};
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

#ifdef __cplusplus
extern "C" {
	#endif
	VOID XapiThreadStartup(void(__cdecl *)(void*), void *, DWORD);
	HANDLE CreateThread(LPVOID, DWORD, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
	DWORD ExCreateThread(PHANDLE, DWORD, LPDWORD, PVOID, LPTHREAD_START_ROUTINE, LPVOID, DWORD);
	LONG XexGetModuleHandle(PCHAR, PHANDLE);
	LONG XexGetProcedureAddress(HANDLE, DWORD, PVOID);
	PVOID RtlImageXexHeaderField(PVOID xexHeaderBase, DWORD key);
	BOOL MmIsAddressValid(PVOID address);
	VOID RtlInitAnsiString(PSTRING DestinationString, CONST CHAR *SourceString);
	LONG ObCreateSymbolicLink(PSTRING SymbolicLinkName, PSTRING DeviceName);
	LONG ObDeleteSymbolicLink(PSTRING SymbolicLinkName);
	VOID HalSendSMCMessage(LPVOID pCommandBuffer, LPVOID pRecvBuffer);

	extern PXBOX_HARDWARE_INFO XboxHardwareInfo;
	extern PXBOX_KRNL_VERSION XboxKrnlBaseVersion;
	extern PXBOX_KRNL_VERSION XboxKrnlVersion;
	extern PLDR_DATA_TABLE_ENTRY* XexExecutableModuleHandle;
	#ifdef __cplusplus
}
#endif

#ifndef __isync
#define __isync() __emit(0x4C00012C)
#endif

SK_INLINE DWORD GetProcedureAddress(HANDLE moduleHandle, DWORD ordinal) {
	DWORD addr = NULL;
	if (moduleHandle)
		((DWORD(*)(HANDLE, DWORD, PVOID))0x8007D1B0)(moduleHandle, ordinal, &addr);
	return addr;
}

SK_INLINE DWORD ResolveFunction(PCHAR moduleName, DWORD ordinal) {
	HANDLE handle; DWORD addr;
	XexGetModuleHandle(moduleName, &handle);
	XexGetProcedureAddress(handle, ordinal, &addr);
	return addr;
}


SK_INLINE uint32_t ByteSwap(uint32_t value) {
	return (value & 0x000000FF) << 0x18
		| (value & 0x0000FF00) << 0x08
		| (value & 0x00FF0000) >> 0x08
		| (value & 0xFF000000) >> 0x18;
}

BOOL MountSysDrives();
BOOL FileExists(const char * fileName);
BOOL TrayOpen();
VOID DbgLog(BOOL printToConsole, const char * fmt, ...);
BOOL SelfDestruct(HANDLE hModule);

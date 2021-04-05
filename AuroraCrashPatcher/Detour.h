#pragma once

VOID HookFunctionStart(PDWORD Address, PDWORD SaveStub, DWORD Destination);
VOID DetourFunction(PDWORD FunctionAddress, DWORD DestinationFunction);

SK_INLINE INT Int24ToInt32(int Value) {
	BYTE lsb = (Value >> 24) & 0xFF;
	Value &= 0x00FFFFFF;

	if (Value & 0x800000 && lsb != 0x48)
		Value |= 0xFF000000;

	if (Value & 1)
		Value -= 1;

	return Value;
}

SK_INLINE INT GetBranchCall(int address) {
	int dest = *(int *)address;
	int temp = dest;
	dest = temp & 0x03FFFFFC;

	if (temp & 0x02000000)
		dest |= 0xFC000000;
	
	return address + dest;
}

SK_INLINE VOID PatchInJump(DWORD* Address, PVOID Dest, BOOL Linked) {
	DWORD Bytes[4];
	DWORD Destination = (DWORD)Dest;

	Bytes[0] = 0x3C000000 + ((Destination >> 16) & 0xFFFF);
	Bytes[1] = 0x60000000 + (Destination & 0xFFFF);
	Bytes[2] = 0x7C0903A6;
	Bytes[3] = 0x4E800420;

	if (Linked)
		Bytes[3] += 1;

	*(__int64 *)((DWORD)Address + 0) = *(__int64 *)&Bytes[0];
	*(__int64 *)((DWORD)Address + 8) = *(__int64 *)&Bytes[2];

	__dcbst(0, Address);
	__sync();
	__isync();
}

static DWORD GetModuleImport(HANDLE HModule, HANDLE HImportedModule, DWORD Ordinal) {
	PLDR_DATA_TABLE_ENTRY Module = (PLDR_DATA_TABLE_ENTRY)HModule;
	DWORD address = (DWORD)GetProcAddress((HMODULE)HImportedModule, (LPCSTR)Ordinal);

	if (address == NULL || HModule == NULL)
		return 0;

	PVOID headerBase = Module->XexHeaderBase;
	PXEX_IMPORT_DESCRIPTOR importDesc = (PXEX_IMPORT_DESCRIPTOR)RtlImageXexHeaderField(headerBase, 0x103FF);

	if (importDesc == NULL)
		return 0;

	CHAR* stringTable = (CHAR*)(importDesc + 1);

	PXEX_IMPORT_TABLE importTable = (PXEX_IMPORT_TABLE)(stringTable + importDesc->NameTableSize);

	for (DWORD x = 0; x < importDesc->ModuleCount; x++) {
		DWORD* importAdd = (DWORD*)(importTable + 1);
		for (DWORD y = 0; y < importTable->ImportTable.ImportCount; y++) {
			DWORD value = *((DWORD*)importAdd[y]);
			if (value == address) {
				return importAdd[y + 1];
			}
		}

		importTable = (PXEX_IMPORT_TABLE)(((BYTE*)importTable) +
			importTable->TableSize);
	}

	return 0;
}

static void __declspec(naked) SetupCaller() {
	__asm {
		mr r3, r4
		mr r4, r5
		mr r5, r6
		mr r6, r7
		mr r7, r8
		mr r8, r9
		mr r9, r10
		blr
	}
}


static BYTE DetourAsm[0x500] ={ 0 };
static DWORD DetourAsmIndex;
static RTL_CRITICAL_SECTION DetourAsmSection;

template<class T>
class Detour {
private:
	BYTE OriginalAsm[0x10];
	DWORD DetourIndex;

public:
	DWORD Addr;
	DWORD SaveStub;
	Detour()
		: Addr(0)
		, SaveStub(0)
		, DetourIndex(0) { };

	virtual ~Detour() { TakeDownDetour(); }

	virtual BOOL SetupDetour(HANDLE hModule, char* ImportedModuleName, int Ordinal, PVOID Destination) {
		DWORD dwAddress;
		HANDLE hImportedModule = (HANDLE)GetModuleHandle(ImportedModuleName);

		if (hImportedModule == NULL)
			return FALSE;

		dwAddress = GetModuleImport(hModule, hImportedModule, Ordinal);

		if (dwAddress == 0)
			return FALSE;

		return SetupDetour(dwAddress, Destination);
	}

	virtual BOOL SetupDetour(char* Module, char* ImportedModuleName, int Ordinal, PVOID Destination) {
		HANDLE hModule = (HANDLE)GetModuleHandle(Module);

		if (hModule == NULL)
			return FALSE;

		return SetupDetour(hModule, ImportedModuleName, Ordinal, Destination);
	}

	virtual BOOL SetupDetour(HANDLE Module, int Ordinal, PVOID Destination) {
		DWORD dwAddress;

		if (Module == NULL)
			return FALSE;

		dwAddress = (int)GetProcAddress((HMODULE)Module, (LPCSTR)Ordinal);

		if (dwAddress == NULL)
			return FALSE;

		return SetupDetour(dwAddress, Destination);
	}

	virtual BOOL SetupDetour(char* Module, int Ordinal, PVOID Destination) {
		DWORD dwAddress;
		HMODULE mHandle = GetModuleHandle(Module);

		if (mHandle == NULL)
			return FALSE;

		dwAddress = (DWORD)GetProcAddress(mHandle, (LPCSTR)Ordinal);

		if (dwAddress == NULL)
			return FALSE;

		return SetupDetour(dwAddress, Destination);
	}

	virtual BOOL SetupDetour(DWORD Address, PVOID Destination) {
		if (DetourAsmSection.Synchronization.RawEvent[0] == 0)
			InitializeCriticalSection(&DetourAsmSection);

		EnterCriticalSection(&DetourAsmSection);

		if (Addr != Address || SaveStub == 0) {
			DetourIndex = DetourAsmIndex;
			SaveStub = (DWORD)&DetourAsm[DetourIndex];
			Addr = Address;
			memcpy(OriginalAsm, (PVOID)Address, 0x10);
			DetourAsmIndex += DetourFunctionStart(Address, SaveStub, Destination);
		} else {
			DetourFunctionStart(Address, SaveStub, Destination);
		}

		LeaveCriticalSection(&DetourAsmSection);

		return TRUE;
	}

	virtual void TakeDownDetour() {
		if (Addr && MmIsAddressValid((PVOID)Addr)) {
			memcpy((PVOID)Addr, OriginalAsm, 0x10);
		}

		Addr = 0;
		SaveStub = 0;
		DetourIndex = 0;
	}

	virtual T CallOriginal(...) {
		SetupCaller();
		return ((T(*)(...))SaveStub)();
	}

protected:
	DWORD DetourFunctionStart(DWORD dwFunctionAddress, DWORD dwStubAddress, PVOID pDestFunc) {
		DWORD dwTemp;
		DWORD dwTempFuncAddr;
		BOOL bTemp;
		DWORD dwLength = 0;

		for (int i = 0; i < 4; i++) {
			dwTempFuncAddr = dwFunctionAddress + (i * 4);
			byte b = *(byte *)dwTempFuncAddr;

			if (b == 0x48 || b == 0x4B) {
				dwTemp = dwTempFuncAddr + Int24ToInt32(*(DWORD *)dwTempFuncAddr);
				bTemp = (*(DWORD *)dwTempFuncAddr & 1) != 0;
				PatchInJump((PDWORD)(dwStubAddress + dwLength), (PVOID)dwTemp, bTemp);
				dwLength += 0x10;

				if (!bTemp)
					goto DoHook;
			}

			else if (*(DWORD *)dwTempFuncAddr == 0)
				break;

			else {
				*(DWORD *)(dwStubAddress + dwLength) = *(DWORD *)dwTempFuncAddr;
				dwLength += 4;
			}
		}

		PatchInJump((PDWORD)(dwStubAddress + dwLength), (PVOID)(dwFunctionAddress + 0x10), FALSE);
		dwLength += 0x10;

	DoHook:
		PatchInJump((PDWORD)dwFunctionAddress, pDestFunc, FALSE);
		return dwLength;
	}
};

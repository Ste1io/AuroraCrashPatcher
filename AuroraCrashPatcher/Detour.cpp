#include "stdafx.h"
#include "Detour.h"

VOID __declspec(naked) GPLR(VOID) {
	__asm {
		std     r14, -0x98(sp)
		std     r15, -0x90(sp)
		std     r16, -0x88(sp)
		std     r17, -0x80(sp)
		std     r18, -0x78(sp)
		std     r19, -0x70(sp)
		std     r20, -0x68(sp)
		std     r21, -0x60(sp)
		std     r22, -0x58(sp)
		std     r23, -0x50(sp)
		std     r24, -0x48(sp)
		std     r25, -0x40(sp)
		std     r26, -0x38(sp)
		std     r27, -0x30(sp)
		std     r28, -0x28(sp)
		std     r29, -0x20(sp)
		std     r30, -0x18(sp)
		std     r31, -0x10(sp)
		stw     r12, -0x8(sp)
		blr
	}
}

DWORD RelinkGPLR(DWORD SFSOffset, PDWORD SaveStubAddress, PDWORD OriginalAddress) {
	DWORD Instruction = 0, Replacing;
	PDWORD Saver = (PDWORD)GPLR;

	if (SFSOffset & 0x2000000) {
		SFSOffset = SFSOffset | 0xFC000000;
	}

	Replacing = OriginalAddress[SFSOffset / 4];

	for (int i = 0; i < 20; i++) {
		if (Replacing == Saver[i]) {
			int NewOffset = (int)&Saver[i] - (int)SaveStubAddress;
			Instruction = 0x48000001 | (NewOffset & 0x3FFFFFC);
		}
	}

	return Instruction;
}

VOID HookFunctionStart(PDWORD Address, PDWORD SaveStub, DWORD Destination) {
	if ((SaveStub != NULL) && (Address != NULL)) {
		DWORD AddressRelocation = (DWORD)(&Address[4]);

		if (AddressRelocation & 0x8000) {
			SaveStub[0] = 0x3D600000 + (((AddressRelocation >> 16) & 0xFFFF) + 1);
		} else {
			SaveStub[0] = 0x3D600000 + ((AddressRelocation >> 16) & 0xFFFF);
		}

		SaveStub[1] = 0x396B0000 + (AddressRelocation & 0xFFFF);
		SaveStub[2] = 0x7D6903A6;

		for (int i = 0; i < 4; i++) {
			if ((Address[i] & 0x48000003) == 0x48000001) {
				SaveStub[i + 3] = RelinkGPLR((Address[i] & ~0x48000003), &SaveStub[i + 3], &Address[i]);
			} else {
				SaveStub[i + 3] = Address[i];
			}
		}

		SaveStub[7] = 0x4E800420;
		__dcbst(0, SaveStub);
		__emit(0x7c0004ac);
		__emit(0x4C00012C);

		DetourFunction(Address, Destination);
	}
}

VOID DetourFunction(PDWORD FunctionAddress, DWORD DestinationFunction) {
	if (DestinationFunction & 0x8000)
		FunctionAddress[0] = 0x3D600000 + (((DestinationFunction >> 16) & 0xFFFF) + 1);
	else FunctionAddress[0] = 0x3D600000 + ((DestinationFunction >> 16) & 0xFFFF);
	FunctionAddress[1] = 0x396B0000 + (DestinationFunction & 0xFFFF);
	FunctionAddress[2] = 0x7D6903A6;
	FunctionAddress[3] = 0x4E800420;
}

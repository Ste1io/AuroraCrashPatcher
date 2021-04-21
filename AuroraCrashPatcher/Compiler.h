#pragma once

extern "C" unsigned long __cdecl DbgPrint(char *, ...);

#define WIN32_LEAN_AND_MEAN

#define SK_INLINE                 __inline
#define SK_FORCE_INLINE           __forceinline

#ifndef NDEBUG
#define skDbgPrint                DbgPrint
#else
#define skDbgPrint
#endif

#define HDD                       "\\Device\\Harddisk0\\Partition1"
#define USB                       "\\Device\\Mass0"
#define SKMOUNT                   "SK_ACP:\\"

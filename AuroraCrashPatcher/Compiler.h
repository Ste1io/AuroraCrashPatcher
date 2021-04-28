#pragma once

extern "C" unsigned long __cdecl DbgPrint(char *, ...);
void DbgLog(bool printToConsole, const char * fmt, ...);

#define WIN32_LEAN_AND_MEAN

#define SK_INLINE                 __inline
#define SK_FORCE_INLINE           __forceinline

#ifndef NDEBUG
#define skDbgPrint                DbgPrint
#define skDbgLog                  DbgLog
#else
#define skDbgPrint
#define skDbgLog
#endif

#define SK_VERSION                "1.2"

#define HDD                       "\\Device\\Harddisk0\\Partition1"
#define USB                       "\\Device\\Mass0"
#define SKMOUNT                   "SK_ACP:\\"

#define SK_LOGPATH                SKMOUNT "AuroraCrashPatcher.log"

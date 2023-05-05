#pragma once
#include <intrin.h>
#include "physMem.h"
#include "winStructs.h"

#define EPROCESS_ImageFileName 0x5a8
#define EPROCESS_ActiveThreads 0x5f0
#define EPROCESS_ActiveProcessLinks 0x448
#define EPROCESS_ProcessId 0x440
#define EPROCESS_Peb 0x550
#define EPROCESS_wow64 0x580
#define EWOW64PROCESS_Peb 0x0

NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process(_In_ PEPROCESS Process);

typedef unsigned long DWORD, * PDWORD, * LPDWORD;

typedef struct _PINFO
{
	DWORD pid;
	PEPROCESS process;
	uintptr_t procBase;
	uintptr_t wow64offset;
}PINFO, * PPINFO;

NTSTATUS GetProcessInfo32(__in PPINFO ProcessInformation, CONST PCHAR ProcessName, CONST PCHAR ModName);
NTSTATUS GetProcessInfo64(__in PPINFO ProcessInformation, CONST PCHAR ProcessName, CONST PCHAR ModName);

PINFO GetUMProcessInfo();
PINFO GetCsgoProcessInfo();

inline BOOL isPinfoValid(PPINFO ProcessInformation)
{
	if (ProcessInformation->pid && ProcessInformation->procBase && ProcessInformation->process)
		return TRUE;
	return FALSE;
}
#include "processInfo.h"

//x64
uintptr_t GetModuleBase64(uintptr_t input_pid, uintptr_t input_peb, const char* modName)
{
	if (!input_peb)
		return 0;
	SIZE_T pebResult;
	PEB64 peb = { 0 };
	ReadProcessMemory(input_pid, (unsigned char*)input_peb, (uintptr_t)&peb, sizeof(peb), &pebResult);

	if (!peb.Ldr)
		return 0;
	SIZE_T pebLdrResult;
	PEB_LDR_DATA ldr;
	ReadProcessMemory(input_pid, (unsigned char*)peb.Ldr, (uintptr_t)&ldr, sizeof(ldr), &pebLdrResult);


	LDR_DATA_TABLE_ENTRY* pModEntry;
	for (LIST_ENTRY* pCur = ldr.InMemoryOrderModuleList.Flink; (uint8_t*)pCur != (uint8_t*)peb.Ldr + offsetof(PEB_LDR_DATA, InMemoryOrderModuleList); pCur = pModEntry->InMemoryOrderLinks.Flink)
	{
		SIZE_T pebLdrDataTableResult;
		LDR_DATA_TABLE_ENTRY curData;
		ReadProcessMemory(input_pid, (unsigned char*)pCur, (uintptr_t)&curData, sizeof(curData), &pebLdrDataTableResult);

		pModEntry = CONTAINING_RECORD(&curData, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
		if (pModEntry->BaseDllName.Buffer)
		{

			SIZE_T wszBuffResult;
			wchar_t wszBuff[260];
			ReadProcessMemory(input_pid, (unsigned char*)pModEntry->BaseDllName.Buffer, (uintptr_t)wszBuff, (SIZE_T)(pModEntry->BaseDllName.Length + 2), &wszBuffResult);

			/*DbgPrintEx(0, 0, " %p: %S\n", pModEntry->DllBase, wszBuff);*/
			if (_wcsicmp(modName, wszBuff) == 0)
			{
				return (uintptr_t)pModEntry->DllBase;
				break;
			}
		}

		if (!pModEntry->InMemoryOrderLinks.Flink)
			return 0;
	}

	return 0;
}

NTSTATUS GetProcessInfo64(__in PPINFO ProcessInformation, CONST PCHAR ProcessName, CONST PCHAR ModName)
{
	if (ProcessInformation)
	{
		PEPROCESS SysProcess = PsInitialSystemProcess;
		PEPROCESS CurrProcess = SysProcess;

		CHAR CurrProcName[15];

		do
		{
			RtlCopyMemory((PVOID)(&CurrProcName), (PVOID)((uintptr_t)CurrProcess + EPROCESS_ImageFileName), sizeof(CurrProcName));

			if (strstr(CurrProcName, ProcessName))
			{
				DWORD ActiveThreads;
				RtlCopyMemory((PVOID)&ActiveThreads, (PVOID)((uintptr_t)CurrProcess + EPROCESS_ActiveThreads), sizeof(ActiveThreads)); //new
				if (ActiveThreads)
				{
					uintptr_t peb;
					RtlCopyMemory((PVOID)&ProcessInformation->pid, (PVOID)((uintptr_t)CurrProcess + EPROCESS_ProcessId), sizeof(ProcessInformation->pid));
					RtlCopyMemory((PVOID)&peb, (PVOID)((uintptr_t)CurrProcess + EPROCESS_Peb), sizeof(peb));
					ProcessInformation->process = CurrProcess; //*process
					if (peb)
					{
						ProcessInformation->procBase = GetModuleBase64(ProcessInformation->pid, peb, ModName);
					}
				}
			}

			PLIST_ENTRY List = (PLIST_ENTRY)((uintptr_t)CurrProcess + EPROCESS_ActiveProcessLinks);
			CurrProcess = (PEPROCESS)((uintptr_t)List->Flink - EPROCESS_ActiveProcessLinks);
		} while (CurrProcess != SysProcess);
	}

}



//x86
uintptr_t GetModuleBase32(uintptr_t input_pid, uintptr_t input_peb, const char* modName)
{
	if (!input_peb)
		return 0;
	SIZE_T pebResult;
	PEB32 peb = { 0 };
	ReadProcessMemory(input_pid, (unsigned char*)input_peb, (uintptr_t)&peb, sizeof(peb), &pebResult);

	if (!peb.Ldr)
		return 0;
	SIZE_T pebLdrResult;
	PEB_LDR_DATA32 ldr;
	ReadProcessMemory(input_pid, (unsigned char*)peb.Ldr, (uintptr_t)&ldr, sizeof(ldr), &pebLdrResult);


	LDR_DATA_TABLE_ENTRY32* pModEntry;
	for (LIST_ENTRY32* pCur = ldr.InMemoryOrderModuleList.Flink; (uint8_t*)pCur != (uint8_t*)peb.Ldr + offsetof(PEB_LDR_DATA32, InMemoryOrderModuleList); pCur = pModEntry->InMemoryOrderLinks.Flink)
	{
		SIZE_T pebLdrDataTableResult;
		LDR_DATA_TABLE_ENTRY32 curData;
		ReadProcessMemory(input_pid, (unsigned char*)pCur, (uintptr_t)&curData, sizeof(curData), &pebLdrDataTableResult);

		pModEntry = CONTAINING_RECORD(&curData, LDR_DATA_TABLE_ENTRY32, InMemoryOrderLinks);
		if (pModEntry->BaseDllName.Buffer)
		{

			SIZE_T wszBuffResult;
			wchar_t wszBuff[260];
			ReadProcessMemory(input_pid, (unsigned char*)pModEntry->BaseDllName.Buffer, (uintptr_t)wszBuff, (SIZE_T)(pModEntry->BaseDllName.Length + 2), &wszBuffResult);

			/*DbgPrintEx(0, 0, " %p: %S\n", pModEntry->DllBase, wszBuff);*/
			if (_wcsicmp(modName, wszBuff) == 0)
			{
				return (uintptr_t)pModEntry->DllBase;
			}
		}

		if (!pModEntry->InMemoryOrderLinks.Flink)
			return 0;
	}

	return 0;
}

NTSTATUS GetProcessInfo32(__in PPINFO ProcessInformation, CONST PCHAR ProcessName, CONST PCHAR ModName)
{
	if (ProcessInformation)
	{
		PEPROCESS SysProcess = PsInitialSystemProcess;
		PEPROCESS CurrProcess = SysProcess;

		CHAR CurrProcName[15];

		do
		{
			RtlCopyMemory((PVOID)(&CurrProcName), (PVOID)((uintptr_t)CurrProcess + EPROCESS_ImageFileName), sizeof(CurrProcName));

			if (strstr(CurrProcName, ProcessName))
			{
				DWORD ActiveThreads;
				RtlCopyMemory((PVOID)&ActiveThreads, (PVOID)((uintptr_t)CurrProcess + EPROCESS_ActiveThreads), sizeof(ActiveThreads)); //new
				if (ActiveThreads)
				{
					RtlCopyMemory((PVOID)&ProcessInformation->pid, (PVOID)((uintptr_t)CurrProcess + EPROCESS_ProcessId), sizeof(ProcessInformation->pid));
					ProcessInformation->process = CurrProcess;

					if (!ProcessInformation->process)
						return STATUS_UNSUCCESSFUL;

					uintptr_t peb32 = (PPEB32)PsGetProcessWow64Process(ProcessInformation->process);
					if (!peb32) { return STATUS_UNSUCCESSFUL; }
					ProcessInformation->wow64offset = peb32;

					ProcessInformation->procBase = GetModuleBase32(ProcessInformation->pid, peb32, ModName);
				}
			}

			PLIST_ENTRY List = (PLIST_ENTRY)((uintptr_t)CurrProcess + EPROCESS_ActiveProcessLinks);
			CurrProcess = (PEPROCESS)((uintptr_t)List->Flink - EPROCESS_ActiveProcessLinks);
		} while (CurrProcess != SysProcess);
	}

}

PINFO GetUMProcessInfo()
{
	PINFO UMProgrammInfo;
	GetProcessInfo64(&UMProgrammInfo, "UM.exe", L"UM.exe");
	return UMProgrammInfo;
}
PINFO GetCsgoProcessInfo()
{
	PINFO ProcessInformation;
	GetProcessInfo32(&ProcessInformation, "csgo.exe", L"client.dll");
	return ProcessInformation;
}

#include "mem.h"

DWORD64 rpmRequestedAddr = 0, addrToReadAddr = 0, resultAddr = 0, pidAddr = 0, RPMWPMAddr = 0, sizeToReadAddr = 0;

typedef struct _DRIVER_DEVICE
{
	BOOL	rpmRequested;
	BOOL	wpm;
	int		sizeToRead;
	int		pid; 
	UINT64	addrToRead;
	UINT64	result;
}DRIVER_DEVICE, * PDRIVER_DEVICE;


ULONG GetKeyInfoSize(HANDLE hKey, PUNICODE_STRING Key)
{
	NTSTATUS Status;
	ULONG KeySize = 1;

	Status = ZwQueryValueKey(hKey, Key, KeyValueFullInformation, 0, 0, &KeySize);

	if (Status == STATUS_BUFFER_TOO_SMALL || Status == STATUS_BUFFER_OVERFLOW)
		return KeySize;

	return 0;
}
DWORD64 ReadRegistry(UNICODE_STRING RegPath, UNICODE_STRING Key)
{
	HANDLE hKey;
	OBJECT_ATTRIBUTES ObjAttr;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	InitializeObjectAttributes(&ObjAttr, &RegPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	Status = ZwOpenKey(&hKey, KEY_ALL_ACCESS, &ObjAttr);

	if (STATUS_SUCCESS == Status)
	{
		ULONG KeyInfoSize = GetKeyInfoSize(hKey, &Key);
		ULONG KeyInfoSizeNeeded;

		if (KeyInfoSize == NULL)
		{
			ZwClose(hKey);
			return 0;
		}
		PKEY_VALUE_FULL_INFORMATION pKeyInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(NonPagedPool, KeyInfoSize);
		RtlZeroMemory(pKeyInfo, KeyInfoSize);

		Status = ZwQueryValueKey(hKey, &Key, KeyValueFullInformation, pKeyInfo, KeyInfoSize, &KeyInfoSizeNeeded);

		if (!NT_SUCCESS(Status) || (KeyInfoSize != KeyInfoSizeNeeded))
		{
			ZwClose(hKey);
			ExFreePoolWithTag(pKeyInfo, 0);
			return 0;
		}

		ZwClose(hKey);
		ExFreePoolWithTag(pKeyInfo, 0);

		return *(DWORD64*)((LONG64)pKeyInfo + pKeyInfo->DataOffset);
	}

	return 0;
}

NTSTATUS getRegs()
{
	UNICODE_STRING regPath = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\SOFTWARE\\BRUH");

	rpmRequestedAddr	= ReadRegistry(regPath, RTL_CONSTANT_STRING(L"bRequested"));
	addrToReadAddr		= ReadRegistry(regPath, RTL_CONSTANT_STRING(L"addr"));
	resultAddr			= ReadRegistry(regPath, RTL_CONSTANT_STRING(L"result"));
	pidAddr				= ReadRegistry(regPath, RTL_CONSTANT_STRING(L"pidAddr"));
	sizeToReadAddr		= ReadRegistry(regPath, RTL_CONSTANT_STRING(L"sizeAddr"));
	RPMWPMAddr			= ReadRegistry(regPath, RTL_CONSTANT_STRING(L"RPMWPMAddr"));

	/*DbgPrintEx(0, 0, "rpmRequestedAddr: 0x%p \naddrToReadAddr: 0x%p \nresultAddr: 0x%p \npidAddr: 0x%p \nsizeToReadAddr: 0x%p \nWPMAddr: 0x%p \n", rpmRequestedAddr, addrToReadAddr, resultAddr, pidAddr, sizeToReadAddr, RPMWPMAddr);*/

	if (rpmRequestedAddr * addrToReadAddr * resultAddr * pidAddr * sizeToReadAddr * RPMWPMAddr != FALSE)
		return STATUS_SUCCESS;

	return STATUS_UNSUCCESSFUL;
}

NTSTATUS readValues(PDRIVER_DEVICE returnComms)
{
	PINFO UMProcessInfo = GetUMProcessInfo();
	if (!isPinfoValid(&UMProcessInfo))
		return STATUS_UNSUCCESSFUL;

	SIZE_T result, result1, result2, result3, result4;
	ReadProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(rpmRequestedAddr)),	&returnComms->rpmRequested, sizeof(returnComms->rpmRequested),	&result);
	ReadProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(addrToReadAddr)),		&returnComms->addrToRead,	sizeof(returnComms->addrToRead),	&result1);
	ReadProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(pidAddr)),			&returnComms->pid,			sizeof(returnComms->pid),			&result2);
	ReadProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(sizeToReadAddr)),		&returnComms->sizeToRead,	sizeof(returnComms->sizeToRead),	&result3);
	ReadProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(RPMWPMAddr)),			&returnComms->wpm,			sizeof(returnComms->wpm),			&result4);

	/*DbgPrintEx(0, 0, "rpmRequested: %d\naddrToRead: %d\npid: %d\nsizeToRead: %d\nwpm: %d\n", returnComms->rpmRequested, returnComms->addrToRead, returnComms->pid, returnComms->sizeToRead, returnComms->wpm);*/
	return STATUS_SUCCESS;
}

VOID callListener()
{
	PINFO UMProcessInfo = GetUMProcessInfo();
	if (!isPinfoValid(&UMProcessInfo))
	{
		rpmRequestedAddr = 0;
		addrToReadAddr = 0;
		resultAddr = 0;
		pidAddr = 0;
		sizeToReadAddr = 0;
		return;
	}

	if (!rpmRequestedAddr || !addrToReadAddr || !resultAddr || !pidAddr || !sizeToReadAddr || !RPMWPMAddr)
	{
		getRegs();
		return;
	}

	DRIVER_DEVICE currComm;

	readValues(&currComm);

	if (currComm.pid <= 0)
	{
		return;
	}

	if (!currComm.rpmRequested)
	{
		return;
	}

	//reading
	if (!currComm.wpm)
	{
		SIZE_T result1; SIZE_T written; SIZE_T written1;

		//read from target
		CHAR read[8] = {0}; 
		ReadProcessMemory(currComm.pid, (PVOID)((UINT64)currComm.addrToRead), &read, currComm.sizeToRead, &result1);

		//write read buffer into UM
		WriteProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(resultAddr)), &read, sizeof(read), &written);

		//set UM rpmRequstedFalse to FALSE
		BOOL rpmRequstedFalse = FALSE;
		WriteProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(rpmRequestedAddr)), &rpmRequstedFalse, sizeof(rpmRequstedFalse), &written1);
	}
	//writing
	else if (currComm.wpm)
	{
		SIZE_T written2; SIZE_T result1; SIZE_T written1;
		
		//read UM result buffer
		CHAR write[8] = {0}; 
		ReadProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)resultAddr), &write, currComm.sizeToRead, &result1);

		//write the buffer called write from UM to target
		WriteProcessMemory(currComm.pid, (PVOID)((UINT64)currComm.addrToRead), &write, currComm.sizeToRead, &written2);

		//set rpmRequstedFalse to FALSE
		BOOL rpmRequstedFalse = FALSE;
		WriteProcessMemory(UMProcessInfo.pid, (PVOID)((UINT64)(rpmRequestedAddr)), &rpmRequstedFalse, sizeof(rpmRequstedFalse), &written1);
	}

	return;
}
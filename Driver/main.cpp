#include "mem.h"

void mainThread()
{

	while (true)
	{
		callListener();
	}
}

extern "C"
NTSTATUS CustomDriverEntry(
	_In_ PDRIVER_OBJECT  kdmapperParam1,
	_In_ PUNICODE_STRING kdmapperParam2
)
{
	UNREFERENCED_PARAMETER(kdmapperParam1);
	UNREFERENCED_PARAMETER(kdmapperParam2);

	DbgPrintEx(0, 0, "driver running\n");


	HANDLE hThread;
	PsCreateSystemThread(&hThread,
		THREAD_ALL_ACCESS, NULL, NULL, NULL,
		(PKSTART_ROUTINE)mainThread, NULL);

	/*PETHREAD ThreadObject;
	ObReferenceObjectByHandle(hThread,
						THREAD_ALL_ACCESS,
						NULL,
						KernelMode,
						&ThreadObject,
						NULL);*/
	ZwClose(hThread);

	DbgPrintEx(0, 0, "driverEntry: exiting\n");
	return 0;
}
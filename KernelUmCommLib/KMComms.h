#pragma once
#include <Windows.h>
#include <iostream>

//*reinterpret_cast<BOOL*>(rpmRequestedAddr);

inline DWORD64 rpmRequestedAddr, RPMWPMAddr, gameAddr, sizeToReadAddr, pidAddr, addrToReadAddr, resultAddr;

typedef struct DRIVER_DEVICE
{
	BOOL rpmRequested;
	BOOL wpm;
	int sizeToRead;
	int pid;
	UINT64 addrToRead;
	UINT64 result;
};

namespace driver
{
	//Init the driverDevice structure
	void InitDriverDevice(DRIVER_DEVICE* DriverDevice, int gamePid);

	//Must be called as first function in main
	void Begin(DRIVER_DEVICE* DriverDevice);
	
	//Must be called on programm exit
	//Is set to run at std::atexit in driver::Begin
	void exit();


	template<typename T>
	T rpm(UINT64 Addr)
	{
		if (sizeof(T) > 8)
		{
			printf("size curr max 8\n");
			return T(0);
		}
		if (*reinterpret_cast<BOOL*>(rpmRequestedAddr) == FALSE)
		{
			*reinterpret_cast<BOOL*>(RPMWPMAddr) = FALSE;

			*reinterpret_cast<UINT64*>(addrToReadAddr) = Addr;

			*reinterpret_cast<int*>(sizeToReadAddr) = sizeof(T);

			*reinterpret_cast<BOOL*>(rpmRequestedAddr) = TRUE;

			while (*reinterpret_cast<BOOL*>(rpmRequestedAddr))
			{
				Sleep(0.2);
			}

			T buffer;
			memcpy(&buffer, &*reinterpret_cast<UINT64*>(resultAddr), sizeof(T));
			return buffer;
		}
	}

	template<typename T>
	void wpm(UINT64 addr, T buffer)
	{
		if (sizeof(buffer) > 8)
		{
			printf("buffer curr max 8\n");
			return;
		}
		if (*reinterpret_cast<BOOL*>(rpmRequestedAddr) == FALSE)
		{
			*reinterpret_cast<BOOL*>(RPMWPMAddr) = TRUE;

			*reinterpret_cast<UINT64*>(addrToReadAddr) = addr;

			*reinterpret_cast<int*>(sizeToReadAddr) = sizeof(T);

			memcpy(&*reinterpret_cast<UINT64*>(resultAddr), &buffer, sizeof(T));

			*reinterpret_cast<BOOL*>(rpmRequestedAddr) = TRUE;

			while (*reinterpret_cast<BOOL*>(rpmRequestedAddr) == TRUE)
			{
				Sleep(0.2);
			}
			return;
		}
	}
}
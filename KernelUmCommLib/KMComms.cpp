#include "KMComms.h"





void driver::exit()
{
	HKEY BRUHKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\BRUH"), &BRUHKey) == ERROR_SUCCESS)
	{

		DWORD64 nullll = 0;
		if (RegSetValueEx(BRUHKey, TEXT("bRequested"), 0, REG_QWORD, (LPBYTE)&nullll, sizeof(nullll)) != ERROR_SUCCESS)
			return;
		if (RegSetValueEx(BRUHKey, TEXT("addr"), 0, REG_QWORD, (LPBYTE)&nullll, sizeof(nullll)) != ERROR_SUCCESS)
			return;
		if (RegSetValueEx(BRUHKey, TEXT("result"), 0, REG_QWORD, (LPBYTE)&nullll, sizeof(nullll)) != ERROR_SUCCESS)
			return;
		if (RegSetValueEx(BRUHKey, TEXT("sizeAddr"), 0, REG_QWORD, (LPBYTE)&nullll, sizeof(nullll)) != ERROR_SUCCESS)
			return;
		if (RegSetValueEx(BRUHKey, TEXT("RPMWPMAddr"), 0, REG_QWORD, (LPBYTE)&nullll, sizeof(nullll)) != ERROR_SUCCESS)
			return;
		if (RegSetValueEx(BRUHKey, TEXT("pidAddr"), 0, REG_QWORD, (LPBYTE)&nullll, sizeof(nullll)) != ERROR_SUCCESS)
			return;

		*reinterpret_cast<int*>(pidAddr) = 0;

		RegCloseKey(BRUHKey);
	}
	return;
}

void driver::Begin(DRIVER_DEVICE* DriverDevice)
{
	HKEY BRUHKey;
	if (RegCreateKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\BRUH"), &BRUHKey) == ERROR_SUCCESS)
	{
		rpmRequestedAddr = (DWORD64)&DriverDevice->rpmRequested;
		if (RegSetValueEx(BRUHKey, TEXT("bRequested"), 0, REG_QWORD, (LPBYTE)&rpmRequestedAddr, sizeof(rpmRequestedAddr)) != ERROR_SUCCESS)
			return;		
		
		RPMWPMAddr = (DWORD64)&DriverDevice->wpm;
		if (RegSetValueEx(BRUHKey, TEXT("RPMWPMAddr"), 0, REG_QWORD, (LPBYTE)&RPMWPMAddr, sizeof(RPMWPMAddr)) != ERROR_SUCCESS)
			return;

		sizeToReadAddr = (DWORD64)&DriverDevice->sizeToRead;
		if (RegSetValueEx(BRUHKey, TEXT("sizeAddr"), 0, REG_QWORD, (LPBYTE)&sizeToReadAddr, sizeof(sizeToReadAddr)) != ERROR_SUCCESS)
			return;

		pidAddr = (DWORD64)&DriverDevice->pid;
		if (RegSetValueEx(BRUHKey, TEXT("pidAddr"), 0, REG_QWORD, (LPBYTE)&pidAddr, sizeof(pidAddr)) != ERROR_SUCCESS)
			return;

		addrToReadAddr = (DWORD64)&DriverDevice->addrToRead;
		if (RegSetValueEx(BRUHKey, TEXT("addr"), 0, REG_QWORD, (LPBYTE)&addrToReadAddr, sizeof(addrToReadAddr)) != ERROR_SUCCESS)
			return;

		resultAddr = (DWORD64)&DriverDevice->result;
		if (RegSetValueEx(BRUHKey, TEXT("result"), 0, REG_QWORD, (LPBYTE)&resultAddr, sizeof(resultAddr)) != ERROR_SUCCESS)
			return;
	
		RegCloseKey(BRUHKey);
		std::atexit(driver::exit);
		return;
	}
}

void driver::InitDriverDevice(DRIVER_DEVICE* DriverDevice, int gamePid)
{
	DriverDevice->rpmRequested = 0;
	DriverDevice->wpm = 0;
	DriverDevice->sizeToRead = 0;
	DriverDevice->addrToRead = 0;
	DriverDevice->result = 0;
	DriverDevice->pid = gamePid;
}

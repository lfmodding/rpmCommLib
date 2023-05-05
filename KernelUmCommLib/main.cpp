#include "KMComms.h"
#include <TlHelp32.h>
#include "offsets.h"

using namespace driver;

DRIVER_DEVICE DriverDevice;

UINT32 GetModuleBaseAddress(const char* modName, int procId) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry)) {
			do {
				if (!strcmp(modEntry.szModule, modName)) {
					CloseHandle(hSnap);
					return (UINT32)modEntry.modBaseAddr;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
}

UINT64 makePtr32(UINT64 ptr64)
{
	UINT32 ptr32 = (UINT32)ptr64;
	return (UINT64)ptr32;
}

void EnableGlow(UINT64 GlowObjectManager, UINT64 playerAddr, FLOAT R, FLOAT G, FLOAT B, FLOAT A)
{
	if (!playerAddr || !GlowObjectManager)
		return;

	int glowIndex = rpm<UINT64>(playerAddr + netvars::m_iGlowIndex);
	if (!glowIndex)
		return;

	wpm<float>(GlowObjectManager + (glowIndex * 0x38) + 0x8, R);

	wpm<float>(GlowObjectManager + (glowIndex * 0x38) + 0xC, G);

	wpm<float>(GlowObjectManager + (glowIndex * 0x38) + 0x10, B);

	wpm<float>(GlowObjectManager + (glowIndex * 0x38) + 0x14, A);

	bool enableGlow = 1;
	wpm<bool>(GlowObjectManager + (glowIndex * 0x38) + 0x28, enableGlow);

	bool enableGlowEx = 1;
	wpm<bool>(GlowObjectManager + (glowIndex * 0x38) + 0x29, enableGlowEx);

	printf("enableGlow %d enableGlowEx %d R: %f\n", rpm<bool>(GlowObjectManager + (glowIndex * 0x38) + 0x28), rpm<bool>(GlowObjectManager + (glowIndex * 0x38) + 0x29), rpm<float>(GlowObjectManager + (glowIndex * 0x38) + 0x8));
	return;
}
UINT64 GetLocalPlayer(UINT64 modBase)
{
	if (!modBase)
		return 0;

	UINT64 localPlayer = rpm<UINT64>(modBase + netvars::dwLocalPlayer);
	return makePtr32(localPlayer);
}
UINT64 GetPlayer(INT player, UINT64 modBase)
{
	if (!modBase)
		return 0;

	UINT64 playeraddr = rpm<UINT64>(modBase + netvars::dwEntityList + player * 0x10);
	return makePtr32(playeraddr);
}
INT GetPlayerHealth(UINT64 playerAddr)
{
	if (!playerAddr)
		return 0;

	return rpm<int>(playerAddr + netvars::m_iHealth);
}

UINT64 GetGlowObjectManager(UINT64 modBase)
{
	if (!modBase)
		return 0;

	UINT64 GlowObjectManager = rpm<UINT64>(modBase + netvars::dwGlowObjectManager);
	return makePtr32(GlowObjectManager);
}

int main()
{
	HWND hwnd; DWORD procId;
	hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive - Direct3D 9");
	GetWindowThreadProcessId(hwnd, &procId);
	uintptr_t modBase = GetModuleBaseAddress("client.dll", procId);
	printf("modBase: 0x%p \n pid: %d\n", modBase, procId);

	driver::InitDriverDevice(&DriverDevice, procId);
	driver::Begin(&DriverDevice);
	
	UINT64 localPlayer = driver::rpm<UINT64>(modBase + 0xDEA964);
	printf("localPlayer: 0x%p \n ", localPlayer);
	if (!localPlayer)
		return 0;

	localPlayer = makePtr32(localPlayer);
	int one = 1;

	while (!GetAsyncKeyState(VK_END))
	{
		UINT64 localPlayer = GetLocalPlayer(modBase);
		int localPlayerHealth = GetPlayerHealth(localPlayer);
		UINT64 glowObjectManager = GetGlowObjectManager(modBase);

		for (int i = 0; i <= 64; i++)
		{
			if (localPlayerHealth <= 0 || localPlayerHealth > 100)
				continue;

			UINT64 currPlayer = GetPlayer(i, modBase);

			int playerHealth = GetPlayerHealth(currPlayer);

			if (playerHealth <= 0 || playerHealth > 100)
				continue;
			EnableGlow(glowObjectManager, currPlayer, 1.f, 0.f, 0.f, 1.f);
			printf(" %d glowEnabled %d %p\n", i, playerHealth, currPlayer);
		}
		Sleep(1);
	}
}
#pragma once
extern "C"
{
#include "dependencies/processInfo.h"
}

inline void mySleep(int ms)
{
	LARGE_INTEGER time = { 0 };
	time.QuadPart = -(ms) * 10 * 1000;
	KeDelayExecutionThread(KernelMode, TRUE, &time);
}

VOID callListener();
#pragma once
#include <wtypes.h>
#include <cstdint>


enum Request {
	GETBASE = 0,
	READPROCESSMEMORY = 1,
	WRITEPROCESSMEMORY = 2,
	OPENHANDLE = 3,
	MOUSE = 4,
};
struct Communication {

	Request Request;
	DWORD processID;
	DWORD Reason; // must be 0xDEADBEEF....
	PVOID Outbase; // output image base for process.

	//mouse
	long x;
	long y;
	unsigned short button_flags;

	/*
	* READ/WRITE PROCESS MEMORY.
	*/
	uintptr_t Address;
	uintptr_t result;
	size_t size;
	const char* moduleName;
};

namespace Comms {
	BOOL Setup(LPCSTR routineName);
	PVOID GetBaseAddress(DWORD processID, const char* name);
	NTSTATUS ReadProcessMemory(DWORD processID, uintptr_t src, void* dest, uint32_t size);
	NTSTATUS WriteProcessMemory(DWORD processID, uintptr_t src, uint8_t* dest, uint32_t size);
}






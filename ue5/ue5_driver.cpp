#pragma once
#include "ue5_driver.h"

typedef INT64(*Nt_UserGetPointerProprietaryId)(uintptr_t);
Nt_UserGetPointerProprietaryId NtUserGetPointerProprietaryId = nullptr;

namespace Comms {

	BOOL Setup(LPCSTR routineName) {
		LoadLibrary(L"user32.dll");

		const auto win32k = LoadLibrary(L"win32u.dll");

		if (!win32k)
		{
			return false;
		}

		*(void**)&NtUserGetPointerProprietaryId = GetProcAddress(win32k, "NtUserGetPointerProprietaryId");

		if (!NtUserGetPointerProprietaryId)
		{
			return false;
		}

		return true;
	}


	PVOID GetBaseAddress(DWORD processID, const char* name) {
		Communication request = {};
		SecureZeroMemory(&request, sizeof(Communication));

		request.Request = Request::GETBASE;
		request.Reason = 0xDEADBEEF;
		request.processID = processID;
		request.moduleName = name;
		request.Outbase = 0;

		NtUserGetPointerProprietaryId(reinterpret_cast<uintptr_t>(&request));
		return request.Outbase;
	}

	NTSTATUS ReadProcessMemory(DWORD processID, uintptr_t src, void* dest, uint32_t size) {


		Communication request = {};
		SecureZeroMemory(&request, sizeof(Communication));

		request.Request = Request::READPROCESSMEMORY;
		request.Reason = 0xDEADBEEF;
		request.processID = processID;
		request.Address = src;
		request.result = (uintptr_t)dest;
		request.size = size;

		return NtUserGetPointerProprietaryId(reinterpret_cast<uintptr_t>(&request));

	}

	NTSTATUS WriteProcessMemory(DWORD processID, uintptr_t src, uint8_t* dest, uint32_t size) {


		Communication request = {};
		SecureZeroMemory(&request, sizeof(Communication));

		request.Request = Request::WRITEPROCESSMEMORY;
		request.Reason = 0xDEADBEEF;
		request.processID = processID;
		request.Address = src;
		request.result = (uintptr_t)dest;
		request.size = size;

		return NtUserGetPointerProprietaryId(reinterpret_cast<uintptr_t>(&request));

	}

}
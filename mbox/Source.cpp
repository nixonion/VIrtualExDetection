//#include "pch.h"
#include <iostream>
#include <Windows.h>

FARPROC messageBoxAddress = NULL;
SIZE_T bytesWritten = 0;
char messageBoxOriginalBytes[6] = {};

LPVOID __stdcall HookedMessageBox(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect) {

	// print intercepted values from the MessageBoxA function
	if (flProtect == PAGE_EXECUTE_READWRITE)
	{
		std::cout << "I caught you hacker\n";
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)messageBoxAddress, messageBoxOriginalBytes, sizeof(messageBoxOriginalBytes), &bytesWritten);
		return NULL;

	}
	
	//std::cout << "Ohai from the hooked function\n";
	//std::cout << "Text: " << (LPCSTR)lpText << "\nCaption: " << (LPCSTR)lpCaption << std::endl;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)messageBoxAddress, messageBoxOriginalBytes, sizeof(messageBoxOriginalBytes), &bytesWritten);


	// unpatch MessageBoxA
	

	// call the original MessageBoxA
	return VirtualAllocEx(hProcess, NULL, dwSize, flAllocationType, flProtect);
}

int main()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPCWSTR cmd;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
	cmd = TEXT("C:\\Windows\\System32\\nslookup.exe");

	if (!CreateProcess(
		cmd,							// Executable
		NULL,							// Command line
		NULL,							// Process handle not inheritable
		NULL,							// Thread handle not inheritable
		FALSE,							// Set handle inheritance to FALSE
		CREATE_NO_WINDOW,	            // Do Not Open a Window
		NULL,							// Use parent's environment block
		NULL,							// Use parent's starting directory 
		&si,			                // Pointer to STARTUPINFO structure
		&pi								// Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	))
	{
		printf("Process could not be created\n");
		return 0;
	}

	//printf("Nslook process is created\n");

	LPVOID base_address = NULL;
	//printf("Memory has been allocated\n");
	base_address = VirtualAllocEx(pi.hProcess, NULL, 750, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	// show messagebox before hooking
	//MessageBoxA(NULL, "hi", "hi", MB_OK);
	//VirtualAllocEx
	HINSTANCE library = LoadLibraryA("kernel32.dll");
	SIZE_T bytesRead = 0;

	// get address of the MessageBox function in memory
	messageBoxAddress = GetProcAddress(library, "VirtualAllocEx");

	// save the first 6 bytes of the original MessageBoxA function - will need for unhooking
	ReadProcessMemory(GetCurrentProcess(), messageBoxAddress, messageBoxOriginalBytes, 6, &bytesRead);

	// create a patch "push <address of new MessageBoxA); ret"
	void* hookedMessageBoxAddress = &HookedMessageBox;
	char patch[6] = { 0 };
	memcpy_s(patch, 1, "\x68", 1);
	memcpy_s(patch + 1, 4, &hookedMessageBoxAddress, 4);
	memcpy_s(patch + 5, 1, "\xC3", 1);

	// patch the MessageBoxA
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)messageBoxAddress, patch, sizeof(patch), &bytesWritten);

	// show messagebox after hooking
	//MessageBoxA(NULL, "hi", "hi", MB_OK);
	base_address = VirtualAllocEx(pi.hProcess, NULL, 750, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


	return 0;
}
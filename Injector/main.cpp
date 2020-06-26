#include "header.h"


int main() {

	DWORD ID = findPID(L"notepad++.exe");
	InjectDLL(ID);
	return 0;
}
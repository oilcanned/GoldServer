#include "definitions.h"

#ifdef __linux__
	void* heartbeat(void* arg);
	void* autosaver(void* arg);
#elif _WIN32
	DWORD WINAPI heartbeat(void* arg);
	DWORD WINAPI autosaver(void* arg);
#endif
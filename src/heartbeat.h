#include "definitions.h"
#include "threads.h"

#ifdef __linux__
	void* heartbeat(void* arg);
	void* autosaver(void* arg);
	void* ping     (void* arg);
#elif _WIN32
	DWORD WINAPI heartbeat(void* arg);
	DWORD WINAPI autosaver(void* arg);
	DWORD WINAPI ping     (void* arg)
#endif
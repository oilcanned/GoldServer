#ifdef __linux__
	#include <pthread.h>
#elif _WIN32
	#include <winsock2.h>
	#include <windows.h>
	#define pthread_exit(NULL) ExitThread(0)
#endif
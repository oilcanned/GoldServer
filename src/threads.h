#ifdef __linux__
	#include <pthread.h>
#elif _WIN32
	#include <winsock2.h>
	#include <windows.h>
#endif

#ifdef _WIN32
	#define pthread_exit ExitThread
#endif
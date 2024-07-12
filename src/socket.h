#ifdef __linux__
	#include <netinet/in.h>
	#include <arpa/inet.h>
#elif _WIN32
	#include <winsock2.h>
	#include <windows.h>
#endif
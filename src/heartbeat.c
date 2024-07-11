#include <unistd.h>
#include <string.h>
#include <curl/curl.h>
#include "heartbeat.h"
#include "structs.h"
#include "worlds.h"
#include "functions.h"

void* heartbeat(void* arg) {
	printf("%sHeartbeat started.\n", AND_6);
	resetColour();
	pointers_t* arguments = (pointers_t*)arg;

	while (1) {
		char header[8196];
		char s_port[10];
		char s_max[10];
		char s_users[10];

		int tmpCount = 0;
		for (int i = 0; i < MAX; ++i) {
			if (players[i].sock) {
				++tmpCount;
			}
		}

		sprintf(s_port,  "%d", server_info.port);
		sprintf(s_max,   "%d", MAX);
		sprintf(s_users, "%d", tmpCount);

		strcpy(header, server_info.heartbeat_url);
		strcat(header, "?port=");
		strcat(header, s_port);
		strcat(header, "&max=");
		strcat(header, s_max);
		strcat(header, "&name=");
		strcat(header, server_info.server_name);
		strcat(header, "&public=True&version=7&salt=");
		strcat(header, server_info.salt);
		strcat(header, "&users=");
		strcat(header, s_users);

		CURL* curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, header);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 		   1);
		curl_easy_perform(curl);

		sleep(45);
	}
}

void* autosaver(void* arg) {
	printf("%sBegan autosave thread.\n", AND_6);
	resetColour();
	
	while (1) {
		sleep(server_info.autosave_interval);

		for (int i = 0; i < MAX; ++i) {
			if (players[i].sock)
				sendMessage(0, i, "Autosaving...");
		}

		for (int i = 0; i < MAX; ++i) {
			if (worlds[i].name) {
				saveworld(i, 0);
			}
		}
	}
}
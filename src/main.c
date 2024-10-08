#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>
#include <zlib.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <sys/stat.h>
#include "commands.h"
#include "structs.h"
#include "definitions.h"
#include "heartbeat.h"
#include "worlds.h"
#include "functions.h"
#include "lexer.h"
#include "socket.h"
#include "threads.h"
#include "unistd.h"

#ifdef __linux__
	void* sendPlayerPositions(void* arg) {
#elif _WIN32
	DWORD WINAPI sendPlayerPositions(void* arg) {
#endif
	int id = *(int*)arg;

	while (1) {
		for (int i = 0; i < MAX; ++i) {
			if (players[i].sock && i != id && players[id].currentWorldId == players[i].currentWorldId) {
				send(players[i].sock, (char*)&players[id].server0x08, sizeof(players[id].server0x08), 0);
			}
		}

		usleep(100000);
	}
}

#ifdef __linux__
	void* handle(void* arg) {
#elif _WIN32
	DWORD WINAPI handle(void* arg) {
#endif
	int id = *(int*)arg;

	players[id].playerId 	   = id;
	players[id].currentWorldId = 0;
	memset(players[id].username, 0, 64);

	players[id].server0x08.packetId = 0x08;
	players[id].server0x08.playerId = id;
	players[id].server0x08.x 		= 0;
	players[id].server0x08.y 		= 0;
	players[id].server0x08.z 		= 0;
	players[id].server0x08.pitch	= 0;
	players[id].server0x08.yaw 		= 0;

	#ifdef __linux__
		{
			pthread_t t;
			pthread_create(&t, NULL, sendPlayerPositions, arg);
		}
	#elif _WIN32
		{
			HANDLE t = CreateThread(NULL, 0, sendPlayerPositions, arg, 0, NULL);
		}
	#endif

	// Create client structures.
	struct client0x00_t client0x00;
	struct client0x05_t client0x05;
	struct client0x08_t client0x08;
	struct client0x0d_t client0x0d;
	struct client0x10_t client0x10;
	struct client0x11_t client0x11;
	client0x00.protocolVersion = 0x00;

	// Create server structures. 
	// Only ones being used in this function are created.
	struct server0x00_t server0x00;
	struct server0x06_t server0x06;
	struct server0x0c_t server0x0c;
	struct server0x0f_t server0x0f;
	struct server0x10_t server0x10;
	struct server0x11_t server0x11;
	struct server0x13_t server0x13;
	server0x00.packetId = 0x00;
	server0x06.packetId = 0x06;
	server0x0c.packetId = 0x0c;
	server0x0f.packetId = 0x0f;
	server0x10.packetId = 0x10;
	server0x11.packetId = 0x11;
	server0x13.packetId = 0x13;

	int initialise = 1;

	while (players[id].sock) {
		uint8_t packetId;
		if (recv(players[id].sock, (char*)&packetId, 1, MSG_WAITALL) <= 0) {
			players[id].sock = 0;
			server0x0c.playerId = id;

			char buffer[64];

			if (players[id].username[0]) {
				strcpy(buffer, "&e");
				strcat(buffer, unpad(players[id].username));
				strcat(buffer, " left the server.");

				for (int i = 0; i < MAX; ++i)
					if (players[i].sock) {
						sendMessage(0, i, buffer);
						send(players[i].sock, (char*)&server0x0c, sizeof(server0x0c), 0);
					}
			}

			printMessage(buffer);
			
			memset(players[id].username, 0, 64);
			pthread_exit(NULL);
		}

		if (initialise) {
			if (packetId != 0x00) {
				if (players[id].sock)
				send(players[id].sock, HTTP, strlen(HTTP), 0);
				int tmpCount = 0;
				for (int i = 0; i < MAX; ++i) {
					if (players[i].sock && i != id) {
						++tmpCount;
						if (players[id].sock) {
							send(players[id].sock, unpad(players[i].username), strlen(unpad(players[i].username)), 0);
							send(players[id].sock, "\n", 1, 0);
						}
					}
				}

				if (!tmpCount) {
					if (players[id].sock)
					send(players[id].sock, "No one is currently online!", 27, 0);
				}

				if (players[id].sock)
				close(players[id].sock);
				players[id].sock = 0;
				pthread_exit(NULL);
			}
			initialise = 0;
		}

		top:
		switch (packetId) {
			case 0x00:
				if (client0x00.protocolVersion)
					break;

				if (recv(players[id].sock, (char*)&client0x00, sizeof(client0x00), MSG_WAITALL) <= 0)
					break;
				
				unpad(client0x00.mpass);

				#ifdef __linux__
					{
						pthread_t t;
						pthread_create(&t, NULL, ping, (void*)&id);
					}
				#elif _WIN32
					{
						HANDLE t = CreateThread(NULL, 0, ping, (void*)&players[id].sock, 0, NULL);
					}
				#endif

				if (!server_info.cracked) {
					char tempUsername[sizeof(client0x00.username)];
					memcpy(tempUsername, client0x00.username, sizeof(client0x00.username));
					unpad(tempUsername);

					char saltAndName[strlen(server_info.salt) + strlen(tempUsername) + 1];
					strcpy(saltAndName, server_info.salt);
					strcat(saltAndName, tempUsername);

					// Following md5 code has been replaced.
					// Originally the deprecated version, but
					// I didn't know the new one, so I used:
					// https://stackoverflow.com/a/71643121

					EVP_MD_CTX* ctx;
					unsigned char* md5_digest;
					unsigned int   md5_digest_len = EVP_MD_size(EVP_md5());

					ctx = EVP_MD_CTX_new();
					EVP_DigestInit_ex(ctx, EVP_md5(), NULL);

					EVP_DigestUpdate(ctx, saltAndName, strlen(saltAndName));

					md5_digest = (unsigned char*)OPENSSL_malloc(md5_digest_len);
					EVP_DigestFinal_ex(ctx, md5_digest, &md5_digest_len);
					EVP_MD_CTX_free(ctx);

					char mdString[md5_digest_len * 2 + 1];
					for (int i = 0; i < md5_digest_len; i++) {
						sprintf(&mdString[i * 2], "%02x", md5_digest[i]);
					}

					mdString[md5_digest_len * 2] = 0;

					if (strcmp(mdString, client0x00.mpass) != 0) {
						disconnectPlayer(id, "Invalid player key, forged username.");
						pthread_exit(NULL);
					}
				}

				if (client0x00.protocolVersion != 0x07) {
					disconnectPlayer(id, "Invalid protocol version.");
					printf("%d, %c\n", client0x00.protocolVersion, client0x00.protocolVersion);
					pthread_exit(NULL);
				}

				if (client0x00.unused == 0x00) {
					disconnectPlayer(id, "Please rejoin with an enhanced classic client.");
					pthread_exit(NULL);
				}

				if (strlen(unpad(client0x00.username)) > 16) {
					disconnectPlayer(id, "Username exceeds 16 characters!");
					pthread_exit(NULL);
				}

				for (int i = 0; i < MAX; ++i) {
					if (strcmp(players[i].username, client0x00.username) == 0) {
						disconnectPlayer(id, "Player with same username already in server.");
						pthread_exit(NULL);
					}
				}

				memcpy(players[id].username, client0x00.username, 64);
				unpad(players[id].username);

				if (checkBanned(id)) {
					pthread_exit(NULL);
				}

				server0x00.protocolVersion = 0x07;
				padcpy(server0x00.serverName, server_info.server_name);
				padcpy(server0x00.serverMotd, server_info.server_motd);
				server0x00.userType = 0x00;
				
				if (players[id].sock)
					send(players[id].sock, (char*)&server0x00, sizeof(server0x00), 0);

				if (sendworld(players[id].currentWorldId, id))
					pthread_exit(NULL);

				{ // Sending join message scope.
					char buffer[64];
					strcpy(buffer, "&e");
					strcat(buffer, players[id].username);
					strcat(buffer, " joined the server.");

					for (int i = 0; i < MAX; ++i)
						if (players[i].sock)
							sendMessage(0, i, buffer);

					printMessage(buffer);
				}

				if (isOp(players[id].username)) {
					server0x0f.userType = 0x64;
					if (players[id].sock)
						send(players[id].sock, (char*)&server0x0f, sizeof(server0x0f), 0);
					
					players[id].opStatus = server0x0f.userType;

					sendMessage(0, id, "&cWelcome, operator!");
				} else {
					server0x0f.userType = 0x00;

					if (players[id].sock)
						send(players[id].sock, (char*)&server0x0f, sizeof(server0x0f), 0);

					players[id].opStatus = server0x0f.userType;
				}

				// CPE!!!
				server0x10.extensionCount = htons(1);
				char combinedSoftwareName[64];
				strcpy(combinedSoftwareName, "&6");
				strcat(combinedSoftwareName, SOFTWARE_NAME);
				strcat(combinedSoftwareName, " &f");
				strcat(combinedSoftwareName, SOFTWARE_VERSION);
				padcpy(server0x10.appName, combinedSoftwareName);
				if (players[id].sock)
					send(players[id].sock, (char*)&server0x10, sizeof(server0x10), 0);

				padcpy(server0x11.extName, "CustomBlocks");
				server0x11.version = htonl(1);
				if (players[id].sock)
					send(players[id].sock, (char*)&server0x11, sizeof(server0x11), 0);

				padcpy(server0x11.extName, "MessageTypes");
				server0x11.version = htonl(1);
				if (players[id].sock)
					send(players[id].sock, (char*)&server0x11, sizeof(server0x11), 0);

				uint8_t tempRecv;
				int exit = 0;
				while (recv(players[id].sock, (char*)&tempRecv, 1, MSG_WAITALL)) {
					switch (tempRecv) {
						case 0x10:
							recv(players[id].sock, (char*)&client0x10, sizeof(client0x10), MSG_WAITALL);
							break;
						case 0x11:
							recv(players[id].sock, (char*)&client0x11, sizeof(client0x11), MSG_WAITALL);
							break;
						default:
							server0x13.supportLevel = 1;

							if (players[id].sock)
							send(players[id].sock, (char*)&server0x13, sizeof(server0x13), 0);

							packetId = tempRecv;
							goto top;
							break;
					}
				}

				break;
			case 0x01:
				{
					uint8_t packet0x01;
					packet0x01 = 0x01;

					if (players[id].sock)
						send(players[id].sock, &packet0x01, 1, 0);
					
					break;
				}
			case 0x05:
				if (recv(players[id].sock, (char*)&client0x05, sizeof(client0x05), MSG_WAITALL) <= 0)
					break;
				
				server0x06.blockType = 0;

				if (client0x05.mode || players[id].paint)
					server0x06.blockType = client0x05.blockType;
				
				server0x06.x = client0x05.x;
				server0x06.y = client0x05.y;
				server0x06.z = client0x05.z;

				for (int i = 0; i < MAX_BLOCK_UPDATES; ++i) {
					if (i == MAX_BLOCK_UPDATES - 1)
						if (worlds[players[id].currentWorldId].blockChanges[i].packetId != 0) {
							for (int j = 0; j < MAX; ++j) {
								if (worlds[j].name) {
									saveworld(j, 0);
									memset(&worlds[players[id].currentWorldId].blockChanges, 0, sizeof(worlds[players[id].currentWorldId].blockChanges));
									printf("World %s has reached its max blocks. Resetting.\n", worlds[players[id].currentWorldId].name);
								}
							}
						}
				}

				for (int i = 0; i < MAX_BLOCK_UPDATES; ++i)
					if (worlds[players[id].currentWorldId].blockChanges[i].packetId == 0) {
						worlds[players[id].currentWorldId].blockChanges[i] = server0x06;
						break;
					}
				
				for (int i = 0; i < MAX; ++i)
					if (players[i].sock && players[id].currentWorldId == players[i].currentWorldId) {
						if  (players[i].sock)
						send(players[i].sock, (char*)&server0x06, sizeof(server0x06), 0);
					}

				break;
			case 0x08:
				if (recv(players[id].sock, (char*)&client0x08, sizeof(client0x08), MSG_WAITALL) <= 0)
					break;

				players[id].server0x08.playerId = id;
				players[id].server0x08.x	    = client0x08.x;
				players[id].server0x08.y 		= client0x08.y;
				players[id].server0x08.z 		= client0x08.z;
				players[id].server0x08.yaw 		= client0x08.yaw;
				players[id].server0x08.pitch 	= client0x08.pitch;
				
				break;
			case 0x0d:
				if (recv(players[id].sock, (char*)&client0x0d, sizeof(client0x0d), MSG_WAITALL) <= 0)
					break;
				
				char stringSpaceCut[128];
				memset(stringSpaceCut, 0, sizeof(stringSpaceCut));
			

				for (int i = 0; i < 64; ++i)
					if (client0x0d.message[i] == '%')
						client0x0d.message[i] =  '&';

				if (client0x0d.message[63] == '&')
					client0x0d.message[63] = ' ';
				
				int j = 0;
				for (int i = 0; i < 64; ++i) {
					if (client0x0d.message[i] != 32 && j + 1 < sizeof(stringSpaceCut))
						stringSpaceCut[j++] = client0x0d.message[i];
					else {
						if (j < (sizeof(stringSpaceCut) - 1))
						stringSpaceCut[j] = 32;
						if (i + 1 < 64 && client0x0d.message[i + 1] != 32)
							++j;
					}
				}

				if (client0x0d.message[0] == '/') { // It means it's a command
					char  arguments[64][64];
					char* token;
					memset(arguments, 0, sizeof(arguments));

					token = strtok(client0x0d.message, " ");
					int i = 0;
					do {
						if (!arguments[i][0]) {
							strcpy(arguments[i], token);
						}
					} while (++i, token = strtok(NULL, " "));

					// Run command
					invokecommand(afterchar(arguments[0], '/'), &arguments, &players[id]);
				} else {
					char* buffer = (char*)malloc((strlen(players[id].username) + 
												  strlen(stringSpaceCut)   	   + 64) 
												  * sizeof(char));
					strcpy(buffer, "&f");
					strcat(buffer, unpad(players[id].username));
					strcat(buffer, "&f: ");
					strcat(buffer, stringSpaceCut);

					printMessage(buffer);

					for (int i = 0; i < MAX; ++i)
						if (players[i].sock)
							sendMessage(0, i, buffer);
					
					free(buffer);
				}

				break;
		}
	}
}

int main(void) {
	unsigned int size;
	struct sockaddr_in server;
	int sock;

	if (lexerMain()) {
		printf("%sThere is an error in the server.properties file!\n", AND_C);
		return 1;
	}

	for (int i = 0; i < strlen(server_info.worlds_folder); ++i) {
		if (!isalnum(server_info.worlds_folder[i])) {
			printf("%sError: worlds folder must only contain alphanumeric characters.\n", AND_C);
			return 1;
		} 
	}

	for (int i = 0; i < strlen(server_info.main_world); ++i) {
		if (!isalnum(server_info.main_world[i])) {
			printf("%sError: worlds folder must only contain alphanumeric characters.\n", AND_C);
			return 1;
		} 
	}

	getcwd(cwd, sizeof(cwd));

	signal(SIGINT, onExit);

	memset(worlds, 		 0, sizeof(worlds));
	memset(&commands, 	 0, sizeof(commands));

	loadcommands();

	if (chdir(server_info.worlds_folder) != 0) {
		#ifdef __linux__
			mkdir(server_info.worlds_folder, 0700);
		#elif _WIN32
			mkdir(server_info.worlds_folder);
		#endif

		chdir(server_info.worlds_folder);
	}

	FILE* file = fopen(server_info.main_world, "rb");
	if (file == NULL)
		buildworld(0, server_info.main_world, 128, 128, 128);
	else
		loadworld(0, server_info.main_world);
	
	generatesalt();

	// Reset all usernames to space.
	for (int i = 0; i < MAX; ++i)
		memset(players[i].username, 32, 64);
	
	// Reset all world info

	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family 	   = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port 	   = htons(server_info.port);
	size = sizeof(server);

	if (bind(sock, (struct sockaddr*)&server, size) == -1) {
		printf("%sCouldn't bind.\n", AND_C);
		return 1;
	}

	if (listen(sock, 3) == -1) {
		printf("%sCouldn't listen.\n", AND_C);
		return 1;
	}

	printf("%sReady, using port %d.\n", AND_3, server_info.port);
	resetColour();

	#ifdef __linux__
		if (server_info.public) {
			pthread_t t;
			pthread_create(&t, NULL, heartbeat, NULL);
		}

		{
			pthread_t t;
			pthread_create(&t, NULL, autosaver, NULL);
		}
	#elif _WIN32
		if (server_info.public) {
			HANDLE t = CreateThread(NULL, 0, heartbeat, NULL, 0, NULL);
		}

		{
			HANDLE t = CreateThread(NULL, 0, autosaver, NULL, 0, NULL);
		}
	#endif

	while (1) {
		int cxn = accept(sock, (struct sockaddr*)&server, &size);
		if (cxn == -1) {
			printf("%sCouldn't accept!\n", AND_C);
			continue;
		}

		for (int i = 0; i < MAX; ++i) {
			if (players[i].sock == 0) {
				#ifdef __linux__
					pthread_t t;
					players[i].sock = cxn;
					pthread_create(&t, NULL, handle, (void*)&i);
				#elif _WIN32
					players[i].sock = cxn;
					HANDLE t = CreateThread(NULL, 0, handle, (void*)&i, 0, NULL);
				#endif
				break;
			}
		}
	}

	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions.h"
#include "definitions.h"
#include "structs.h"
#include "worlds.h"
#include "socket.h"
#include "unistd.h"

// This entire file is a disgrace and i hate looking through it
// If you are not me and you are reading this, i am so so sorry
// I won't be paying for your therapy tho that's on you

// The following is the definition of "If it works, don't change it"

int generatesalt() {
	time_t t;
	srand(time(&t) * 23);
	for (int i = 0; i < 16; ++i)
		server_info.salt[i] = ALPHABET[rand() % strlen(ALPHABET)];
	server_info.salt[16] = 0;
	return 0;
}

int getBlock(int id, int16_t x, int16_t y, int16_t z) {
	return (y * worlds[id].zSize + z) * worlds[id].xSize + x;
}

char* padcpy(char* dest, const char* src) {
	memset(dest, 32, 64);

	for (int i = 0; i < strlen(src); ++i)
		if (i < 64)
			dest[i] = src[i];

	return dest;
}

char* unpad(char* src) {
	for (int i = 63; i > 0; --i) {
		if (src[i] == 32)
			src[i] = 0;
		else
			break;
	}
	return src;
}

void onExit(int test) {
	printf("\n%sServer closing...\n", AND_C);
	resetColour();
	
	struct server0x0e_t server0x0e;
	server0x0e.packetId = 0x0e;

	for (int i = 0; i < MAX; ++i) {
		if (players[i].sock) {
			disconnectPlayer(i, "Server closed.");
		}
	}

	for (int i = 0; i < MAX; ++i) {
		if (worlds[i].name) {
			if (saveworld(i, 0) == 1)
				while (saveworld(i, 1));
			free(worlds[i].name);
		}
	}

	printf("%sServer closed.\n", AND_C);
	resetColour();

	exit(0);
}

int sendMessage(int type, int id, const char* message) {
	struct server0x0d_t server0x0d;
	server0x0d.packetId = 0x0d;
	server0x0d.playerId = type;

	memset(server0x0d.message, 32, sizeof(server0x0d.message));
	for (int i = 0; i < strlen(message); ++i) {
		if ((i > 0 && i % 64 == 0) || i == strlen(message) - 1) {
			if (i == strlen(message) - 1)
				if ((strlen(message) - 1) % 64 != 0)
					server0x0d.message[(strlen(message) - 1) % 64] = message[i];
			
			if (server0x0d.message[0] == 32) {
				for (int i = 0; i < 63; ++i)
					server0x0d.message[i] = server0x0d.message[i + 1];
				server0x0d.message[63] = 32;
			}

			if (players[id].sock)
			send(players[id].sock, (char*)&server0x0d, sizeof(server0x0d), 0);
			memset(server0x0d.message, 32, sizeof(server0x0d.message));
		}
		server0x0d.message[i % 64] = message[i];
	}

	return 0;
}

void printMessage(const char* message) {
	printf("\033[0m");
	for (int i = 0; i < strlen(message); ++i) {
		if (message[i] == '&') {
			++i;
			const char* colourStrings[] = {
				AND_0, AND_1, AND_2, AND_3, AND_4, AND_5, AND_6, AND_7, AND_8, AND_9,
				AND_A, AND_B, AND_C, AND_D, AND_E, AND_F
			};

			const char colourCode[] = {message[i], 0};

			printf("%s", colourStrings[strtol(colourCode, NULL, 16)]);
		} else
			putchar(message[i]);
	}
	printf("\033[0m");
	putchar(10);
}

int disconnectPlayer(int id, const char* message) {
	struct server0x0e_t server0x0e;
	struct server0x0c_t server0x0c;
	server0x0e.packetId = 0x0e;
	server0x0c.packetId = 0x0c;
	server0x0c.playerId = id;

	if (players[id].username[0]) {
		char buffer[64];
		strcpy(buffer, "&e");
		strcat(buffer, players[id].username);
		strcat(buffer, " left the server.");
		
		for (int i = 0; i < MAX; ++i)
			if (players[i].sock && i != id) {
				sendMessage(0, i, buffer);
				send(players[i].sock, (char*)&server0x0c, sizeof(server0x0c), 0);
			}
	
		printMessage(buffer);
		printf("Closed %s's connection.\n", players[id].username);
	}

	memset(server0x0e.disconnectReason, 32, sizeof(server0x0e.disconnectReason));
	padcpy(server0x0e.disconnectReason, message);
	if (players[id].sock)
	send(players[id].sock, (char*)&server0x0e, sizeof(server0x0e), 0);

	close(players[id].sock);
	players[id].sock = 0;

	memset(players[id].username, 0, 64);

	return 0;
}

char* afterchar(char* src, char character) { /* Used once lol */
	for (int j = 0; j < strlen(src); ++j)
		if (src[j] == character)
			return &src[j + 1];
	return src;
}

int isOp(const char* username) {
	char oplist_dir[strlen(cwd) + strlen(OPLIST_FILE) + 2];
	strcpy(oplist_dir, cwd);
	strcat(oplist_dir, "/");
	strcat(oplist_dir, OPLIST_FILE);
	FILE* file = fopen(oplist_dir, "rb");

	if (file == NULL) {
		printf("ERROR!!! Oplist file was not found!\n");
		return -1;
	}

	char buffer[64];
	while (fgets(buffer, sizeof(buffer), file)) {
		for (int i = 0; i < strlen(buffer); ++i)
			if (buffer[i] == '\n')
				buffer[i] = 0;
		if (strcmp(username, buffer) == 0) {
			return 1;
		}
	}

	fclose(file);
	return 0;
}

int makeOp(int id, const char* username) {
	if (!isOp(players[id].username)) {
		return 1;
	}

	char oplist_dir[strlen(cwd) + strlen(OPLIST_FILE) + 2];
	strcpy(oplist_dir, cwd);
	strcat(oplist_dir, "/");
	strcat(oplist_dir, OPLIST_FILE);
	FILE* file = fopen(oplist_dir, "rb");

	if (file == NULL) {
		printf("ERROR!!! Oplist file was not found!\n");
		return 1;
	}

	fseek(file, 0, SEEK_END);
	int  length = ftell(file);
	char buffer[length + 1];
	rewind(file);
	fread(buffer, length, 1, file);
	buffer[length] = 0;
	rewind(file);

	char line[64];
	while (fgets(line, sizeof(line), file)) {
		for (int i = 0; i < strlen(line); ++i)
			if (line[i] == 10)
				line[i] = 0;
		if (strcmp(line, username) == 0) {
			if (players[id].sock)
			sendMessage(0, id, "User is already opped!");
			return 1;
		}
	}

	fclose(file);

	file = fopen(oplist_dir, "a");

	if (file == NULL) {
		printf("%sError: oplist file was not found.\n", AND_C);
		return 1;
	}

	if (buffer[length - 1] == 10)
		fprintf(file, "%s", username);
	else
		fprintf(file, "\n%s", username);

	for (int i = 0; i < MAX; ++i)
		if (strcmp(players[i].username, username) == 0) {
			players[i].opStatus = 0x64;
			struct server0x0f_t server0x0f;
			server0x0f.packetId = 0x0f;
			server0x0f.userType = 0x64;
			if  (players[i].sock)
			send(players[i].sock, (char*)&server0x0f, sizeof(server0x0f), 0);
			sendMessage(0, i, "&aYou have been opped!");
		}

	fclose(file);
	return 0;
}

int deOp(int id, const char* username) {
	if (!isOp(players[id].username)) {
		return 1;
	}

	char oplist_dir[strlen(cwd) + strlen(OPLIST_FILE) + 2];
	strcpy(oplist_dir, cwd);
	strcat(oplist_dir, "/");
	strcat(oplist_dir, OPLIST_FILE);
	FILE* file = fopen(oplist_dir, "rb");

	if (file == NULL) {
		printf("%sError: oplist file was not found.\n", AND_C);
		return 1;
	}

	fseek(file, 0, SEEK_END);
	int  length = ftell(file);
	char buffer[length + 1];
	rewind(file);
	strcpy(buffer, "");
	
	char line[64];
	int  wasopped = 0;
	while (fgets(line, sizeof(line), file)) {
		for (int i = 0; i < strlen(line); ++i)
			if (line[i] == 10)
				line[i] = 0;

		if (strcmp(line, username) != 0) {
			strcat(buffer, line);
			strcat(buffer, "\n");
		} else
			wasopped = 1;
	}

	if (!wasopped) {
		sendMessage(0, id, "Player is not opped.");
		return 1;
	}

	fclose(file);

	file = fopen(oplist_dir, "wb");
	fwrite(buffer, strlen(buffer), 1, file);
	fclose(file);

	for (int i = 0; i < MAX; ++i)
		if (strcmp(players[i].username, username) == 0) {
			players[i].opStatus = 0x00;
			struct server0x0f_t server0x0f;
			server0x0f.packetId = 0x0f;
			server0x0f.userType = 0x00;
			if  (players[i].sock)
			send(players[i].sock, (char*)&server0x0f, sizeof(server0x0f), 0);
			sendMessage(0, i, "&cYou have been deopped!");
		}
	
	return 0;
}

int opList(int id) {
	char oplist_dir[strlen(cwd) + strlen(OPLIST_FILE) + 2];
	strcpy(oplist_dir, cwd);
	strcat(oplist_dir, "/");
	strcat(oplist_dir, OPLIST_FILE);
	FILE* file = fopen(oplist_dir, "rb");

	if (file == NULL) {
		printf("%sError: oplist file was not found.\n", AND_C);
		return 1;
	}

	sendMessage(0, id, "&6List of opped players:");

	char buffer[64];
	while (fgets(buffer, sizeof(buffer), file)) {
		for (int i = 0; i < strlen(buffer); ++i)
			if (buffer[i] == 10)
				buffer[i] = 0;
		sendMessage(0, id, buffer);		
	}

	return 0;
}

int banPlayer(int id, const char* username) {
	if (!isOp(players[id].username)) {
		return 1;
	}

	char banlist_dir[strlen(cwd) + strlen(BANLIST_FILE) + 2];
	strcpy(banlist_dir, cwd);
	strcat(banlist_dir, "/");
	strcat(banlist_dir, BANLIST_FILE);
	FILE* file = fopen(banlist_dir, "rb");

	if (file == NULL) {
		printf("ERROR!!! banlist file was not found!\n");
		return 1;
	}

	fseek(file, 0, SEEK_END);
	int  length = ftell(file);
	char buffer[length + 1];
	rewind(file);
	fread(buffer, length, 1, file);
	buffer[length] = 0;
	rewind(file);

	char line[64];
	while (fgets(line, sizeof(line), file)) {
		for (int i = 0; i < strlen(line); ++i)
			if (line[i] == 10)
				line[i] = 0;
		if (strcmp(line, username) == 0) {
			sendMessage(0, id, "User is already banned!");
			return 1;
		}
	}

	fclose(file);

	file = fopen(banlist_dir, "a");

	if (file == NULL) {
		printf("%sError: banlist file was not found.\n", AND_C);
		return 1;
	}

	if (buffer[length - 1] == 10)
		fprintf(file, "%s", username);
	else
		fprintf(file, "\n%s", username);

	for (int i = 0; i < MAX; ++i)
		if (strcmp(players[i].username, username) == 0) {
			disconnectPlayer(i, "You have been banned!");
		}

	fclose(file);
	return 0;
}

int unbanPlayer(int id, const char* username) {
	if (!isOp(players[id].username)) {
		return 1;
	}

	char banlist_dir[strlen(cwd) + strlen(BANLIST_FILE) + 2];
	strcpy(banlist_dir, cwd);
	strcat(banlist_dir, "/");
	strcat(banlist_dir, BANLIST_FILE);
	FILE* file = fopen(banlist_dir, "rb");

	if (file == NULL) {
		printf("%sError: banlist file was not found.\n", AND_C);
		return 1;
	}

	fseek(file, 0, SEEK_END);
	int  length = ftell(file);
	char buffer[length + 1];
	rewind(file);
	strcpy(buffer, "");
	
	char line[64];
	int  wasopped = 0;
	while (fgets(line, sizeof(line), file)) {
		for (int i = 0; i < strlen(line); ++i)
			if (line[i] == 10)
				line[i] = 0;

		if (strcmp(line, username) != 0) {
			strcat(buffer, line);
			strcat(buffer, "\n");
		} else
			wasopped = 1;
	}

	if (!wasopped) {
		sendMessage(0, id, "Player is not banned.");
		return 1;
	}

	fclose(file);

	file = fopen(banlist_dir, "wb");
	fwrite(buffer, strlen(buffer), 1, file);
	fclose(file);

	return 0;
}

int banList(int id) {
	char banlist_dir[strlen(cwd) + strlen(BANLIST_FILE) + 2];
	strcpy(banlist_dir, cwd);
	strcat(banlist_dir, "/");
	strcat(banlist_dir, BANLIST_FILE);
	FILE* file = fopen(banlist_dir, "rb");

	if (file == NULL) {
		printf("%sError: banlist file was not found.\n", AND_C);
		return 1;
	}

	sendMessage(0, id, "&6List of banned players:");

	char buffer[64];
	while (fgets(buffer, sizeof(buffer), file)) {
		for (int i = 0; i < strlen(buffer); ++i)
			if (buffer[i] == 10)
				buffer[i] = 0;
		sendMessage(0, id, buffer);		
	}

	return 0;
}

int checkBanned(int id) {
	char banlist_dir[strlen(cwd) + strlen(BANLIST_FILE) + 2];
	strcpy(banlist_dir, cwd);
	strcat(banlist_dir, "/");
	strcat(banlist_dir, BANLIST_FILE);
	FILE* file = fopen(banlist_dir, "rb");

	if (file == NULL) {
		printf("%sError: banlist file not found. Creating it instead.\n", AND_C);
		file = fopen("banlist", "w");
		fclose(file);
		return 0;
	}

	char buffer[64];
	while (fgets(buffer, sizeof(buffer), file)) {
		for (int i = 0; i < strlen(buffer); ++i)
			if (buffer[i] == 10)
				buffer[i] = 0;
		if (strcmp(players[id].username, buffer) == 0) {
			disconnectPlayer(id, "You have been banned!");
			return 1;
		}
	}

	return 0;
}

void resetColour() {
	printf(AND_F);
}
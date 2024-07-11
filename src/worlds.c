#include <stdio.h>
#include <string.h>
#include <math.h>
#include <zlib.h>
#include <stdlib.h>
#include <ctype.h>
#include <netinet/in.h>
#include "definitions.h"
#include "structs.h"
#include "functions.h"

int buildworld(int id, const char* name, int16_t xSize, int16_t ySize, int16_t zSize) {
	for (int i = 0; i < strlen(name); ++i) {
		if (!isdigit(name[i]) && !isalpha(name[i]))
			return 1;
	}

	worlds[id].name = (char*)malloc(((strlen(name)) * sizeof(char)) + 1);
	strcpy(worlds[id].name, name);
	worlds[id].xSize  = xSize;
	worlds[id].ySize  = ySize;
	worlds[id].zSize  = zSize;
	worlds[id].saving = 0;
	memset(worlds[id].blockChanges, 0, sizeof(worlds[id].blockChanges));

	for (int i = 0; i < MAX; ++i) {
		if (worlds[i].name && id != i)
			if (strcmp(worlds[id].name, worlds[i].name) == 0) {
				free(worlds[id].name);
				worlds[id].name = NULL;
				return 1;
			}
	}

	int32_t  volume    = (worlds[id].xSize) * (worlds[id].ySize) * (worlds[id].zSize);
	uint8_t* bytearray = (uint8_t*)malloc(volume * sizeof(uint8_t));
	memset(bytearray, 0, volume * sizeof(uint8_t));

	printf("%sWorld [%s]: Building dirt...\n", AND_6, name);

	for (int i = 0; i < ((worlds[id].xSize) * (worlds[id].zSize) * (((worlds[id].ySize) / 2) - 1)); ++i) {
		if (i >= 0)
			bytearray[i] = 3;
	}

	printf("%sWorld [%s]: Building grass...\n", AND_6, name);

	for (int i = ((worlds[id].xSize) * (worlds[id].zSize) * (((worlds[id].ySize) / 2) - 1)); i < ((worlds[id].xSize) * (worlds[id].zSize) * ((worlds[id].ySize) / 2)); ++i) {
		if (i >= 0)
			bytearray[i] = 2;
	}

	volume = htonl(volume);

	printf("%sWorld [%s]: Compressing...\n", AND_6, name);

	gzFile file = gzopen(worlds[id].name, "w");
	gzfwrite(&volume, 	sizeof(volume),    		  		 1, file);
	gzfwrite(bytearray, ntohl(volume) * sizeof(uint8_t), 1, file);
	gzclose(file);

	printf("%sWorld [%s]: Finalising...\n", AND_6, name);

	FILE* plainFile = fopen(worlds[id].name, "a");
	if (plainFile == NULL) {
		printf("%sWorld [%s]: Could not append world dimensions to end of file. Loading the world may no longer work.", AND_C, name);
		return 1;
	}
	
	fwrite(&worlds[id].xSize, sizeof(worlds[id].xSize), 1, plainFile);
	fwrite(&worlds[id].ySize, sizeof(worlds[id].ySize), 1, plainFile);
	fwrite(&worlds[id].zSize, sizeof(worlds[id].zSize), 1, plainFile);
	fclose(plainFile);

	free(bytearray);

	printf("%sWorld [%s]: Done!\n", AND_6, name);
	resetColour();

	return 0;
}

int loadworld(int16_t id, const char* name) {
	for (int i = 0; i < strlen(name); ++i) {
		if (!isdigit(name[i]) && !isalpha(name[i]))
			return 1;
	}

	worlds[id].name = (char*)malloc((strlen(name) * sizeof(char)) + 1);
	strcpy(worlds[id].name, name);
	worlds[id].saving = 0;
	memset(worlds[id].blockChanges, 0, sizeof(worlds[id].blockChanges));

	for (int i = 0; i < MAX; ++i) {
		if (worlds[i].name && id != i)
			if (strcmp(worlds[id].name, worlds[i].name) == 0) {
				free(worlds[id].name);
				worlds[id].name = NULL;
				return 1;
			}
	}

	FILE* file = fopen(worlds[id].name, "rb");
	if (file == NULL) {
		free(worlds[id].name);
		worlds[id].name = NULL;
		return 1;
	}

	fseek(file, 0, SEEK_END);
	int 	length = ftell(file);
	uint8_t bytearray[length];
	rewind(file);
	fread(bytearray, length, 1, file);
	fclose(file);
	
	worlds[id].xSize = 256 * bytearray[length - 5] + bytearray[length - 6];
	worlds[id].ySize = 256 * bytearray[length - 3] + bytearray[length - 4];
	worlds[id].zSize = 256 * bytearray[length - 1] + bytearray[length - 2];

	players[id].currentWorldId = 1;
	return 0;
}

int saveworld(int16_t id, int waiting) {
	if (worlds[id].saving == 1)
		return 1;

	if (waiting)
		return 0;

	printf("%sWorld [%s]: Saving world...\n", AND_6, worlds[id].name);

	worlds[id].saving = 1;

	gzFile file = gzopen(worlds[id].name, "r");
	if (file == 0) {
		printf("%sWorld [%s]: Error opening world file! Saving aborted.\n", AND_C, worlds[id].name);
		resetColour();
		worlds[id].saving = 0;
		return 1;
	}

	int32_t volume;
	if (gzread(file, &volume, 4) <= 0) {
		printf("World [%s]: World file possibly corrupted. Saving aborted.\n", AND_C, worlds[id].name);
		resetColour();
		worlds[id].saving = 0;
		return 1;
	}
	volume = ntohl(volume);
	uint8_t* bytearray = (uint8_t*)malloc(volume * sizeof(uint8_t));
	if (gzread(file, bytearray, volume) <= 0) {
		free(bytearray);
		printf("%sWorld [%s]: World file possibly corrupted. Saving aborted.\n", AND_C, worlds[id].name);
		resetColour();
		worlds[id].saving = 0;
		return 1;
	}

	gzclose(file);

	for (int i = 0; i < MAX_BLOCK_UPDATES; ++i)
		if (worlds[id].blockChanges[i].packetId == 0x06) {
				int tx = ntohs(worlds[id].blockChanges[i].x);
				int ty = ntohs(worlds[id].blockChanges[i].y);
				int tz = ntohs(worlds[id].blockChanges[i].z);
				bytearray[getBlock(id, tx, ty, tz)] = worlds[id].blockChanges[i].blockType;
		}

	volume = htonl(volume);

	printf("%sWorld [%s]: Compressing...\n", AND_6, worlds[id].name);

	file = gzopen(worlds[id].name, "w");
	if (file == 0) {
		printf("%sWorld [%s]: Error opening world file! Saving aborted.\n", AND_C, worlds[id].name);
		resetColour();
		worlds[id].saving = 0;
		return 1;
	}

	gzfwrite(&volume, 	sizeof(volume), 1, file);
	gzfwrite(bytearray,  ntohl(volume), 1, file);
	gzclose(file);

	printf("%sWorld [%s]: Finalising...\n", AND_6, worlds[id].name);

	FILE* plainFile = fopen(worlds[id].name, "a");
	if (plainFile == NULL) {
		printf("%sWorld [%s]: Could not append world dimensions to end of file. Loading the world may no longer work.\n", AND_C, worlds[id].name);
		resetColour();
		worlds[id].saving = 0;
		return 1;
	}

	fwrite(&worlds[id].xSize, sizeof(worlds[id].xSize), 1, plainFile);
	fwrite(&worlds[id].ySize, sizeof(worlds[id].ySize), 1, plainFile);
	fwrite(&worlds[id].zSize, sizeof(worlds[id].zSize), 1, plainFile);
	fclose(plainFile);

	printf("%sWorld [%s]: Saving complete.\n", AND_2, worlds[id].name);
	resetColour();

	free(bytearray);

	worlds[id].saving = 0;
	return 0;
}

int sendworld(int16_t newWorldId, int16_t playerId) {
	struct server0x02_t server0x02;
	struct server0x03_t server0x03;
	struct server0x04_t server0x04;
	struct server0x07_t server0x07;
	struct server0x0e_t server0x0e;
	server0x02.packetId = 0x02;
	server0x03.packetId = 0x03;
	server0x04.packetId = 0x04;
	server0x07.packetId = 0x07;
	server0x0e.packetId = 0x0e;

	send(players[playerId].sock, &server0x02, sizeof(server0x02), 0);

	// world stuff here!
	{
		FILE* file = fopen(worlds[players[playerId].currentWorldId].name, "rb");
		if (file == NULL) {
			printf("%sError! World could not be loaded. %s\n", AND_C, worlds[players[playerId].currentWorldId].name);
			disconnectPlayer(playerId, "World could not be loaded. Something is broken!");
			return 1;
		}

		fseek(file, 0, SEEK_END);
		int 	length = ftell(file);
		uint8_t buffer[length];
		rewind(file);
		fread(buffer, length, 1, file);

		memset(server0x03.chunkData, 0, CHUNK_SIZE);

		for (int i = 0; i < length; ++i) {
			if (i > 0 && i % 1024 == 0 || i == length - 1) {
				server0x03.chunkLength	   = htons((int16_t)1024);
				server0x03.percentComplete = round((i / length) * 100);
				send(players[playerId].sock, &server0x03, sizeof(server0x03), 0);
				memset(server0x03.chunkData, 0, CHUNK_SIZE);
			}

			server0x03.chunkData[i % 1024] = buffer[i];
		}

	}

	// end world stuff here!

	server0x04.xSize = htons(worlds[players[playerId].currentWorldId].xSize);
	server0x04.ySize = htons(worlds[players[playerId].currentWorldId].ySize);
	server0x04.zSize = htons(worlds[players[playerId].currentWorldId].zSize);
	send(players[playerId].sock, &server0x04, sizeof(server0x04), 0);

	// To alert everyone and the player:
	server0x07.playerId = -1;
	memcpy(server0x07.playerName, players[playerId].username, 64);
	server0x07.x = htons((worlds[players[playerId].currentWorldId].xSize  / 2) * 32);
	server0x07.y = htons(((worlds[players[playerId].currentWorldId].ySize / 2) + 1) * 32);
	server0x07.z = htons((worlds[players[playerId].currentWorldId].zSize  / 2) * 32);
	server0x07.yaw = 0;
	server0x07.pitch = 0;
	send(players[playerId].sock, &server0x07, sizeof(server0x07), 0);

	for (int i = 0; i < MAX; ++i) { // TO THE PLAYER
		if (players[i].sock && i != playerId) {
			server0x07.playerId = i;
			memcpy(server0x07.playerName, players[i].username, 64);
			server0x07.x = 0;
			server0x07.y = htons(1);
			server0x07.z = 0;
			server0x07.yaw = 0;
			server0x07.pitch = 0;
			send(players[playerId].sock, &server0x07, sizeof(server0x07), 0);
		}
	}

	server0x07.playerId = playerId;
	memcpy(server0x07.playerName, players[playerId].username, 64);
	server0x07.x = 0;
	server0x07.y = htons(1);
	server0x07.z = 0;
	server0x07.yaw = 0;
	server0x07.pitch = 0;
	for (int i = 0; i < MAX; ++i) { // FROM THE PLAYER
		if (players[i].sock && i != playerId) {
			send(players[i].sock, &server0x07, sizeof(server0x07), 0);
		}
	}

	// Finally, it will send all the block updates.

	for (int i = 0; i < MAX_BLOCK_UPDATES; ++i)
		if (worlds[players[playerId].currentWorldId].blockChanges[i].packetId)
			send(players[playerId].sock, &worlds[players[playerId].currentWorldId].blockChanges[i], 
								   sizeof(worlds[players[playerId].currentWorldId].blockChanges[i]), 0);
		else
			break;

	return 0;
}

int swapworld(int16_t newWorldId, int16_t playerId) {
	players[playerId].currentWorldId = newWorldId;
	return sendworld(newWorldId, playerId);
}

int unload(int16_t id) {
	free(worlds[id].name);
	worlds[id].name  = NULL;
	worlds[id].xSize = 0;
	worlds[id].ySize = 0;
	worlds[id].zSize = 0;
	memset(worlds[id].blockChanges, 0, sizeof(worlds[id].blockChanges));

	for (int i = 0; i < MAX; ++i) {
		if (players[i].currentWorldId == id && players[i].sock) {
			disconnectPlayer(i, "The world you were just in has been unloaded.");
		}
	}

	return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <netinet/in.h>
#include "worlds.h"
#include "definitions.h"
#include "functions.h"

int command_help(char (*arguments_ptr)[64][64], player_t* player) {
	sendMessage(0, player->playerId, "&eCurrent commands:");

	for (int i = 0; i < MAX; ++i) {
		if (commands.list[i]) {
			char toSend[2000];
			strcpy(toSend, "    &e- &f/");
			strcat(toSend, commands.name[i]);
			strcat(toSend, " ");
			strcat(toSend, commands.description[i]);
			sendMessage(0, player->playerId, toSend);
		}
	}
}

int command_create(char (*arguments_ptr)[64][64], player_t* player) {
	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	int16_t xSize = atoi(arguments[2]);
	int16_t ySize = atoi(arguments[3]);
	int16_t zSize = atoi(arguments[4]);

	if (xSize == 0 || ySize == 0 || zSize == 0) {
		sendMessage(0, player->playerId, "Size arguments must be signed 16 bit integers larger than 0.");
	} else {
		for (int i = 0; i < MAX; ++i) {
			if (!worlds[i].name) {
				if (buildworld(i, arguments[1], xSize, ySize, zSize)) {
					sendMessage(0, player->playerId, "Failed to create level.");
				} else 
					sendMessage(0, player->playerId, "Level created.");
				return 0;
			}
		}
	}
}

int command_delete(char (*arguments_ptr)[64][64], player_t* player) {
	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	for (int i = 0; i < strlen(arguments[1]); ++i) {
		if (!isdigit(arguments[1][i]) && !isalpha(arguments[1][i])) {
			sendMessage(0, player->playerId, "Worlds can't have non-alphanumeric characters in their names.");
			return 1;
		}
	}

	if (strcmp(arguments[1], server_info.main_world) == 0) {
		sendMessage(0, player->playerId, "The main world cannot be deleted. Please manually remove the file when the server is turned off.");
		return 1;
	}

	if (remove(arguments[1]) == -1) {
		sendMessage(0, player->playerId, "World failed to delete.");
		return 1;
	}

	sendMessage(0, player->playerId, "World deleted.");

	for (int i = 0; i < MAX; ++i) {
		if (worlds[i].name) {
			if (strcmp(arguments[1], worlds[i].name) == 0) {
				unload(i);
				return 0;
			}
		}
	}
}

int command_load(char (*arguments_ptr)[64][64], player_t* player) {
	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	if (!arguments[1][0]) {
		sendMessage(0, player->playerId, "Can't load nothing.");
		return 1;
	}

	for (int i = 0; i < MAX; ++i) {
		if (!worlds[i].name) {
			if (loadworld(i, arguments[1])) {
				sendMessage(0, player->playerId, "Failed to load level.");
			} else 
				sendMessage(0, player->playerId, "Level loaded.");
			break;
		}
	}
}

int command_goto(char (*arguments_ptr)[64][64], player_t* player) {
	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	for (int i = 0; i < MAX; ++i) {
		if (worlds[i].name) {
			if (strcmp(arguments[1], worlds[i].name) == 0) {
				swapworld(i, player->playerId);
				return 0;
			}
		}
		
		if (i == MAX - 1)
			sendMessage(0, player->playerId, "World either isn't loaded or doesn't exist.");
	}

	return 1;
}

int command_stop(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	onExit(0);
}

int command_list(char (*arguments_ptr)[64][64], player_t* player) {
	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	sendMessage(0, player->playerId, "List of worlds (loaded = green):");
	for (int i = 0; i < MAX; ++i) {
		if (worlds[i].name) {								
			char toSend[strlen(worlds[i].name) + 3];
			strcpy(toSend, "&2");
			strcat(toSend, worlds[i].name);
			sendMessage(0, player->playerId, toSend);
		}
	}

	DIR* dir = opendir(".");
	struct dirent* d;
	while (d = readdir(dir)) {
		int  loaded = 0;

		if (d->d_name[0] == '.')
			continue;

		for (int i = 0; i < MAX; ++i) {
			if (worlds[i].name)
				if (strcmp(d->d_name, worlds[i].name) == 0) {
					loaded = 1;
				}
		}

		if (loaded != 1) {
			char toSend[64];
			strcpy(toSend, "&6");
			strcat(toSend, d->d_name);
			sendMessage(0, player->playerId, toSend);
		}

	}
}

int command_kick(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}
	
	char arguments[64][64];
	char reason[64];
	int  idToKick = -1;
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));
	memset(reason, 0, 64);

	if (!arguments[1]) {
		sendMessage(0, player->playerId, "You need to input a username to kick.");
		return 1;
	}

	for (int i = 0; i < MAX; ++i) {
		if (strcmp(arguments[1], players[i].username) == 0) {
			idToKick = i;
			break;
		}
	}

	if (idToKick == -1) {
		sendMessage(0, player->playerId, "Player doesn't exist.");
		return 1;
	}
	
	if (arguments[2][0])
		strcpy(reason, arguments[2]);
	else
		strcpy(reason, "Kicked from server: No reason provided.");
	
	for (int i = 3; arguments[i][0]; ++i) {
		strcat(reason, " ");
		strcat(reason, arguments[i]);
	}

	disconnectPlayer(idToKick, reason);
	return 0;
}

int command_op(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	for (int i = 0; i < MAX; ++i) {
		if (strcmp(arguments[1], players[i].username) == 0) {
			break;
		}
	}


	if (!arguments[1]) {
		sendMessage(0, player->playerId, "You need to name a player to op them.");
		return 1;
	}

	if (makeOp(player->playerId, arguments[1])) {
		sendMessage(0, player->playerId, "Could not op player.");
		return 1;
	}

	sendMessage(0, player->playerId, "Player opped.");

	return 0;
}

int command_deop(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	if (!arguments[1]) {
		sendMessage(0, player->playerId, "You need to name a player to deop them.");
		return 1;
	}

	if (deOp(player->playerId, arguments[1])) {
		sendMessage(0, player->playerId, "Could not deop player.");
		return 1;
	}

	sendMessage(0, player->playerId, "Player deopped.");

	return 0;
}

int command_oplist(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	opList(player->playerId);
	return 0;	
}

int command_ban(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	for (int i = 0; i < MAX; ++i) {
		if (strcmp(arguments[1], players[i].username) == 0) {
			break;
		}
	}


	if (!arguments[1]) {
		sendMessage(0, player->playerId, "You need to name a player to ban them.");
		return 1;
	}

	if (banPlayer(player->playerId, arguments[1])) {
		sendMessage(0, player->playerId, "Could not ban player.");
		return 1;
	}

	sendMessage(0, player->playerId, "Player banned.");

	return 0;
}

int command_unban(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	char arguments[64][64];
	memcpy(arguments, arguments_ptr, sizeof(char[64][64]));

	if (!arguments[1]) {
		sendMessage(0, player->playerId, "You need to name a player to unban them.");
		return 1;
	}

	if (unbanPlayer(player->playerId, arguments[1])) {
		sendMessage(0, player->playerId, "Could not unban player.");
		return 1;
	}

	sendMessage(0, player->playerId, "Player unbanned.");

	return 0;
}

int command_banlist(char (*arguments_ptr)[64][64], player_t* player) {
	if (player->opStatus != 0x64) {
		sendMessage(0, player->playerId, "&cYou need to be OP'd to run this command.");
		return 1;
	}

	banList(player->playerId);
	return 0;	
}

int loadcommand(const char* name, void* function, const char* description) {
	for (int i = 0; i < MAX; ++i) {
		if (strcmp(name, commands.name[i]) == 0) {
			printf("Command %s exists.\n", commands.name[i]);
			return 1;
		}
	}

	for (int i = 0; i < MAX; ++i) {
		if (!commands.name[i][0]) {
			strcpy(commands.name[i], name);
			commands.description[i] = (char*)malloc(strlen(description) + 1);
			strcpy(commands.description[i], description);
			commands.list[i] = function;
			printf("%sLoaded command %s with ID of %d.\n", AND_D, name, i);
			resetColour();
			break;
		}
	}

	return 0;
}

void loadcommands() {
	loadcommand("help",    &command_help,    "");
	loadcommand("create",  &command_create,  "&e[world name] [x] [y] [z] &c(OP)");
	loadcommand("delete",  &command_delete,  "&e[world name] &c(OP)");
	loadcommand("load",    &command_load,    "&e[world name]");
	loadcommand("goto",    &command_goto,    "&e[world name]");
	loadcommand("list",    &command_list,    "");
	loadcommand("stop",    &command_stop,    "&c(OP)");
	loadcommand("kick",    &command_kick,    "&e[username] [reason] &c(OP)");
	loadcommand("op",  	   &command_op,      "&e[username] &c(OP)");
	loadcommand("deop",    &command_deop,    "&e[username] &c(OP)");
	loadcommand("oplist",  &command_oplist,  "&c(OP)");
	loadcommand("ban",     &command_ban,     "&e[username] &c(OP)");
	loadcommand("unban",   &command_unban,   "&e[username] &c(OP)");
	loadcommand("banlist", &command_banlist, "&c(OP)");
}

int invokecommand(const char* name, char (*arguments_ptr)[64][64], player_t* player) {
	for (int i = 0; i < MAX; ++i) {
		if (commands.name[i][0]) {
			if (strcmp(name, commands.name[i]) == 0) {
				void (*command)(char (*)[64][64], player_t*) = commands.list[i];
				command(arguments_ptr, player);
				return 0;
			}
		}
	}

	sendMessage(0, player->playerId, "Unknown command");
	return 1;
}
#define SOFTWARE_NAME 	 "GoldServer"
#define SOFTWARE_VERSION "v1.0"
#define MAX  127 // This is seperately defined in structs.h because i'm an idiot
#define HTTP "HTTP 501\nContent-Type: text/plain\n\nGoldServer - players online:\n"
#define CHUNK_SIZE 		  1024
#define MAX_BLOCK_UPDATES 2097152
#define ALPHABET 	   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
#define BANLIST_FILE   "banlist"
#define IPBANLIST_FILE "ipbanlist"
#define OPLIST_FILE    "oplist"
#define FILE_MAX       500 // Maximum amount of input for banlist, ipbanlist and oplist.

// Colours
#define AND_0 "\x1b[38;2;0;0;0m"
#define AND_1 "\x1b[38;2;0;0;191m"
#define AND_2 "\x1b[38;2;0;191;0m"
#define AND_3 "\x1b[38;2;0;191;191m"
#define AND_4 "\x1b[38;2;191;0;0m"
#define AND_5 "\x1b[38;2;191;0;191m"
#define AND_6 "\x1b[38;2;191;191;0m"
#define AND_7 "\x1b[38;2;191;191;191m"
#define AND_8 "\x1b[38;2;64;64;64m"
#define AND_9 "\x1b[38;2;64;64;255m"
#define AND_A "\x1b[38;2;64;255;64m"
#define AND_B "\x1b[38;2;64;255;255m"
#define AND_C "\x1b[38;2;255;64;64m"
#define AND_D "\x1b[38;2;255;64;255m"
#define AND_E "\x1b[38;2;255;255;64m"
#define AND_F "\033[0m"


#include "structs.h"

extern server_info_t server_info;
extern commands_t 	 commands;
extern player_t   	 players[MAX];
extern world_t    	 worlds[MAX];

#pragma once
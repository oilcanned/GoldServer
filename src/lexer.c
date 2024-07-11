#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "functions.h"
#include "definitions.h"

typedef enum {
	TYPE_STRING,
	TYPE_EQUALS,
	TYPE_IDENTIFIER,
	TYPE_COMMA,
	TYPE_DATA
} type_t;

typedef struct token_t {
	type_t type;
	char   data[255];
} token_t;

const char* returnType(type_t type) {
	if (type == TYPE_STRING) 	 return "TYPE_STRING";
	if (type == TYPE_EQUALS) 	 return "TYPE_EQUALS";
	if (type == TYPE_IDENTIFIER) return "TYPE_IDENTIFIER";
	if (type == TYPE_DATA) 		 return "TYPE_DATA";
	return "TYPE_WTF";
}

token_t tokens[255];

int lexerMain() {
	FILE* file = fopen("server.properties", "rb");
	if (file == NULL) {
		printf("error\n");
		return 1;
	}

	fseek(file, 0, SEEK_END);
	int  length = ftell(file);
	char buffer[length + 1];
	rewind(file);
	fread(buffer, length, 1, file);
	buffer[length] = 0;
	fclose(file);

	int counter = 0;
	for (int i = 0; i < strlen(buffer); ++i) {
		switch (buffer[i]) {
			case '=':
				tokens[counter].type    = TYPE_EQUALS;
				tokens[counter].data[0]	= '=';
				tokens[counter].data[1] = 0;
				counter++;
				break;
			case '"':
				tokens[counter].type = TYPE_DATA;

				int j = 0;
				while (i++, buffer[i] != '"')
					tokens[counter].data[j++] = buffer[i];

				tokens[counter].data[j] = 0;
				counter++;
				break;
			default:
				if ((isalnum(buffer[i]) != 0) || buffer[i] == '_') {
					tokens[counter].type = TYPE_IDENTIFIER;
					if (counter > 0)
						if (tokens[counter - 1].type == TYPE_EQUALS)
							tokens[counter].type = TYPE_DATA;

					int j = 0;
					--i;
					while (i++, (isalnum(buffer[i]) != 0) || buffer[i] == '_')
						tokens[counter].data[j++] = buffer[i];

					tokens[counter].data[j] = 0;
					counter++;
					--i;
				}
				break;
		}
	}

	for (int i = 0; i < counter; ++i) {
		if (tokens[i].type == TYPE_IDENTIFIER) {
			if (strcmp(tokens[i].data, "server_name") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;
				
				if (strlen(tokens[i].data) > 64) {
					printf("server_name is too long!\n");
					return 1;
				}

				strcpy(server_info.server_name, tokens[i].data);
			} else if (strcmp(tokens[i].data, "server_motd") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;

				if (strlen(tokens[i].data) > 64) {
					printf("server_motd is too long!\n");
					return 1;
				}

				strcpy(server_info.server_motd, tokens[i].data);
			} else if (strcmp(tokens[i].data, "autosave_interval") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;

				server_info.autosave_interval = atoi(tokens[i].data);
			} else if (strcmp(tokens[i].data, "port") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;

				server_info.port = (uint16_t)atoi(tokens[i].data);
			} else if (strcmp(tokens[i].data, "worlds_folder") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;

				strcpy(server_info.worlds_folder, tokens[i].data);
			} else if (strcmp(tokens[i].data, "main_world") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;

				strcpy(server_info.main_world, tokens[i].data);
			} else if (strcmp(tokens[i].data, "heartbeat_url") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;

				strcpy(server_info.heartbeat_url, tokens[i].data);
			} else if (strcmp(tokens[i].data, "public") == 0) {
				++i;
				if (tokens[i++].type != TYPE_EQUALS) return 1;
				if (tokens[i].type   != TYPE_DATA)   return 1;

				if (strcmp(tokens[i].data, "true") == 0)
					server_info.public = 1;
				else
					server_info.public = 0;
			}
		}
	}

	return 0;
}
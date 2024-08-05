#include <stdint.h>
#define MAX_BLOCK_UPDATES 32000
#define MAX 127
#pragma once

typedef char 	String[64];
typedef uint8_t Byte;
typedef int8_t  SByte;
typedef int8_t  FByte;
typedef int16_t Short;
typedef int16_t FShort;
typedef uint8_t ByteArray[1024];

// Client -> Server packets
// Packet IDs are NOT included.

/* Player Identification */
struct __attribute__((packed)) client0x00_t {
	Byte   protocolVersion;
	String username;
	String mpass;
	Byte   unused;
};

/* Set Block */
struct __attribute__((packed)) client0x05_t {
	Short x;
	Short y;
	Short z;
	Byte  mode;
	Byte  blockType;
};

/* Position and Orientation */
struct __attribute__((packed)) client0x08_t {
	SByte  playerId;
	FShort x;
	FShort y;
	FShort z;
	Byte   yaw;
	Byte   pitch;
};

/* Message */
struct __attribute__((packed)) client0x0d_t {
	SByte playerId;
	String message;
};

/* ExtInfo */
struct __attribute__((packed)) client0x10_t {
	String appName;
	Short  extensionCount;
};

/* ExtEntry */
struct __attribute__((packed)) client0x11_t {
	String extName;
	int    version;
};

// Server -> Client packets

/* Server Identification */
struct __attribute__((packed)) server0x00_t {
	Byte   packetId;
	Byte   protocolVersion;
	String serverName;
	String serverMotd;
	Byte   userType;
};

/* Ping */
struct __attribute__((packed)) server0x01_t {
	Byte packetId;
};

/* Level Initialise */
struct __attribute__((packed)) server0x02_t {
	Byte packetId;
};

/* Level Data Chunk */
struct __attribute__((packed)) server0x03_t {
	Byte  	  packetId;
	Short 	  chunkLength;
	ByteArray chunkData;
	Byte 	  percentComplete;
};

/* Level Finalise */
struct __attribute__((packed)) server0x04_t {
	Byte  packetId;
	Short xSize;
	Short ySize;
	Short zSize;
};

/* Set Block */
struct __attribute__((packed)) server0x06_t {
	Byte  packetId;
	Short x;
	Short y;
	Short z;
	Byte blockType;
};

/* Spawn Player */
struct __attribute__((packed)) server0x07_t {
	Byte   packetId;
	SByte  playerId;
	String playerName;
	FShort x;
	FShort y;
	FShort z;
	Byte   yaw;
	Byte   pitch;
};

/* Set Position and Orientation (Player Teleport) */
struct __attribute__((packed)) server0x08_t {
	Byte   packetId;
	SByte  playerId;
	FShort x;
	FShort y;
	FShort z;
	Byte   yaw;
	Byte   pitch;
};

/* Position and Orientation Update */
struct __attribute__((packed)) server0x09_t {
	Byte   packetId;
	SByte  playerId;
	FByte  xChange;
	FByte  yChange;
	FByte  zChange;
	Byte   yaw;
	Byte   pitch;
};

/* Position Update */
struct __attribute__((packed)) server0x0a_t {
	Byte   packetId;
	SByte  playerId;
	FByte  xChange;
	FByte  yChange;
	FByte  zChange;
};

/* Orienation Update */
struct __attribute__((packed)) server0x0b_t {
	Byte  packetId;
	SByte playerId;
	Byte  yaw;
	Byte  pitch;
};

/* Despawn Player */
struct __attribute__((packed)) server0x0c_t {
	Byte  packetId;
	SByte playerId;
};

/* Message */
struct __attribute__((packed)) server0x0d_t {
	Byte   packetId;
	SByte  playerId;
	String message;
};

/* Disconnect Player */
struct __attribute__((packed)) server0x0e_t {
	Byte   packetId;
	String disconnectReason;
};

/* Update User Type */
struct __attribute__((packed)) server0x0f_t {
	Byte packetId;
	Byte userType;
};

/* ExtInfo */
struct __attribute__((packed)) server0x10_t {
	Byte   packetId;
	String appName;
	Short  extensionCount;
};

/* ExtEntry */
struct __attribute__((packed)) server0x11_t {
	Byte   packetId;
	String extName;
	int    version;
};


/* Custom Block Support Level */
struct __attribute__((packed)) server0x13_t {
	Byte packetId;
	Byte supportLevel;
};

// Other

/* World structure. */
typedef struct world_t {
	char* 	name;
	int16_t xSize;
	int16_t ySize;
	int16_t zSize;
	uint8_t saving;
	struct  server0x06_t blockChanges[MAX_BLOCK_UPDATES];
} world_t;

/* For passing multiple arguments to functions used in multithreading. */
typedef struct pointers_t {
	void* arg1;
	void* arg2;
	void* arg3;
} pointers_t; // I don't think i even use this but i can't remember anymore i changed all this shit up

/* Player struct */
typedef struct player_t {
	char 	username[64];
	int16_t currentWorldId;
	int16_t playerId;
	int  	sock;
	struct  server0x08_t server0x08;
	uint8_t opStatus;
	uint8_t paint;
} player_t;

typedef struct commands_t {
	void* list[MAX];
	char  name[MAX][64];
	char* description[MAX];
} commands_t;

typedef struct server_info_t {
	uint16_t port;
	char server_name[64];
	char server_motd[64];
	char worlds_folder[128];
	char main_world[128];
	char heartbeat_url[128];
	int  autosave_interval;
	int  public;
	int  cracked;

	char salt[17];
} server_info_t;
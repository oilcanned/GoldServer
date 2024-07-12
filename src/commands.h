#include "definitions.h"
int command_help  (char (*arguments_ptr)[64][64], player_t* player);
int command_create(char (*arguments_ptr)[64][64], player_t* player);
int command_delete(char (*arguments_ptr)[64][64], player_t* player);
int command_load  (char (*arguments_ptr)[64][64], player_t* player);
int command_goto  (char (*arguments_ptr)[64][64], player_t* player);
int command_list  (char (*arguments_ptr)[64][64], player_t* player);
int command_stop  (char (*arguments_ptr)[64][64], player_t* player);
int command_kick  (char (*arguments_ptr)[64][64], player_t* player);
int command_op	  (char (*arguments_ptr)[64][64], player_t* player);
int command_deop  (char (*arguments_ptr)[64][64], player_t* player);
int loadcommand   (const char* name, void* function, const char* description);
int loadcommands  ();
int invokecommand (const char* name, char (*arguments_ptr)[64][64], player_t* player);
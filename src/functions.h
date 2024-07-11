#include <stdint.h>

int   generatesalt();
int   getBlock(int id, int16_t x, int16_t y, int16_t z);
char* padcpy(char* dest, const char* src);
char* unpad(char* src);
void  onExit(int test);
int   sendMessage(int type, int id, const char message[64]);
void  printMessage(const char* message);
int   disconnectPlayer(int id, const char* message);
char* afterchar(char* src, char character);
int   isOp(const char* username);
int   makeOp(int id, const char* username);
int   deOp(int id, const char* username);
int   opList(int id);
int   banPlayer(int id, const char* username);
int   unbanPlayer(int id, const char* username);
int   banList(int id);
int   checkBanned(int id);
void  resetColour();

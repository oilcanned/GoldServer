int buildworld( int16_t id, const char* name, int16_t xSize, int16_t ySize, int16_t zSize);
int loadworld(  int16_t id, const char* name);
int saveworld(  int16_t id, int waiting);
int sendworld(  int16_t worldId, int16_t playerId);
int swapworld(  int16_t worldId, int16_t playerId);
int unload(		int16_t id);
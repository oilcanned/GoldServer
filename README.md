<p align="center"><img width="200px" src="assets/GoldServer.png"></p>
<p align="center">A Server Software for Classic Minecraft, Linux only.</p>

# Dependencies
`OpenSSL` \
`Curl` \
`zlib`

# Compiling
Run `make` in the root directory. \
Alternatively, you can just run ``gcc -s src/*.c -o bin/main -lz -lpthread -lcurl -lm -lssl -lcrypto``

# Configuring
The server.properties file provided allows for minor changes to be made to the server. \
`server_name`       - defines the server name. \
`server_motd`       - defines the server motd. \
`port`              - defines the server port. \
`autosave_interval` - defines the interval (in seconds) between automatic world saving. \
`worlds_folder`     - defines the folder in which worlds are stored in. \
`main_world`        - defines the main world that will be sent to the player when they join. \
`heartbeat_url`     - defines the URL for the heartbeat, allowing the server to be shown on the server list. \
`public`            - defines whether the server is shown on the server list or not. \
`cracked`           - defines whether players need a valid mpass to join thes server. \
The default hearbeat is the ClassiCube heartbeat.

> [!NOTE]
> The code is very rushed. If you see any _interesting_ code, please alert me.

> [!NOTE]
> This is in super early development.

# Limitations
- Can only create superflat worlds.
- Maximum player count cannot be changed.
- No ranks.
- World do not have their own MOTD.
- Possibly many more.

# Known Features
- [x] Multithreaded 
- [x] Commands 
- [x] Operator commands 
- [x] Players can swap worlds
- [x] Supports extra block types
- [x] Supports message types 
- [x] Idk what else to add here 

# Commands
For a list of commands, run /help. \
Basic operator commands are available. 

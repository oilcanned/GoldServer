<p align="center"><img width="200px" src="assets/GoldServer.png"></p>
<p align="center">A Server Software for Classic Minecraft</p>

# Compiling
A simple bash file, `compile`, located in the root of the server, is used for compiling. \
Give it execute permissions by running ``chmod +x compile`` \
Compile by running ./compile \
Alternatively, you can just run ``gcc -s src/*.c -o bin/main -lz -lpthread -lcurl -lm -lssl -lcrypto``
# Configuring

The server.properties file provided allows for minor changes to be made to the server. \
`server_name`       - defines the server name. \
`server_motd`       - defines the server motd. \
`port               - defines the server port. \
`autosave_interval` - defines the interval (in seconds) between automatic world saving. \
`worlds_folder`     - defines the folder in which worlds are stored in. \
`main_world`        - defines the main world that will be sent to the player when they join. \
`heartbeat_url`     - defines the URL for the heartbeat, allowing the server to be shown on the server list. \
`public`            - defines whether the server is shown on the server list or not. 

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
[x] Multithreaded \
[x] Commands \
[x] Operator commands \
[x] Players can swap worlds \
[x] Idk what else to add here 

# Commands
For a list of commands, run /help. \
Basic operator commands are available. 

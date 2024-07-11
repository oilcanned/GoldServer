CC   = gcc
SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
BIN  = bin/main
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename ${SRC})))
LIBS = -lz -lpthread -lcurl -lm -lssl -lcrypto

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< -o $@

compile: ./bin $(OBJ) $(SRC) $(DEPS)
	$(CC) $(OBJ) $(LIBS) -o $(BIN)
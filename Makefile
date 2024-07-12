OS  := $(shell uname -s)
CC   = gcc
SRC  = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
BIN  = bin/main
OBJ  = $(addsuffix .o,$(subst src/,bin/,$(basename ${SRC})))
LIBS = -lz -lcurl -lm -lssl -lcrypto -lws2_32

ifeq ($(OS), Linux)
	LIBS = -lz -lcurl -lm -lssl -lcrypto -lpthread
endif

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< -o $@

compile: ./bin $(OBJ) $(SRC) $(DEPS)
	$(CC) $(OBJ) $(LIBS) -o $(BIN)
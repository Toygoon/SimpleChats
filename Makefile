# Directory Settings
BUILD_DIR := ./build
SRC_DIRS := ./src
CLIENT_APP = client
SERVER_APP = server

# Compiler Settings
CC := gcc
CFLAGS := `pkg-config --cflags gtk+-3.0`
LFLAGS := `pkg-config --libs gtk+-3.0`

all: server client

server:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(SERVER_APP).out $(SRC_DIRS)/$(SERVER_APP).c $(LFLAGS)

client:
	mkdir -p $(BUILD_DIR)
	$(CC) $(SRC_DIRS)/$(CLIENT_APP).c -o $(BUILD_DIR)/$(CLIENT_APP).out $(CFLAGS)

clean:
	rm -r $(BUILD_DIR)

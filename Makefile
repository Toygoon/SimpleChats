# Project Name
#TARGET_EXEC := SimpleChats

# Directory Settings
BUILD_DIR := ./build
SRC_DIRS := ./src
SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(shell find $(SRCS) -exec basename {} \; | sed -e "s/.c/.out/g")

# Compiler Settings
CC := gcc
CFLAGS := -lpthread

all:
	mkdir -p $(BUILD_DIR)
	$(CC) $(SRCS) -o $(BUILD_DIR)/$(OBJS) $(CFLAGS)

clean:
	rm -r $(BUILD_DIR)
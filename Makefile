CC = gcc
CFLAGS = -Wall -Wextra -I./include -I./lib/parson
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC_DIR = src
LIB_DIR = lib/parson
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.c) $(LIB_DIR)/parson.c
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

TARGET = world_map

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

init:
	git submodule update --init --recursive
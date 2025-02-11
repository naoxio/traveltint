# Common variables
SRC_DIR = src
LIB_DIR = lib/parson
BUILD_DIR = build
WEB_BUILD_DIR = build_web
RAYLIB_WEB_DIR = $(BUILD_DIR)/raylib_web

SOURCES = $(wildcard $(SRC_DIR)/*.c) $(LIB_DIR)/parson.c
TARGET = traveltint

# Native build configuration
CC = gcc
CFLAGS = -Wall -Wextra -I./include -I./lib/parson
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Web build configuration
EMCC = emcc
EMFLAGS = -Wall -Wextra -I./include -I./lib/parson -I$(RAYLIB_WEB_DIR)/src -DPLATFORM_WEB
EMLDFLAGS = -s USE_GLFW=3 -s WASM=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 \
            -s INITIAL_MEMORY=67108864 \
            --preload-file assets \
            --preload-file shaders \
            -s EXPORTED_RUNTIME_METHODS=ccall \
            --shell-file shell.html

.PHONY: all clean web raylib_web

# Default target (native build)
all: $(BUILD_DIR)/$(TARGET)

# Native build rules
$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Raylib web build
$(RAYLIB_WEB_DIR)/src/libraylib.a:
	@mkdir -p $(RAYLIB_WEB_DIR)
	@if [ ! -d "$(RAYLIB_WEB_DIR)/src" ]; then \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_WEB_DIR); \
	fi
	cd $(RAYLIB_WEB_DIR)/src && \
	$(EMCC) -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c utils.c -Os -Wall -DPLATFORM_WEB -DHOST_EMSCRIPTEN && \
	$(EMCC) -c raudio.c -Os -Wall -DPLATFORM_WEB -DHOST_EMSCRIPTEN && \
	emar rcs libraylib.a rcore.o rshapes.o rtextures.o rtext.o rmodels.o utils.o raudio.o

raylib_web: $(RAYLIB_WEB_DIR)/src/libraylib.a

# Web build target

web: raylib_web
	@mkdir -p $(WEB_BUILD_DIR)
	$(EMCC) $(SOURCES) -o $(WEB_BUILD_DIR)/index.html $(EMFLAGS) $(EMLDFLAGS) $(RAYLIB_WEB_DIR)/src/libraylib.a -DPLATFORM_WEB

clean:
	rm -rf $(BUILD_DIR) $(WEB_BUILD_DIR)

init:
	git submodule update --init --recursive
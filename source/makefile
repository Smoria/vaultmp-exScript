CC=g++
CFLAGS=-c -Wall -std=gnu++0x -m32

ifeq ($(OS),Windows_NT)
TARGET_OS=Win32
TARGET_FORMAT=dll
MKDIR=-mkdir
RMDIR=rmdir /S /Q
else
TARGET_OS=Unix
TARGET_FORMAT=so
MKDIR=mkdir -p
RMDIR=rm -rf
endif

OBJ_PATH=../obj/$(TARGET_OS)/
BIN_PATH=../bin/$(TARGET_OS)/
OUT_LIB=$(BIN_PATH)ex.$(TARGET_FORMAT)
OFLAGS=-shared -static -m32 -o $(OUT_LIB)

all: directories main.o
	$(CC) $(OBJ_PATH)main.o $(OFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c -o $(OBJ_PATH)$@ $<

directories:
	$(MKDIR) "$(OBJ_PATH)"
	$(MKDIR) "$(BIN_PATH)"

clean:
	$(RMDIR) "$(OBJ_PATH)"
	$(RMDIR) "$(BIN_PATH)"

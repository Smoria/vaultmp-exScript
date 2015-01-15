CC=g++
CFLAGS=-c -Wall -std=gnu++0x -m32

OBJ_PATH=../obj/Win32/
BIN_PATH=../bin/Win32/
OUT_LIB=$(BIN_PATH)ex.dll
OFLAGS=-shared -static -m32 -o $(OUT_LIB)

all: directories main.o
	$(CC) $(OBJ_PATH)main.o $(OFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c -o $(OBJ_PATH)$@ $<

directories:
	mkdir "$(OBJ_PATH)"
	mkdir "$(BIN_PATH)"

clean:
	rmdir /S /Q "$(OBJ_PATH)"
	rmdir /S /Q "$(BIN_PATH)"
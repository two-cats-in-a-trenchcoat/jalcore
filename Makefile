# Jalcore1 Emulator makefile.
CC=g++
CFLAGS=-lSDL2 -lSDL2main -O3
TARGET=jalcore1-v0.1.exe
SRC=emulator.cpp
.DEFAULT_GOAL := emulator

emulator: $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)
	@echo $(TARGET) Succesfully installed.
	

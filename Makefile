# Jalcore1 Emulator makefile.
CC=g++
CFLAGS=-lSDL2 -lSDL2main -O3

TARGET=jalcore1-0.1.0.exe
TARGET2=compatest.exe
SRC=emulator.cpp
SRC2=software/devdemos/display_colors.cpp

.DEFAULT_GOAL := emulator

emulator: $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)
	@echo $(TARGET) Succesfully installed.

compatest: $(SRC2)
	$(CC) $(SRC2) -o $(TARGET2) $(CFLAGS)
	@echo Compatablity test ran successfully.

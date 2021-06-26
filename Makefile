# Jalcore1 Emulator makefile.
CC=g++
BFLAGS= -v -Wall -g 
CFLAGS= -O3 -lSDL2 -lSDL2main 

TARGET=jalcore1-0.1.0.exe
TARGET2=compatest.exe
SRC=emulator.cpp
SRC2=software/devdemos/display_colors.cpp

.DEFAULT_GOAL := emulator

emulator: $(SRC)
	$(CC) $(BFLAGS) $(SRC) -o $(TARGET) $(CFLAGS)
	@echo $(TARGET) Succesfully installed.
	
standalone-release:
	$ tar -xf libs.zip
	$(CC) $(BFLAGS) $(SRC) -o $(TARGET) $(CFLAGS)
	@echo $(TARGET) Succesfully installed.
	@echo This file may still require Microsoft Visual C++ to function correctly.
	

compatest: $(SRC2)
	$(CC) $(SRC2) -o $(TARGET2) $(CFLAGS)
	@echo Compatablity test ran successfully.

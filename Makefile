# Jalcore1 Emulator makefile.
CC=g++
BFLAGS= -v -Wall -g 
CFLAGS= -O3 -lSDL2 -lSDL2main resources/emulator.res

TARGET=jalcore1-0.1.0.exe
TARGET2=compatest.exe
SRC=emulator.cpp
SRC2=software/devdemos/display-colors.cpp

.DEFAULT_GOAL := emulator

emulator: $(SRC)
	$(CC) $(BFLAGS) $(SRC) -o $(TARGET) $(CFLAGS)
	@echo $(TARGET) Succesfully installed.
	
standalone-release: $(SRC)
	@echo The standalone release still requires a full install of g++ with native SDL2 to compile.
	@echo This is only for sharing relases, not for compiling on insufficient systems.
	$ tar -xf libs.zip
	$(CC) $(BFLAGS) $(SRC) -o $(TARGET) $(CFLAGS)
	@echo $(TARGET) Succesfully installed.
	@echo This file may still require Microsoft Visual C++ to function correctly.
	

compatest: $(SRC2)
	$(CC) $(SRC2) -o $(TARGET2) $(CFLAGS)
	@echo Compatablity test compiled successfully.
	
standalone-compatest: $(SRC2)
	@echo The standalone release still requires a full install of g++ with native SDL2 to compile.
	@echo This is only for sharing relases, not for compiling on insufficient systems.
	$ tar -xf libs.zip
	$(CC) $(SRC2) -o $(TARGET2) $(CFLAGS)
	@echo Compatablity test compiled successfully.
	@echo This file may still require Microsoft Visual C++ to function correctly.

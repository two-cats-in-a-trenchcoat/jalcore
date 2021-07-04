# Jalcore simulated 8-bit CPU
A fully-fledged, 8-bit, ~7MHz CPU Running in C++. Also, free bloatware.

# Build Instructions (on Windows with g++)
1. Install [SDL2](https://wiki.libsdl.org/Installation)
2. Navigate to your jalcore install folder.
3. Run the make file using your make command of choice  e.g: `mingw32-make`, `make`, `cmake`
4. The make options are `emulator`, `standalone-release`, `compatest`, and `standalone-compatest`

If You don't want to build it yourself you can visit the releases page where there will be pre-built stable versions.
This is also possible with visual C++, but requires special setup and manually compiling the files.

# Build Instructions (on Linux with g++)
1. Install [SDL2](https://wiki.libsdl.org/Installation)
2. Navigate to your jalcore install folder.
3. Run the makefile with `make` or `cmake`
4. The make options are `emulator`, `standalone-release`(deprecated), `compatest`, and `standalone-compatest`(deprecated)

There are no standalone releases for linux, due to the fact that neither of us have the capabilities to compile on linux, although with the current state of Windows 11 that might change soon. Either build from source or use WINE or virtualisation only, sorry!

# Mac
We don't know. Sorry!


CC = g++

# Compiler flags: enable warnings, debugging, and C++11 standard
CFLAGS = -Wall -g -std=c++11

# Linker flags for SDL2 (using pkg-config for portability)
LDFLAGS = `pkg-config --libs sdl2`

# Include flags for SDL2 headers
INCLUDES = `pkg-config --cflags sdl2`

# Target executable name
TARGET = CHIP-8-EMULATOR

# Source files 
SOURCES = main.cpp chip8.cpp

# Object files (generated from source files)
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = chip8.h opcodes.h

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run the emulator (assuming a ROM file is provided as an argument)
run: $(TARGET)
	./$(TARGET) rom_file.ch8

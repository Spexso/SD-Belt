# Makefile for Conveyor Belt Controller

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Target executable
TARGET = conveyor_controller

# Source files
SOURCES = main.cpp ArduinoSerial.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install rule (optional)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Help rule
help:
	@echo "Makefile for Conveyor Belt Controller"
	@echo "Available targets:"
	@echo "  all        - Build the conveyor controller (default)"
	@echo "  clean      - Remove object files and executable"
	@echo "  install    - Install the executable to /usr/local/bin/"
	@echo "  help       - Display this help message"

# Phony targets
.PHONY: all clean install help
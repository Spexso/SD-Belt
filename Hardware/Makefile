# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Source and target
SRC = main.cpp ArduinoSerial.cpp ArduinoSerial.h httplib.h
TARGET = app 

# Build rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean rule
clean:
	del $(TARGET)

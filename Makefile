# Makefile for POSIX OS Lab Toolkit
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET = OSLabToolkit
SRCS = main_menu.cpp shell.cpp scheduler.cpp threads.cpp filesystem.cpp memory.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

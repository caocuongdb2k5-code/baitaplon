CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Ithird_party
SRCS = $(wildcard src/*.cpp)
TARGET = hotel

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -static -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe


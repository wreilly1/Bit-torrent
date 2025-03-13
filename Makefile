CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
LIBS := -lssl -lcrypto

TARGET := bt_client

SRCS := bencode_parser.cpp \
        bt_messages.cpp \
        peer_network.cpp \
        torrent_info.cpp \
        main.cpp

OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)


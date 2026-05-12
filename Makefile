CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -g
LDFLAGS :=

SRCS := \
	main.cpp \
	ConfigLoader.cpp \
	listen_parser.cpp \
	WebServer.cpp \
	Server.cpp \
	Socket.cpp

TEST_SRCS := test.cpp

OBJS := $(SRCS:.cpp=.o)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)
TARGET := webserv
TEST_TARGET := test

.PHONY: all clean rebuild run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) $(LDFLAGS)

rebuild: clean all

clean:
	rm -f *.o $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET) a.out

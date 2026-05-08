NAME := webserv

CXX := c++
CXXFLAGS := -std=c++98 -Wall -Wextra -Werror

SRCS := \
	test.cpp \
	config_parse.cpp \
	listen_parser.cpp \
	socket.cpp \
	request/HttpRequest.cpp	

OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

NAME = Webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS =  main.cpp response.cpp ResponseHandler.cpp ../request/HttpRequest.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp response.hpp ResponseHandler.hpp ../request/HttpRequest.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
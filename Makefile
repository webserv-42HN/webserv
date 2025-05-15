# CPP = c++

# FLAGS	= -c -g -Wall -Werror -Wextra -std=c++11
# OBJDIR	= obj
# NAME = webserv

# SRCS	= main.cpp ConfigParser.cpp Request.cpp Response.cpp utils.cpp Server.cpp
# HEADERS = ConfigParser.hpp Request.hpp Response.hpp utils.hpp Server.hpp
# OBJS	= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

# all: $(NAME)

# $(OBJDIR)/%.o: %.cpp $(HEADERS) Makefile
# 	@mkdir -p $(dir $@)
# 	$(CPP) $(CFLAGS) -c $< -o $@

# $(NAME): $(OBJS)
# 	$(CPP) $(OBJS) -o $(NAME)

# clean:
# 	rm -rf $(OBJDIR)

# fclean: clean
# 	rm -f $(NAME)

# re: fclean $(NAME)

# .PHONY: all clean fclean re

CPP     = c++

CXXFLAGS = -c -g -Wall -Werror -Wextra -std=c++11
OBJDIR  = obj
NAME    = webserv

SRCS    = main.cpp ConfigParser.cpp Request.cpp Response.cpp utils.cpp Server.cpp
HEADERS = ConfigParser.hpp Request.hpp Response.hpp utils.hpp Server.hpp
OBJS    = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

all: $(NAME)

$(OBJDIR)/%.o: %.cpp $(HEADERS) Makefile
	@mkdir -p $(dir $@)
	$(CPP) $(CXXFLAGS) -o $@ $<

$(NAME): $(OBJS)
	$(CPP) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

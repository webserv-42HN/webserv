
# CPP     = c++

# CXXFLAGS = -c -g -Wall -Werror -Wextra -std=c++11
# OBJDIR  = obj
# NAME    = webserv

# SRCS    = main.cpp ConfigParser.cpp Request.cpp Response.cpp utils.cpp Server.cpp Request_utils.cpp Response_To_Post.cpp Response_utils.cpp Router.cpp
# HEADERS = ConfigParser.hpp Request.hpp Response.hpp utils.hpp Server.hpp Router.hpp
# OBJS    = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

# all: $(NAME)

# $(OBJDIR)/%.o: %.cpp $(HEADERS) Makefile
# 	@mkdir -p $(dir $@)
# 	$(CPP) $(CXXFLAGS) -o $@ $<

# $(NAME): $(OBJS)
# 	$(CPP) $(OBJS) -o $(NAME)

# clean:
# 	rm -rf $(OBJDIR)

# fclean: clean
# 	rm -f $(NAME)

# re: fclean all

# .PHONY: all clean fclean re

CPP = c++

CFLAGS = -c -g -Wall -Werror -Wextra -std=c++17
OBJDIR = obj
NAME = webserv

SRCDIR = src
INCDIR = includes

# Automatically find all .cpp files in src/
SRCS = $(wildcard $(SRCDIR)/*.cpp)
# Extract just the filenames without path for objects
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# For headers, you can keep it manual or also wildcard if needed:
HEADERS = $(wildcard $(INCDIR)/*.hpp)

all: $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) Makefile
	@mkdir -p $(dir $@)
	$(CPP) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(NAME): $(OBJS)
	$(CPP) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

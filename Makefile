CPP = c++

CFLAGS = -c -g -Wall -Werror -Wextra -std=c++17
OBJDIR = obj
NAME = webserv

SRCDIR = src
INCDIR = includes

SRCS = Client_Hander.cpp \
        Config_Manager.cpp main.cpp \
        Request_utils.cpp \
        Request.cpp \
        Response_CGI.cpp \
        Response_To_Post.cpp \
        Response.cpp \
        Response_utils.cpp \
        Router.cpp \
        Server_utils.cpp \
        Server.cpp \
        Utils.cpp

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

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

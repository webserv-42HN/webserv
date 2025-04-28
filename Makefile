NAME := server

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17

SRC_PATH = ./srcs
OBJ_PATH = ./ojbs
SRC := $(SRC_PATH)/main.cpp

OBJ := $(SRC:.c=.o)

PURPLE := \033[95m
GREEN := \033[92m
RESET := \033[0m

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(PURPLE)"
	@echo "  ██████╗  ██████╗   ██████╗"
	@echo "  ██╔═══   ██   ██║  ██   ██║"
	@echo "  ██║      ██████╔╝  ██████╔╝"
	@echo "  ██║      ██╔═══╝   ██╔═══╝"
	@echo "  ██████║  ██║       ██║"
	@echo "  ╚═════╝  ╚═╝       ╚═╝"
	@echo "Creating $(NAME)..."
	@echo "$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	@mkdir -p $(OBJ_PATH)
	@echo "$(GREEN)"
	@echo "Compiling $<..."
	@echo "$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(PURPLE)"
	@echo "Cleaning object files..."
	@echo "$(RESET)"
	@rm -f $(OBJ_PATH)/*.o

fclean: clean
	@echo "$(PURPLE)"
	@echo "Cleaning $(NAME)..."
	@echo "$(RESET)"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

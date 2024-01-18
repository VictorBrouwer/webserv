# 			GENERAL VARS
NAME 		:= 	webserv

OBJ_DIR		:=	./obj
SRC_DIR 	:=	./src

SRC 		:=	server/main.cpp \
				server/server.cpp \

OBJ			:=	$(SRC:%.cpp=$(OBJ_DIR)/%.o)

CC			:= c++
FLAGS 		:= -std=c++20 
# -Wall -Werror -Wextra
ifdef DEBUG
	FLAGS += -g
endif

ifdef FSAN
	FLAGS += -fsanitize=address,undefined
endif
COMPILE		:= $(CC) $(FLAGS)

###############			RECIPES		###############
all: $(NAME)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@ 

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: clean all

debug:
	$(MAKE) DEBUG=1

rebug: fclean
	$(MAKE) debug

fsan:
	$(MAKE) DEBUG=1 FSAN=1

resan: fclean fsan

.PHONY: all clean fclean re debug rebug

.DEFAULT_GOAL := all

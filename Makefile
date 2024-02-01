# 			GENERAL VARS
NAME 		:= 	webserv

OBJ_DIR		:=	./obj
SRC_DIR 	:=	./src

INC			:=	-I include 

HEADERS		:=	$(wildcard include/*.hpp)

SRC			:=	main.cpp \
				HTTPserver.cpp \
				poll.cpp \

OBJ			:=	$(addprefix $(OBJ_DIR)/,$(SRC:.cpp=.o))
SRC			:=	$(addprefix $(SRC_DIR)/,$(SRC))

CC			:=	c++
FLAGS 		:= -std=c++20 -Wall -Werror -Wextra

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

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(FLAGS) $(INC) -c $< -o $@

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $^ -o $(NAME)

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

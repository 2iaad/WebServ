NAME = webserv

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98

SRC	=	./pars_config/config.cpp main.cpp ./AllServer/HttpServer.cpp \
		./FunctionTools.cpp ./Request/Request.cpp ./Request/Post.cpp \
		./cgi/cgiMain.cpp ./cgi/cgiExecPars.cpp ./cgi/cgiResponse.cpp \
		./cgi/cgiListDir.cpp \
		./Response/responseMain.cpp ./Response/sendResponse.cpp ./Response/delete.cpp \
		./Response/postResponse.cpp ./Response/get.cpp

OBJ_DIR = obj

OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

VPATH = ./pars_config:./AllServer:./Request:./cgi:./Response

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%.o: %.cpp ./pars_config/config.hpp  ./AllServer/HttpServer.hpp Request/Request.hpp Request/Post.hpp ./Response/responseHeader.hpp ./cgi/cgiHeader.hpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/09/11 16:35:01 by lantonio          #+#    #+#              #
#    Updated: 2025/01/10 15:32:44 by lantonio         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= minishell
CC		= cc
FLAGS	= -Wall -Wextra -Werror -g

LIBFT_DIR = ./libft
LIBFT_A = $(LIBFT_DIR)/libft.a

SRCS	= minishell.c\
			./src/ft_aux_str.c\
			./src/ft_args_main.c\
			./src/tokenizer.c\
			./src/mudar_dps.c\
			./src/build_command.c\
			./env/list_manipulation.c\
			./checkers/command.c\
			./checkers/string.c\
			./signal/signal.c\
			./commands/pwd.c\
			./commands/echo.c\
			./commands/export.c\
			./commands/unset.c\
			./commands/cd.c\
			./commands/cd_aux.c\
			./commands/env.c\
			./commands/exit.c\
			./env/env_utils.c\
			memory_file2.c\

OBJS	= $(SRCS:.c=.o)

$(NAME): $(LIBFT_A) $(OBJS)
	@$(CC) $(FLAGS) $(OBJS) $(LIBFT_A) -Iincludes -o $(NAME) -lreadline
	@echo "\033[1;32mSUCESS!\033[0m"

%.o:%.c
	@$(CC) $(FLAGS) -c $< -o $@

$(LIBFT_A):
	@$(MAKE) bonus -C $(LIBFT_DIR)

all: $(NAME)

run : $(NAME)
	@echo "\033[1;32mMINISHELL_RUNNING..."
	@./minishell


clean :
	@rm -rf $(OBJS)
	@$(MAKE) clean -C $(LIBFT_DIR)
	@echo "Cleaning objects"

fclean : clean
	@rm -rf $(NAME)
	@$(MAKE) fclean -C $(LIBFT_DIR)
	@echo "cleaning executable"

re : fclean all

valgrind:
	valgrind  --leak-check=full ./minishell
	
valgrind_all:
	valgrind  --leak-check=full --show-leak-kinds=all ./minishell

.PHONY : all clean fclean re
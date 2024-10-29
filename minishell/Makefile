# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/09/11 16:35:01 by lantonio          #+#    #+#              #
#    Updated: 2024/10/29 14:52:54 by lantonio         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= minishell
CC		= cc
FLAGS	= -Wall -Wextra -Werror -g

SRCS	= minishell.c\
			./src/funts_aux.c\
			./checkers/signal.c\
			./signal/signal.c\
			./commands/pwd.c\
			./commands/echo.c\
			./src/ft_split.c\

OBJS	= $(SRCS:.c=.o)

$(NAME): $(OBJS)
	@$(CC) $(FLAGS) $(OBJS) -Iincludes -o $(NAME) -lreadline
	@echo "\033[1;32mMINISHELL CREATED"

%.o:%.c
	@$(CC) $(FLAGS) -c $< -o $@

all: $(NAME)

clean :
	@rm -rf $(OBJS)
	@echo "Cleaning objects"

fclean : clean
	@rm -rf $(NAME)
	@echo "cleaning executable"

re : fclean all

.PHONY : all clean fclean re
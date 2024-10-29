/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 10:06:57 by lantonio          #+#    #+#             */
/*   Updated: 2024/10/29 14:52:13 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <limits.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <fcntl.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

void	pwd(char **str);
void	echo(char **str);

// checkers
int		check_signal_exit(char *str);

// Signal
void	signal_new_line(int signum);
void	configure_signal(void);

// aux_funct
void	identify_command(char *command);
char    **ft_split(char const *s, char c);
int	    ft_strcmp(char *s1, char *s2);
void	ft_putstr_fd(char *s, int fd);
int 	isset_in_mat(char **mat, char *str);

#endif
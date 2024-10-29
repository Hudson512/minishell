/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   funts_aux.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:56:22 by hmateque          #+#    #+#             */
/*   Updated: 2024/10/29 16:20:26 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int	isset_in_mat(char **mat, char *str)
{
	int	i;

	i = -1;
	while (mat[++i])
		if (!ft_strcmp(mat[i], str))
			return (i);
	return (0);
}

void	ft_putstr_fd(char *s, int fd)
{
	int	i;

	i = -1;
	while (s[++i])
		write(fd, &s[i], 1);
}

int	ft_strcmp(char *s1, char *s2)
{
	int	i;

	i = 0;
	while (s1[i] == s2[i] && s1[i] && s2[i])
		i++;
	return ((int)((unsigned char)s1[i] - (unsigned char)s2[i]));
}

#include <stdlib.h>

char *remove_quote(char *str) {
    int i = 0, j = 0, len = 0;
    char *new_str;

    while (str[i]) {
        if (str[i] != '"' && str[i] != '\'')
            len++;
        i++;
    }
    new_str = (char *)malloc(sizeof(char) * (len + 1));
    if (!new_str) return NULL;

    i = 0;
    while (str[i]) {
        if (str[i] != '"' && str[i] != '\'') {
            new_str[j++] = str[i];
        }
        i++;
    }
    new_str[j] = '\0';
    return new_str;
}

char **remove_quotes(char **str) {
    int i = 0, j, qtd_quotes;

    while (str[i]) {
        j = 0;
        qtd_quotes = 0;
        while (str[i][j]) {
            if (str[i][j] == '"') qtd_quotes++;
            if (str[i][j] == '\'') qtd_quotes++;
            j++;
        }
        if (qtd_quotes % 2 == 0) {
            char *new_str = remove_quote(str[i]);
            if (new_str) {
                free(str[i]);
                str[i] = new_str;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
        i++;
    }
    return str;
}

void	identify_command(char *command)
{
	char	**str;

	str = ft_split(command, ' ');
	str = remove_quotes(str);
	if (str)
	{
		if (str[0])
		{
			if (!ft_strcmp(str[0], "pwd"))
			pwd(str);
		else if (!ft_strcmp(str[0], "echo"))
			echo(str);
		else if (!ft_strcmp(str[0], "exit"))
			exit(0);
		}
	}
	else
		printf("Error\n");
}

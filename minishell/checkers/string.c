/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:30:56 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/08 17:23:34 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int	check_quote_syntax(char *input)
{
	int		inside_quote;
	char	quote_type;

	inside_quote = 0;
	quote_type = '\0';
	while (*input)
	{
		if (*input == '"' || *input == '\'')
		{
			if (!inside_quote)
			{
				inside_quote = 1;
				quote_type = *input;
			}
			else if (*input == quote_type)
			{
				inside_quote = 0;
				quote_type = '\0';
			}
		}
		input++;
	}
	if (inside_quote)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

int	check_read_from(char **str)
{
	int	i;

	i = -1;
	while (str[++i])
		if (!ft_strcmp(str[i], "<") || !ft_strcmp(str[i], "<<"))
			return (1);
	return (0);
}

int	check_arg(char *str)
{
	int	i;

	i = 0;
	if (!ft_isalpha(str[i]) && str[i] != '_')
		return (0);
	i++;
	while (str[i] && str[i] != '=')
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	check_cipher(char *str, int fd, t_env *env)
{
	int		i;
	char	*result;
	char	*new_str;

	i = 0;
	result = ft_strchr(str, '$');
	if (result == NULL)
		return (0);
	result++;
	while (result[i] != '\0')
		i++;
	new_str = allocate_mem((i + 1), sizeof(char));
	i = 0;
	while (result[i] != '\0')
	{
		new_str[i] = result[i];
		i++;
	}
	new_str[i] = '\0';
	search_and_print_list(env, new_str, fd);
	return (1);
}

int	ft_isspace(char c)
{
	if (c == ' ')
		return (1);
	if (c == '\t')
		return (1);
	if (c == '\n')
		return (1);
	if (c == '\v')
		return (1);
	if (c == '\f')
		return (1);
	if (c == '\r')
		return (1);
	return (0);
}

char	*trim_spaces(char *str, size_t i)
{
	char	*start;
	char	*end;
	char	*trimmed;
	size_t	new_length;

	if (!str)
		return (NULL);
	start = str;
	while (*start && ft_isspace((unsigned char)*start))
		start++;
	if (*start == '\0')
	{
		free(str);
		return (NULL);
	}
	end = str + ft_strlen(str) - 1;
	while (end > start && ft_isspace((unsigned char)*end))
		end--;
	new_length = end - start + 1;
	trimmed = allocate_mem(new_length + 1, sizeof(char));
	collect_mem(trimmed, MEM_CHAR_PTR, 0);
	while (++i < new_length)
		trimmed[i] = start[i];
	trimmed[new_length] = '\0';
	return (free(str), trimmed);
}

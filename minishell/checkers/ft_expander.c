/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_expander.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 16:32:35 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 16:45:09 by hmateque         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static char	*handle_single_quotes(char *str)
{
	char	*temp;

	temp = ft_strndup(str + 1, ft_strlen(str) - 2);
	return (temp);
}

static char	*handle_double_quotes(char *str, t_env *env, int *g_returns)
{
	char	*temp;
	char	*expanded;

	temp = ft_strndup(str + 1, ft_strlen(str) - 2);
	expanded = expand_variable(temp, env, g_returns);
	collect_mem(expanded, MEM_CHAR_PTR, 0);
	return (expanded);
}

static char	*handle_regular_expansion(char *str, t_env *env, int *g_returns)
{
	char	*expanded;
	char	*temp;

	expanded = expand_variable(str, env, g_returns);
	if (avoid_double_quote_error(expanded))
	{
		temp = remove_double_quotes(expanded);
		collect_mem(temp, MEM_CHAR_PTR, 0);
		free(expanded);
		return (temp);
	}
	if (avoid_single_quote_error(expanded))
	{
		temp = remove_single_quotes(expanded);
		collect_mem(temp, MEM_CHAR_PTR, 0);
		free(expanded);
		return (temp);
	}
	collect_mem(expanded, MEM_CHAR_PTR, 0);
	return (expanded);
}

char	**expander(char **str, t_env *env, int *g_returns, int wordcount)
{
	int		i;

	i = -1;
	if (!str)
		return (NULL);
	while (++i < wordcount)
	{
		if (should_skip_expansion(str[i]))
			continue ;
		if (str[i][0] == '\'' && str[i][ft_strlen(str[i]) - 1] == '\'')
			str[i] = handle_single_quotes(str[i]);
		else if (str[i][0] == '"' && str[i][ft_strlen(str[i]) - 1] == '"')
			str[i] = handle_double_quotes(str[i], env, g_returns);
		else
			str[i] = handle_regular_expansion(str[i], env, g_returns);
	}
	return (str);
}

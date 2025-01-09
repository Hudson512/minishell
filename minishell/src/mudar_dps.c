/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mudar_dps.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/12 11:13:21 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/09 13:14:33 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

t_token_type	identify_token(char *token)
{
	if (ft_strncmp(token, "|", 1) == 0)
		return (TOKEN_PIPE);
	else if (ft_strncmp(token, ">>", 2) == 0)
		return (TOKEN_APPEND_OUT);
	else if (ft_strncmp(token, ">", 1) == 0)
		return (TOKEN_REDIRECT_OUT);
	else if (ft_strncmp(token, "<<", 2) == 0)
		return (TOKEN_HEREDOC);
	else if (ft_strncmp(token, "<", 1) == 0)
		return (TOKEN_REDIRECT_IN);
	return (TOKEN_ARG);
}

t_token	**classify_tokens(char **tokens, int wc, t_env **env, int *g_returns);
t_token	**allocate_classified_tokens(int wc);
int		process_token(t_token **classified_tokens,
					char **tokens, int i, int wc);
void	classify_token_type(t_token **classified_tokens, char **tokens, int i);
int		set_token_value(t_token *token, char *value);
char	*strip_quotes(char *token, size_t len);

t_token	**classify_tokens(char **tokens, int wc, t_env **env, int *g_returns)
{
	t_token	**classified_tokens;
	int		i;

	(void)env;
	(void)g_returns;
	classified_tokens = allocate_classified_tokens(wc);
	if (!classified_tokens)
		return (NULL);
	i = 0;
	while (i < wc)
	{
		if (!process_token(classified_tokens, tokens, i, wc))
			return (NULL);
		i++;
	}
	classified_tokens[i] = NULL;
	return (classified_tokens);
}

t_token	**allocate_classified_tokens(int wc)
{
	t_token	**classified_tokens;

	classified_tokens = malloc((wc + 1) * sizeof(t_token *));
	if (!classified_tokens)
		return (NULL);
	collect_mem(classified_tokens, MEM_CHAR_MATRIX, (wc + 1));
	return (classified_tokens);
}

int	process_token(t_token **classified_tokens, char **tokens, int i, int wc)
{
	(void)wc;
	classified_tokens[i] = malloc(sizeof(t_token));
	if (!classified_tokens[i])
	{
		free_all_mem();
		return (0);
	}
	collect_mem(classified_tokens[i], MEM_TOKEN_PTR, 0);
	classify_token_type(classified_tokens, tokens, i);
	if (!set_token_value(classified_tokens[i], tokens[i]))
	{
		free_all_mem();
		return (0);
	}
	return (1);
}

void	classify_token_type(t_token **classified_tokens, char **tokens, int i)
{
	classified_tokens[i]->type = identify_token(tokens[i]);
	if (classified_tokens[i]->type == TOKEN_ARG)
	{
		if (i == 0 || (i > 0 && classified_tokens[i - 1]->type == TOKEN_PIPE))
		{
			classified_tokens[i]->type = TOKEN_COMMAND;
		}
	}
}

int	set_token_value(t_token *token, char *value)
{
	size_t	len;
	char	*stripped_token;

	len = ft_strlen(value);
	if ((value[0] == 39 && value[len - 1] == 39)
		|| (value[0] == 34 && value[len - 1] == 34))
	{
		stripped_token = strip_quotes(value, len);
		if (!stripped_token)
			return (0);
		token->value = stripped_token;
	}
	else
		token->value = value;
	return (1);
}

char	*strip_quotes(char *token, size_t len)
{
	char	*stripped_token;

	stripped_token = malloc((len - 1) * sizeof(char));
	if (!stripped_token)
		return (NULL);
	collect_mem(stripped_token, MEM_CHAR_PTR, 0);
	strncpy(stripped_token, token + 1, len - 2);
	stripped_token[len - 2] = '\0';
	return (stripped_token);
}

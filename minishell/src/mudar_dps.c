/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mudar_dps.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/12 11:13:21 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 15:42:30 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

TokenType	identify_token(char *token)
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

Token	**allocate_classified_tokens(int wc);
Token	*allocate_token(void);
char	*strip_quotes(const char *token);
void	classify_token_type(Token **classified_tokens, char **tokens, int wc);

Token	**classify_tokens(char **tokens, int wc, t_env **env, int *g_returns)
{
	Token	**classified_tokens;

	(void)env;
	(void)g_returns;
	classified_tokens = allocate_classified_tokens(wc);
	if (!classified_tokens)
		return (NULL);
	classify_token_type(classified_tokens, tokens, wc);
	classified_tokens[wc] = NULL;
	return (classified_tokens);
}

Token	**allocate_classified_tokens(int wc)
{
	Token	**classified_tokens;

	classified_tokens = malloc((wc + 1) * sizeof(Token *));
	if (!classified_tokens)
		return (NULL);
	collect_mem(classified_tokens, MEM_CHAR_MATRIX, (wc + 1));
	return (classified_tokens);
}

Token	*allocate_token(void)
{
	Token	*token;

	token = malloc(sizeof(Token));
	if (!token)
	{
		free_all_mem();
		return (NULL);
	}
	collect_mem(token, MEM_TOKEN_PTR, 0);
	return (token);
}

char	*strip_quotes(const char *token)
{
	size_t	len;
	char	*stripped_token;

	len = ft_strlen(token);
	if ((token[0] == 39 && token[len - 1] == 39)
		|| (token[0] == 34 && token[len - 1] == 34))
	{
		stripped_token = malloc((len - 1) * sizeof(char));
		if (!stripped_token)
		{
			free_all_mem();
			return (NULL);
		}
		collect_mem(stripped_token, MEM_CHAR_PTR, 0);
		strncpy(stripped_token, token + 1, len - 2);
		stripped_token[len - 2] = '\0';
		return (stripped_token);
	}
	return ((char *)token);
}

void	classify_token_type(Token **classified_tokens, char **tokens, int wc)
{
	int	i;

	i = -1;
	while (++i < wc)
	{
		classified_tokens[i] = allocate_token();
		if (!classified_tokens[i])
			return ;
		classified_tokens[i]->type = identify_token(tokens[i]);
		if (classified_tokens[i]->type == TOKEN_ARG && i > 0
			&& classified_tokens[i - 1]->type == TOKEN_PIPE)
		{
			classified_tokens[i]->type = TOKEN_COMMAND;
		}
		else if (classified_tokens[i]->type == TOKEN_ARG && i == 0)
		{
			classified_tokens[i]->type = TOKEN_COMMAND;
		}
		classified_tokens[i]->value = strip_quotes(tokens[i]);
	}
}

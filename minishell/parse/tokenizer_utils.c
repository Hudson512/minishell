/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 10:15:04 by lantonio          #+#    #+#             */
/*   Updated: 2025/01/11 10:15:05 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void	lookfor_token(t_node **node, char **input, char *temp, int i)
{
	lookfor_meta(node, input, temp, i);
	if (*node)
		return ;
	if (temp[i] == '$')
		*node = lookfor_word(input, EXPAND);
	else if (ft_strchr("|<> \"\'", temp[i]) == 0)
		*node = lookfor_word(input, WORD);
	else if (temp[i] && temp[i + 1] && temp[i] == '\'')
		*node = lookfor_quotes(input, '\'', SING_QUOTE);
	else if (temp[i] && temp[i + 1] && temp[i] == '"')
		*node = lookfor_quotes(input, '"', DOUB_QUOTE);
}

void	lookfor_meta(t_node **node, char **input, char *temp, int i)
{
	if (temp[i] == '&' && temp[i + 1] == '&')
		*node = create_meta_node(input, "&&", 2);
	else if (temp[i] == '|' && temp[i + 1] == '|')
		*node = create_meta_node(input, "||", 2);
	else if (temp[i] == '|')
		*node = create_meta_node(input, "|", 1);
	else if (temp[i] == '>' && temp[i + 1] == '>')
		*node = create_meta_node(input, ">>", 2);
	else if (temp[i] == '<' && temp[i + 1] == '<')
		*node = create_meta_node(input, "<<", 2);
	else if (temp[i] == '<')
		*node = create_meta_node(input, "<", 1);
	else if (temp[i] == '>')
		*node = create_meta_node(input, ">", 1);
	else if (temp[i] == '(')
		*node = create_meta_node(input, "(", 1);
	else if (temp[i] == ')')
		*node = create_meta_node(input, ")", 1);
}

t_node	*lookfor_word(char **input, int type)
{
	int		i;
	char	*temp;
	char	*word;
	t_node	*node;

	i = 0;
	temp = *input;
	while (ft_strchr("|<> \"\'()\n\t", temp[i]) == 0)
	{
		if (temp[i] == '&' && temp[i + 1] && temp[i + 1] == '&')
			break ;
		i++;
		*input = *input + 1;
	}
	word = ft_strndup(temp, i);
	collect_mem(word);
	node = create_word_node(word, type, temp[i]);
	return (node);
}

t_node	*lookfor_quotes(char **input, char symbol, int type)
{
	int		i;
	char	*temp;
	char	*quote;
	t_node	*node;

	i = 1;
	temp = *input;
	*input = *input + 1;
	while (temp[i] && temp[i] != symbol)
	{
		i++;
		*input = *input + 1;
	}
	quote = ft_strndup(temp, i + 1);
	collect_mem(quote);
	node = create_word_node(quote, type, temp[i + 1]);
	*input = *input + 1;
	return (node);
}

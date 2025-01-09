/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 17:25:05 by lantonio          #+#    #+#             */
/*   Updated: 2025/01/08 17:31:01 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

typedef struct s_expand_var_state
{
	char	*result;
	char	*current;
	int		inside_single_quotes;
	t_env	*env;
	int		*g_returns;
}			t_expand_var_state;

static char	*append_char_to_result(char *result, char c)
{
	char	str[2];

	str[0] = c;
	str[1] = '\0';
	return (ft_strjoin_free(result, ft_strdup(str)));
}

static char	*handle_exit_status(t_expand_var_state *state)
{
	char	*exit_status;

	exit_status = ft_itoa(*(state->g_returns));
	state->result = ft_strjoin_free(state->result, exit_status);
	free(exit_status);
	state->current++;
	return (state->result);
}

static char	*expand_env_variable(t_expand_var_state *state)
{
	char	*var_end;
	char	*var_name;
	t_env	*env_var;

	var_end = state->current;
	while (*var_end && (ft_isalnum(*var_end) || *var_end == '_'))
		var_end++;
	var_name = ft_strndup(state->current, var_end - state->current);
	env_var = find_env_var(state->env, var_name);
	if (env_var)
		state->result = ft_strjoin_free(state->result, env_var->value);
	free(var_name);
	state->current = var_end;
	return (state->result);
}

static char	*process_character(t_expand_var_state *state)
{
	if (*state->current == '\'')
	{
		state->inside_single_quotes = !state->inside_single_quotes;
		state->result = append_char_to_result(state->result, *state->current);
		state->current++;
	}
	else if (*state->current == '$' && !state->inside_single_quotes)
	{
		state->current++;
		if (*state->current == '?')
			state->result = handle_exit_status(state);
		else
			state->result = expand_env_variable(state);
	}
	else
	{
		state->result = append_char_to_result(state->result, *state->current);
		state->current++;
	}
	return (state->result);
}

char	*expand_variable(char *var, t_env *env, int *g_returns)
{
	t_expand_var_state	state;

	state.result = ft_strdup("");
	state.current = var;
	state.inside_single_quotes = 0;
	state.env = env;
	state.g_returns = g_returns;
	while (*state.current)
		state.result = process_character(&state);
	return (state.result);
}

char	*remove_single_quotes(char *str)
{
	int		i;
	int		j;
	char	*result;

	i = 0;
	j = 0;
	result = malloc(strlen(str) + 1);
	while (str[i])
	{
		if (str[i] != '\'')
		{
			result[j] = str[i];
			j++;
		}
		i++;
	}
	result[j] = '\0';
	return (result);
}

char	*remove_double_quotes(char *str)
{
	int		i;
	int		j;
	char	*result;

	i = 0;
	j = 0;
	result = malloc(strlen(str) + 1);
	while (str[i])
	{
		if (str[i] != '"')
		{
			result[j] = str[i];
			j++;
		}
		i++;
	}
	result[j] = '\0';
	return (result);
}

int	avoid_single_quote_error(char *str)
{
	int	i;
	int	quote_count;

	i = 0;
	quote_count = 0;
	while (str[i])
	{
		if (str[i] == '\'')
			quote_count++;
		i++;
	}
	if (quote_count > 1)
		return (1);
	return (0);
}

int	avoid_double_quote_error(char *str)
{
	int	i;
	int	quote_count;

	i = 0;
	quote_count = 0;
	while (str[i])
	{
		if (str[i] == '"')
			quote_count++;
		i++;
	}
	if (quote_count > 1)
		return (1);
	return (0);
}

static char	*process_single_quotes(char *word)
{
	char	*temp;

	temp = ft_strndup(word + 1, ft_strlen(word) - 2);
	collect_mem(temp, MEM_CHAR_PTR, 0);
	return (temp);
}

static char	*process_double_quotes(char *word, t_env *env, int *g_returns)
{
	char	*temp;
	char	*expanded;

	temp = ft_strndup(word + 1, ft_strlen(word) - 2);
	expanded = expand_variable(temp, env, g_returns);
	collect_mem(expanded, MEM_CHAR_PTR, 0);
	free(temp);
	return (expanded);
}

static char	*process_unquoted(char *word, t_env *env, int *g_returns)
{
	char	*expanded;
	char	*temp;

	expanded = expand_variable(word, env, g_returns);
	if (avoid_double_quote_error(expanded))
	{
		temp = remove_double_quotes(expanded);
		collect_mem(temp, MEM_CHAR_PTR, 0);
		free(expanded);
		return (temp);
	}
	else if (avoid_single_quote_error(expanded))
	{
		temp = remove_single_quotes(expanded);
		collect_mem(temp, MEM_CHAR_PTR, 0);
		free(expanded);
		return (temp);
	}
	collect_mem(expanded, MEM_CHAR_PTR, 0);
	return (expanded);
}

static char	*process_word(char *word, t_env *env, int *g_returns)
{
	if (word[1] == '|' || word[1] == '>' || word[1] == '<' || word[1] == '$')
		return (word);
	if (word[0] == '\'' && word[ft_strlen(word) - 1] == '\'')
		return (process_single_quotes(word));
	else if (word[0] == '"' && word[ft_strlen(word) - 1] == '"')
		return (process_double_quotes(word, env, g_returns));
	return (process_unquoted(word, env, g_returns));
}

char	**expander(char **str, t_env *env, int *g_returns, int wordcount)
{
	int		i;

	i = 0;
	while (i < wordcount)
	{
		str[i] = process_word(str[i], env, g_returns);
		i++;
	}
	return (str);
}

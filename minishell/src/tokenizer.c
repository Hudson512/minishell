/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 09:39:33 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 17:30:50 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

#define MEM_CHAR_PTR 1
#define MEM_CHAR_PTR_PTR 2

typedef struct s_word_data {
	char	**matrix;
	char	*word;
	int		word_len;
	int		word_capacity;
	int		word_index;
	bool	in_word;
	char	quote;
}	t_word_data;

char		**split_words(const char *input, int *word_count);
static int	finalize_quoted_word(t_word_data *data);
static int	append_char_to_word(t_word_data *data, char current_char);

typedef struct s_wc_state{
	int		count;
	bool	in_word;
	char	quote;
}	t_wc_state;

static void	init_state(t_wc_state *state)
{
	state->count = 0;
	state->in_word = false;
	state->quote = '\0';
}

static void	handle_quote2(char c, t_wc_state *state)
{
	if (state->quote)
	{
		if (c == state->quote)
			state->quote = '\0';
	}
	else if (c == '"' || c == '\'')
	{
		if (!state->in_word)
			state->count++;
		state->quote = c;
		state->in_word = true;
	}
}

static void	handle_non_quote(char c, t_wc_state *state)
{
	if (isspace(c))
		state->in_word = false;
	else if (!state->in_word)
	{
		state->count++;
		state->in_word = true;
	}
}

static int	count_words(const char *input)
{
	t_wc_state	state;
	int			i;

	init_state(&state);
	i = 0;
	while (input[i] != '\0')
	{
		if (state.quote)
			handle_quote2(input[i], &state);
		else
			handle_non_quote(input[i], &state);
		i++;
	}
	return (state.count);
}

static char	*allocate_word(int capacity)
{
	return ((char *)malloc(capacity * sizeof(char)));
}

static char	*resize_word(char *word, int *capacity)
{
	char	*new_word;
	int		i;
	int		new_capacity;

	new_capacity = *capacity * 2;
	new_word = (char *)malloc(new_capacity * sizeof(char));
	if (new_word == NULL)
		return (NULL);
	collect_mem(new_word, MEM_CHAR_PTR, 0);
	i = 0;
	while (i < *capacity)
	{
		new_word[i] = word[i];
		i++;
	}
	*capacity = new_capacity;
	return (new_word);
}

static int	handle_quote(t_word_data *data, char current_char)
{
	if (data->quote)
	{
		if (current_char == data->quote)
			return (finalize_quoted_word(data));
		else
			return (append_char_to_word(data, current_char));
	}
	return (0);
}

static int	finalize_quoted_word(t_word_data *data)
{
	data->word[data->word_len++] = data->quote;
	data->word[data->word_len] = '\0';
	data->matrix[data->word_index++] = data->word;
	data->in_word = false;
	data->word = NULL;
	data->word_len = 0;
	data->word_capacity = 16;
	data->quote = '\0';
	return (1);
}

static int	append_char_to_word(t_word_data *data, char current_char)
{
	if (data->word_len + 1 >= data->word_capacity)
	{
		data->word = resize_word(data->word, &data->word_capacity);
		if (data->word == NULL)
			return (-1);
	}
	data->word[data->word_len++] = current_char;
	return (1);
}

static int	handle_quote_start(t_word_data *data, char current_char)
{
	if (current_char == '"' || current_char == '\'')
	{
		if (!data->in_word)
		{
			data->in_word = true;
			data->word = allocate_word(data->word_capacity);
			if (data->word == NULL)
				return (-1);
			collect_mem(data->word, MEM_CHAR_PTR, 0);
		}
		data->quote = current_char;
		data->word[data->word_len++] = current_char;
		return (1);
	}
	return (0);
}

static int	handle_space(t_word_data *data)
{
	if (data->in_word)
	{
		data->word[data->word_len] = '\0';
		data->matrix[data->word_index++] = data->word;
		data->in_word = false;
		data->word = NULL;
		data->word_len = 0;
		data->word_capacity = 16;
	}
	return (0);
}

static int	handle_regular_char(t_word_data *data, char current_char)
{
	if (!data->in_word)
	{
		data->in_word = true;
		data->word = allocate_word(data->word_capacity);
		if (data->word == NULL)
			return (-1);
		collect_mem(data->word, MEM_CHAR_PTR, 0);
	}
	return (append_char_to_word(data, current_char));
}

static int	finalize_word(t_word_data *data)
{
	if (data->in_word)
	{
		data->word[data->word_len] = '\0';
		data->matrix[data->word_index++] = data->word;
	}
	data->matrix[data->word_index] = NULL;
	return (data->word_index);
}

static void	initialize_extraction(t_word_data *data)
{
	data->word_index = 0;
	data->in_word = false;
	data->word = NULL;
	data->word_len = 0;
	data->word_capacity = 16;
	data->quote = '\0';
}

static int	process_char(t_word_data *data, char current_char)
{
	int	result;

	result = handle_quote(data, current_char);
	if (result == -1)
		return (-1);
	if (result == 0)
	{
		result = handle_quote_start(data, current_char);
		if (result == -1)
			return (-1);
		if (result == 0)
		{
			if (isspace(current_char))
				handle_space(data);
			else if (handle_regular_char(data, current_char) == -1)
				return (-1);
		}
	}
	return (0);
}

static int	extract_words(const char *input, t_word_data *data)
{
	int	i;

	initialize_extraction(data);
	i = 0;
	while (input[i] != '\0')
	{
		if (process_char(data, input[i]) == -1)
			return (-1);
		i++;
	}
	return (finalize_word(data));
}

char	**split_words(const char *input, int *word_count)
{
	t_word_data	data;
	int			count;

	count = count_words(input);
	data.matrix = (char **)malloc((count + 1) * sizeof(char *));
	if (data.matrix == NULL)
		return (NULL);
	collect_mem(data.matrix, MEM_CHAR_PTR_PTR, 0);
	*word_count = extract_words(input, &data);
	if (*word_count == -1)
		return (NULL);
	return (data.matrix);
}

char	**ft_tokens(const char *input, int *word_count)
{
	t_word_data	data;
	char		**matrix;

	*word_count = count_words(input);
	matrix = (char **)allocate_mem((*word_count + 1), sizeof(char *));
	if (matrix == NULL)
	{
		free_all_mem();
		*word_count = 0;
		return (NULL);
	}
	data.matrix = matrix;
	collect_mem(matrix, MEM_CHAR_MATRIX, (*word_count + 1));
	if (extract_words(input, &data) == -1)
	{
		free_all_mem();
		*word_count = 0;
		return (NULL);
	}
	return (matrix);
}

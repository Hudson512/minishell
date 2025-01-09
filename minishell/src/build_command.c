/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_command.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/03 11:35:13 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/09 13:14:36 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

t_cmd	*initialize_command(int wordcount)
{
	t_cmd	*new_cmd;

	new_cmd = malloc(sizeof(t_cmd));
	collect_mem(new_cmd, MEM_COMMAND, 0);
	new_cmd->command = NULL;
	new_cmd->args = ft_calloc((wordcount + 1), sizeof(char *));
	collect_mem(new_cmd->args, MEM_CHAR_MATRIX, matrix_len(new_cmd->args));
	new_cmd->redirect_out = NULL;
	new_cmd->redirect_in = NULL;
	new_cmd->redirect_out_type = 0;
	new_cmd->next = NULL;
	new_cmd->heredoc = 0;
	new_cmd->heredoc_end = NULL;
	return (new_cmd);
}

int	process_redirect_out(t_token **tokens, int *i, t_cmd *current)
{
	current->redirect_out_type = 1;
	if (tokens[*i + 1])
	{
		current->redirect_out = tokens[++(*i)]->value;
		return (1);
	}
	printf("minishell: syntax error near unexpected token `newline 1'\n");
	return (0);
}

int	process_append_out(t_token **tokens, int *i, t_cmd *current)
{
	current->redirect_out_type = 2;
	if (tokens[*i + 1])
	{
		current->redirect_out = tokens[++(*i)]->value;
		return (1);
	}
	printf("minishell: syntax error near unexpected token `newline 2'\n");
	return (0);
}

int	process_redirect_in(t_token **tokens, int *i, t_cmd *current)
{
	if (tokens[*i + 1])
	{
		current->redirect_in = tokens[++(*i)]->value;
		return (1);
	}
	printf("minishell: syntax error near unexpected token `newline 3'\n");
	return (0);
}

int	process_heredoc(t_token **tokens, int *i, t_cmd *current)
{
	if (tokens[*i + 1])
	{
		current->heredoc = 1;
		current->heredoc_end = tokens[++(*i)]->value;
		return (1);
	}
	printf("minishell: syntax error near unexpected token `newline 4'\n");
	return (0);
}

void	process_command_args(t_token **tokens, int *i, t_cmd *cmd, int *index)
{
	while (tokens[*i + 1] != NULL && tokens[*i + 1]->type == TOKEN_ARG)
	{
		if (*index == 0)
			cmd->args[(*index)++] = cmd->command;
		else
			cmd->args[(*index)++] = tokens[++(*i)]->value;
	}
}

static bool	handle_redirection(t_token **tokens, int *i, t_cmd *current)
{
	if (tokens[*i]->type == TOKEN_REDIRECT_OUT)
		return (process_redirect_out(tokens, i, current));
	if (tokens[*i]->type == TOKEN_APPEND_OUT)
		return (process_append_out(tokens, i, current));
	if (tokens[*i]->type == TOKEN_REDIRECT_IN)
		return (process_redirect_in(tokens, i, current));
	if (tokens[*i]->type == TOKEN_HEREDOC)
		return (process_heredoc(tokens, i, current));
	return (true);
}

static t_cmd	*initialize_new_command(t_token *token, t_cmd **root,
	t_cmd **current, int wordcount)
{
	t_cmd	*new_cmd;

	new_cmd = initialize_command(wordcount);
	if (token->type == TOKEN_COMMAND)
		new_cmd->command = token->value;
	if (!(*root))
		*root = new_cmd;
	else
		(*current)->next = new_cmd;
	*current = new_cmd;
	return (new_cmd);
}

static void	process_arguments(t_token **tokens, int *i,
	t_cmd *current, int *arg_index)
{
	process_command_args(tokens, i, current, arg_index);
}

static void	process_tokens(t_token **tokens, int wordcount, t_cmd **root)
{
	int		i;
	int		arg_index;
	t_cmd	*current;

	i = 0;
	current = NULL;
	while (tokens[i] != NULL)
	{
		if (tokens[i]->type == TOKEN_COMMAND || current == NULL)
		{
			initialize_new_command(tokens[i], root, &current, wordcount);
			arg_index = 0;
		}
		if (!handle_redirection(tokens, &i, current))
			break ;
		process_arguments(tokens, &i, current, &arg_index);
		i++;
	}
}

t_cmd	*build_cmd(t_token **tokens, int wordcount)
{
	t_cmd	*root;

	root = NULL;
	process_tokens(tokens, wordcount, &root);
	return (root);
}

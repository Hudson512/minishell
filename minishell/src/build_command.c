/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_command.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/03 11:35:13 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 16:39:25 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

Command	*initialize_command(int wordcount)
{
	Command	*new_cmd;

	new_cmd = malloc(sizeof(Command));
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

int	process_redirect_out(Token **tokens, int *i, Command *current)
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

int	process_append_out(Token **tokens, int *i, Command *current)
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

int	process_redirect_in(Token **tokens, int *i, Command *current)
{
	if (tokens[*i + 1])
	{
		current->redirect_in = tokens[++(*i)]->value;
		return (1);
	}
	printf("minishell: syntax error near unexpected token `newline 3'\n");
	return (0);
}

int	process_heredoc(Token **tokens, int *i, Command *current)
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

void	process_command_args(Token **tokens, int *i, Command *cmd, int *index)
{
	while (tokens[*i + 1] != NULL && tokens[*i + 1]->type == TOKEN_ARG)
	{
		if (*index == 0)
			cmd->args[(*index)++] = cmd->command;
		else
			cmd->args[(*index)++] = tokens[++(*i)]->value;
	}
}

static Command	*handle_new_cmd(Command **root,
	Command *cur, Token *token, int wc)
{
	Command	*new_cmd;

	new_cmd = initialize_command(wc);
	if (new_cmd == NULL)
		return (NULL);
	if (token->type == TOKEN_COMMAND)
		new_cmd->command = token->value;
	if (*root == NULL)
		*root = new_cmd;
	else
		cur->next = new_cmd;
	return (new_cmd);
}

static int	handle_redirections(Token **tokens, int *i, Command *current)
{
	if (tokens[*i]->type == TOKEN_REDIRECT_OUT)
		return (process_redirect_out(tokens, i, current));
	else if (tokens[*i]->type == TOKEN_APPEND_OUT)
		return (process_append_out(tokens, i, current));
	else if (tokens[*i]->type == TOKEN_REDIRECT_IN)
		return (process_redirect_in(tokens, i, current));
	else if (tokens[*i]->type == TOKEN_HEREDOC)
		return (process_heredoc(tokens, i, current));
	return (1);
}

Command	*build_cmd(Token **tokens, int wordcount)
{
	int		i;
	int		arg_index;
	Command	*root;
	Command	*current;

	i = 0;
	root = NULL;
	current = NULL;
	while (tokens[i] != NULL)
	{
		if (tokens[i]->type == TOKEN_COMMAND || current == NULL)
		{
			current = handle_new_cmd(&root, current, tokens[i], wordcount);
			if (current == NULL)
				return (NULL);
			arg_index = 0;
		}
		if (!handle_redirections(tokens, &i, current))
			return (NULL);
		process_command_args(tokens, &i, current, &arg_index);
		i++;
	}
	return (root);
}

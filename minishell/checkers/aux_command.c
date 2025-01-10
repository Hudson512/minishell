/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   aux_command.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 16:02:08 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 16:06:21 by hmateque         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int	check_quote_syntax_return(char *line)
{
	if (ft_strcmp(line, "\"\"") == 0 || ft_strcmp(line, "''") == 0)
	{
		free(line);
		return (1);
	}
	return (0);
}

int	check_command(char *str, int *g_returns, int status)
{
	if (str && status == 1)
	{
		ft_putstr_fd("Syntax error\n", 1);
		free(str);
		*g_returns = 2;
		return (1);
	}
	else if (!str && status == 2)
	{
		free_all_mem();
		*g_returns = 0;
		return (1);
	}
	else if (!str && status == 3)
	{
		free_all_mem();
		*g_returns = 2;
		return (1);
	}
	return (0);
}

void	create_files(Command *command)
{
	int		fd;
	Command	*cmd;

	if (!command)
		return ;
	cmd = command;
	while (cmd)
	{
		if (cmd->redirect_out)
		{
			if (cmd->redirect_out_type == 1)
				fd = open(cmd->redirect_out,
						O_WRONLY | O_CREAT | O_TRUNC, 0644);
			else if (cmd->redirect_out_type == 2)
				fd = open(cmd->redirect_out,
						O_WRONLY | O_CREAT | O_APPEND, 0644);
			else
				fd = -1;
			if (fd != -1)
				close(fd);
		}
		cmd = cmd->next;
	}
}

int	handle_redirection(Command *cmd)
{
	int	fd;
	int	saved_stdout;

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout == -1)
		return (perror("Error duplicating STDOUT"), -1);
	if (cmd->redirect_out)
	{
		if (cmd->redirect_out_type == 1)
			fd = open(cmd->redirect_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else if (cmd->redirect_out_type == 2)
			fd = open(cmd->redirect_out, O_WRONLY | O_CREAT | O_APPEND, 0644);
		else
			fd = -1;
		if (fd == -1)
			return (perror("Error opening redirection file"), -1);
		if (dup2(fd, STDOUT_FILENO) == -1)
			return (perror("Error redirecting output"), close(fd), -1);
		close(fd);
	}
	return (saved_stdout);
}

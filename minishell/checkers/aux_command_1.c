/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   aux_command_1.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 16:02:08 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 16:45:22 by hmateque         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int	check_red_in(Command *cmd, int *fd_in)
{
	if (cmd->redirect_in != NULL)
	{
		if (access(cmd->redirect_in, R_OK) == -1)
		{
			printf("minishell: %s: No such file or directory\n",
				cmd->redirect_in);
			return (1);
		}
		*fd_in = open(cmd->redirect_in, O_RDONLY);
		if (*fd_in == -1)
			return (perror("Open input file error"), 1);
		if (dup2(*fd_in, STDIN_FILENO) == -1)
			return (perror("Dup2 error"), close(*fd_in), 1);
	}
	return (0);
}

int	built_ins(Command *cmd, t_env **env, int *g_returns)
{
	if (!ft_strcmp(cmd->command, "echo"))
		return (echo(cmd->args, g_returns), 1);
	else if (!ft_strcmp(cmd->command, "cd"))
		return (cd(cmd->args, g_returns, env), 1);
	else if (!ft_strcmp(cmd->command, "export"))
		return (ft_export(cmd->args, env, g_returns), 1);
	else if (!ft_strcmp(cmd->command, "env"))
		return (ft_env(cmd->args, g_returns, env), 1);
	else if (!ft_strcmp(cmd->command, "pwd"))
		return (pwd(cmd->args, g_returns), 1);
	else if (!ft_strcmp(cmd->command, "unset"))
		return (ft_unset(cmd->args, env, g_returns), 1);
	else if (!ft_strcmp(cmd->command, "exit"))
		return (ft_exit(env, 1), 1);
	return (0);
}

void	handle_sigint_child(int sig)
{
	(void)sig;
	exit(0);
}

bool	should_skip_expansion(char *str)
{
	return (str[1] == '|' || str[1] == '>' || str[1] == '<' || str[1] == '$');
}

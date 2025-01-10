/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:28:57 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 16:43:47 by hmateque         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int	path_commands(Command *cmd, t_env **env, char **envp, int *g_returns)
{
	char		**paths;
	char		path[1024];
	int			i;
	pid_t		pid;
	int			status;
	t_env		*env_copy;

	i = -1;
	env_copy = *env;
	if (!env_copy)
		return (printf("Env Error\n"), -1);
	if (access(cmd->command, X_OK) == 0)
	{
		pid = fork();
		if (pid == -1)
			return (perror("Fork error"), -1);
		if (pid == 0)
		{
			if (execve(cmd->command, cmd->args, envp) == -1)
				return (perror("Exec error"), -1);
			exit(EXIT_SUCCESS);
		}
		else
		{
			if (waitpid(pid, &status, 0) == -1)
				return (perror("Waitpid error"), -1);
			if (WIFEXITED(status))
				*g_returns = WEXITSTATUS(status);
			else if (WIFSIGNALED(status))
				*g_returns = WTERMSIG(status);
			return (*g_returns);
		}
	}
	if (cmd->command[0] == 47)
		return (printf("minishell: %s: No such file or directory\n", cmd->command), -1);
	while (env_copy)
	{
		if (!ft_strcmp(env_copy->name, "PATH"))
			break ;
		env_copy = env_copy->next;
	}
	if (!env_copy->value || !*env_copy->value)
		return (printf("Command not found\n"), -1);
	paths = ft_split(env_copy->value, ':');
	while (paths[++i] != NULL)
	{
		ft_strlcpy(path, paths[i], sizeof(path));
		ft_strlcat(path, "/", sizeof(path));
		ft_strlcat(path, cmd->command, sizeof(path));
		if (access(path, X_OK) == 0)
		{
			pid = fork();
			if (pid == -1)
				return (perror("Fork error"), -1);
			if (pid == 0)
			{
				if (execve(path, cmd->args, envp) == -1)
					return (perror("Exec error"), -1);
				exit(EXIT_SUCCESS);
			}
			else
			{
				if (waitpid(pid, &status, 0) == -1)
					return (perror("Waitpid error"), -1);
				if (WIFEXITED(status))
					*g_returns = WEXITSTATUS(status);
				else if (WIFSIGNALED(status))
					*g_returns = WTERMSIG(status);
				free_matrix(paths);
				return (*g_returns);
			}
		}
	}
	*g_returns = 127;
	free_matrix(paths);
	return (printf("%s: command not found\n", cmd->command), -1);
}

int	run_commands(Command *cmd, char **str, t_env **env, char **envp, int *g_returns)
{
	int		fd[2];
	int		old_fd[2];
	pid_t	pid;
	int		status;
	int		fd_in;
	int		heredoc_fd[2];
	char	*str2;

	old_fd[0] = dup(STDOUT_FILENO);
	if (old_fd[0] == -1)
		return (perror("dup error"), -1);
	old_fd[1] = dup(STDIN_FILENO);
	if (old_fd[1] == -1)
		return (perror("dup error"), close(old_fd[0]), -1);
	if (cmd->heredoc)
	{
		if (pipe(heredoc_fd) == -1)
			return (perror("Error"), close(old_fd[1]), close(old_fd[0]), -1);
		pid = fork();
		if (pid == -1)
			return (perror("Fork error"), close(heredoc_fd[0])
				, close(heredoc_fd[1]), close(old_fd[0]), close(old_fd[1]), -1);
		if (pid == 0)
		{
			close(heredoc_fd[0]);
			while (1)
			{
				str2 = readline("> ");
				if (str2 == NULL)
					return (-1);
				if (strcmp(str2, cmd->heredoc_end) == 0)
				{
					free(str2);
					break ;
				}
				write(heredoc_fd[1], str2, ft_strlen(str2));
				write(heredoc_fd[1], "\n", 1);
				free(str2);
			}
			close(heredoc_fd[1]);
			free_env_list(env);
			free_all_mem();
			exit(EXIT_SUCCESS);
		}
		else
		{
			close(heredoc_fd[1]);
			if (dup2(heredoc_fd[0], STDIN_FILENO) == -1)
				return (perror("Dup2 error"), close(heredoc_fd[0])
					, close(old_fd[0]), close(old_fd[1]), -1);
			close(heredoc_fd[0]);
			waitpid(pid, &status, 0);
		}
	}
	fd[1] = handle_redirection(cmd);
	if (fd[1] == -1)
		return (close(old_fd[0]), close(old_fd[1]), -1);
	if (check_red_in(cmd, &fd_in))
		return (close(old_fd[0]), close(old_fd[1]), -1);
	if (cmd->command)
	{
		if (cmd->next != NULL)
		{
			if (pipe(fd) == -1)
				return (perror("Err"), close(old_fd[0]), close(old_fd[1]), -1);
			pid = fork();
			if (pid == -1)
				return (close(fd[0]), close(fd[1]), perror("Fork error")
					, close(old_fd[0]), close(old_fd[1]), -1);
			if (pid == 0)
			{
				close(fd[0]);
				if (dup2(fd[1], STDOUT_FILENO) == -1)
				{
					perror("Dup2 error");
					exit(EXIT_FAILURE);
				}
				close(fd[1]);
				if (!built_ins(cmd, env, g_returns))
					path_commands(cmd, env, envp, g_returns);
				free_env_list(env);
				free_all_mem();
				exit(EXIT_SUCCESS);
			}
			else
			{
				close(fd[1]);
				pid = fork();
				if (pid == -1)
					return (close(fd[0]), perror("Fork error")
						, close(old_fd[0]), close(old_fd[1]), -1);
				if (pid == 0)
				{
					if (dup2(fd[0], STDIN_FILENO) == -1)
						(perror("Dup2 error"), exit(EXIT_FAILURE));
					close(fd[0]);
					if (cmd->next != NULL)
					{
						run_commands(cmd->next, str, env, envp, g_returns);
						free_env_list(env);
						free_all_mem();
					}
					exit(EXIT_SUCCESS);
				}
				else
				{
					close(fd[0]);
					waitpid(pid, &status, 0);
				}
			}
		}
		else
		{
			if (!built_ins(cmd, env, g_returns))
				path_commands(cmd, env, envp, g_returns);
		}
	}
	while (waitpid(-1, &status, WNOHANG) > 0)
		;
	if (dup2(old_fd[0], STDOUT_FILENO) == -1)
		return (perror("Dup2 error"), close(old_fd[0]), close(old_fd[1]), -1);
	close(old_fd[0]);
	if (dup2(old_fd[1], STDIN_FILENO) == -1)
		return (perror("Dup2 error"), close(old_fd[1]), -1);
	close(old_fd[1]);
	return (1);
}

char	*close_pipe(char *command, int i, int j)
{
	char	*complete;
	char	*temp;
	pid_t	pid;

	i = ft_strlen(command);
	complete = NULL;
	if (command[0] == '|')
		return (printf("minishell: parse error near '|'\n"), NULL);
	j = i - 2;
	if (command[--i] == '|')
	{
		while (command[j] == ' ' || command[j] <= 13)
			j--;
		if (command[j] == '|')
			return (printf("minishell: parse error near '|'\n"), NULL);
		pid = fork();
		if (pid == -1)
			return (perror("Fork failed"), NULL);
		if (pid == 0)
		{
			signal(SIGINT, handle_sigint_child);
			complete = readline("> ");
			if (!complete)
			{
				free(complete);
				printf("minishell: syntax error: unexpected end of file\n");
				exit(1);
			}
			while (complete[0] == '\0')
			{
				free(complete);
				complete = readline("pipe> ");
				if (!complete)
				{
					free(complete);
					printf("minishell: syntax error: unexpected end of file\n");
					exit(1);
				}
			}
			exit(0);
		}
		else
		{
			wait(NULL);
			temp = ft_strjoin(command, " ");
			collect_mem(temp, MEM_CHAR_PTR, 0);
			command = temp;
			temp = ft_strjoin(command, complete);
			collect_mem(temp, MEM_CHAR_PTR, 0);
			command = temp;
			free(complete);
		}
	}
	else
		return (command);
	return (command);
}

char	*expand_variable(char *var, t_env *env, int *g_returns)
{
	char	*result;
	char	*current;
	int		inside_single_quotes;
	char	*exit_status;
	char	*var_end;
	t_env	*env_var;
	char	*var_name;

	result = ft_strdup("");
	current = var;
	inside_single_quotes = 0;
	while (*current)
	{
		if (*current == '\'')
		{
			inside_single_quotes = !inside_single_quotes;
			result = ft_strjoin_free(result, ft_strndup(current, 1));
			current++;
		}
		else if (*current == '$' && !inside_single_quotes)
		{
			current++;
			if (*current == '?')
			{
				exit_status = ft_itoa(*g_returns);
				result = ft_strjoin_free(result, exit_status);
				free(exit_status);
				current++;
			}
			else
			{
				var_end = current;
				while (*var_end && (ft_isalnum(*var_end) || *var_end == '_'))
					var_end++;
				var_name = ft_strndup(current, var_end - current);
				env_var = find_env_var(env, var_name);
				if (env_var)
					result = ft_strjoin_free(result, env_var->value);
				current = var_end;
			}
		}
		else
		{
			result = ft_strjoin_free(result, ft_strndup(current, 1));
			current++;
		}
	}
	return (result);
}


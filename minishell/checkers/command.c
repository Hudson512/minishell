/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:28:57 by hmateque          #+#    #+#             */
/*   Updated: 2024/12/11 11:24:20 by hmateque         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static void	print_command_tree(Command *root)
{
	int		i;
	Command	*current;

	current = root;
	return ;
	while (current != NULL)
	{
		printf("Comando: \033[0;32m%s\033[0m\n", current->command);
		printf("Argumentos:");
		i = -1;
		while (current->args[++i] != NULL)
			printf(" \033[0;34m%s\033[0m", current->args[i]);
		printf("\n");
		if (current->redirect_out != NULL)
			printf("Redirecionamento de saída: \033[0;33m%s\033[0m\n",
				current->redirect_out);
		if (current->redirect_in != NULL)
			printf("Redirecionamento de entrada: \033[0;33m%s\033[0m\n",
				current->redirect_in);
		if (current->next != NULL)
			printf("|\n|\nV\n");
		current = current->next;
	}
}

int	handle_redirection(Command *command_tree)
{
	int	fd;

	fd = dup(STDOUT_FILENO);
	if (command_tree->redirect_out != NULL)
	{
		if (command_tree->redirect_out_type == 1)
			fd = open(command_tree->redirect_out, O_WRONLY
					| O_CREAT | O_TRUNC, 0644);
		else
			fd = open(command_tree->redirect_out, O_WRONLY
					| O_CREAT | O_APPEND, 0644);
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
	return (fd);
}

int	check_red_in(Command *command_tree, int *fd_in)
{
	if (command_tree->redirect_in != NULL)
	{
		if (access(command_tree->redirect_in, R_OK) == -1)
		{
			printf("minishell: %s: No such file or directory\n",
				command_tree->redirect_in);
			return (1);
		}
		*fd_in = open(command_tree->redirect_in, O_RDONLY);
		if (*fd_in == -1)
			return (perror("Open input file error"), 1);
		if (dup2(*fd_in, STDIN_FILENO) == -1)
			return (perror("Dup2 error"), close(*fd_in), 1);
	}
	return (0);
}

int	built_ins(Command *command_tree, t_env **env, int *g_returns)
{
	if (!ft_strcmp(command_tree->command, "echo"))
		return (echo(command_tree->args, g_returns), 1);
	else if (!ft_strcmp(command_tree->command, "cd"))
		return (cd(command_tree->args, g_returns, env), 1);
	else if (!ft_strcmp(command_tree->command, "export"))
		return (ft_export(command_tree->args, env, g_returns), 1);
	else if (!ft_strcmp(command_tree->command, "env"))
		return (ft_env(command_tree->args, g_returns, env), 1);
	else if (!ft_strcmp(command_tree->command, "pwd"))
		return (pwd(command_tree->args, g_returns), 1);
	else if (!ft_strcmp(command_tree->command, "unset"))
		return (ft_unset(command_tree->args, env, g_returns), 1);
	else if (!ft_strcmp(command_tree->command, "exit"))
		return (ft_exit(command_tree, env), 1);
	return (0);
}

int	path_commands(Command *command_tree, t_env **env, char **envp, int *g_returns)
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
	if (access(command_tree->command, X_OK) == 0)
	{
		pid = fork();
		if (pid == -1)
			return (perror("Fork error"), -1);
		if (pid == 0)
		{
			if (execve(command_tree->command, command_tree->args, envp) == -1)
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
		ft_strlcat(path, command_tree->command, sizeof(path));
		if (access(path, X_OK) == 0)
		{
			pid = fork();
			if (pid == -1)
				return (perror("Fork error"), -1);
			if (pid == 0)
			{
				if (execve(path, command_tree->args, envp) == -1)
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
	}
	*g_returns = 127;
	return (printf("%s: command not found\n", command_tree->command), -1);
}

int	run_commands(Command *command_tree, char **str, t_env **env, char **envp, int *g_returns)
{
	int		fd[2];
	int		old_fd[2];
	pid_t	pid;
	int		status;
	int		fd_in;
	int		heredoc_fd[2];

	old_fd[0] = dup(STDOUT_FILENO);
	if (old_fd[0] == -1)
		return (perror("dup error"), -1);
	old_fd[1] = dup(STDIN_FILENO);
	if (old_fd[1] == -1)
		return (perror("dup error"), close(old_fd[0]), -1);
	if (command_tree->heredoc)
	{
		if (pipe(heredoc_fd) == -1)
			return (perror("Pipe error"), close(old_fd[1]), close(old_fd[0]), -1);
		pid = fork();
		if (pid == -1)
			return (perror("Fork error"), close(heredoc_fd[0])
				, close(heredoc_fd[1]), close(old_fd[0]), close(old_fd[1]), -1);
		if (pid == 0) {
			close(heredoc_fd[0]);
			char *str;
			while (1)
			{
				str = readline("> ");
				if (str == NULL)
					return (-1);
				if (strcmp(str, command_tree->heredoc_end) == 0) {
					free(str);
					break;
				}
				write(heredoc_fd[1], str, ft_strlen(str));
				write(heredoc_fd[1], "\n", 1);
				free(str);
			}
			close(heredoc_fd[1]);
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
	fd[1] = handle_redirection(command_tree);
	if (fd[1] == -1) 
		return (close(old_fd[0]), close(old_fd[1]), -1);
	if (check_red_in(command_tree, &fd_in))
		return (close(old_fd[0]), close(old_fd[1]), -1);
	if (command_tree->command)
	{
		if (command_tree->next != NULL) {
			if (pipe(fd) == -1)
				return (perror("Pipe error"), close(old_fd[0]), close(old_fd[1]), -1);
			pid = fork();
			if (pid == -1)
				return (close(fd[0]), close(fd[1]), perror("Fork error")
					, close(old_fd[0]), close(old_fd[1]), -1);
			if (pid == 0)
			{
				close(fd[0]);
				if (dup2(fd[1], STDOUT_FILENO) == -1) {
					perror("Dup2 error");
					exit(EXIT_FAILURE);
				}
				close(fd[1]);
				if (!built_ins(command_tree, env, g_returns))
					path_commands(command_tree, env, envp, g_returns);
				exit(EXIT_SUCCESS);
			} else {
				close(fd[1]);
				pid = fork();
				if (pid == -1)
					return (close(fd[0]), perror("Fork error")
						, close(old_fd[0]), close(old_fd[1]), -1);
				if (pid == 0)
				{
					if (dup2(fd[0], STDIN_FILENO) == -1) {
						perror("Dup2 error");
						exit(EXIT_FAILURE);
					}
					close(fd[0]);
					if (command_tree->next != NULL)
						run_commands(command_tree->next, str, env, envp, g_returns);
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
			if (!built_ins(command_tree, env, g_returns))
				path_commands(command_tree, env, envp, g_returns);
		}
	}
	while (waitpid(-1, &status, WNOHANG) > 0);
	if (dup2(old_fd[0], STDOUT_FILENO) == -1)
		return (perror("Dup2 error"), close(old_fd[0]), close(old_fd[1]), -1);
	close(old_fd[0]);
	if (dup2(old_fd[1], STDIN_FILENO) == -1)
		return (perror("Dup2 error"), close(old_fd[1]), -1);
	close(old_fd[1]);
	return (1);
}

char	*close_pipe(char *command)
{
	int		i;
	char	*complete;

	i = 0;
	complete = NULL;
	while (command[i])
		i++;
	if (command[0] == '|')
		return (printf("minishell: parse error near '|'\n"), NULL);
	if (command[--i] == '|')
	{
		complete = readline("pipe> ");
		if (!complete)
		{
			printf("minishell: syntax error: unexpected end of file\n");
			ft_exit(NULL, NULL);
		}
		while (complete[0] == '\0')
		{
			if (command[0] == '\0')
				printf("KO\n");
			free(complete);
			complete = readline("pipe> ");
		}
	}
	else
		return (command);
	command = ft_strjoin(command, " ");
	command = ft_strjoin(command, complete);
	free(complete);
	return (command);
}

char	*expand_variable(char *var, t_env *env, int *g_returns)
{
	char	*name;
	char	*fim;
	char	*new_str;

	if (ft_strcmp(var, "$?") == 0)
		return ft_itoa(*g_returns);
	if (var[0] == '$')
	{
		name = ft_strdup(var + 1);
		while (env)
		{
			if (!ft_strncmp(env->name, name, ft_strlen(env->name)))
			{
				fim = var + (ft_strlen(env->name) + 1);
				new_str = ft_strjoin(env->value, fim);
				free(name);
				free(var);
				return (new_str);
			}
			env = env->next;
		}
		free(name);
		return (ft_strdup(""));
	}
	return (ft_strdup(var));
}

char **expander(char **str, t_env *env, int *g_returns, int wordcount)
{
	int		i;
	char	*expanded;
	
	i = -1;
	while (++i < wordcount)
	{
		expanded = expand_variable(str[i], env, g_returns);
		if (expanded)
		{
			free(str[i]);
			str[i] = expanded;
		}
	}
	return (str);
}

void	create_files(char **str, int wordcount)
{
	int	i;
	int	fd;

	i = -1;
	while (++i < wordcount)
	{
		if (!ft_strcmp(str[i], ">>"))
		{
			if (str[i + 1])
			{
				fd = open(str[++i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd != -1)
					close(fd);
			}
		}
		else if (!ft_strcmp(str[i], ">"))
		{
			if (str[i + 1])
			{
				fd = open(str[++i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd != -1)
					close(fd);
			}
		}
	}
}

void	identify_command(char *command, t_env **env, char **envp, int *g_returns)
{
	Token	**classified_tokens;
	char	**str;
	int		word_count;
	Command	*command_tree;

	str = NULL;
	command = close_pipe(command);
	command = trim_spaces(command);
	if (!command)
		return ;
	str = ft_tokens(command, &word_count);
	classified_tokens = classify_tokens(str, word_count, env, g_returns);
	if (!classified_tokens)
		return ;
	command_tree = build_command_tree(classified_tokens, word_count);
	create_files(str, word_count);
	print_command_tree(command_tree);
	if (command_tree)
		run_commands(command_tree, str, env, envp, g_returns);
	free_matrix_tokens(str, word_count);
}

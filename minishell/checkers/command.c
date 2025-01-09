/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:28:57 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/09 18:19:14 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static void	print_cmd(t_cmd *root)
{
	int		i;
	t_cmd	*current;

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

int	handle_redirection(t_cmd *cmd)
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

int	check_red_in(t_cmd *cmd, int *fd_in)
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

int	built_ins(t_cmd *cmd, t_env **env, int *g_returns)
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
		return (ft_exit(cmd->args[1], env, 1), 1);
	return (0);
}

typedef struct s_path_cmd
{
	char		path[1024];
	char		**paths;
	pid_t		pid;
	int			status;
	t_env		*env_copy;
}	t_path_cmd;

static int	execute_direct_command(t_cmd *cmd, char **envp, int *g_returns)
{
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid == -1)
		return (perror("Fork error"), -1);
	if (pid == 0)
	{
		if (execve(cmd->command, cmd->args, envp) == -1)
			return (perror("Exec error"), -1);
		exit(EXIT_SUCCESS);
	}
	if (waitpid(pid, &status, 0) == -1)
		return (perror("Waitpid error"), -1);
	if (WIFEXITED(status))
		*g_returns = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		*g_returns = WTERMSIG(status);
	return (*g_returns);
}

static t_env	*find_path_env(t_env *env)
{
	while (env)
	{
		if (!ft_strcmp(env->name, "PATH"))
			return (env);
		env = env->next;
	}
	return (NULL);
}

static void	construct_path(t_path_cmd *vars, t_cmd *cmd, int i)
{
	ft_strlcpy(vars->path, vars->paths[i], sizeof(vars->path));
	ft_strlcat(vars->path, "/", sizeof(vars->path));
	ft_strlcat(vars->path, cmd->command, sizeof(vars->path));
}

int	wait_and_set_return(t_path_cmd *vars, int *ret)
{
	if (waitpid(vars->pid, &vars->status, 0) == -1)
		return (perror("Waitpid error"), -1);
	if (WIFEXITED(vars->status))
		*ret = WEXITSTATUS(vars->status);
	else if (WIFSIGNALED(vars->status))
		*ret = WTERMSIG(vars->status);
	return (0);
}

static int	fork_and_execute(t_path_cmd *vars,
	t_cmd *cmd, char **envp, int *ret)
{
	vars->pid = fork();
	if (vars->pid == -1)
		return (perror("Fork error"), -1);
	if (vars->pid == 0)
	{
		if (execve(vars->path, cmd->args, envp) == -1)
			return (perror("Exec error"), -1);
		exit(EXIT_SUCCESS);
	}
	return (wait_and_set_return(vars, ret));
}

static int	try_run(t_path_cmd *vars, t_cmd *cmd, char **envp, int *ret)
{
	int	i;
	int	result;

	i = -1;
	while (vars->paths[++i] != NULL)
	{
		construct_path(vars, cmd, i);
		if (access(vars->path, X_OK) == 0)
		{
			result = fork_and_execute(vars, cmd, envp, ret);
			if (result != 0)
				return (result);
			return (1);
		}
	}
	return (0);
}

int	path_commands(t_cmd *cmd, t_env **env, char **envp, int *g_returns)
{
	t_path_cmd	cmd_vars;

	if (!*env)
		return (printf("Env Error\n"), -1);
	if (access(cmd->command, X_OK) == 0)
		return (execute_direct_command(cmd, envp, g_returns));
	if (cmd->command[0] == '/')
		return (printf("minishell: %s: No such file or directory\n",
				cmd->command), -1);
	cmd_vars.env_copy = find_path_env(*env);
	if (!cmd_vars.env_copy || !cmd_vars.env_copy->value
		|| !*cmd_vars.env_copy->value)
		return (printf("t_cmd not found\n"), -1);
	cmd_vars.paths = ft_split(cmd_vars.env_copy->value, ':');
	if (!cmd_vars.paths)
		return (printf("Memory allocation error\n"), -1);
	if (try_run(&cmd_vars, cmd, envp, g_returns))
	{
		free_matrix(cmd_vars.paths);
		return (*g_returns);
	}
	*g_returns = 127;
	free_matrix(cmd_vars.paths);
	return (printf("%s: command not found\n", cmd->command), -1);
}

int	setup_fds(t_fds *fds)
{
	fds->old_fd[0] = dup(STDOUT_FILENO);
	if (fds->old_fd[0] == -1)
		return (perror("dup error"), -1);
	fds->old_fd[1] = dup(STDIN_FILENO);
	if (fds->old_fd[1] == -1)
		return (perror("dup error"), close(fds->old_fd[0]), -1);
	return (0);
}

int	handle_heredoc_child(t_cmd *cmd, t_fds *fds)
{
	char	*str2;

	close(fds->heredoc_fd[0]);
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
		write(fds->heredoc_fd[1], str2, ft_strlen(str2));
		write(fds->heredoc_fd[1], "\n", 1);
		free(str2);
	}
	close(fds->heredoc_fd[1]);
	exit(EXIT_SUCCESS);
}

int	handle_heredoc_parent(t_fds *fds, int *status)
{
	close(fds->heredoc_fd[1]);
	if (dup2(fds->heredoc_fd[0], STDIN_FILENO) == -1)
		return (perror("Dup2 error"), close(fds->heredoc_fd[0]), -1);
	close(fds->heredoc_fd[0]);
	waitpid(-1, status, 0);
	return (0);
}

int	handle_heredoc(t_cmd *cmd, t_exec_data *data)
{
	pid_t	pid;
	int		status;

	if (pipe(data->fds->heredoc_fd) == -1)
		return (perror("Error"), -1);
	pid = fork();
	if (pid == -1)
		return (perror("Fork error"), close(data->fds->heredoc_fd[0]),
			close(data->fds->heredoc_fd[1]), -1);
	if (pid == 0)
		return (handle_heredoc_child(cmd, data->fds));
	else
		return (handle_heredoc_parent(data->fds, &status));
}

int	handle_redirections(t_cmd *cmd, t_exec_data *data)
{
	data->fds->fd[1] = handle_redirection(cmd);
	if (data->fds->fd[1] == -1)
		return (-1);
	if (check_red_in(cmd, &data->fds->fd_in))
		return (-1);
	return (0);
}

int	handle_piped_command_child(t_cmd *cmd, t_exec_data *data)
{
	close(data->fds->fd[0]);
	if (dup2(data->fds->fd[1], STDOUT_FILENO) == -1)
	{
		perror("Dup2 error");
		exit(EXIT_FAILURE);
	}
	close(data->fds->fd[1]);
	if (!built_ins(cmd, data->env, data->g_returns))
		path_commands(cmd, data->env, data->envp, data->g_returns);
	exit(EXIT_SUCCESS);
}

int	handle_piped_command_parent_child(t_cmd *cmd, t_exec_data *data)
{
	if (dup2(data->fds->fd[0], STDIN_FILENO) == -1)
	{
		perror("Dup2 error");
		exit(EXIT_FAILURE);
	}
	close(data->fds->fd[0]);
	if (cmd->next != NULL)
	{
		run_commands(cmd->next, data->str, data->env, data->envp);
		free_all_mem();
	}
	exit(EXIT_SUCCESS);
}

int	handle_piped_command_parent(t_cmd *cmd, t_exec_data *data)
{
	pid_t	pid;
	int		status;

	close(data->fds->fd[1]);
	pid = fork();
	if (pid == -1)
		return (perror("Fork error"), -1);
	if (pid == 0)
		return (handle_piped_command_parent_child(cmd, data));
	else
	{
		close(data->fds->fd[0]);
		waitpid(pid, &status, 0);
	}
	return (0);
}

int	handle_piped_command(t_cmd *cmd, t_exec_data *data)
{
	pid_t	pid;

	if (pipe(data->fds->fd) == -1)
		return (perror("Err"), -1);
	pid = fork();
	if (pid == -1)
		return (perror("Fork error"), -1);
	if (pid == 0)
		return (handle_piped_command_child(cmd, data));
	else
		return (handle_piped_command_parent(cmd, data));
}

int	cleanup_fds(t_fds *fds)
{
	int	status;

	while (waitpid(-1, &status, WNOHANG) > 0)
		;
	if (dup2(fds->old_fd[0], STDOUT_FILENO) == -1)
		return (perror("Dup2 error"), -1);
	close(fds->old_fd[0]);
	if (dup2(fds->old_fd[1], STDIN_FILENO) == -1)
		return (perror("Dup2 error"), -1);
	close(fds->old_fd[1]);
	return (1);
}

int	setup_command_fds(t_fds *fds)
{
	return (setup_fds(fds));
}

int	handle_heredoc_and_redirections(t_cmd *cmd, t_exec_data *data)
{
	if (cmd->heredoc && handle_heredoc(cmd, data) == -1)
		return (-1);
	if (handle_redirections(cmd, data) == -1)
		return (-1);
	return (0);
}

int	execute_command(t_cmd *cmd, t_exec_data *data)
{
	if (cmd->command)
	{
		if (cmd->next != NULL)
		{
			return (handle_piped_command(cmd, data));
		}
		else
		{
			if (!built_ins(cmd, data->env, data->g_returns))
				path_commands(cmd, data->env, data->envp, data->g_returns);
		}
	}
	return (0);
}

int	run_commands(t_cmd *cmd, char **str, t_env **env, char **envp)
{
	t_fds		fds;
	t_exec_data	data;

	data.str = str;
	data.env = env;
	data.envp = envp;
	data.g_returns = &g_return;
	data.fds = &fds;
	if (setup_command_fds(&fds) == -1)
		return (-1);
	if (handle_heredoc_and_redirections(cmd, &data) == -1)
		return (-1);
	if (execute_command(cmd, &data) == -1)
		return (-1);
	return (cleanup_fds(&fds));
}

void	handle_sigint_child(int sig)
{
	(void)sig;
	exit(0);
}

char	*handle_pipe_end(char *command)
{
	pid_t	pid;
	char	*complete;
	char	*temp;

	pid = fork();
	if (pid == -1)
		return (perror("Fork failed"), NULL);
	if (pid == 0)
		return (handle_child_process());
	wait(NULL);
	complete = handle_parent_process();
	temp = ft_strjoin(command, " ");
	collect_mem(temp, MEM_CHAR_PTR, 0);
	command = temp;
	temp = ft_strjoin(command, complete);
	collect_mem(temp, MEM_CHAR_PTR, 0);
	command = temp;
	free(complete);
	return (command);
}

char	*handle_parent_process(void)
{
	char	*complete;

	complete = readline("> ");
	if (!complete)
	{
		printf("minishell: syntax error: unexpected end of file\n");
		return (NULL);
	}
	while (complete[0] == '\0')
	{
		free(complete);
		complete = readline("pipe> ");
		if (!complete)
		{
			printf("minishell: syntax error: unexpected end of file\n");
			return (NULL);
		}
	}
	return (complete);
}

char	*handle_child_process(void)
{
	char	*complete;

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

char	*close_pipe(char *command, int i, int j)
{
	if (command[0] == '|')
		return (printf("minishell: parse error near '|'\n"), NULL);
	i = ft_strlen(command) - 1;
	j = i - 1;
	if (command[i] != '|')
		return (command);
	while (j >= 0 && (command[j] == ' ' || command[j] <= 13))
		j--;
	if (j >= 0 && command[j] == '|')
		return (printf("minishell: parse error near '|'\n"), NULL);
	return (handle_pipe_end(command));
}

void	create_files(t_cmd *command)
{
	int		fd;
	t_cmd	*cmd;

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

void	identify_command(char *line, t_env **env, char **envp, int *g_returns)
{
	t_token	**classified_tokens;
	char	**str;
	int		word_count;
	t_cmd	*cmd;

	if (check_command(line, g_returns, check_quote_syntax(line)))
		return ;
	line = trim_spaces(line, -1);
	if (check_command(line, g_returns, 2))
		return ;
	line = close_pipe(line, 0, 0);
	signal(SIGINT, signal_new_line_2);
	if (check_command(line, g_returns, 3))
		return ;
	str = ft_tokens(line, &word_count);
	str = expander(str, *env, g_returns, word_count);
	if (!str)
		return ;
	classified_tokens = classify_tokens(str, word_count, env, g_returns);
	if (!classified_tokens)
		return ;
	cmd = build_cmd(classified_tokens, word_count);
	(create_files(cmd), print_cmd(cmd));
	if (cmd)
		run_commands(cmd, str, env, envp);
}

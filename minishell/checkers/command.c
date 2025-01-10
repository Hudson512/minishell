/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:28:57 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 17:41:39 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

static void	print_cmd(Command *root)
{
	int		i;
	Command	*current;

	return ;
	current = root;
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

void	handle_sigint_child(int sig)
{
	(void)sig;
	exit(0);
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

char	**expander(char **str, t_env *env, int *g_returns, int wordcount)
{
	int		i;
	char	*expanded;
	char	*temp;

	i = -1;
	if (!str)
		return (NULL);
	while (++i < wordcount)
	{
		if (str[i][1] == '|' || str[i][1] == '>' || str[i][1] == '<' || str[i][1] == '$')
			continue ;
		else if (str[i][0] == '\'' && str[i][ft_strlen(str[i]) - 1] == '\'')
		{
			temp = ft_strndup(str[i] + 1, ft_strlen(str[i]) - 2);
			str[i] = temp;
		}
		else if (str[i][0] == '"' && str[i][ft_strlen(str[i]) - 1] == '"')
		{
			temp = ft_strndup(str[i] + 1, ft_strlen(str[i]) - 2);
			expanded = expand_variable(temp, env, g_returns);
			collect_mem(expanded, MEM_CHAR_PTR, 0);
			str[i] = expanded;
		}
		else
		{
			expanded = expand_variable(str[i], env, g_returns);
			if (avoid_double_quote_error(expanded))
			{
				temp = remove_double_quotes(expanded);
				collect_mem(temp, MEM_CHAR_PTR, 0);
				free(expanded);
				str[i] = temp;
			}
			else if (avoid_single_quote_error(expanded))
			{	
				temp = remove_single_quotes(expanded);
				collect_mem(temp, MEM_CHAR_PTR, 0);
				free(expanded);
				str[i] = temp;
			}
			else
			{
				collect_mem(expanded, MEM_CHAR_PTR, 0);
				str[i] = expanded;
			}
		}
	}
	return (str);
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

//Cria uma funcao para retornar -1 em caso da string ser igual a "" ou ''
int	check_quote_syntax_return(char *line)
{
	if (ft_strcmp(line, "\"\"") == 0 || ft_strcmp(line, "''") == 0)
	{
		free(line);
		return (1);
	}
	return (0);
}

int	identify_command(char *line, t_env **env, char **envp, int *g_returns)
{
	Token	**classified_tokens;
	char	**str;
	int		word_count;
	Command	*cmd;

	str = NULL;
	if (check_command(line, g_returns, check_quote_syntax(line)))
		return (-1);
	line = trim_spaces(line, -1);
	if (check_command(line, g_returns, 2) || check_quote_syntax_return(line))
		return (-1);
	line = close_pipe(line, 0, 0);
	signal(SIGINT, signal_new_line_2);
	if (check_command(line, g_returns, 3))
		return (-1);
	str = ft_tokens(line, &word_count);
	str = expander(str, *env, g_returns, word_count);
	if (!str)
		return (1);
	classified_tokens = classify_tokens(str, word_count, env, g_returns);
	if (!classified_tokens)
		return (1);
	cmd = build_cmd(classified_tokens, word_count);
	create_files(cmd);
	print_cmd(cmd);
	if (cmd)
		run_commands(cmd, str, env, envp, g_returns);
	return (1);
}

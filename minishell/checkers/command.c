/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:28:57 by hmateque          #+#    #+#             */
/*   Updated: 2024/12/15 08:29:42 by hmateque         ###   ########.fr       */
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
	int	saved_stdout;

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout == -1)
		return (perror("Error duplicating STDOUT"), -1);
	if (command_tree->redirect_out)
	{
		if (command_tree->redirect_out_type == 1)
			fd = open(command_tree->redirect_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else if (command_tree->redirect_out_type == 2)
			fd = open(command_tree->redirect_out, O_WRONLY | O_CREAT | O_APPEND, 0644);
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
			collect_mem(temp);
			command = temp;
			temp = ft_strjoin(command, complete);
			collect_mem(temp);
			command = temp;
            free(complete);
        }
    }
    else
        return (command);
    return (command);
}



char *expand_variable(char *var, t_env *env, int *g_returns)
{
    char *result = ft_strdup("");
    char *current = var;
    int inside_single_quotes = 0;

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
                char *exit_status = ft_itoa(*g_returns);
                result = ft_strjoin_free(result, exit_status);
                free(exit_status);
                current++;
            }
            else
            {
                char *var_end = current;
                while (*var_end && (ft_isalnum(*var_end) || *var_end == '_'))
                    var_end++;
                
                char *var_name = ft_strndup(current, var_end - current);
                t_env *env_var = find_env_var(env, var_name);
                if (env_var)
                    result = ft_strjoin_free(result, env_var->value);
                free(var_name);
                current = var_end;
            }
        }
        else
        {
            result = ft_strjoin_free(result, ft_strndup(current, 1));
            current++;
        }
    }

    return result;
}

char *remove_single_quotes(char *str)
{
    char *result = malloc(strlen(str) + 1);
    int i = 0, j = 0;

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

char *remove_double_quotes(char *str)
{
    char *result = malloc(strlen(str) + 1);
    int i = 0, j = 0;

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

int avoid_single_quote_error(char *str)
{
    int i;
    int quote_count;

    i = 0;
    quote_count = 0;
    while (str[i])
    {
        if (str[i] == '\'')
            quote_count++;
        i++;
    }
    
    if (quote_count > 1)
        return 1;
    return 0;
}

int avoid_double_quote_error(char *str)
{
    int i;
    int quote_count;

    i = 0;
    quote_count = 0;
    while (str[i])
    {
        if (str[i] == '"')
            quote_count++;
        i++;
    }
    
    if (quote_count > 1)
        return 1;
    return 0;
}

char **expander(char **str, t_env *env, int *g_returns, int wordcount)
{
	int i;
	char *expanded;
	char *temp;

	i = -1;
	if (!str)
		return (NULL);
	while (++i < wordcount)
	{
		if (str[i][1] == '|' || str[i][1] == '>' || str[i][1] == '<' || str[i][1] == '$')
			continue;
		else if (str[i][0] == '\'' && str[i][ft_strlen(str[i]) - 1] == '\'')
		{
			// Remove as aspas simples
			temp = ft_strndup(str[i] + 1, ft_strlen(str[i]) - 2);
			collect_mem(temp);
			free(str[i]);
			str[i] = temp;
		}
		else if (str[i][0] == '"' && str[i][ft_strlen(str[i]) - 1] == '"')
		{
			// Remove as aspas duplas e expande as variáveis
			temp = ft_strndup(str[i] + 1, ft_strlen(str[i]) - 2);
			expanded = expand_variable(temp, env, g_returns);
			collect_mem(expanded);
			free(temp);
			free(str[i]);
			str[i] = expanded;
		}
		else
		{
			// Expande as variáveis normalmente
			expanded = expand_variable(str[i], env, g_returns);
			if (avoid_double_quote_error(expanded))
    		{
				temp = remove_double_quotes(expanded);
				collect_mem(temp);
				free(expanded);
        		free(str[i]);
        		str[i] = temp;
    		}
			else if (avoid_single_quote_error(expanded))
			{	
				temp = remove_single_quotes(expanded);
				collect_mem(temp);
				free(expanded);
            	free(str[i]);
            	str[i] = temp;
			}
			else
			{
			 	collect_mem(expanded);
        	 	free(str[i]);
        		str[i] = expanded;
			}
		}
	}
	return str;
}
void create_files(Command *command_tree) {
	int		fd;
	Command	*command;

	command = command_tree;
	while (command)
	{
		if (command->redirect_out)
		{
			if (command->redirect_out_type == 1)
				fd = open(command->redirect_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			else if (command->redirect_out_type == 2)
				fd = open(command->redirect_out, O_WRONLY | O_CREAT | O_APPEND, 0644);
			else
				fd = -1;
			if (fd != -1)
				close(fd);
		}
		command = command->next;
	}
}

void print_classified_tokens(Token **tokens) {
	for (int i = 0; tokens[i] != NULL; i++) {
		printf("Token %d: Value = '%s', Type = ", i, tokens[i]->value);
		switch (tokens[i]->type) {
			case TOKEN_ARG: printf("ARG\n"); break;
			case TOKEN_PIPE: printf("PIPE\n"); break;
			case TOKEN_APPEND_OUT: printf("APPEND_OUT\n"); break;
			case TOKEN_REDIRECT_OUT: printf("REDIRECT_OUT\n"); break;
			case TOKEN_HEREDOC: printf("HEREDOC\n"); break;
			case TOKEN_REDIRECT_IN: printf("REDIRECT_IN\n"); break;
			case TOKEN_COMMAND: printf("COMMAND\n"); break;
			default: printf("UNKNOWN\n");
		}
	}
	printf("\n");
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

void	identify_command(char *command, t_env **env, char **envp, int *g_returns)
{
	Token	**classified_tokens;
	char	**str;
	int		word_count;
	Command	*command_tree;

	str = NULL;
	if (check_command(command , g_returns, check_quote_syntax(command)))
		return ;
	command = trim_spaces(command, -1);
	if (check_command(command, g_returns, 2))
		return ;
	command = close_pipe(command, 0, 0);
	signal(SIGINT, signal_new_line_2);
	if (check_command(command, g_returns, 3))
		return ;
	str = ft_tokens(command, &word_count);
	str = expander(str, *env, g_returns, word_count);
	if (!str)
		return ;
	classified_tokens = classify_tokens(str, word_count, env, g_returns);
	if (!classified_tokens)
		return ;
	//print_classified_tokens(classified_tokens);
	command_tree = build_command_tree(classified_tokens, word_count);
	create_files(command_tree);
	print_command_tree(command_tree);
	if (command_tree)
		run_commands(command_tree, str, env, envp, g_returns);
	//free_matrix_tokens(str, word_count);
}

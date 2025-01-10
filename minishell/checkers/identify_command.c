/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   identify_command.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 15:29:53 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 15:58:34 by hmateque         ###   ########.fr       */
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

static int	prepare_command_tokens(char *line, t_env **env, int *g_returns,
		char ***str)
{
	int	word_count;

	if (check_command(line, g_returns, check_quote_syntax(line)))
		return (-1);
	line = trim_spaces(line, -1);
	if (check_command(line, g_returns, 2) || check_quote_syntax_return(line))
		return (-1);
	line = close_pipe(line, 0, 0);
	signal(SIGINT, signal_new_line_2);
	if (check_command(line, g_returns, 3))
		return (-1);
	*str = ft_tokens(line, &word_count);
	*str = expander(*str, *env, g_returns, word_count);
	if (!*str)
		return (1);
	return (word_count);
}

int	identify_command(char *line, t_env **env, char **envp, int *g_returns)
{
	Token	**classified_tokens;
	char	**str;
	int		word_count;
	Command	*cmd;

	str = NULL;
	word_count = prepare_command_tokens(line, env, g_returns, &str);
	if (word_count <= 0)
		return (word_count);
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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 12:32:54 by lantonio          #+#    #+#             */
/*   Updated: 2025/01/10 15:36:50 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./includes/minishell.h"

int	g_return ;

void	collect_mem(void *ptr, t_mem_type type, size_t size)
{
	t_memory	*mem;

	if (!ptr)
		return ;
	mem = malloc(sizeof(t_memory));
	if (!mem)
		exit(ENOMEM);
	mem->ptr = ptr;
	mem->type = type;
	mem->size = size;
	ft_lstadd_back(get_mem_address(), ft_lstnew(mem));
}

void	signal_new_line(int signum)
{
	(void)signum;
	g_return = 130;
	printf("\n");
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

void	signal_new_line_2(int signum)
{
	(void)signum;
	printf("\n");
}

static char	*int_read(t_env **env)
{
	char	*input;

	configure_signal();
	input = readline("minishell$ ");
	if (!input)
	{
		ft_exit(env, 0);
	}
	return (input);
}

int	main(int ac, char **av, char **env)
{
	char	*command;
	t_env	*all_env;
	int		status;

	all_env = NULL;
	ft_set_value(ac, av, env, &all_env);
	while (1)
	{
		command = int_read(&all_env);
		if (!ft_strlen(command))
			continue ;
		add_history(command);
		signal(SIGINT, signal_new_line_2);
		status = identify_command(command, &all_env, env, &g_return);
		signal(SIGINT, signal_new_line);
		if (status != -1)
			free_all_mem();
	}
	rl_clear_history();
	return (0);
}

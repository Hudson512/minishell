/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/19 08:23:31 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/09 10:56:44 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

void	free_env_list(t_env **env)
{
	t_env	*current;
	t_env	*next;

	if (!env || !*env)
		return ;
	current = *env;
	while (current)
	{
		next = current->next;
		free(current->name);
		free(current->value);
		free(current);
		current = next;
	}
	*env = NULL;
}

int	is_numeric(const char *str)
{
	if (*str == '-' || *str == '+')
		str++;
	while (*str)
	{
		if (!isdigit(*str))
			return (0);
		str++;
	}
	return (1);
}

void	return_value(char *exit_status)
{
	long	ret;

	if (exit_status[0] == '\0')
	{
		free(exit_status);
		exit(0);
	}
	if (!is_numeric(exit_status))
	{
		printf("minsihell: exit: %s: numeric argument required\n", exit_status);
		free(exit_status);
		exit(2);
	}
	ret = ft_atoi(exit_status);
	if (ret < 0 || ret > 255)
	{
		free(exit_status);
		exit(ret % 256);
	}
	free(exit_status);
	exit((int)ret);
}

void	ft_exit(char *exit_status, t_env **env, int status)
{
	char	*ret;

	if (!exit_status)
		ret = ft_strdup("");
	else
		ret = ft_strdup(exit_status);
	free_env_list(env);
	if (status == 1)
		free_all_mem();
	ft_putstr_fd("exit\n", 1);
	return_value(ret);
}

void	free_matrix(char **matrix)
{
	int	i;

	i = 0;
	if (!matrix)
		return ;
	while (matrix[i] != NULL)
	{
		free(matrix[i]);
		i++;
	}
	free(matrix);
}

// void	free_all_mem(void)
// {
// 	t_list	**mem_list;
// 	t_list	*current;
// 	t_list	*next;

// 	mem_list = get_mem_address();
// 	current = *mem_list;
// 	while (current)
// 	{
// 		next = current->next;
// 		if (current->content)
// 		{
// 			void **matrix = (void **)current->content;
// 			size_t i = 0;
// 			if (matrix)
// 			{
// 				while (matrix[i])
// 				{
// 					free(matrix[i]);
// 					i++;
// 				}
// 			}
// 			free(matrix);
// 		}
// 		free(current);
// 		current = next;
// 	}
// 	*mem_list = NULL;
// }
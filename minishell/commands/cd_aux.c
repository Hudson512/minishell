/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd_aux.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 15:31:15 by lantonio          #+#    #+#             */
/*   Updated: 2025/01/10 15:31:49 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

char	*build_env_string(const char *key, const char *value)
{
	char	*str;
	char	*temp;

	str = ft_strdup("export ");
	if (!str)
		return (NULL);
	temp = str;
	str = ft_strjoin(str, key);
	free(temp);
	if (!str)
		return (NULL);
	temp = str;
	str = ft_strjoin(str, "=");
	free(temp);
	if (!str)
		return (NULL);
	temp = str;
	str = ft_strjoin(str, value);
	free(temp);
	return (str);
}

void	execute_env_update(char *str, t_env **env, int *g_returns)
{
	char	**args;

	args = ft_split(str, ' ');
	if (!args)
		return ;
	ft_export(args, env, g_returns);
	free_matrix(args);
}

void	update_env_variable(char *key, char *value, t_env **env, int *g_returns)
{
	char	*str;

	str = build_env_string(key, value);
	if (!str)
		return ;
	execute_env_update(str, env, g_returns);
	free(str);
}

void	update_oldpwd_pwd(char *old_path, t_env **env, int *g_returns)
{
	char	new_path[PATH_MAX];

	update_env_variable("OLDPWD", old_path, env, g_returns);
	if (getcwd(new_path, sizeof(new_path)) == NULL)
		return ;
	update_env_variable("PWD", new_path, env, g_returns);
}

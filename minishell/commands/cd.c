/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 10:41:02 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/10 15:31:46 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

void				update_oldpwd_pwd(char *old_path, t_env **env, int *g_returns);

int	navigate_to_directory(char *path, t_env **env, int *g_returns)
{
	char	old_path[PATH_MAX];

	if (getcwd(old_path, sizeof(old_path)) == NULL)
		return (perror("Erro ao obter diretório atual"), 0);
	if (chdir(path) != 0)
		return (perror("Erro ao mudar de diretório"), 0);
	update_oldpwd_pwd(old_path, env, g_returns);
	return (1);
}

t_env	*get_home_environment(t_env *env)
{
	while (env)
	{
		if (!ft_strcmp(env->name, "HOME"))
			return (env);
		env = env->next;
	}
	return (NULL);
}

int	handle_cd_no_args(t_env **env, int *g_returns)
{
	t_env	*home_env;

	home_env = get_home_environment(*env);
	if (!home_env || !home_env->value || !*home_env->value)
		return (printf("minishell: cd: HOME not set\n"), 0);
	return (navigate_to_directory(home_env->value, env, g_returns));
}

int	handle_cd_with_args(char **args, t_env **env, int *g_returns)
{
	if (matrix_len(args) > 2)
		return (printf("minishell: cd: too many arguments\n"), 0);
	return (navigate_to_directory(args[1], env, g_returns));
}

int	cd(char **args, int *g_returns, t_env **env)
{
	if (matrix_len(args) == 0)
		return (handle_cd_no_args(env, g_returns));
	return (handle_cd_with_args(args, env, g_returns));
}

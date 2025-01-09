/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 08:22:00 by lantonio          #+#    #+#             */
/*   Updated: 2025/01/09 10:53:56 by lantonio         ###   ########.fr       */
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

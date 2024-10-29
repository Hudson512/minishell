/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:10:13 by lantonio          #+#    #+#             */
/*   Updated: 2024/10/29 14:47:55 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

void	pwd(char **str)
{
	int		fd;
	int		vet_len;
	char	path[PATH_MAX];

	fd = 1;
	vet_len = 0;
	while (str[vet_len])
		vet_len++;
	if (vet_len > 2)
	{
		if (isset_in_mat(str, ">"))
			fd = open(str[isset_in_mat(str, ">") + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		else if (isset_in_mat(str, ">>"))
			fd = open(str[isset_in_mat(str, ">>") + 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	}
	if (vet_len > 3)
	{
		printf("pwd: too many arguments\n");
		return ;
	}
	if (!isset_in_mat(str, "<<"))
	{
		if (getcwd(path, sizeof(path)) != NULL)
			ft_putstr_fd(path, fd);
		else
			perror("Erro ao obter o diretório");
	}
	ft_putstr_fd("\n", fd);
	if (fd != 1)
		close(fd);
}

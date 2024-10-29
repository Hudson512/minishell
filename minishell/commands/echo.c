/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:52:12 by lantonio          #+#    #+#             */
/*   Updated: 2024/10/29 16:20:31 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

void	echo(char **str)
{
	int	i;
	int	fd;
	int	vet_len;

	i = 0;
	fd = 1;
	vet_len = 0;
	while (str[vet_len])
		vet_len++;
	if (vet_len >= 2)
	{
		if (isset_in_mat(str, ">"))
			fd = open(str[isset_in_mat(str, ">") + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		else if (isset_in_mat(str, ">>"))
			fd = open(str[isset_in_mat(str, ">>") + 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
		if (!ft_strcmp(str[1], "-n"))
			i = 1;
	}
	if (!isset_in_mat(str, ">>") || !isset_in_mat(str, ">"))
	{
		while (str[++i])
		{
			if (!(!ft_strcmp(str[i], ">") || !ft_strcmp(str[i], ">>") || !ft_strcmp(str[i - 1], ">") || !ft_strcmp(str[i - 1], ">>")))
					ft_putstr_fd(str[i], fd);
			if (str[i + 1])
				ft_putstr_fd(" ", fd);
		}
		if (str[1])
			if (!ft_strcmp(str[1], "-n"))
				ft_putstr_fd("\033[30m\033[47m%\033[0m", fd);
	}
	ft_putstr_fd("\n", fd);
	if (fd != 1)
		close(fd);
}

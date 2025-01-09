/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   memory_file2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 12:17:57 by hmateque          #+#    #+#             */
/*   Updated: 2025/01/09 13:15:29 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./includes/minishell.h"

t_list	**get_mem_address(void)
{
	static t_list	*ptr;

	return (&ptr);
}

// Função para coletar memória alocada
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

void	*allocate_mem(size_t nmemb, size_t size)
{
	void	*content;

	content = ft_calloc(nmemb, size);
	if (content == NULL)
		exit(ENOMEM);
	return (content);
}

// Função para liberar toda a memória
void	free_mem_ptr(t_memory *mem)
{
	if (mem->ptr)
	{
		free(mem->ptr);
		mem->ptr = NULL;
	}
}

void	free_mem_node(t_list *node)
{
	t_memory	*mem;

	if (node->content)
	{
		mem = (t_memory *)node->content;
		free_mem_ptr(mem);
		free(mem);
	}
	free(node);
}

void	free_all_mem(void)
{
	t_list	**mem_list;
	t_list	*current;
	t_list	*next;

	return ;
	mem_list = get_mem_address();
	if (!mem_list || !*mem_list)
		return ;
	current = *mem_list;
	while (current)
	{
		next = current->next;
		free_mem_node(current);
		current = next;
	}
	*mem_list = NULL;
}

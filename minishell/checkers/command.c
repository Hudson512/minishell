/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:28:57 by hmateque          #+#    #+#             */
/*   Updated: 2024/10/31 08:17:08 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

char **tokenizar(const char *str, char delimitador) {
    char **tokens = malloc(MAX_TOKENS * sizeof(char *));
    int token_count = 0;
    int inside_quotes = 0;
    char *temp = malloc(ft_strlen(str) + 1);
    int temp_index = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '"')
            inside_quotes = !inside_quotes;  // Alterna o estado de dentro de aspas
        
        if (inside_quotes) {
            // Adiciona caractere à string temporária se estiver dentro de aspas
            temp[temp_index++] = str[i];
        } else {
            // Verifica se o caractere atual é o delimitador
            if (str[i] == delimitador) {
                if (temp_index > 0) {
                    temp[temp_index] = '\0'; // Termina a string temporária
                    tokens[token_count++] = strdup(temp); // Duplica a string para o token
                    temp_index = 0; // Reseta o índice da string temporária
                }
            } else {
                // Adiciona caractere à string temporária
                temp[temp_index++] = str[i];
            }
        }
    }

    // Adiciona o último token se necessário
    if (temp_index > 0) {
        temp[temp_index] = '\0';
        tokens[token_count++] = strdup(temp);
    }

    free(temp); // Libera a memória temporária
    tokens[token_count] = NULL; // Define o final da lista de tokens
    return tokens; // Retorna a matriz de tokens
}

void	identify_command(char *command, t_env **env)
{
	char	**str;

	//str = ft_split(command, ' ');
	str = tokenizar(command, ' ');
	str = remove_quotes(str);
	if (str)
	{
		if (str[0])
		{
			if (!ft_strcmp(str[0], "pwd"))
				pwd(str);
			else if (!ft_strcmp(str[0], "echo"))
				echo(str, *env);
			else if (!ft_strcmp(str[0], "env"))
				print_list(*env);
			else if (!ft_strcmp(str[0], "exit"))
				exit(0);
		}
	}
	else
		printf("Error\n");
}

int	check_cipher(char *str, int fd, t_env *env)
{
	int		i;
	char	*result;
	char	*new_str;

	i = 0;
	result = ft_strchr(str, '$');
	if (result == NULL)
		return (0);
	result++;
	while (result[i] != '\0')
		i++;
	new_str = (char *)malloc(sizeof(char) * (i + 1));
	i = 0;
	while (result[i] != '\0')
	{
		new_str[i] = result[i];
		i++;
	}
	new_str[i] = '\0';
	search_and_print_list(env, new_str, fd);
	free(new_str);
	return (1);
}

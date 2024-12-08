/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 09:39:33 by hmateque          #+#    #+#             */
/*   Updated: 2024/12/04 11:01:26 by hmateque         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

char** ft_tokens(const char* input, int* word_count) {
    int count = 0;
    int in_word = 0;
    int in_quotes = 0;
    char quote_char = '\0';
    
    // Count words
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"' || input[i] == '\'') {
            if (!in_quotes) {
                in_quotes = 1;
                quote_char = input[i];
                if (!in_word) {
                    count++;
                    in_word = 1;
                }
            } else if (input[i] == quote_char) {
                in_quotes = 0;
                quote_char = '\0';
            }
        } else if (isspace(input[i])) {
            if (!in_quotes) {
                in_word = 0;
            }
        } else {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
    }
    
    // Allocate matrix
    char** matrix = (char**)malloc(count * sizeof(char*));
    if (matrix == NULL) {
        *word_count = 0;
        return NULL;
    }
    
    // Extract words
    int word_index = 0;
    in_word = 0;
    in_quotes = 0;
    quote_char = '\0';
    char* word = NULL;
    int word_len = 0;
    int word_capacity = 0;
    
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"' || input[i] == '\'') {
            if (!in_quotes) {
                in_quotes = 1;
                quote_char = input[i];
                if (!in_word) {
                    in_word = 1;
                    word_len = 0;
                    word_capacity = 16; // Initial capacity
                    word = (char*)malloc(word_capacity * sizeof(char));
                    if (word == NULL) {
                        // Handle allocation failure
                        for (int j = 0; j < word_index; j++) {
                            free(matrix[j]);
                        }
                        free(matrix);
                        *word_count = 0;
                        return NULL;
                    }
                }
            } else if (input[i] == quote_char) {
                in_quotes = 0;
                quote_char = '\0';
            } else {
                if (word_len + 1 >= word_capacity) {
                    word_capacity *= 2;
                    char* new_word = (char*)realloc(word, word_capacity * sizeof(char));
                    if (new_word == NULL) {
                        // Handle reallocation failure
                        free(word);
                        for (int j = 0; j < word_index; j++) {
                            free(matrix[j]);
                        }
                        free(matrix);
                        *word_count = 0;
                        return NULL;
                    }
                    word = new_word;
                }
                word[word_len++] = input[i];
            }
        } else if (isspace(input[i])) {
            if (!in_quotes) {
                if (in_word) {
                    word[word_len] = '\0';
                    matrix[word_index] = word;
                    word_index++;
                    in_word = 0;
                    word = NULL;
                    word_len = 0;
                    word_capacity = 0;
                }
            } else {
                if (word_len + 1 >= word_capacity) {
                    word_capacity *= 2;
                    char* new_word = (char*)realloc(word, word_capacity * sizeof(char));
                    if (new_word == NULL) {
                        // Handle reallocation failure
                        free(word);
                        for (int j = 0; j < word_index; j++) {
                            free(matrix[j]);
                        }
                        free(matrix);
                        *word_count = 0;
                        return NULL;
                    }
                    word = new_word;
                }
                word[word_len++] = input[i];
            }
        } else {
            if (!in_word) {
                in_word = 1;
                word_len = 0;
                word_capacity = 16; // Initial capacity
                word = (char*)malloc(word_capacity * sizeof(char));
                if (word == NULL) {
                    // Handle allocation failure
                    for (int j = 0; j < word_index; j++) {
                        free(matrix[j]);
                    }
                    free(matrix);
                    *word_count = 0;
                    return NULL;
                }
            }
            if (word_len + 1 >= word_capacity) {
                word_capacity *= 2;
                char* new_word = (char*)realloc(word, word_capacity * sizeof(char));
                if (new_word == NULL) {
                    // Handle reallocation failure
                    free(word);
                    for (int j = 0; j < word_index; j++) {
                        free(matrix[j]);
                    }
                    free(matrix);
                    *word_count = 0;
                    return NULL;
                }
                word = new_word;
            }
            word[word_len++] = input[i];
        }
    }
    
    // Handle last word
    if (in_word) {
        word[word_len] = '\0';
        matrix[word_index] = word;
    }
    
    *word_count = count;
    return matrix;
}

void free_matrix_tokens(char** matrix, int word_count) 
{
    for (int i = 0; i < word_count; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

TokenType	identify_token(char *token)
{
	if (ft_strncmp(token, "|", 1) == 0)
		return (TOKEN_PIPE);
	else if (ft_strncmp(token, ">>", 2) == 0)
		return (TOKEN_APPEND_OUT);
	else if (ft_strncmp(token, ">", 1) == 0)
		return (TOKEN_REDIRECT_OUT);
	else if (ft_strncmp(token, "<<", 2) == 0)
		return (TOKEN_HEREDOC);
	else if (ft_strncmp(token, "<", 1) == 0)
		return (TOKEN_REDIRECT_IN);
	return (TOKEN_ARG);
}

Token **classify_tokens(char **tokens, int word_count)
{
    Token **classified_tokens;
    int i;

    classified_tokens = malloc((word_count + 1) * sizeof(Token *));
    if (!classified_tokens)
        return NULL;

    for (i = 0; i < word_count; i++)
    {
        classified_tokens[i] = malloc(sizeof(Token));
        if (!classified_tokens[i])
        {
            // Free previously allocated memory
            while (--i >= 0)
                free(classified_tokens[i]);
            free(classified_tokens);
            return NULL;
        }

        classified_tokens[i]->value = tokens[i];
        classified_tokens[i]->type = identify_token(tokens[i]);

        // If the token is an argument and follows a pipe, it's a command
        if (classified_tokens[i]->type == TOKEN_ARG && i > 0 && 
            classified_tokens[i-1]->type == TOKEN_PIPE)
        {
            classified_tokens[i]->type = TOKEN_COMMAND;
        }
        // If it's the first token and an argument, it's a command
        else if (classified_tokens[i]->type == TOKEN_ARG && i == 0)
        {
            classified_tokens[i]->type = TOKEN_COMMAND;
        }
    }

    classified_tokens[i] = NULL;
    return classified_tokens;
}

int	validate_command_tree(Command *root)
{
	Command	*current;

	current = root;
	while (current != NULL)
	{
		if (current->redirect_out != NULL)
		{
			if (access(current->redirect_out, F_OK) == -1)
			{
				printf("Erro: Arquivo %s não encontrado para saída\n",
					current->redirect_out);
				return (0);
			}
		}
		if (current->redirect_in != NULL)
		{
			if (access(current->redirect_in, R_OK) == -1)
			{
				printf("Erro: Arquivo %s não encontrado para entrada\n",
					current->redirect_in);
				return (0);
			}
		}
		if (current->command == NULL || current->args == NULL)
			return (printf("Erro: Comando inválido\n"), 0);
		current = current->next;
	}
	return (1);
}

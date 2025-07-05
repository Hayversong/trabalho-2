#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matriz.h"

// declaração da variável global raiz_do_sistema que ta na matriz.c
extern No *raiz_do_sistema;

int main() {
    char comando[256];
    char arg[256];
    char *caminho_atual_str;

    No *diretorio_atual = ler_arquivo_in_txt("in.txt");
    if (diretorio_atual == NULL) {
        fprintf(stderr, "Erro ao carregar o sistema de arquivos. Encerrando\n");
        return 1;
    }

    printf("bem-vindo ao simulador de sistema de arquivos\n");
    printf("digite 'help' para ver os comandos disponiveis\n");

    while (1) {
        // exibe o prompt com o caminho atual
        caminho_atual_str = obter_caminho_completo(diretorio_atual);
        if (caminho_atual_str) {
            printf("%s> ", caminho_atual_str);
            free(caminho_atual_str); // libera a string alocada por obter_caminho_completo
        } else {
            printf("erro_Caminho> ");
        }

        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            fprintf(stderr, "erro ao ler comando.\n");
            break;
        }

        // Remove o '\n' do final, se existir
        comando[strcspn(comando, "\n")] = 0;

        // Extrai o primeiro token (o comando em si)
        char *cmd_token = strtok(comando, " ");
        char *arg_token = strtok(NULL, ""); // pega o resto da linha como argumento

        // converte o comando para minúsculas para facilitar a comparação
        if (cmd_token != NULL) {
            for (char *p = cmd_token; *p; ++p) *p = tolower(*p);
        }

        if (cmd_token == NULL || strlen(cmd_token) == 0) {
            continue;
        }
            if (strcmp(cmd_token, "cd") == 0) {
            if (arg_token != NULL && strlen(arg_token) > 0) {
                diretorio_atual = comando_cd(diretorio_atual, arg_token);
            } else {
                printf("uso: cd <diretorio>\n");
            }
        } else if (strcmp(cmd_token, "search") == 0) {
            if (arg_token != NULL && strlen(arg_token) > 0) {
                comando_search(raiz_do_sistema, arg_token);
            } else {
                printf("uso: search <arg>\n");
            }
            } else if (strcmp(cmd_token, "rm") == 0) {
            if (arg_token != NULL && strlen(arg_token) > 0) {
                comando_rm(diretorio_atual, arg_token);
            } else {
                printf("uso: rm <alvo>\n");
            }
            }else if (strcmp(cmd_token, "list") == 0) {
                comando_list(diretorio_atual);
            }else if (strcmp(cmd_token, "mkdir") == 0) {
            if(arg_token != NULL && strlen(arg_token) > 0) {
                comando_mkdir(diretorio_atual, arg_token);
            }else {
                printf("uso: mkdir <nome>\n");
            }
            }else if (strcmp(cmd_token, "clear") == 0) {
                comando_clear();
            }else if (strcmp(cmd_token, "help") == 0) {
                comando_help();
            }else if (strcmp(cmd_token, "exit") == 0) {
                comando_exit(raiz_do_sistema);
        }   else {
            printf("comando desconhecido: '%s'. digite 'help' para ver os comandos\n", cmd_token);
        }
    }

    return 0;
}

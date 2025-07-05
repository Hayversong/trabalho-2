#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matriz.h"

// variavel global para manter a referencia ao no raiz da arvore
// e importante ter acesso a raiz para funcoes como 'search' ou 'exit'
No *raiz_do_sistema = NULL;

// funcoes de gerenciamento da arvore
No *criar_no(const char *nome, TipoNo tipo) {
    No *novo_no = (No *)malloc(sizeof(No));
    if (novo_no == NULL) {
        perror("erro ao alocar memoria para o no");
        return NULL;
    }

    novo_no->nome = strdup(nome);
    if (novo_no->nome == NULL) {
        perror("erro ao alocar memoria para o nome do no");
        free(novo_no);
        return NULL;
    }

    novo_no->tipo = tipo;
    novo_no->pai = NULL;
    novo_no->primeiro_filho = NULL;
    novo_no->proximo_irmao = NULL;

    return novo_no;
}

void adicionar_filho(No *pai, No *filho) {
    if (pai == NULL || filho == NULL) {
        fprintf(stderr, "erro: pai ou filho nulo ao adicionar filho.\n");
        return;
    }

    filho->pai = pai;

    if (pai->primeiro_filho == NULL) {
        pai->primeiro_filho = filho;
    } else {
        No *irmao_atual = pai->primeiro_filho;
        while (irmao_atual->proximo_irmao != NULL) {
            irmao_atual = irmao_atual->proximo_irmao;
        }
        irmao_atual->proximo_irmao = filho;
    }
}

void liberar_arvore(No *no) {
    if (no == NULL) return;

    No *filho_atual = no->primeiro_filho;
    while (filho_atual != NULL) {
        No *proximo_filho = filho_atual->proximo_irmao;
        liberar_arvore(filho_atual);
        filho_atual = proximo_filho;
    }

    free(no->nome);
    free(no);
}

// Funcoes auxiliares
No *encontrar_filho(No *pai, const char *nome, TipoNo filtro_tipo) {
    if (pai == NULL || nome == NULL) return NULL;

    No *filho_atual = pai->primeiro_filho;
    while (filho_atual != NULL) {
        if (strcmp(filho_atual->nome, nome) == 0) {
            if (filtro_tipo == TIPO_DIRETORIO || filtro_tipo == TIPO_ARQUIVO) {
                if (filho_atual->tipo == filtro_tipo) {
                    return filho_atual;
                }
            } else {
                return filho_atual;
            }
        }
        filho_atual = filho_atual->proximo_irmao;
    }
    return NULL;
}

No *obter_raiz(No *no) {
    if (no == NULL) return NULL;
    while (no->pai != NULL) {
        no = no->pai;
    }
    return no;
}

char *obter_caminho_completo(No *no) {
    if (no == NULL) return strdup("");

    if (no->pai == NULL) return strdup("/");

    char *caminho_pai = obter_caminho_completo(no->pai);
    if (caminho_pai == NULL) return NULL;

    size_t tamanho_caminho = strlen(caminho_pai) + 1 + strlen(no->nome) + 1;
    char *caminho_completo = (char *)malloc(tamanho_caminho);
    if (caminho_completo == NULL) {
        perror("erro ao alocar memoria para o caminho completo");
        free(caminho_pai);
        return NULL;
    }

    strcpy(caminho_completo, caminho_pai);
    if (strcmp(caminho_completo, "/") != 0) {
        strcat(caminho_completo, "/");
    }

    strcat(caminho_completo, no->nome);
    free(caminho_pai);
    return caminho_completo;
}
// Funcao de leitura de arquivo
No *ler_arquivo_in_txt(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("erro ao abrir o arquivo in.txt");
        return NULL;
    }

    No *raiz = criar_no("/", TIPO_DIRETORIO);
    if (raiz == NULL) {
        fclose(arquivo);
        return NULL;
    }
    raiz_do_sistema = raiz;

    char linha[1024];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        linha[strcspn(linha, "\n")] = 0;

        if (strlen(linha) == 0) continue;

        char *token;
        char *copia_linha = strdup(linha);
        if (copia_linha == NULL) {
            perror("erro ao duplicar linha para parsing");
            liberar_arvore(raiz);
            fclose(arquivo);
            return NULL;
        }

        No *no_atual = raiz;
        char *ptr = copia_linha;
        while ((token = strtok(ptr, "/")) != NULL) {
            ptr = NULL;

            int eh_ultimo_token = 0;
            if (strtok(NULL, "/") == NULL) {
                eh_ultimo_token = 1;
            }
            int possui_extensao = (strchr(token, '.') != NULL && strchr(token, '.') != token);

            No *proximo_no = encontrar_filho(no_atual, token, (TipoNo)-1);

            if (proximo_no == NULL) {
                TipoNo tipo_novo_no;
                if (eh_ultimo_token && possui_extensao) {
                    tipo_novo_no = TIPO_ARQUIVO;
                } else {
                    tipo_novo_no = TIPO_DIRETORIO;
                }

                proximo_no = criar_no(token, tipo_novo_no);
                if (proximo_no == NULL) {
                    fprintf(stderr, "erro: falha ao criar no para '%s'.\n", token);
                    free(copia_linha);
                    liberar_arvore(raiz);
                    fclose(arquivo);
                    return NULL;
                }
                adicionar_filho(no_atual, proximo_no);
            }
            no_atual = proximo_no;
        }
        free(copia_linha);
    }

    fclose(arquivo);
    return raiz;
}

No *comando_cd(No *no_atual, const char *nome_diretorio) {
    if (no_atual == NULL) {
        fprintf(stderr, "erro: No atual invalido.\n");
        return NULL;
    }

    if (strcmp(nome_diretorio, "..") == 0) {
        if (no_atual->pai != NULL) {
            return no_atual->pai;
        } else {
            printf("ja esta na raiz do sistema.\n");
            return no_atual;
        }
    } else if (strcmp(nome_diretorio, "/") == 0) {
        return obter_raiz(no_atual);
    }

    No *diretorio_alvo = encontrar_filho(no_atual, nome_diretorio, TIPO_DIRETORIO);

    if (diretorio_alvo != NULL) {
        return diretorio_alvo;
    } else {
        printf("diretorio '%s' nao encontrado. alternativas:\n", nome_diretorio);
        int encontrou_alternativa = 0;
        No *filho_atual = no_atual->primeiro_filho;
        while (filho_atual != NULL) {
            if (filho_atual->tipo == TIPO_DIRETORIO && strstr(filho_atual->nome, nome_diretorio) == filho_atual->nome) {
                printf(" - %s\n", filho_atual->nome);
                encontrou_alternativa = 1;
            }
            filho_atual = filho_atual->proximo_irmao;
        }
        if (!encontrou_alternativa) {
            printf("nenhuma alternativa encontrada.\n");
        }
        return no_atual;
    }
}

void comando_search(No *raiz, const char *argumento) {
    if (raiz == NULL || argumento == NULL || strlen(argumento) == 0) return;

    static int encontrou = 0;

    if (strcmp(raiz->nome, argumento) == 0) {
        char *caminho = obter_caminho_completo(raiz);
        if (caminho) {
            printf("encontrado: %s\n", caminho);
            free(caminho);
            encontrou = 1;
        }
    }

    No *filho_atual = raiz->primeiro_filho;
    while (filho_atual != NULL) {
        comando_search(filho_atual, argumento);
        filho_atual = filho_atual->proximo_irmao;
    }

    if (raiz == raiz_do_sistema && !encontrou) {
        printf(" '%s' nao encontrado.\n", argumento);
    }
    if (raiz == raiz_do_sistema) {
        encontrou = 0;
    }
}

void comando_rm(No *no_atual, const char *nome_alvo) {
    if (no_atual == NULL || nome_alvo == NULL) {
        fprintf(stderr, "erro: parametros invalidos para rm.\n");
        return;
    }

    No *no_a_remover = NULL;
    No *anterior = NULL;
    No *atual = no_atual->primeiro_filho;

    while (atual != NULL) {
        if (strcmp(atual->nome, nome_alvo) == 0) {
            no_a_remover = atual;
            break;
        }
        anterior = atual;
        atual = atual->proximo_irmao;
    }

    if (no_a_remover == NULL) {
        printf("'%s' nao encontrado na pasta atual.\n", nome_alvo);
        return;
    }

    if (anterior == NULL) {
        no_atual->primeiro_filho = no_a_remover->proximo_irmao;
    } else {
        anterior->proximo_irmao = no_a_remover->proximo_irmao;
    }

    liberar_arvore(no_a_remover);
    printf("'%s' removido com sucesso.\n", nome_alvo);
}

void comando_list(No *no_atual) {
    if (no_atual == NULL) {
        fprintf(stderr, "erro: no atual nulo para listar.\n");
        return;
    }

    if (no_atual->primeiro_filho == NULL) {
        printf("pasta vazia\n");
        return;
    }

    printf("Conteudo de '%s':\n", no_atual->nome);
    No *filho_atual = no_atual->primeiro_filho;
    while (filho_atual != NULL) {
        printf("%s %s\n", (filho_atual->tipo == TIPO_DIRETORIO) ? "[D]" : "[A]", filho_atual->nome);
        filho_atual = filho_atual->proximo_irmao;
    }
}

void comando_exit(No *raiz) {
    printf("encerrando o programa. Liberando memoria...\n");
    liberar_arvore(raiz);// Libera toda a árvore
    printf("memoria liberada. encerrado!\n");
    exit(0);
}

void comando_clear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void comando_mkdir(No *no_atual, char *nome) {
    No *filho = no_atual->primeiro_filho;
    while (filho != NULL) {
        if (strcmp(filho->nome, nome) == 0) {
            printf("mkdir: diretorio '%s' ja existe\n", nome);
            return;
        }
        filho = filho->proximo_irmao;
    }

    No *novo = criar_no(nome, TIPO_DIRETORIO);
    novo->pai = no_atual;

    novo->proximo_irmao = no_atual->primeiro_filho;
    no_atual->primeiro_filho = novo;
}

void comando_help() {
    printf("Comandos disponiveis:\n");
    printf("cd <dir>       - entra no diretorio <dir>\n");
    printf("cd ..          - volta para o diretorio pai\n");
    printf("mkdir <nome>   - cria um novo diretorio\n");
    printf("rm <nome>      - remove arquivo ou diretorio vazio\n");
    printf("list           - lista arquivos e diretorios\n");
    printf("search <nome>  - procura arquivo ou diretorio a partir do atual\n");
    printf("clear          - limpa a tela\n");
    printf("help           - mostra esta mensagem\n");
    printf("exit           - sai do programa\n");
}


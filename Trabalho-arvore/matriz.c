#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matriz.h"

// vari�vel global para manter a refer�ncia ao n� raiz da �rvore
// � importante ter acesso � raiz para fun��es como 'search' ou 'exit'
No *raiz_do_sistema = NULL;

//fun��es de gerenciamento da arvore
No *criar_no(const char *nome, TipoNo tipo) {
    No *novo_no = (No *)malloc(sizeof(No));
    if (novo_no == NULL) {
        perror("erro ao alocar memoria para o n�");
        return NULL;
    }

    novo_no->nome = strdup(nome);
    if (novo_no->nome == NULL) {
        perror("erro ao alocar mem�ria para o nome do n�");
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

    // se o pai n�o tem filhos, o novo filho se torna o primeiro filho
    if (pai->primeiro_filho == NULL) {
        pai->primeiro_filho = filho;
    } else {
        // encontra o �ltimo irm�o na lista de filhos do pai
        No *irmao_atual = pai->primeiro_filho;
        while (irmao_atual->proximo_irmao != NULL) {
            irmao_atual = irmao_atual->proximo_irmao;
        }
        irmao_atual->proximo_irmao = filho; // adiciona o novo filho ao final da lista
    }
}

void liberar_arvore(No *no) {
    if (no == NULL) {
        return;
    }

    // libera os filhos primeiro (recursivamente)
    No *filho_atual = no->primeiro_filho;
    while (filho_atual != NULL) {
        No *proximo_filho = filho_atual->proximo_irmao;
        liberar_arvore(filho_atual);
        filho_atual = proximo_filho;
    }

    // libera o nome do n�
    free(no->nome);
    // libera o pr�prio n�
    free(no);
}

//Fun��es auxiliares
No *encontrar_filho(No *pai, const char *nome, TipoNo filtro_tipo) {
    if (pai == NULL || nome == NULL) {
        return NULL;
    }

    No *filho_atual = pai->primeiro_filho;
    while (filho_atual != NULL) {
        if (strcmp(filho_atual->nome, nome) == 0) {
            // Se n�o h� filtro de tipo ou o tipo corresponde, retorna
            if (filtro_tipo == TIPO_DIRETORIO || filtro_tipo == TIPO_ARQUIVO) { // Assume que TipoNo pode ser apenas 0 ou 1
                 if (filho_atual->tipo == filtro_tipo) {
                    return filho_atual;
                 }
            } else { // n�o h� filtro de tipo, retorna o primeiro que encontrar
                return filho_atual;
            }
        }
        filho_atual = filho_atual->proximo_irmao;
    }
    return NULL; // filho n�o encontrado
}

No *obter_raiz(No *no) {
    if (no == NULL) {
        return NULL;
    }
    while (no->pai != NULL) {
        no = no->pai;
    }
    return no;
}

char *obter_caminho_completo(No *no) {
    if (no == NULL) {
        return strdup(""); // Retorna uma string vazia para n� nulo
    }

    // Caso base: � a raiz
    if (no->pai == NULL) {
        return strdup("/"); // Raiz � representada por "/"
    }

    char *caminho_pai = obter_caminho_completo(no->pai);
    if (caminho_pai == NULL) {
        return NULL; // Erro na recurs�o
    }

    //calcula o tamanho necess�rio para o caminho completo
    size_t tamanho_caminho = strlen(caminho_pai) + 1 + strlen(no->nome) + 1;
    char *caminho_completo = (char *)malloc(tamanho_caminho);
    if (caminho_completo == NULL) {
        perror("erro ao alocar memoria para o caminho completo");
        free(caminho_pai);
        return NULL;
    }

    //concatena o caminho do pai com o nome do n� atual
    strcpy(caminho_completo, caminho_pai);
    if (strcmp(caminho_completo, "/") != 0) { // Evita adicionar "//" se o pai for a raiz
        strcat(caminho_completo, "/");
    } else if (no->pai->pai == NULL && strcmp(no->nome, "/") != 0) { // Se o pai � a raiz e o n� atual n�o � a pr�pria raiz
         // N�o faz nada, j� tem "/"
    }


    strcat(caminho_completo, no->nome);

    free(caminho_pai);

    return caminho_completo;
}

//Fun��o de Leitura de Arquivo
No *ler_arquivo_in_txt(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("erro ao abrir o arquivo in.txt");
        return NULL;
    }

    // Cria o n� raiz do sistema (e.g., "/")
    No *raiz = criar_no("/", TIPO_DIRETORIO);
    if (raiz == NULL) {
        fclose(arquivo);
        return NULL;
    }
    raiz_do_sistema = raiz; // Define a raiz global

    char linha[1024];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        linha[strcspn(linha, "\n")] = 0;    // Remove o '\n' do final da linha, se existir

        if (strlen(linha) == 0) {
            continue;
        }

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

            // verifica se � o �ltimo token da linha
            // para saber se � um arquivo ou um diret�rio no final
            int eh_ultimo_token = 0;
            if (strtok(NULL, "/") == NULL) {
                eh_ultimo_token = 1;
            }
            // strcspn(token, ".") > 0 verifica se h� um '.' no nome do token,
            // e strcspn(token, ".") < strlen(token) garante que n�o � o primeiro caractere.
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
                    fprintf(stderr, "erro: falha ao criar n� para '%s'.\n", token);
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
        fprintf(stderr, "erro: N� atual invalido.\n");
        return NULL;
    }

    if (strcmp(nome_diretorio, "..") == 0) {
        if (no_atual->pai != NULL) {
            return no_atual->pai;
        } else {
            printf("j� esta na raiz do sistema.\n");
            return no_atual;
        }
    } else if (strcmp(nome_diretorio, "/") == 0) {
        return obter_raiz(no_atual);
    }

    No *diretorio_alvo = encontrar_filho(no_atual, nome_diretorio, TIPO_DIRETORIO);

    if (diretorio_alvo != NULL) {
        return diretorio_alvo;
    } else {
        printf("diret�rio '%s' n�o encontrado. alternativas:\n", nome_diretorio);
        int encontrou_alternativa = 0;
        No *filho_atual = no_atual->primeiro_filho;
        while(filho_atual != NULL) {
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

void comando_search(No *raiz, const char *argumento) {
    if (raiz == NULL || argumento == NULL || strlen(argumento) == 0) {
        return;
    }

    static int encontrou = 0; // Flag para saber se encontrou algo na busca atual

    if (strcmp(raiz->nome, argumento) == 0) {
        char *caminho = obter_caminho_completo(raiz);
        if (caminho) {
            printf("encontrado: %s\n", caminho);
            free(caminho);
            encontrou = 1;
        }
    }

    // Chamada recursiva para os filhos
    No *filho_atual = raiz->primeiro_filho;
    while (filho_atual != NULL) {
        comando_search(filho_atual, argumento);
        filho_atual = filho_atual->proximo_irmao;
    }

    if (raiz == raiz_do_sistema && !encontrou) {
        printf(" '%s' n�o encontrado.\n", argumento);
    }
    if (raiz == raiz_do_sistema) {
        encontrou = 0;
    }
}

void comando_rm(No *no_atual, const char *nome_alvo) {
    if (no_atual == NULL || nome_alvo == NULL) {
        fprintf(stderr, "erro: par�metros inv�lidos para rm.\n");
        return;
    }

    No *no_a_remover = NULL;
    No *anterior = NULL;
    No *atual = no_atual->primeiro_filho;

    // Encontra o n� a ser removido e seu irm�o anterior
    while (atual != NULL) {
        if (strcmp(atual->nome, nome_alvo) == 0) {
            no_a_remover = atual;
            break;
        }
        anterior = atual;
        atual = atual->proximo_irmao;
    }

    if (no_a_remover == NULL) {
        printf("'%s' n�o encontrado na pasta atual.\n", nome_alvo);
        return;
    }

    // Remove o n� da lista de filhos do pai
    if (anterior == NULL) {
        // O n� a ser removido � o primeiro filho
        no_atual->primeiro_filho = no_a_remover->proximo_irmao;
    } else {
        // O n� a ser removido est� no meio ou no final da lista
        anterior->proximo_irmao = no_a_remover->proximo_irmao;
    }

    liberar_arvore(no_a_remover);
    printf("'%s' removido com sucesso.\n", nome_alvo);
}

void comando_list(No *no_atual) {
    if (no_atual == NULL) {
        fprintf(stderr, "erro: n� atual nulo para listar.\n");
        return;
    }

    if (no_atual->primeiro_filho == NULL) {
        printf("pasta vazia\n");
        return;
    }

    printf("Conte�do de '%s':\n", no_atual->nome);
    No *filho_atual = no_atual->primeiro_filho;
    while (filho_atual != NULL) {
        printf("%s %s\n", (filho_atual->tipo == TIPO_DIRETORIO) ? "[DIR]" : "[ARQ]", filho_atual->nome);
        filho_atual = filho_atual->proximo_irmao;
    }
}

void comando_exit(No *raiz) {
    printf("encerrando o programa. Liberando mem�ria...\n");
    liberar_arvore(raiz);// Libera toda a �rvore
    printf("mem�ria liberada. encerrado!\n");
    exit(0);
}

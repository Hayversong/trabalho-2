#ifndef MATRIZ_H
#define MATRIZ_H


typedef enum {
    TIPO_DIRETORIO,
    TIPO_ARQUIVO
} TipoNo;


typedef struct No {
    char *nome;
    TipoNo tipo;
    struct No *pai;         // Ponteiro para o nó pai
    struct No *primeiro_filho; // Ponteiro para o primeiro filho na lista encadeada de filhos
    struct No *proximo_irmao;  // Ponteiro para o próximo irmão na lista encadeada de filhos do mesmo pai
} No;

No *criar_no(const char *nome, TipoNo tipo);
void adicionar_filho(No *pai, No *filho);
void liberar_arvore(No *no);               // Libera recursivamente toda a memória alocada para a árvore a partir de um nó


No *comando_cd(No *no_atual, const char *nome_diretorio); // Implementa o comando 'cd' (entrar em um diretório)
void comando_search(No *raiz, const char *argumento);    // Implementa o comando 'search' (buscar arquivo/pasta)
void comando_rm(No *no_atual, const char *nome_alvo);    // Implementa o comando 'rm' (remover pasta/arquivo)
void comando_list(No *no_atual);                          // Implementa o comando 'list' (listar componentes da pasta atual)
void comando_mkdir(No *no_atual, char *nome);            // Implementa o comando 'mkdir' (criar nova pasta)
void comando_clear();                                     // Implementa o comando 'clear' (limpar a tela)
void comando_help();                                      // Implementa o comando 'help' (exibir ajuda)
void comando_exit(No *raiz);                              // Implementa o comando 'exit' (encerrar o programa e liberar memória)

No *encontrar_filho(No *pai, const char *nome, TipoNo filtro_tipo);
No *obter_raiz(No *no);
char *obter_caminho_completo(No *no);


No *ler_arquivo_in_txt(const char *nome_arquivo); // Lê o arquivo in.txt e monta a árvore inicial

#endif // MATRIZ_H

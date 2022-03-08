/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Organização de Recuperação da Informação
 * Prof. Tiago A. Almeida
 *
 * Trabalho 01 - Indexação
 *
 * RA: 792194
 * Aluno: Lucas Victorio Paiola
 * ========================================================================== */

/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

typedef enum {false, true} bool;

/* Tamanho dos campos dos registros */
/* Campos de tamanho fixo */
#define TAM_ID_USER 12
#define TAM_CELULAR 12
#define TAM_SALDO 14
#define TAM_DATE 9
#define TAM_ID_GAME 9
#define QTD_MAX_CATEGORIAS 3

/* Campos de tamanho variável (tamanho máximo) */
#define TAM_MAX_USER 48
#define TAM_MAX_TITULO 44
#define TAM_MAX_EMPRESA 48
#define TAM_MAX_EMAIL 42
#define TAM_MAX_CATEGORIA 20

#define MAX_REGISTROS 1000
#define TAM_REGISTRO_USUARIO (TAM_ID_USER+TAM_MAX_USER+TAM_MAX_EMAIL+TAM_SALDO+TAM_CELULAR)
#define TAM_REGISTRO_JOGO 256
#define TAM_REGISTRO_COMPRA (TAM_ID_USER+TAM_DATE+TAM_ID_GAME-3)
#define TAM_ARQUIVO_USUARIO (TAM_REGISTRO_USUARIO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGO (TAM_REGISTRO_JOGO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRA (TAM_REGISTRO_COMPRA * MAX_REGISTROS + 1)

#define TAM_RRN_REGISTRO 4
#define TAM_CHAVE_USUARIOS_IDX (TAM_ID_USER + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_JOGOS_IDX (TAM_ID_GAME + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_COMPRAS_IDX (TAM_ID_USER + TAM_ID_GAME + TAM_RRN_REGISTRO - 2)
#define TAM_CHAVE_TITULO_IDX (TAM_MAX_TITULO + TAM_ID_GAME - 2)
#define TAM_CHAVE_DATA_USER_GAME_IDX (TAM_DATE + TAM_ID_USER + TAM_ID_GAME - 3)
#define TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX (TAM_MAX_CATEGORIA - 1)
#define TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX (TAM_ID_GAME - 1)

#define TAM_ARQUIVO_USUARIOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRAS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_CATEGORIAS_IDX (1000 * MAX_REGISTROS + 1)

/* Mensagens padrões */
#define SUCESSO                          "OK\n"
#define REGS_PERCORRIDOS                "Registros percorridos:"
#define AVISO_NENHUM_REGISTRO_ENCONTRADO "AVISO: Nenhum registro encontrado\n"
#define ERRO_OPCAO_INVALIDA              "ERRO: Opcao invalida\n"
#define ERRO_MEMORIA_INSUFICIENTE        "ERRO: Memoria insuficiente\n"
#define ERRO_PK_REPETIDA                 "ERRO: Ja existe um registro com a chave %s\n"
#define ERRO_REGISTRO_NAO_ENCONTRADO     "ERRO: Registro nao encontrado\n"
#define ERRO_SALDO_NAO_SUFICIENTE        "ERRO: Saldo insuficiente\n"
#define ERRO_CATEGORIA_REPETIDA          "ERRO: O jogo %s ja possui a categoria %s\n"
#define ERRO_VALOR_INVALIDO              "ERRO: Valor invalido\n"
#define ERRO_ARQUIVO_VAZIO               "ERRO: Arquivo vazio\n"
#define ERRO_NAO_IMPLEMENTADO            "ERRO: Funcao %s nao implementada\n"

/* Registro de Usuario */
typedef struct {
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    double saldo;
} Usuario;

/* Registro de Jogo */
typedef struct {
    char id_game[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char data_lancamento[TAM_DATE];
    double preco;
    char categorias[QTD_MAX_CATEGORIAS][TAM_MAX_CATEGORIA];
} Jogo;

/* Registro de Compra */
typedef struct {
    char id_user_dono[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
    char data_compra[TAM_DATE];
} Compra;


/*----- Registros dos índices -----*/

/* Struct para o índice primário dos usuários */
typedef struct {
    char id_user[TAM_ID_USER];
    int rrn;
} usuarios_index;

/* Struct para o índice primário dos jogos */
typedef struct {
    char id_game[TAM_ID_GAME];
    int rrn;
} jogos_index;

/* Struct para índice primário dos compras */
typedef struct {
    char id_user[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
    int rrn;
} compras_index;

/* Struct para o índice secundário dos titulos */
typedef struct {
    char titulo[TAM_MAX_TITULO];
    char id_game[TAM_ID_GAME];
} titulos_index;

/* Struct para o índice secundário das datas das compras */
typedef struct {
    char data[TAM_DATE];
    char id_user[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
} data_user_game_index;

/* Struct para o índice secundário das categorias (lista invertida) */
typedef struct {
    char chave_secundaria[TAM_MAX_CATEGORIA];   //string com o nome da categoria
    int primeiro_indice;
} categorias_secundario_index;

/* Struct para o índice primário das categorias (lista invertida) */
typedef struct {
    char chave_primaria[TAM_ID_GAME];   //string com o id do jogo
    int proximo_indice;
} categorias_primario_index;

/* Struct para os parâmetros de uma lista invertida */
typedef struct {
    // Ponteiro para o índice secundário
    categorias_secundario_index *categorias_secundario_idx;

    // Ponteiro para o arquivo de índice primário
    categorias_primario_index *categorias_primario_idx;

    // Quantidade de registros de índice secundário
    unsigned qtd_registros_secundario;

    // Quantidade de registros de índice primário
    unsigned qtd_registros_primario;

    // Tamanho de uma chave secundária nesse índice
    unsigned tam_chave_secundaria;

    // Tamanho de uma chave primária nesse índice
    unsigned tam_chave_primaria;

    // Função utilizada para comparar as chaves do índice secundário.
    // Igual às funções de comparação do bsearch e qsort.
    int (*compar)(const void *key, const void *elem);
} inverted_list;

/* Variáveis globais */
/* Arquivos de dados */
char ARQUIVO_USUARIOS[TAM_ARQUIVO_USUARIO];
char ARQUIVO_JOGOS[TAM_ARQUIVO_JOGO];
char ARQUIVO_COMPRAS[TAM_ARQUIVO_COMPRA];

/* Índices */
usuarios_index *usuarios_idx = NULL;
jogos_index *jogos_idx = NULL;
compras_index *compras_idx = NULL;
titulos_index *titulo_idx = NULL;
data_user_game_index *data_user_game_idx = NULL;
inverted_list categorias_idx = {
    .categorias_secundario_idx = NULL,
    .categorias_primario_idx = NULL,
    .qtd_registros_secundario = 0,
    .qtd_registros_primario = 0,
    .tam_chave_secundaria = TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX,
    .tam_chave_primaria = TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX,
};

/* Funções auxiliares para o qsort.
 * Com uma pequena alteração, é possível utilizá-las no bsearch, assim, evitando código duplicado.
 * */
int qsort_usuarios_idx(const void *a, const void *b);
int qsort_jogos_idx(const void *a, const void *b);
int qsort_compras_idx(const void *a, const void *b);
int qsort_titulo_idx(const void *a, const void *b);
int qsort_data_user_game_idx(const void *a, const void *b);
int qsort_data_idx(const void *a, const void *b);
int qsort_categorias_secundario_idx(const void *a, const void *b);
int qsort_id_games(const void *a, const void *b);

/* Contadores */
unsigned qtd_registros_usuarios = 0;
unsigned qtd_registros_jogos = 0;
unsigned qtd_registros_compras = 0;

/* Funções de geração determinística de números pseudo-aleatórios */
uint64_t prng_seed;

void prng_srand(uint64_t value) {
    prng_seed = value;
}

uint64_t prng_rand() {
    // https://en.wikipedia.org/wiki/Xorshift#xorshift*
    uint64_t x = prng_seed; // O estado deve ser iniciado com um valor diferente de 0
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    prng_seed = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

/* Funções de manipulação de data */
int64_t epoch;

void set_time(int64_t value) {
    epoch = value;
}

void tick_time() {
    epoch += prng_rand() % 864000; // 10 dias
}

struct tm gmtime_(const int64_t lcltime) {
    // based on https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/time/gmtime_r.c;
    struct tm res;
    long days = lcltime / 86400 + 719468;
    long rem = lcltime % 86400;
    if (rem < 0) {
        rem += 86400;
        --days;
    }

    res.tm_hour = (int) (rem / 3600);
    rem %= 3600;
    res.tm_min = (int) (rem / 60);
    res.tm_sec = (int) (rem % 60);

    int weekday = (3 + days) % 7;
    if (weekday < 0) weekday += 7;
    res.tm_wday = weekday;

    int era = (days >= 0 ? days : days - 146096) / 146097;
    unsigned long eraday = days - era * 146097;
    unsigned erayear = (eraday - eraday / 1460 + eraday / 36524 - eraday / 146096) / 365;
    unsigned yearday = eraday - (365 * erayear + erayear / 4 - erayear / 100);
    unsigned month = (5 * yearday + 2) / 153;
    unsigned day = yearday - (153 * month + 2) / 5 + 1;
    month += month < 10 ? 2 : -10;

    int isleap = ((erayear % 4) == 0 && (erayear % 100) != 0) || (erayear % 400) == 0;
    res.tm_yday = yearday >= 306 ? yearday - 306 : yearday + 59 + isleap;
    res.tm_year = (erayear + era * 400 + (month <= 1)) - 1900;
    res.tm_mon = month;
    res.tm_mday = day;
    res.tm_isdst = 0;

    return res;
}

/**
 * Escreve a <i>data</i> atual no formato <code>AAAAMMDD</code> em uma <i>string</i>
 * fornecida como parâmetro.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * char timestamp[TAM_DATE];<br />
 * current_date(date);<br />
 * printf("data atual: %s&#92;n", date);<br />
 * </code>
 *
 * @param buffer String de tamanho <code>TAM_DATE</code> no qual será escrita
 * a <i>timestamp</i>. É terminado pelo caractere <code>\0</code>.
 */
void current_date(char buffer[TAM_DATE]) {
    // http://www.cplusplus.com/reference/ctime/strftime/
    // http://www.cplusplus.com/reference/ctime/gmtime/
    // AAAA MM DD
    // %Y   %m %d
    struct tm tm_ = gmtime_(epoch);
    strftime(buffer, TAM_DATE, "%Y%m%d", &tm_);
}

/* Remove comentários (--) e caracteres whitespace do começo e fim de uma string */
void clear_input(char *str) {
    char *ptr = str;
    int len = 0;

    for (; ptr[len]; ++len) {
        if (strncmp(&ptr[len], "--", 2) == 0) {
            ptr[len] = '\0';
            break;
        }
    }

    while(len-1 > 0 && isspace(ptr[len-1]))
        ptr[--len] = '\0';

    while(*ptr && isspace(*ptr))
        ++ptr, --len;

    memmove(str, ptr, len + 1);
}


/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */

/* Cria o índice respectivo */
void criar_usuarios_idx();
void criar_jogos_idx();
void criar_compras_idx();
void criar_titulo_idx();
void criar_data_user_game_idx();
void criar_categorias_idx();

/* Exibe um registro com base no RRN */
bool exibir_usuario(int rrn);
bool exibir_jogo(int rrn);
bool exibir_compra(int rrn);

/* Recupera do arquivo o registro com o RRN informado
 * e retorna os dados nas structs Usuario, Jogo e Compra */
Usuario recuperar_registro_usuario(int rrn);
Jogo recuperar_registro_jogo(int rrn);
Compra recuperar_registro_compra(int rrn);

/* Escreve em seu respectivo arquivo na posição informada (RRN) */
void escrever_registro_usuario(Usuario u, int rrn);
void escrever_registro_jogo(Jogo j, int rrn);
void escrever_registro_compra(Compra c, int rrn);

/* Funções principais */
void cadastrar_usuario_menu(char* id_user, char* username, char* email);
void cadastrar_celular_menu(char* id_user, char* celular);
void remover_usuario_menu(char *id_user);
void cadastrar_jogo_menu(char* titulo, char* desenvolvedor, char* editora, char* lancamento, double preco);
void adicionar_saldo_menu(char* id_user, double valor);
void comprar_menu(char* id_user, char* titulo);
void cadastrar_categoria_menu(char* titulo, char* categoria);

/* Busca */
void buscar_usuario_id_user_menu(char *id_user);
void buscar_jogo_id_menu(char *id_game);
void buscar_jogo_titulo_menu(char *titulo);

/* Listagem */
void listar_usuarios_id_user_menu();
void listar_jogos_categorias_menu(char *categoria);
void listar_compras_periodo_menu(char *data_inicio, char *data_fim);

/* Liberar espaço */
void liberar_espaco_menu();

/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu();
void imprimir_arquivo_jogos_menu();
void imprimir_arquivo_compras_menu();

/* Imprimir índices primários */
void imprimir_usuarios_idx_menu();
void imprimir_jogos_idx_menu();
void imprimir_compras_idx_menu();

/* Imprimir índices secundários */
void imprimir_titulo_idx_menu();
void imprimir_data_user_game_idx_menu();
void imprimir_categorias_secundario_idx_menu();
void imprimir_categorias_primario_idx_menu();

/* Liberar memória e encerrar programa */
void liberar_memoria_menu();

/* Funções de manipulação de Lista Invertida */
/**
 * Responsável por inserir duas chaves (chave_secundaria e chave_primaria) em uma Lista Invertida (t).<br />
 * Atualiza os parâmetros dos índices primário e secundário conforme necessário.<br />
 * As chaves a serem inseridas devem estar no formato correto e com tamanho t->tam_chave_primario e t->tam_chave_secundario.<br />
 * O funcionamento deve ser genérico para qualquer Lista Invertida, adaptando-se para os diferentes parâmetros presentes em seus structs.<br />
 *
 * @param chave_secundaria Chave a ser buscada (caso exista) ou inserida (caso não exista) no registro secundário da Lista Invertida.
 * @param chave_primaria Chave a ser inserida no registro primário da Lista Invertida.
 * @param t Ponteiro para a Lista Invertida na qual serão inseridas as chaves.
 */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t);

/**
 * Responsável por buscar uma chave no índice secundário de uma Lista invertida (T). O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e conterá o índice inicial das chaves no registro primário.<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. A chave encontrada deverá ser retornada e o caminho não deve ser informado.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse na chave encontrada, apenas se ela existe, e o caminho não deve ser informado.<br />
 * ...<br />
 * bool found = inverted_list_secondary_search(NULL, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse no caminho feito para encontrar a chave.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, true, categoria, &categorias_idx);<br />
 * </code>
 *
 * @param result Ponteiro para ser escrito o índice inicial (primeira ocorrência) das chaves do registro primário. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave_secundaria Chave a ser buscada na Árvore-B.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t);

/**
 * Responsável por percorrer o índice primário de uma Lista invertida (T). O valor de retorno indica a quantidade de chaves encontradas.
 * O ponteiro para o vetor de strings result pode ser fornecido opcionalmente, e será populado com a lista de todas as chaves encontradas.
 * O ponteiro para o inteiro indice_final também pode ser fornecido opcionalmente, e deve conter o índice do último campo da lista encadeada 
 * da chave primaria fornecida (isso é útil na inserção de um novo registro).<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. As chaves encontradas deverão ser retornadas e tanto o caminho quanto o indice_final não devem ser informados.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, false, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse nas chaves encontradas, apenas no indice_final, e o caminho não deve ser informado.<br />
 * ...<br />
 * int indice_final;
 * int qtd = inverted_list_primary_search(NULL, false, indice, &indice_final, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse nas chaves encontradas e no caminho feito.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, true, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * </code>
 *
 * @param result Ponteiro para serem escritas as chaves encontradas. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param indice Índice do primeiro registro da lista encadeada a ser procurado.
 * @param indice_final Ponteiro para ser escrito o índice do último registro encontrado (cujo campo indice é -1). É ignorado caso NULL.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica a quantidade de chaves encontradas.
 */
int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t);

/**
 * Preenche uma string str com o caractere pad para completar o tamanho size.<br />
 *
 * @param str Ponteiro para a string a ser manipulada.
 * @param pad Caractere utilizado para fazer o preenchimento à direita.
 * @param size Tamanho desejado para a string.
 */
char* strpadright(char *str, char pad, unsigned size);

/* Funções de busca binária */
/**
 * Função Genérica de busca binária, que aceita parâmetros genéricos (assinatura baseada na função bsearch da biblioteca C).
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou NULL se não encontrou.
 */
void* busca_binaria(const void *key, const void *base0, size_t nmemb, size_t size, int (*compar)(const void *, const void *), bool exibir_caminho);

/**
 * Função Genérica de busca binária que encontra o elemento de BAIXO mais próximo da chave.
 * Sua assinatura também é baseada na função bsearch da biblioteca C.
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou o de BAIXO mais próximo.
 */
void* busca_binaria_piso(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*));

/**
 * Função Genérica de busca binária que encontra o elemento de CIMA mais próximo da chave.
 * Sua assinatura também é baseada na função bsearch da biblioteca C.
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou o de CIMA mais próximo.
 */
void* busca_binaria_teto(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*));

/* <<< COLOQUE AQUI OS DEMAIS PROTÓTIPOS DE FUNÇÕES, SE NECESSÁRIO >>> */


/* ==========================================================================
 * ============================ FUNÇÃO PRINCIPAL ============================
 * =============================== NÃO ALTERAR ============================== */

int main() {
    // variáveis utilizadas pelo interpretador de comandos
    char input[500];
    uint64_t seed = 2;
    uint64_t time = 1616077800; // UTC 18/03/2021 14:30:00
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    char id[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char lancamento[TAM_DATE];
    char categoria[TAM_MAX_CATEGORIA];
    double valor;
    char data_inicio[TAM_DATE];
    char data_fim[TAM_DATE];

    scanf("SET ARQUIVO_USUARIOS '%[^\n]\n", ARQUIVO_USUARIOS);
    int temp_len = strlen(ARQUIVO_USUARIOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_usuarios = (temp_len - 2) / TAM_REGISTRO_USUARIO;
    ARQUIVO_USUARIOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_JOGOS '%[^\n]\n", ARQUIVO_JOGOS);
    temp_len = strlen(ARQUIVO_JOGOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_jogos = (temp_len - 2) / TAM_REGISTRO_JOGO;
    ARQUIVO_JOGOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_COMPRAS '%[^\n]\n", ARQUIVO_COMPRAS);
    temp_len = strlen(ARQUIVO_COMPRAS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_compras = (temp_len - 2) / TAM_REGISTRO_COMPRA;
    ARQUIVO_COMPRAS[temp_len - 2] = '\0';

    // inicialização do gerador de números aleatórios e função de datas
    prng_srand(seed);
    set_time(time);

    criar_usuarios_idx();
    criar_jogos_idx();
    criar_compras_idx();
    criar_titulo_idx();
    criar_data_user_game_idx();
    criar_categorias_idx();

    while (1) {
        fgets(input, 500, stdin);
        printf("%s", input);
        clear_input(input);

        if (strcmp("", input) == 0)
            continue; // não avança o tempo nem imprime o comando este seja em branco

        /* Funções principais */
        if (sscanf(input, "INSERT INTO usuarios VALUES ('%[^']', '%[^']', '%[^']');", id_user, username, email) == 3)
            cadastrar_usuario_menu(id_user, username, email);
        else if (sscanf(input, "UPDATE usuarios SET celular = '%[^']' WHERE id_user = '%[^']';", celular, id_user) == 2)
            cadastrar_celular_menu(id_user, celular);
        else if (sscanf(input, "DELETE FROM usuarios WHERE id_user = '%[^']';", id_user) == 1)
            remover_usuario_menu(id_user);
        else if (sscanf(input, "INSERT INTO jogos VALUES ('%[^']', '%[^']', '%[^']', '%[^']', %lf);", titulo, desenvolvedor, editora, lancamento, &valor) == 5)
            cadastrar_jogo_menu(titulo, desenvolvedor, editora, lancamento, valor);
        else if (sscanf(input, "UPDATE usuarios SET saldo = saldo + %lf WHERE id_user = '%[^']';", &valor, id_user) == 2)
            adicionar_saldo_menu(id_user, valor);
        else if (sscanf(input, "INSERT INTO compras VALUES ('%[^']', '%[^']');", id_user, titulo) == 2)
            comprar_menu(id_user, titulo);
        else if (sscanf(input, "UPDATE jogos SET categorias = array_append(categorias, '%[^']') WHERE titulo = '%[^']';", categoria, titulo) == 2)
            cadastrar_categoria_menu(titulo, categoria);

        /* Busca */
        else if (sscanf(input, "SELECT * FROM usuarios WHERE id_user = '%[^']';", id_user) == 1)
            buscar_usuario_id_user_menu(id_user);
        else if (sscanf(input, "SELECT * FROM jogos WHERE id_game = '%[^']';", id) == 1)
            buscar_jogo_id_menu(id);
        else if (sscanf(input, "SELECT * FROM jogos WHERE titulo = '%[^']';", titulo) == 1)
            buscar_jogo_titulo_menu(titulo);

        /* Listagem */
        else if (strcmp("SELECT * FROM usuarios ORDER BY id_user ASC;", input) == 0)
            listar_usuarios_id_user_menu();
        else if (sscanf(input, "SELECT * FROM jogos WHERE '%[^']' = ANY (categorias) ORDER BY titulo ASC;", categoria) == 1)
            listar_jogos_categorias_menu(categoria);
        else if (sscanf(input, "SELECT * FROM compras WHERE data_compra BETWEEN '%[^']' AND '%[^']' ORDER BY data_compra ASC;", data_inicio, data_fim) == 2)
            listar_compras_periodo_menu(data_inicio, data_fim);

        /* Liberar espaço */
        else if (strcmp("VACUUM usuarios;", input) == 0)
            liberar_espaco_menu();

        /* Imprimir arquivos de dados */
        else if (strcmp("\\echo file ARQUIVO_USUARIOS", input) == 0)
            imprimir_arquivo_usuarios_menu();
        else if (strcmp("\\echo file ARQUIVO_JOGOS", input) == 0)
            imprimir_arquivo_jogos_menu();
        else if (strcmp("\\echo file ARQUIVO_COMPRAS", input) == 0)
            imprimir_arquivo_compras_menu();
        
        /* Imprimir índices primários */
        else if (strcmp("\\echo index usuarios_idx", input) == 0)
            imprimir_usuarios_idx_menu();
        else if (strcmp("\\echo index jogos_idx", input) == 0)
            imprimir_jogos_idx_menu();
        else if (strcmp("\\echo index compras_idx", input) == 0)
            imprimir_compras_idx_menu();

        /* Imprimir índices secundários */
        else if (strcmp("\\echo index titulo_idx", input) == 0)
            imprimir_titulo_idx_menu();
        else if (strcmp("\\echo index data_user_game_idx", input) == 0)
            imprimir_data_user_game_idx_menu();
        else if (strcmp("\\echo index categorias_secundario_idx", input) == 0)
            imprimir_categorias_secundario_idx_menu();
        else if (strcmp("\\echo index categorias_primario_idx", input) == 0)
            imprimir_categorias_primario_idx_menu();

        /* Liberar memória eventualmente alocada e encerrar programa */
        else if (strcmp("\\q", input) == 0)
            { liberar_memoria_menu(); return 0; }
        else if (sscanf(input, "SET SRAND %lu;", &seed) == 1)
            { prng_srand(seed); printf(SUCESSO); continue; }
        else if (sscanf(input, "SET TIME %lu;", &time) == 1)
            { set_time(time); printf(SUCESSO); continue; }
        else
            printf(ERRO_OPCAO_INVALIDA);

        tick_time();
    }
}

/* ========================================================================== */

/* Cria o índice primário usuarios_idx */
void criar_usuarios_idx() {
    if (!usuarios_idx)
        usuarios_idx = malloc(MAX_REGISTROS * sizeof(usuarios_index));

    if (!usuarios_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_usuarios; ++i) {
        Usuario u = recuperar_registro_usuario(i);

        if (strncmp(u.id_user, "*|", 2) == 0)
            usuarios_idx[i].rrn = -1; // registro excluído
        else
            usuarios_idx[i].rrn = i;

        strcpy(usuarios_idx[i].id_user, u.id_user);
    }

    qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);
}

/* Cria o índice primário jogos_idx */
void criar_jogos_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if(!jogos_idx)
        jogos_idx = malloc(MAX_REGISTROS * sizeof(jogos_idx));

    if(!jogos_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for(unsigned i = 0; i < qtd_registros_jogos; i++) {
        Jogo j = recuperar_registro_jogo(i);

        if (strncmp(j.id_game, "*|", 2) == 0)
            jogos_idx[i].rrn = -1; // registro excluido
        else
            jogos_idx[i].rrn = i;

        strcpy(jogos_idx[i].id_game, j.id_game);
    }

    qsort(jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx);

    //printf(ERRO_NAO_IMPLEMENTADO, "criar_jogos_idx");
}

/* Cria o índice primário compras_idx */
void criar_compras_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    if(!compras_idx)
        compras_idx = malloc(MAX_REGISTROS * sizeof(compras_index));

    if(!compras_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for(int i = 0; i < qtd_registros_compras; i++) {
        Compra c = recuperar_registro_compra(i);
        
        compras_idx[i].rrn = i;
        strcpy(compras_idx[i].id_user, c.id_user_dono);
        strcpy(compras_idx[i].id_game, c.id_game);
    }

    qsort(compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "criar_compras_idx");
}

/* Cria o índice secundário titulo_idx */
void criar_titulo_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    if(!titulo_idx)
        titulo_idx = malloc(MAX_REGISTROS * sizeof(jogos_index));

    if(!titulo_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for(int i = 0; i < qtd_registros_jogos; i++) {
        Jogo j = recuperar_registro_jogo(i);

        strcpy(titulo_idx[i].id_game, j.id_game);
        strcpy(titulo_idx[i].titulo, j.titulo);
    }
    
    qsort(titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx);

    //printf(ERRO_NAO_IMPLEMENTADO, "criar_titulo_idx");
}

/* Cria o índice secundário data_user_game_idx */
void criar_data_user_game_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    if(!data_user_game_idx)
        data_user_game_idx = malloc(MAX_REGISTROS * sizeof(data_user_game_index));

    if(!data_user_game_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for(int i = 0; i < qtd_registros_compras; i++) {
        Compra c = recuperar_registro_compra(i);

        strcpy(data_user_game_idx[i].data, c.data_compra);
        strcpy(data_user_game_idx[i].id_user, c.id_user_dono);
        strcpy(data_user_game_idx[i].id_game, c.id_game);
    }

    qsort(data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_user_game_idx);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "criar_data_user_game_idx");
}

/* Cria os índices (secundário e primário) de categorias_idx */
void criar_categorias_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    if(!categorias_idx.categorias_primario_idx)
        categorias_idx.categorias_primario_idx = malloc(MAX_REGISTROS * sizeof(categorias_idx.tam_chave_primaria));

    if(!categorias_idx.categorias_secundario_idx)
        categorias_idx.categorias_secundario_idx = malloc(MAX_REGISTROS * sizeof(categorias_idx.categorias_secundario_idx));

    if(!categorias_idx.categorias_primario_idx || !categorias_idx.categorias_secundario_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        return;
    }

    Jogo j;

    for(int i = 0; i < qtd_registros_jogos; i++) {
        j = recuperar_registro_jogo(i);

        for(int k = 0; k < 3; k++) {
            // jogo possui categoria
            if(strcmp(j.categorias[k], "") != 0) {
                // insere a categoria encontrada
                inverted_list_insert(j.categorias[k], j.id_game, &categorias_idx);
            } else {
                break; // jogo sem outra categoria
            }
        }
    }  

    
    //printf(ERRO_NAO_IMPLEMENTADO, "criar_categorias_idx");
}


/* Exibe um usuario dado seu RRN */
bool exibir_usuario(int rrn) {
    if (rrn < 0)
        return false;

    Usuario u = recuperar_registro_usuario(rrn);

    printf("%s, %s, %s, %s, %.2lf\n", u.id_user, u.username, u.email, u.celular, u.saldo);
    return true;
}

/* Exibe um jogo dado seu RRN */
bool exibir_jogo(int rrn) {
    if (rrn < 0)
        return false;

    Jogo j = recuperar_registro_jogo(rrn);

    printf("%s, %s, %s, %s, %s, %.2lf\n", j.id_game, j.titulo, j.desenvolvedor, j.editora, j.data_lancamento, j.preco);
    return true;
}

/* Exibe uma compra dado seu RRN */
bool exibir_compra(int rrn) {
    if (rrn < 0)
        return false;

    Compra c = recuperar_registro_compra(rrn);

    printf("%s, %s, %s\n", c.id_user_dono, c.data_compra, c.id_game);

    return true;
}


/* Recupera do arquivo de usuários o registro com o RRN
 * informado e retorna os dados na struct Usuario */
Usuario recuperar_registro_usuario(int rrn) {
    Usuario u;
    char temp[TAM_REGISTRO_USUARIO + 1], *p;
    strncpy(temp, ARQUIVO_USUARIOS + (rrn * TAM_REGISTRO_USUARIO), TAM_REGISTRO_USUARIO);
    temp[TAM_REGISTRO_USUARIO] = '\0';

    p = strtok(temp, ";");
    strcpy(u.id_user, p);
    p = strtok(NULL, ";");
    strcpy(u.username, p);
    p = strtok(NULL, ";");
    strcpy(u.email, p);
    p = strtok(NULL, ";");
    strcpy(u.celular, p);
    p = strtok(NULL, ";");
    u.saldo = atof(p);
    p = strtok(NULL, ";");

    return u;
}

/* Recupera do arquivo de jogos o registro com o RRN
 * informado e retorna os dados na struct Jogo */
Jogo recuperar_registro_jogo(int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    Jogo j;
    char temp[TAM_REGISTRO_JOGO + 1], *p;
    strncpy(temp, ARQUIVO_JOGOS + (rrn * TAM_REGISTRO_JOGO), TAM_REGISTRO_JOGO);
    temp[TAM_REGISTRO_JOGO] = '\0';

    p = strtok(temp, ";");
    strcpy(j.id_game, p);
    p = strtok(NULL, ";");
    strcpy(j.titulo, p);
    p = strtok(NULL, ";");
    strcpy(j.desenvolvedor, p);
    p = strtok(NULL, ";");
    strcpy(j.editora, p);
    p = strtok(NULL, ";");
    strcpy(j.data_lancamento, p);
    p = strtok(NULL, ";");
    j.preco = atof(p);
    p = strtok(NULL, ";|#");
    // como fazer com as categorias ?
    if(p != NULL) {
        strcpy(j.categorias[0], p);
        p = strtok(NULL, ";|#");
    } else {
        strcpy(j.categorias[0], "");
    }
    if(p != NULL) {
        strcpy(j.categorias[1], p);
        p = strtok(NULL, ";|#");
    } else {
        strcpy(j.categorias[1], "");
    }
    if(p != NULL) {
        strcpy(j.categorias[2], p);
        p = strtok(NULL, ";|#");
    } else {
        strcpy(j.categorias[2], "");
    }
    p = strtok(NULL, ";");
    

    return j;

    //printf(ERRO_NAO_IMPLEMENTADO, "recuperar_registro_jogo");
}

/* Recupera do arquivo de compras o registro com o RRN
 * informado e retorna os dados na struct Compra */
Compra recuperar_registro_compra(int rrn) {     
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    Compra c;

    strncpy(c.id_user_dono, ARQUIVO_COMPRAS + (rrn * TAM_REGISTRO_COMPRA), TAM_ID_USER - 1);
    strncpy(c.data_compra, ARQUIVO_COMPRAS + (rrn * TAM_REGISTRO_COMPRA) + TAM_ID_USER - 1, TAM_ID_GAME - 1);
    strncpy(c.id_game, ARQUIVO_COMPRAS + (rrn * TAM_REGISTRO_COMPRA) + TAM_ID_USER + TAM_ID_GAME - 2, TAM_DATE - 1);

    c.id_user_dono[TAM_ID_USER - 1] = '\0';
    c.id_game[TAM_ID_GAME - 1] = '\0';
    c.data_compra[TAM_DATE - 1] = '\0';
    
    return c;

    //printf(ERRO_NAO_IMPLEMENTADO, "recuperar_registro_compra");
}


/* Escreve no arquivo de usuários na posição informada (RRN)
 * os dados na struct Usuario */
void escrever_registro_usuario(Usuario u, int rrn) {
    char temp[TAM_REGISTRO_USUARIO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, u.id_user);
    strcat(temp, ";");
    strcat(temp, u.username);
    strcat(temp, ";");
    strcat(temp, u.email);
    strcat(temp, ";");
    strcat(temp, u.celular);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", u.saldo);
    strcat(temp, p);
    strcat(temp, ";");

    for (int i = strlen(temp); i < TAM_REGISTRO_USUARIO; i++)
        temp[i] = '#';

    strncpy(ARQUIVO_USUARIOS + rrn*TAM_REGISTRO_USUARIO, temp, TAM_REGISTRO_USUARIO);
    ARQUIVO_USUARIOS[qtd_registros_usuarios*TAM_REGISTRO_USUARIO] = '\0';
}

/* Escreve no arquivo de jogos na posição informada (RRN)
 * os dados na struct Jogo */
void escrever_registro_jogo(Jogo j, int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    char temp[TAM_REGISTRO_JOGO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, j.id_game);
    strcat(temp, ";");
    strcat(temp, j.titulo);
    strcat(temp, ";");
    strcat(temp, j.desenvolvedor);
    strcat(temp, ";");
    strcat(temp, j.editora);
    strcat(temp, ";");
    strcat(temp, j.data_lancamento);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", j.preco);
    strcat(temp, p);
    strcat(temp, ";");
    if(strcmp(j.categorias[0], "") != 0) {
        strcat(temp, j.categorias[0]);
    }
    if(strcmp(j.categorias[1], "") != 0) {
        strcat(temp, "|");
        strcat(temp, j.categorias[1]);
    }
    if(strcmp(j.categorias[2], "") != 0) {
        strcat(temp, "|");
        strcat(temp, j.categorias[2]);
    }
    strcat(temp, ";");

    for(int i = strlen(temp); i < TAM_REGISTRO_JOGO; i++)
        temp[i] = '#';

    strncpy(ARQUIVO_JOGOS + rrn*TAM_REGISTRO_JOGO, temp, TAM_REGISTRO_JOGO);
    ARQUIVO_JOGOS[qtd_registros_jogos*TAM_REGISTRO_JOGO] = '\0';
    
    
    //printf(ERRO_NAO_IMPLEMENTADO, "escrever_registro_jogo");
}

/* Escreve no arquivo de compras na posição informada (RRN)
 * os dados na struct Compra */
void escrever_registro_compra(Compra c, int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    char temp[TAM_REGISTRO_COMPRA + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, c.id_user_dono);
    strcat(temp, c.data_compra);
    strcat(temp, c.id_game);

    strcpy(ARQUIVO_COMPRAS + rrn * TAM_REGISTRO_COMPRA, temp);
    ARQUIVO_COMPRAS[qtd_registros_compras * TAM_REGISTRO_COMPRA] = '\0';
    
    //printf(ERRO_NAO_IMPLEMENTADO, "escrever_registro_compra");
}


/* Funções principais */
void cadastrar_usuario_menu(char *id_user, char *username, char *email) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    // cria um indice primario para o usuario
    usuarios_index novo_usuario;
    strcpy(novo_usuario.id_user, id_user);
    novo_usuario.rrn = qtd_registros_usuarios;

    // verifica se o usuario ja existe com a busca binaria
    void *item;
    item = busca_binaria((void *)&novo_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

    // se item for null entao o ID nao foi encontrado e portanto ainda nao foi cadastrado
    if(item != NULL) {
        printf(ERRO_PK_REPETIDA, id_user);
        return;
    }

    Usuario u;
    strcpy(u.id_user, id_user);
    strcpy(u.username, username);
    strcpy(u.email, email);
    strcpy(u.celular, "***********");
    u.saldo = 0.0;

    usuarios_idx[qtd_registros_usuarios] = novo_usuario;

    qtd_registros_usuarios++;
    
    // escreve o usuario cadastrado no arquivo
    escrever_registro_usuario(u, novo_usuario.rrn);

    // ordena de acordo com os IDs dos usuarios
    qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);

    printf(SUCESSO);
}

void cadastrar_celular_menu(char* id_user, char* celular) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // cria um struct de indice primario para o usuario
    usuarios_index novo_usuario;
    strcpy(novo_usuario.id_user, id_user);

    // verifica se o usuario existe
    usuarios_index *user = busca_binaria((void *)&novo_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

    // se user for null entao o ID nao foi encontrado e portanto ainda nao foi cadastrado
    if(user == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    // atribui ao usuario encontrado o seu rrn
    novo_usuario.rrn = user->rrn;

    // recupera o registro do usuario a partir do RRN e atualiza o saldo na struct
    Usuario u = recuperar_registro_usuario(novo_usuario.rrn);
    strcpy(u.celular, celular);

    // atualiza o saldo no arquivo de usuario
    escrever_registro_usuario(u, novo_usuario.rrn);

    printf(SUCESSO);
}

void remover_usuario_menu(char *id_user) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // cria o indice primario do usuario
    usuarios_index usuario_buscado;
    strcpy(usuario_buscado.id_user, id_user);

    // busca binaria pelo usuario
    usuarios_index *removido = busca_binaria((void*)&usuario_buscado, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

    // busca binaria nao encontrou o id informado ou aquele usuario ja foi removido
    if(removido == NULL || removido->rrn == -1) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    usuario_buscado.rrn = removido->rrn;

    // recuperar o usuario
    Usuario u = recuperar_registro_usuario(usuario_buscado.rrn);

    // trocar o id user por *|
    strncpy(u.id_user, "*|", 2);

    // escrever no arquivo de usuario
    escrever_registro_usuario(u, usuario_buscado.rrn);

    // coloca o rrn como -1 no indice primario correspondente aquele usuario
    removido->rrn = - 1;

    //printf(ERRO_NAO_IMPLEMENTADO, "remover_usuario_menu");

    printf(SUCESSO);
}

void cadastrar_jogo_menu(char *titulo, char *desenvolvedor, char *editora, char* lancamento, double preco) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // cria o indice primario para o jogo
    jogos_index novo_jogo_indice;
    char temp[TAM_ID_GAME]; // armazena em forma de string o ID_game
    temp[0] = '\0';
    sprintf(temp, "%08d", qtd_registros_jogos);
    strcpy(novo_jogo_indice.id_game, temp);
    novo_jogo_indice.rrn = qtd_registros_jogos;

    // cria um indice secundario do jogo
    titulos_index novo_titulo;
    strcpy(novo_titulo.titulo, titulo);
    strcpy(novo_titulo.id_game, temp);

    // faz a busca binaria pelo titulo
    titulos_index *title = busca_binaria((void *)&novo_titulo, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);
    
    // se title nao for null entao um titulo com o mesmo nome ja foi cadastrado 
    if(title != NULL) {
        printf(ERRO_PK_REPETIDA, titulo);
        return;
    }

    // Cria uma struct para o novo jogo
    Jogo j;
    strcpy(j.id_game, temp);
    strcpy(j.titulo, titulo);
    strcpy(j.desenvolvedor, desenvolvedor);
    strcpy(j.editora, editora);
    strcpy(j.data_lancamento, lancamento);
    j.preco = preco;
    j.categorias[0][0] = '\0';
    j.categorias[1][0] = '\0';
    j.categorias[2][0] = '\0';

    // atualiza os vetores de indices primarios e secundarios
    jogos_idx[qtd_registros_jogos] = novo_jogo_indice;
    titulo_idx[qtd_registros_jogos] = novo_titulo;

    qtd_registros_jogos++;
    
    // escreve o jogo no arquivo
    escrever_registro_jogo(j, novo_jogo_indice.rrn);

    // atualiza os indices primarios
    qsort(jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx);

    // atualiza os indices secundarios
    qsort(titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx);

    //printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_jogo_menu");

    printf(SUCESSO);
}

void adicionar_saldo_menu(char *id_user, double valor) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // verifica se o valor inserido eh valido
    if(valor <= 0) {
        printf(ERRO_VALOR_INVALIDO);
        return;
    }

    // cria um struct de indice primario para o usuario
    usuarios_index novo_usuario;
    strcpy(novo_usuario.id_user, id_user);

    // verifica se o usuario existe
    usuarios_index *user = busca_binaria((void *)&novo_usuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

    // se user for null entao o ID nao foi encontrado e portanto ainda nao foi cadastrado
    if(user == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    // atribui ao usuario o seu rrn
    novo_usuario.rrn = user->rrn;

    // recupera o registro do usuario a partir do RRN e atualiza o saldo na struct
    Usuario u = recuperar_registro_usuario(novo_usuario.rrn);
    u.saldo += valor;

    // atualiza o saldo no arquivo de usuario
    escrever_registro_usuario(u, novo_usuario.rrn);

    printf(SUCESSO);
}

void comprar_menu(char *id_user, char *titulo) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // indice primario usuario
    usuarios_index usuario_comprador;
    strcpy(usuario_comprador.id_user, id_user);

    // indice secundario jogo
    titulos_index jogo_comprado;
    strcpy(jogo_comprado.titulo, titulo);

    // busca pelo usuario
    usuarios_index *usuario_buscado = busca_binaria((void*)&usuario_comprador, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

    // busca pelo titulo
    titulos_index *jogo_buscado = busca_binaria((void*)&jogo_comprado, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);
    
    // usuario ou jogo nao encontrado
    if(usuario_buscado == NULL || usuario_buscado->rrn == -1 || jogo_buscado == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    // indice primario compra
    compras_index compra_indice;
    strcpy(compra_indice.id_user, usuario_buscado->id_user);
    strcpy(compra_indice.id_game, jogo_buscado->id_game);
    compra_indice.rrn = qtd_registros_compras;

    compras_index *compra_buscada = busca_binaria((void*)&compra_indice, compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx, false);

    // compra ja registrada
    if(compra_buscada != NULL) {
        printf("ERRO: Ja existe um registro com a chave %s", compra_buscada->id_user);
        printf("%s\n", compra_buscada->id_game);
        return;
    }
    
    // recupera os dados do usuario
    Usuario u = recuperar_registro_usuario(usuario_buscado->rrn);

    // recupera dados do jogo
    jogos_index jogo_indice;
    strcpy(jogo_indice.id_game, jogo_buscado->id_game);

    // indice primario do jogo
    jogos_index *jogo_indice2 = busca_binaria((void*)&jogo_indice, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, false);

    // recupera dados do jogo
    Jogo j = recuperar_registro_jogo(jogo_indice2->rrn);

    // verifica o saldo do usuario
    if(u.saldo < j.preco) {
        printf(ERRO_SALDO_NAO_SUFICIENTE);
        return;
    // se o saldo for sufiente, subtrai do saldo atual o valor do jogo
    } else {
        u.saldo -= j.preco;
        escrever_registro_usuario(u, usuario_buscado->rrn);
    }

    // indice secundario compra
    data_user_game_index data_indice;
    strcpy(data_indice.id_user, id_user);
    strcpy(data_indice.id_game, j.id_game);
    current_date(data_indice.data); // coloca a data atual

    // atualiza o vetor de indice primario e secundario das compras
    compras_idx[qtd_registros_compras] = compra_indice;
    data_user_game_idx[qtd_registros_compras] = data_indice;

    qtd_registros_compras++;

    // struct de compra usada para escrever no arquivo de jogos
    Compra c;
    strcpy(c.id_user_dono, id_user);
    strcpy(c.id_game, j.id_game);
    current_date(c.data_compra);

    // escreve no arquivo de jogos
    escrever_registro_compra(c, compra_indice.rrn);

    // ordena o indice primario
    qsort(compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx);

    // ordena o indice secundario
    qsort(data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_user_game_idx);

    //printf(ERRO_NAO_IMPLEMENTADO, "comprar_menu");

    printf(SUCESSO);
}

void cadastrar_categoria_menu(char* titulo, char* categoria) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // cria indice primario do jogo
    jogos_index novo_jogo;

    // cria um indice secundario do jogo
    titulos_index novo_titulo;
    strcpy(novo_titulo.titulo, titulo);

    // faz a busca binaria pelo titulo
    titulos_index *title = busca_binaria((void *)&novo_titulo, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);

    // se title for null entao o jogo nao esta cadastrado
    if(title == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    // busca pelo jogo para recuperar seu rrn
    strcpy(novo_jogo.id_game, title->id_game);
    jogos_index *novo_jogo_indice = busca_binaria((void*)&novo_jogo, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, false);

    Jogo j = recuperar_registro_jogo(novo_jogo_indice->rrn);

    // analisa se o jogo ja tem tal categoria
    for(int i = 0; i < 3; i++) {
        if(strcmp(j.categorias[i], categoria) == 0) {
            printf(ERRO_CATEGORIA_REPETIDA, j.titulo, categoria);
            return;
        }
    }

    // faz a atribuicao da categoria ao jogo
    if(j.categorias[0][0] == '\0')
        strcpy(j.categorias[0], categoria);
    else if(j.categorias[1][0] == '\0')
        strcpy(j.categorias[1], categoria);
    else 
        strcpy(j.categorias[2], categoria);

    // escreve no arquivo de jogos
    escrever_registro_jogo(j, novo_jogo_indice->rrn);

    // atualiza os indices da categoria
    inverted_list_insert(categoria, j.id_game, &categorias_idx);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_categoria_menu");

    printf(SUCESSO);
}


/* Busca */
void buscar_usuario_id_user_menu(char *id_user) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // cria um indice primario para o usuario
    usuarios_index usuario_buscado;
    strcpy(usuario_buscado.id_user, id_user);
    
    // realiza a busca binaria procurando o id informado
    usuarios_index *encontrado = busca_binaria((void*)&usuario_buscado, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, true);

    // se a busca retornar null entao o id nao foi encontrado
    if(encontrado == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    // imprime os dados do usuario de forma formatada
    if(encontrado != NULL)
        exibir_usuario(encontrado->rrn);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "buscar_usuario_id_user_menu");
}

void buscar_jogo_id_menu(char *id_game) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // cria um indice primario para o jogo
    jogos_index jogo_buscado;
    strcpy(jogo_buscado.id_game, id_game);

    // realiza a busca binaria
    jogos_index *buscado = busca_binaria((void*)&jogo_buscado, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, true);

    // se a busca retornar null entao o id nao foi encontrado
    if(buscado == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    // printa os dados de forma formatada
    if(buscado != NULL)
        exibir_jogo(buscado->rrn);

    //printf(ERRO_NAO_IMPLEMENTADO, "buscar_jogo_id_menu");
}

void buscar_jogo_titulo_menu(char *titulo) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    // indice secundario do jogo
    titulos_index titulo_procurado;
    strcpy(titulo_procurado.titulo, titulo);

    // busca binaria pelo titulo
    titulos_index *titulo_buscado = busca_binaria((void*)&titulo_procurado, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, true);

    // busca nao encontrou o titulo
    if(titulo_buscado == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    // indice primario do jogo
    jogos_index jogo_procurado;
    strcpy(jogo_procurado.id_game, titulo_buscado->id_game);

    // busca binaria pelo id_game para saber o rrn
    jogos_index *jogo_buscado = busca_binaria((void*)&jogo_procurado, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, true);

    // printa os dados de forma formatada
    if(jogo_buscado != NULL)
        exibir_jogo(jogo_buscado->rrn);

    //printf(ERRO_NAO_IMPLEMENTADO, "buscar_jogo_titulo_menu");
}


/* Listagem */
void listar_usuarios_id_user_menu() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    // verifica se existe algum usuario
    if(qtd_registros_usuarios <= 0) {
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }

    // indice primario do usuario
    usuarios_index usuario_atual;

    // percorre o laco e imprime em ordem crescente de id_user os dados dos usuarios
    for(int i = 0; i < qtd_registros_usuarios; i++) {
        usuario_atual = usuarios_idx[i]; // usuarios_idx ja esta ordenado por ID
        if(usuario_atual.rrn != -1) {
            exibir_usuario(usuario_atual.rrn);
        }
    }
}

void listar_jogos_categorias_menu(char *categoria) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    int result, qtd_jogos, indice_final;
    char id_games[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];

    // analisa se algum jogo foi encontrado
    if(!inverted_list_secondary_search(&result, false, categoria, &categorias_idx)) {
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }

    // recebe quantos jogos foram encontrados
    qtd_jogos = inverted_list_primary_search(id_games, true, result, &indice_final, &categorias_idx);

    // ordena os id_games encontrados
    qsort(id_games, qtd_jogos, TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX, qsort_id_games);

    Jogo j;

    // indice primario jogo
    jogos_index jogo_indice;
    jogos_index *jogo_indice_ptr;

    for(int i = 0; i < qtd_jogos; i++) {
        strncpy(jogo_indice.id_game, id_games[i], TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX);
        jogo_indice.id_game[TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX] = '\0'; // como o id_games[] tem um char a menos, o \0 deve ser inserido manualmente
        // procura o jogo a partir do id_game
        jogo_indice_ptr = busca_binaria((void*)&jogo_indice, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, false);

        // imprime os dados do jogo de forma formatada
        if(jogo_indice_ptr != NULL)
            exibir_jogo(jogo_indice_ptr->rrn);

    }
    
    //printf(ERRO_NAO_IMPLEMENTADO, "listar_jogo_categorias_menu");
}

void listar_compras_periodo_menu(char *data_inicio, char *data_fim) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    if(qtd_registros_compras <= 0) {
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }

    // indices secundarios e primarios de compras
    data_user_game_index compra_piso, compra_teto;
    data_user_game_index *compra_piso_ptr, *compra_teto_ptr;
    compras_index *compra_buscada;
    int encontrado = 0;

    strcpy(compra_piso.data, data_fim);
    compra_piso_ptr = busca_binaria_piso((void*)&compra_piso, data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_idx);

    strcpy(compra_teto.data, data_inicio);
    compra_teto_ptr = busca_binaria_teto((void*)&compra_teto, data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_idx);

    Compra c;
    for(data_user_game_index *i = compra_teto_ptr; i <= compra_teto_ptr; i++) {
        strcpy(c.id_user_dono, i->id_user);
        strcpy(c.id_game, i->id_game);
        strcpy(c.data_compra, i->data);
        compra_buscada = busca_binaria((void*)&c, compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx, true);
        if(compra_buscada != NULL) {
            exibir_compra(compra_buscada->rrn);
            encontrado = 1;
        }
    }

    // nenhum registro encontrado no periodo especificado
    if(!encontrado)
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "listar_compras_periodo_menu");
}


/* Liberar espaço */
void liberar_espaco_menu() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    Usuario u; 
    usuarios_index *usuario_aux;
    usuarios_index usuario_aux2;
    usuarios_index indices_aux[MAX_REGISTROS];
    int rrn_atual = 0; unsigned int removidos = 0;
    char arquivo_usuario_aux[TAM_ARQUIVO_USUARIO + 1];
    strncpy(arquivo_usuario_aux, "\0", TAM_ARQUIVO_USUARIO);

    for(int i = 0; i < qtd_registros_usuarios; i++) {
        u = recuperar_registro_usuario(i);
        strcpy(usuario_aux2.id_user, u.id_user);
        
        usuario_aux = busca_binaria((void*)&usuario_aux2, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

        // usuario ainda existente no arquivo
        if(strncmp(u.id_user, "*|", 2) != 0) {
            strncpy(arquivo_usuario_aux +(rrn_atual * TAM_REGISTRO_USUARIO), ARQUIVO_USUARIOS +(i * TAM_REGISTRO_USUARIO), TAM_REGISTRO_USUARIO);
            strcpy(indices_aux[rrn_atual].id_user, u.id_user);
            indices_aux[rrn_atual].rrn = rrn_atual;
            rrn_atual++;
        // usuario foi removido
        } else {
            removidos++;
        }
    }

    // atualiza a qtd de registros
    qtd_registros_usuarios -= removidos;

    // ordena os indices primarios
    qsort(indices_aux, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);

    // atualiza os valores de usuarios_idx
    for(int i = 0; i < qtd_registros_usuarios; i++) {
        usuarios_idx[i] = indices_aux[i];
    }

    // atualiza o arquivo de usuarios
    strncpy(ARQUIVO_USUARIOS, arquivo_usuario_aux, TAM_ARQUIVO_USUARIO);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "liberar_espaco_menu");

    printf(SUCESSO);
}


/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu() {
    if (qtd_registros_usuarios == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_USUARIOS);
}

void imprimir_arquivo_jogos_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_JOGOS);
}

void imprimir_arquivo_compras_menu() {
    if (qtd_registros_compras == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_COMPRAS);
}


/* Imprimir índices primários */
void imprimir_usuarios_idx_menu() {
    if (qtd_registros_usuarios == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_usuarios; ++i)
        printf("%s, %d\n", usuarios_idx[i].id_user, usuarios_idx[i].rrn);
}

void imprimir_jogos_idx_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_jogos; ++i)
        printf("%s, %d\n", jogos_idx[i].id_game, jogos_idx[i].rrn);
}

void imprimir_compras_idx_menu() {
    if (qtd_registros_compras == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_compras; ++i)
        printf("%s, %s, %d\n", compras_idx[i].id_user, compras_idx[i].id_game, compras_idx[i].rrn);
}


/* Imprimir índices secundários */
void imprimir_titulo_idx_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_jogos; ++i)
        printf("%s, %s\n", titulo_idx[i].titulo, titulo_idx[i].id_game);
}

void imprimir_data_user_game_idx_menu() {
    if (qtd_registros_compras == 0) {
        printf(ERRO_ARQUIVO_VAZIO);
        return;
    }

    for (unsigned i = 0; i < qtd_registros_compras; ++i)
        printf("%s, %s, %s\n", data_user_game_idx[i].data, data_user_game_idx[i].id_user, data_user_game_idx[i].id_game);
}

void imprimir_categorias_secundario_idx_menu() {
    if (categorias_idx.qtd_registros_secundario == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < categorias_idx.qtd_registros_secundario; ++i)
        printf("%s, %d\n", (categorias_idx.categorias_secundario_idx)[i].chave_secundaria, (categorias_idx.categorias_secundario_idx)[i].primeiro_indice);
}

void imprimir_categorias_primario_idx_menu() {
    if (categorias_idx.qtd_registros_primario == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < categorias_idx.qtd_registros_primario; ++i)
        printf("%s, %d\n", (categorias_idx.categorias_primario_idx)[i].chave_primaria, (categorias_idx.categorias_primario_idx)[i].proximo_indice);
}


/* Liberar memória e encerrar programa */
void liberar_memoria_menu() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    free(usuarios_idx);
    free(jogos_idx);
    free(titulo_idx);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "liberar_memoria_menu");
}


/* Função de comparação entre chaves do índice usuarios_idx */
int qsort_usuarios_idx(const void *a, const void *b) {

    return strcmp( ( (usuarios_index *)a )->id_user, ( (usuarios_index *)b )->id_user);
}

/* Função de comparação entre chaves do índice jogos_idx */
int qsort_jogos_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return strcmp( ( (jogos_index *)a )->id_game, ((jogos_index *)b )->id_game);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_jogos_idx");
}

/* Função de comparação entre chaves do índice compras_idx */
int qsort_compras_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    int cmp_id_user = strcmp( ( (compras_index *)a )->id_user, ( (compras_index *)b )->id_user);

    int cmp_id_game = strcmp( ( (compras_index *)a )->id_game, ((compras_index *)b )->id_game);
    
    // se o usuario for o mesmo, ordena pelo id do jogo
    if(cmp_id_user == 0) {
        return cmp_id_game;
    } else {
        return cmp_id_user;
    }
    
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_compras_idx");
}

/* Função de comparação entre chaves do índice titulo_idx */
int qsort_titulo_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    return strcmp( ( (titulos_index *)a )->titulo, ((titulos_index *)b )->titulo);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_titulo_idx");
}

/* Funções de comparação entre chaves do índice data_user_game_idx */
int qsort_data_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    return strcmp( ( (data_user_game_index *)a )->data, ((data_user_game_index *)b)->data);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_data_idx");
}

int qsort_data_user_game_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    int cmp_data = qsort_data_idx(a, b);

    int cmp_id_user = strcmp( ( (data_user_game_index *)a )->id_user, ( (data_user_game_index *)b )->id_user);

    int cmp_id_game = strcmp( ( (data_user_game_index *)a )->id_game, ((data_user_game_index *)b )->id_game);
    
    // se a data for a mesma, ordena pelo usuario, se o usuario for o mesmo, ordena pelo id do jogo
    if(cmp_data == 0) {
        if(cmp_id_user == 0) {
            return cmp_id_game;
        } else {
            return cmp_id_user;
        }
    } else {
        return cmp_data;
    }
    
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_data_user_game_idx");
}

/* Função de comparação entre chaves do índice secundário de categorias_idx */
int qsort_categorias_secundario_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    return strcmp( ( (categorias_secundario_index*)a )->chave_secundaria, ((categorias_secundario_index *)b)->chave_secundaria);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_categorias_secundario_idx");
}

/* Função de ordenar uma matriz contendo id_games, usada para listar os jogos com a mesma categoria em ordem */
int qsort_id_games(const void *a, const void *b) {
    return ( strncmp ( ( char * ) a , ( char * ) b, TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX ));
}

/* Funções de manipulação de Lista Invertida */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    int result, indice_final;
    int qtd_encontrada;
    char resultado[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];

    // se a busca encontrar aquela chave secundaria
    if(inverted_list_secondary_search(&result, false, chave_secundaria, t)) {
        
        qtd_encontrada = inverted_list_primary_search(resultado, false, result, &indice_final, t);

        // atualiza o proximo indice do registro primario final
        if(&indice_final != NULL)
            t->categorias_primario_idx[indice_final].proximo_indice = t->qtd_registros_primario;

        // atualiza o indice do ultimo agora com -1 e a sua chave primaria
        t->categorias_primario_idx[t->qtd_registros_primario].proximo_indice = -1;
        strcpy(t->categorias_primario_idx[t->qtd_registros_primario].chave_primaria, chave_primaria);

        t->qtd_registros_primario++;

    } else { // busca nao encontrou aquela chave secundaria
        // atualiza a chave secundaria e coloca o primeiro indice como sendo o proximo indice primario livre
        strcpy(t->categorias_secundario_idx[t->qtd_registros_secundario].chave_secundaria, chave_secundaria);
        t->categorias_secundario_idx[t->qtd_registros_secundario].primeiro_indice = t->qtd_registros_primario;
        t->qtd_registros_secundario++;

        // atualiza o indice do ultimo agora com -1 e a sua chave primaria
        t->categorias_primario_idx[t->qtd_registros_primario].proximo_indice = -1;
        strcpy(t->categorias_primario_idx[t->qtd_registros_primario].chave_primaria, chave_primaria);

        t->qtd_registros_primario++;

        // ordena as chaves secundarias
        qsort(t->categorias_secundario_idx, t->qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx);
    }
    
    //printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_insert");
}

bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    categorias_secundario_index categoria_buscada;
    strcpy(categoria_buscada.chave_secundaria, chave_secundaria);

    categorias_secundario_index *categoria_encontrada;

    categoria_encontrada = busca_binaria((void*)&categoria_buscada, t->categorias_secundario_idx, t->qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx, exibir_caminho);

    if(categoria_encontrada != NULL) {
        categoria_buscada.primeiro_indice = categoria_encontrada->primeiro_indice;
        *result = categoria_encontrada->primeiro_indice; // valor de result recebe a primeira ocorrencia
        return true;
    } else {
        return false;
    }

    //printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_secondary_search");
}

int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    categorias_primario_index categoria_atual;

    if(exibir_caminho)
        printf(REGS_PERCORRIDOS);

    int i = 0;
    while(indice != -1) {
        if(exibir_caminho)
            printf(" %d", indice);
        *indice_final = indice;
        categoria_atual = t->categorias_primario_idx[indice];
        strncpy(result[i], categoria_atual.chave_primaria, TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX); // escreve em result o id_game
        indice = categoria_atual.proximo_indice;
        i++;
    }

    if(exibir_caminho)
        printf("\n");

    return i; // retorna a qtd de chaves encontradas
    
    //printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_primary_search");
}


char* strpadright(char *str, char pad, unsigned size) {
    for (unsigned i = strlen(str); i < size; ++i)
        str[i] = pad;
    str[size] = '\0';
    return str;
}


/* Funções da busca binária */
void* busca_binaria(const void *key, const void *base0, size_t nmemb, size_t size, int (*compar)(const void *, const void *), bool exibir_caminho) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    const char *base = (const char *) base0;
	int lim, cmp, pos_atual;
	const void *p;

    if(exibir_caminho)
        printf(REGS_PERCORRIDOS);

	for (lim = nmemb; lim != 0; lim /= 2) {
		p = base + (lim / 2) * size;
        pos_atual = ((int)p - (int)base0) / size; // subtrai a posicao de endereco atual da posicao do inicio do vetor e depois divide pelo tamanho de cada indice para printar o indice atual
        if(exibir_caminho) printf(" %d", pos_atual); 
		cmp = (*compar)(key, p);
		if (cmp == 0) {
            if(exibir_caminho) printf("\n");
			return (void *)p;
        }
		if (cmp > 0) {
			base = (const char *)p + size;
			lim--;
		}
	}

    if(exibir_caminho) printf("\n");
	return (NULL);
    
    //printf(ERRO_NAO_IMPLEMENTADO, "busca_binaria");
}

void* busca_binaria_piso(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*)) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    const char *base_aux = (const char *) base;
	int lim, cmp, cmp2;
	const void *meio, *maior, *menor, *meio_menos_um, *piso = NULL;
    int menor_idx = 0, maior_idx = num - 1, meio_idx;

    menor = base_aux + (menor_idx * size);
    maior = base_aux + (maior_idx * size);

    while(menor_idx <= maior_idx) {
        
        // se a menor for maior que a maior
        cmp = (*compar)(menor, maior);
        if (cmp > 0)
            return NULL;
    
        // compara a chave atual com o maior atual
        cmp = (*compar)(key, maior);
        if (cmp >= 0)
            return (void*)maior;

        // encontra a metade
        meio = base_aux + ((maior_idx + menor_idx) / 2) * size;
        meio_idx = (maior_idx + menor_idx) / 2;

        // encontrou o elemento
        cmp = (*compar)(key, meio);
        if (cmp == 0) {
            return (void *)meio;
        }

        // se estiver entre meio - 1 e meio
        meio_menos_um = base_aux + ((meio_idx - 1) * size);
        if(meio_idx > 0) {
            cmp = (*compar)(key, meio_menos_um);
            cmp2 = (*compar)(key, meio);
            if (cmp >= 0 && cmp2 < 0)
                return (void*)meio_menos_um;
        }
        
        // se a chave for menor que o meio, entao esta na metade da esquerda, senao, esta na metade da direita
        cmp = (*compar)(key, meio);
        if (cmp < 0) {
            maior = meio_menos_um;
            maior_idx = meio_idx - 1;
        }
        else {
            menor = base_aux + ((meio_idx + 1) * size);
            menor_idx = meio_idx + 1;
            piso = meio;
        }

    }

    return (void*) piso;
    
    //printf(ERRO_NAO_IMPLEMENTADO, "busca_binaria_piso");
}

void* busca_binaria_teto(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*)) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    const char *base_aux = (const char *) base;
	int lim, cmp, cmp2;
	const void *meio, *maior, *menor, *meio_mais_um, *meio_menos_um, *teto = NULL;
    int menor_idx = 0, maior_idx = num - 1, meio_idx;

    menor = base_aux + (menor_idx * size);
    maior = base_aux + (maior_idx * size);

    while(menor_idx <= maior_idx) {
        
        // chave menor ou igual ao primeiro elemento
        cmp = (*compar)(key, menor);
        if (cmp <= 0)
            return (void*) menor;
    
        // chave maior que o ultimo elemento
        cmp = (*compar)(key, maior);
        if (cmp > 0)
            return NULL;

        // encontra a metade
        meio = base_aux + ((maior_idx + menor_idx) / 2) * size;
        meio_idx = (maior_idx + menor_idx) / 2;

        // encontrou o elemento
        cmp = (*compar)(key, meio);
        if (cmp == 0) {
            return (void *) meio;
        }

        // se a chave for maior que o meio, entao o teto eh meio + 1 ou esta entre meio + 1 e maior
        meio_mais_um = base_aux + ((meio_idx + 1) * size);
        if(cmp > 0) {
            cmp = (*compar)(meio_mais_um, maior);
            cmp2 = (*compar)(key, meio_mais_um);
            if(cmp <= 0 && cmp2 <= 0)
                return (void*) meio_mais_um;
            else {
                menor = meio_mais_um;
                menor_idx = meio_idx + 1;
            }
        }
        // se a chave for menor que o meio, entao ou o meio eh o teto, ou o teto esta entre menor e meio - 1
        else {
            meio_menos_um = base_aux + ((meio_idx - 1) * size);
            cmp = (*compar)(key, meio_menos_um);

            if(meio_idx - 1 <= meio_idx && cmp > 0) {
                return (void*) meio;
            } else {
                maior = meio_menos_um;
                maior_idx = meio_idx - 1;
                teto = meio;
            }
        }

    }

    return (void*)teto;
    
    //printf(ERRO_NAO_IMPLEMENTADO, "busca_binaria_teto");
}
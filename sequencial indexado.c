#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

#define TAMANHO_EVENT_TIME 24
#define TAMANHO_EVENT_TYPE 11
#define TAMANHO_PRODUCT_ID 11
#define TAMANHO_CATEGORY_ID 20
#define TAMANHO_CATEGORY_CODE 40
#define TAMANHO_BRAND 31
#define TAMANHO_PRICE 9
#define TAMANHO_USER_ID 10
#define TAMANHO_USER_SESSION 40




#define BUFFER_SIZE 7000000 // Ajuste para n�o usar muita mem�ria
#define PREFIXO_ARQ "temp_"

typedef struct { //struct de User
    unsigned long serial;
    char event_time[TAMANHO_EVENT_TIME];
    char event_type[TAMANHO_EVENT_TYPE];
    char user_id[TAMANHO_USER_ID];
    char user_session[TAMANHO_USER_SESSION];
    int excluido; // 0 para ativo, 1 para exclu�do
    long elo; //campo que contem o endere�o do pr�ximo Usu�rio que acessou o mesmo produto, cont�m NULL se foi o �ltimo a acessar
} User;

typedef struct { //struct de Prod
    unsigned long serial;
    char product_id[TAMANHO_PRODUCT_ID];
    char category_id[TAMANHO_CATEGORY_ID];
    char category_code[TAMANHO_CATEGORY_CODE];
    char brand[TAMANHO_BRAND];
    char price[TAMANHO_PRICE];
    long endereco; //campo que cont�m o endere�o do primeiro acesso desse produto, ser� formado pegando o serial*sizeof(User) antes de remover os repetidos
} Produto;

void menu(){
    printf("1 - extrair csv original\n");
    printf("2 - Compacta e ordena Arquivo de Produtos\n");
    printf("3 - Exibir Arquivo de usu�rios\n");
    printf("4 - Exibir Arquivo de usu�rios ordenado por produtos pesquisados\n");
    printf("5 - Exibir Arquivo de Produtos\n");
    printf("6 - Consultar Produto por ID\n");
    printf("7 - Consultar todos os usu�rios que pesquisaram por um produto\n");
    printf("8 - Consultar usu�rio por serial\n");
    printf("0 - Sair\n");
}

// Fun��o de compara��o para ordenar por category_id
int comparar(const void *a, const void *b) {
    Produto *produtoA = (Produto *)a;
    Produto *produtoB = (Produto *)b;
    return strcmp(produtoA->product_id, produtoB->product_id);
}

// Fun��o para ordena��o e escrita de uma parti��o
void ordena(FILE *input, int part_num, Produto buffer[], size_t contador_leitura) {
    qsort(buffer, contador_leitura, sizeof(Produto), comparar);

    char nome_temp[20];
    snprintf(nome_temp, sizeof(nome_temp), "%s%d.bin", PREFIXO_ARQ, part_num);
    FILE *temp_file = fopen(nome_temp, "wb");
    fwrite(buffer, sizeof(Produto), contador_leitura, temp_file);
    fclose(temp_file);
}

// Fun��o para fazer o merge dos arquivos tempor�rios
void merge_files(int num_parts) {


    char nome_temp[20];
    snprintf(nome_temp, sizeof(nome_temp), "Arquivo_Ord.bin");
    FILE *output = fopen(nome_temp, "wb");
    if(output == NULL){
        printf("Erro ao abrir o arquivo ordenado");
        return;
    }




    FILE **temp_files = malloc(num_parts * sizeof(FILE *));
    Produto *buffer = malloc(num_parts * sizeof(Produto));
    int *concluidos = malloc(num_parts * sizeof(int));

    if (temp_files == NULL || buffer == NULL || concluidos == NULL) {
        printf("Erro ao alocar mem�ria.\n");
        return;
    }




    for (int i = 0; i < num_parts; i++) {
        char nome_temp[20];
        snprintf(nome_temp, sizeof(nome_temp), "%s%d.bin", PREFIXO_ARQ, (i));
        temp_files[i] = fopen(nome_temp, "rb");
        concluidos[i] = (fread(&buffer[i], sizeof(Produto), 1, temp_files[i]) != 1);
    }


    while (1) {
        int menor = -1;
        for (int i = 0; i < num_parts; i++) {
            if (!concluidos[i] && (menor == -1 || strcmp(buffer[i].product_id, buffer[menor].product_id) < 0)) {
                menor = i;
            }
        }
        if (menor == -1) break; // Todos os arquivos foram lidos

        fwrite(&buffer[menor], sizeof(Produto), 1, output);
        concluidos[menor] = (fread(&buffer[menor], sizeof(Produto), 1, temp_files[menor]) != 1);
    }

    for (int i = 0; i < num_parts; i++) {
        fclose(temp_files[i]);
    }
    fclose(output);
}


int excluirUsuarioPorId(const char *nomeArquivo, const char *user_id_excluir) {
    FILE *arquivo = fopen(nomeArquivo, "rb+");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return -1;
    }

    User user;
    int encontrado = 0;


    while (fread(&user, sizeof(User), 1, arquivo) == 1) {
        if (strcmp(user.user_id, user_id_excluir) == 0 && user.excluido == 0) {

            user.excluido = 1;
            fseek(arquivo, -sizeof(User), SEEK_CUR);
            fwrite(&user, sizeof(User), 1, arquivo);
            encontrado = 1;
            printf("Usu�rio com ID %s exclu�do logicamente.\n", user_id_excluir);
            break;
        }
    }

    fclose(arquivo);

    if (!encontrado) {
        printf("Usu�rio com ID %s n�o encontrado ou j� exclu�do.\n", user_id_excluir);
        return -1;
    }

    return 0;
}


void compactarArquivo(char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "rb");
    FILE *arquivoNovo = fopen("arquivo_compactado.bin", "wb");

    if (arquivo == NULL || arquivoNovo == NULL) {
        printf("Erro ao abrir os arquivos.\n");
        return;
    }

    User user;
    while (fread(&user, sizeof(User), 1, arquivo) == 1) {
        if (user.excluido == 0) {
            fwrite(&user, sizeof(User), 1, arquivoNovo);
        }
    }

    fclose(arquivo);
    fclose(arquivoNovo);
    remove(nomeArquivo);
    rename("arquivo_compactado.bin", nomeArquivo);
}


// Fun��o para preencher o arquivo bin�rio com dados do struct
void preencherBinario(User *user, Produto *produto, FILE *arqUser, FILE *arqProd) {
    fwrite(user, sizeof(User), 1, arqUser);
    fwrite(produto, sizeof(Produto), 1, arqProd);
}

void verifica_arq(char *nomeArquivo){
    FILE *arquivo = fopen(nomeArquivo, "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", nomeArquivo);
        return;
    }

    unsigned long contador = 0;
    User user;
    Produto produto;
        while (fread(&produto, sizeof(Produto), 1, arquivo) == 1) {
            contador++;
        }
    printf("\n Contagem de registros em %s: %lu\n",nomeArquivo, contador);
    printf("\nUser:%u", produto.serial);
    fclose(arquivo);
}

// Fun��o para ler e imprimir o arquivo bin�rio
void lerArquivoBin(char *nomeArquivo, int aux) {
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", nomeArquivo);
        return;
    }

    if (aux == 0) { // Ler Users
        User user;
        while (fread(&user, sizeof(User), 1, arquivo) == 1) {
            if(user.excluido != 0){
            //printf("Serial: %lu\n", user.serial);
            printf("Event time: %s\n", user.event_time);
            printf("Event type: %s\n", user.event_type);
            printf("User ID: %s\n", user.user_id);
            printf("User Session: %s\n", user.user_session);
            //printf("Elo: %ld\n", user.elo);
            printf("-------------------------------\n");
            }
        }
    } else {
        if (aux == 1){ // Ler Produtos
            Produto produto;
            while (fread(&produto, sizeof(Produto), 1, arquivo) == 1) {
                    //printf("Serial: %lu\n", produto.serial);
                    printf("Product ID: %s\n", produto.product_id);
                    printf("Category ID: %s\n", produto.category_id);
                    printf("Category code: %s\n", produto.category_code);
                    printf("Brand: %s\n", produto.brand);
                    printf("Price: %s USD\n", produto.price);
                    printf("-------------------------------\n");
            }
        }
    }
    fclose(arquivo);
}


void Extrai_csv(char* nome_arq,char* nome_arqUser,char* nome_arqProd){
    FILE *arq = fopen(nome_arq, "r");
    if (arq == NULL) {
        printf("Erro ao abrir o CSV\n");
        return;
    }

    FILE *arqUser = fopen(nome_arqUser, "wb");
    if (arqUser == NULL) {
        printf("Erro ao abrir o arquivo bin�rio de usu�rios\n");
        return;
    }

    FILE *arqProd = fopen(nome_arqProd, "wb");
    if (arqProd == NULL) {
        printf("Erro ao abrir o arquivo bin�rio de produtos\n");
        return;
    }

    char linha[MAX_LINE_LENGTH];
    unsigned long serial = 0;
    int linha_lida = 0;


    fgets(linha, sizeof(linha), arq); // Ignorar a primeira linha (cabe�alho)
    printf("\nExtraindo Arquivos do CSV...\n");
    while (fgets(linha, sizeof(linha), arq)&& linha_lida < 2930 ) { // Retirar o limite quando quiser processar o arquivo inteiro && linha_lida < 320930
    User user;
    Produto produto;

    user.serial = serial;
    produto.serial = serial;

    char *token;

    // Verifica o token de cada campo e assegura que n�o esteja vazio ou mal formatado
    token = strtok(linha, ",");
    if (token == NULL) continue; // Pular linhas vazias ou mal formatadas
    strncpy(user.event_time, token, TAMANHO_EVENT_TIME - 1);
    user.event_time[TAMANHO_EVENT_TIME - 1] = '\0'; // Garante que a string termine corretamente

    token = strtok(NULL, ",");
    if (token == NULL) continue;
    strncpy(user.event_type, token, TAMANHO_EVENT_TYPE - 1);
    user.event_type[TAMANHO_EVENT_TYPE - 1] = '\0';

    token = strtok(NULL, ",");
    if (token == NULL) continue;
    strncpy(produto.product_id, token, TAMANHO_PRODUCT_ID - 1);
    produto.product_id[TAMANHO_PRODUCT_ID - 1] = '\0';

    token = strtok(NULL, ",");
    if (token == NULL) continue;
    strncpy(produto.category_id, token, TAMANHO_CATEGORY_ID - 1);
    produto.category_id[TAMANHO_CATEGORY_ID - 1] = '\0';

    token = strtok(NULL, ",");
    if (token == NULL) continue;
    strncpy(produto.category_code, token, TAMANHO_CATEGORY_CODE - 1);
    produto.category_code[TAMANHO_CATEGORY_CODE - 1] = '\0';

    token = strtok(NULL, ",");
    if (token == NULL) continue;
    strncpy(produto.brand, token, TAMANHO_BRAND - 1);
    produto.brand[TAMANHO_BRAND - 1] = '\0';

    token = strtok(NULL, ",");
    if (token == NULL) continue;
    strncpy(produto.price, token, TAMANHO_PRICE - 1);
    produto.price[TAMANHO_PRICE - 1] = '\0';

    token = strtok(NULL, ",");
    if (token == NULL) continue;
    strncpy(user.user_id, token, TAMANHO_USER_ID - 1);
    user.user_id[TAMANHO_USER_ID - 1] = '\0';

    token = strtok(NULL, "\n");
    if (token == NULL) continue;
    strncpy(user.user_session, token, TAMANHO_USER_SESSION - 1);
    user.user_session[TAMANHO_USER_SESSION - 1] = '\0';

    // Preencher os dados bin�rios
    preencherBinario(&user, &produto, arqUser, arqProd);

    serial++;
    linha_lida++;
}

    fclose(arqUser);
    fclose(arqProd);
    fclose(arq);
    printf("Registros extraidos com sucesso!\n");
}


void OrdenaEMerge(char* nome_arqProd){

    printf("\nParticionando...\n");
    FILE *arqProdOrdena = fopen(nome_arqProd, "rb");
    if(arqProdOrdena == NULL){
        printf("erro ao abrir arquivo para ordena��o");
        return;
    }
    int part_num = 0;


    Produto *buffer = malloc(BUFFER_SIZE * sizeof(Produto));  // Aloca dinamicamente
    if (buffer == NULL) {
    printf("Erro ao alocar mem�ria.\n");
    return;
    };
    size_t contador_leitura;

    while ((contador_leitura = fread(buffer, sizeof(Produto), BUFFER_SIZE, arqProdOrdena)) > 0) {
    ordena(arqProdOrdena, part_num, buffer, contador_leitura);
    part_num++;
    }
    printf("Arquivo Particionado!\n");
    fclose(arqProdOrdena);

    printf("Ordenando...\n");
    merge_files(part_num);
    free(buffer);
    printf("Arquivo Ordenado!\n");

    // Remover arquivos tempor�rios
    for (int i = 0; i < part_num; i++) {
        char temp_filename[20];
        snprintf(temp_filename, sizeof(temp_filename), "%s%d.bin", PREFIXO_ARQ, i);
        remove(temp_filename);
    }

}

unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

// Fun��o para verificar se j� existe um product_id
int jaExiste(char product_id[], unsigned long *hashTable, int tamanhoHash) {
    unsigned long pos = hash(product_id) % tamanhoHash;
    return hashTable[pos] != 0;
}

// Fun��o para adicionar um product_id � tabela hash
void adicionarTabela(char product_id[], unsigned long *hashTable, int tamanhoHash) {
    unsigned long pos = hash(product_id) % tamanhoHash;
    hashTable[pos] = 1; // 1 indica que o product_id est� presente
}

// Fun��o para remover produtos duplicados de um array
void remove_repetidos(char* nomeArquivo) {

    FILE *arquivo = fopen(nomeArquivo, "rb");
    FILE *arquivoNovo = fopen("arqProdatt.bin", "wb");
    FILE *arquivoUsers = fopen("arqUser.bin", "rb+"); // tem que fazer algo pra ele n�o excluir o conte�do antigo

    if (arquivo == NULL || arquivoNovo == NULL || arquivoUsers == NULL) {
        printf("Erro ao abrir arquivo para remover repetidos\n");
        return;
    }

    Produto *produtoBuffer = malloc(BUFFER_SIZE * sizeof(Produto));
    if (produtoBuffer == NULL) {
        printf("Erro ao alocar mem�ria 1\n");
        return;
    }

    while (1) {
        size_t lidos = fread(produtoBuffer, sizeof(Produto), BUFFER_SIZE, arquivo);
        if (lidos == 0) break; // fim do arquivo

        int tamanhoHash = BUFFER_SIZE * 2;  // Tamanho da tabela hash, aproximadamente o dobro do n�mero de produtos
        unsigned long *hashTable = calloc(tamanhoHash, sizeof(unsigned long));

        if (hashTable == NULL) {
            printf("Erro ao alocar mem�ria para a tabela hash.\n");
            free(produtoBuffer);
            return;
        }

        int countSemRepetidos = 0;

        Produto *produtosSemRepetidos = malloc(BUFFER_SIZE * sizeof(Produto));
        if (produtosSemRepetidos == NULL) {
            printf("Erro ao alocar mem�ria 2\n");
            free(hashTable);
            free(produtoBuffer);
            return;
        }

        long enderecoUsuario;
        long enderecoaux;
        User *user = malloc(sizeof(User));
        if (user == NULL) {
            printf("Erro ao alocar mem�ria para usu�rio.\n");
            free(hashTable);
            free(produtosSemRepetidos);
            free(produtoBuffer);
            return;
        }
        User useraux;
        for (size_t i = 0; i < lidos; i++) {
            if (!jaExiste(produtoBuffer[i].product_id, hashTable, tamanhoHash)) {
            // Se n�o existe, adiciona � tabela hash e ao novo array
                adicionarTabela(produtoBuffer[i].product_id, hashTable, tamanhoHash);
                produtoBuffer[i].endereco = produtoBuffer[i].serial * sizeof(User);
                enderecoUsuario = produtoBuffer[i].endereco;
                produtosSemRepetidos[countSemRepetidos++] = produtoBuffer[i];
            } else { // Se j� existe, atualize o elo dos usu�rios
                // Buscar o usu�rio na posi��o correta
                fseek(arquivoUsers, enderecoUsuario, SEEK_SET);
                fread(user, sizeof(User), 1, arquivoUsers);

                // Atualizar o elo com o novo serial do produto

                user->elo = produtoBuffer[i].serial * sizeof(User);
                enderecoUsuario = user->elo;

                // Reposiciona o cursor no arquivo para atualizar o registro do usu�rio
                fseek(arquivoUsers, -sizeof(User), SEEK_CUR);
                fwrite(user, sizeof(User), 1, arquivoUsers);
            }
        }

        fwrite(produtosSemRepetidos, sizeof(Produto), countSemRepetidos, arquivoNovo);  // Escreve os produtos sem repeti��o
        free(hashTable);  // Libera a mem�ria alocada para a tabela hash
        free(produtosSemRepetidos);  // Agora pode liberar ap�s a escrita no arquivo
    }

    free(produtoBuffer);  // Libera o buffer do produto ap�s a leitura completa
    fclose(arquivo);
    fclose(arquivoNovo);
    fclose(arquivoUsers);

    printf("Remocao de repetidos concluida.\n");
}

Produto *pesquisa_binaria(char *nome_arquivo,char *product_id) {
    FILE* arquivo = fopen(nome_arquivo,"rb");
    if(arquivo == NULL){
        printf("Erro na abertura de arquivo para pesquisa bin�ria");
        return 0;
    }

    fseek(arquivo,0,SEEK_END);
    long tamanho_bytes = ftell(arquivo);  // Tamanho total do arquivo em bytes
    int total_produtos = tamanho_bytes / sizeof(Produto);  // N�mero total de produtos
    fseek(arquivo, 0, SEEK_SET);

    int inicio = 0;
    int fim = total_produtos - 1;

    Produto *produto = malloc(sizeof(Produto));

    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;


        fseek(arquivo,meio*sizeof(Produto),SEEK_SET);

        fread(produto,sizeof(Produto),1,arquivo);

        // Comparar o product_id no meio com o product_id buscado
        int comparacao = strcmp(produto->product_id, product_id);

        if (comparacao == 0) {
            // Encontrou o produto
            fclose(arquivo);
            return produto;
        } else if (comparacao < 0) {
            //product_id buscado � maior, ajustar in�cio
            inicio = meio + 1;
        } else {
            //product_id buscado � menor, ajustar fim
            fim = meio - 1;
        }
    }

    //n�o encontrou
    fclose(arquivo);
    free(produto);
    return NULL;
}

void pesquisa_binaria_serial(char *nome_arquivo,unsigned long serial) {
    FILE* arquivo = fopen(nome_arquivo,"rb");
    if(arquivo == NULL){
        printf("Erro na abertura de arquivo para pesquisa bin�ria");
        return 0;
    }

    fseek(arquivo,0,SEEK_END);
    long tamanho_bytes = ftell(arquivo);  // Tamanho total do arquivo em bytes
    int total_users = tamanho_bytes / sizeof(User);  // N�mero total de produtos
    fseek(arquivo, 0, SEEK_SET);

    int inicio = 0;
    int fim = total_users - 1;

    User *user = malloc(sizeof(User));

    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;

        fseek(arquivo,meio*sizeof(User),SEEK_SET);

        fread(user,sizeof(User),1,arquivo);

        int comparacao = user->serial - serial;

        if (comparacao == 0) {
            // Encontrou o usu�rio
            fclose(arquivo);
            if(user->excluido != 1){
            printf("Serial: %lu\n", user->serial);
            printf("Event time: %s\n", user->event_time);
            printf("Event type: %s\n", user->event_type);
            printf("User ID: %s\n", user->user_id);
            printf("User Session: %s\n", user->user_session);
            }
            else{
                printf("\nUsu�rio n�o encontrado!\n");
            }
            return;
        }
        else if (comparacao < 0) {
            //product_id buscado � maior, ajustar in�cio
            inicio = meio + 1;
            }
            else {
                //product_id buscado � menor, ajustar fim
                fim = meio - 1;
            }
    }

    //n�o encontrou
    fclose(arquivo);
    free(user);
    printf("\nUsu�rio n�o encontrado!\n");
}

void consulta_clientes_por_product_id(char* nome_prod, char* nome_user, char* product_id){
    FILE* arquivoUser = fopen(nome_user,"rb");
    if(arquivoUser == NULL){
        printf("Erro ao abrir arquivo para consulta");
        return;
    }

    User* user = malloc(sizeof(User));
if (user == NULL) {
    printf("Erro ao alocar mem�ria para o usu�rio.\n");
    return;
}
    Produto* produto = pesquisa_binaria(nome_prod,product_id);
    if (produto == NULL) {
        printf("Produto n�o encontrado.\n");
        free(user);
        fclose(arquivoUser);
        return;
    }



    fseek(arquivoUser,0,SEEK_END);
    long aux = ftell(arquivoUser);

    fseek(arquivoUser,produto->endereco,SEEK_SET);
    if (fread(user, sizeof(User), 1, arquivoUser) != 1) {
        printf("Erro ao ler usu�rio do arquivo.\n");
        free(user);  // Libere a mem�ria antes de sair
        fclose(arquivoUser);
        return;
    }
    printf("\n\tUsu�rios que pesquisaram por esse produto:\n");
    if(user->excluido != 1){
            printf("Serial: %lu\n", user->serial);
            printf("Event time: %s\n", user->event_time);
            printf("Event type: %s\n", user->event_type);
            printf("User ID: %s\n", user->user_id);
            printf("User Session: %s\n", user->user_session);
            //printf("Elo: %ld\n", user->elo);
            printf("-------------------------------\n");
    }
    while(user->elo != 0){
            fseek(arquivoUser,user->elo,SEEK_SET);
            if (fread(user, sizeof(User), 1, arquivoUser) != 1) {
                printf("Erro ao ler usu�rio do arquivo.\n");
                free(user);  // Libere a mem�ria antes de sair
                fclose(arquivoUser);
                return;
            }
            if(user->excluido != 1){
            printf("Serial: %lu\n", user->serial);
            printf("Event time: %s\n", user->event_time);
            printf("Event type: %s\n", user->event_type);
            printf("User ID: %s\n", user->user_id);
            printf("User Session: %s\n", user->user_session);
            //printf("Elo: %ld\n", user->elo);
            printf("-------------------------------\n");
            }
    }

    free(user);
    fclose(arquivoUser);
}

void arq_user_ordenado_por_ID_produto(char *nome_user,char *nome_prod){

    FILE *arquivo = fopen(nome_prod, "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", nome_prod);
        return;
    }
    Produto produto;
    while (fread(&produto, sizeof(Produto), 1, arquivo) == 1) {
        printf("\tProduto:\n");
        //printf("Serial: %lu\n", produto.serial);
        printf("Product ID: %s\n", produto.product_id);
        printf("Category ID: %s\n", produto.category_id);
        printf("Category code: %s\n", produto.category_code);
        printf("Brand: %s\n", produto.brand);
        printf("Price: %s USD\n", produto.price);
        consulta_clientes_por_product_id(nome_prod,nome_user,produto.product_id);
        printf("*******************************************\n");
    }

}


int main() {
    char* nome_arq = "arquivo_modificado.csv";
    char* nome_arqUser = "arqUser.bin";
    char* nome_arqProd = "arqProd.bin";

        if (_setmaxstdio(2048) == -1) {
        printf("Erro ao aumentar o limite de arquivos abertos.\n");
    }
    // Parti��es e Merge

    Produto* resultado = malloc(sizeof(Produto));
    char entrada[11] = "";
    unsigned long entrada_serial = 0;


    int opcao = 1;
    menu();
    scanf("%d",&opcao);
    while (opcao!=0){
        switch(opcao){
            case 1:
                Extrai_csv(nome_arq,nome_arqUser,nome_arqProd);
                break;
            case 2:
                OrdenaEMerge(nome_arqProd);
                remove(nome_arqProd);
                rename("Arquivo_Ord.bin", nome_arqProd);
                printf("Removendo registros repetidos...\n");
                remove_repetidos(nome_arqProd);
                remove(nome_arqProd);
                rename("arqProdatt.bin", nome_arqProd);
                break;
            case 3:
                printf("\nUsu�rios:\n");
                lerArquivoBin(nome_arqUser, 0);
                break;
            case 4:
                arq_user_ordenado_por_ID_produto(nome_arqUser,nome_arqProd);
                break;
            case 5:
                printf("\nProdutos:\n");
                lerArquivoBin(nome_arqProd, 1);
                break;
            case 6:
                printf("\nDigite o ID a ser pesquisado:\n");
                scanf("%s",entrada);
                resultado = pesquisa_binaria(nome_arqProd, entrada);
                if (resultado != NULL) {
                    printf("Produto encontrado:\n");
                    printf("ID: %s, Category code: %s, Brand:%s, Price: %s USD, Endere�o: %ld\n", resultado->product_id, resultado->category_code, resultado->brand,resultado->price, resultado->endereco);
                    free(resultado);  // Libera a mem�ria alocada
                }
                else {
                    printf("Produto n�o encontrado.\n");
                }
                break;
            case 7:
                printf("\nDigite o ID a ser pesquisado:\n");
                scanf("%s",entrada);
                consulta_clientes_por_product_id(nome_arqProd,nome_arqUser,entrada);
                break;
            case 8:
                printf("\nDigite o serial a ser pesquisado:\n");
                scanf("%lu",&entrada_serial);
                pesquisa_binaria_serial(nome_arqUser,entrada_serial);
                break;
            case 9:
                verifica_arq(nome_arqProd);
                break;
        }







        menu();
        scanf("%d",&opcao);
    }

    return 0;
}

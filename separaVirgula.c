#include <stdio.h>
#include <string.h>

void substituirVirgulasDuplas(char *linha) {
    char linha_modificada[1024];
    int i = 0, j = 0;

    while (linha[i] != '\0') {

        if (linha[i] == ',' && linha[i+1] == ',') {
            linha_modificada[j++] = ',';
            linha_modificada[j++] = 'N';
            linha_modificada[j++] = '/';
            linha_modificada[j++] = 'A';
        } else {
            linha_modificada[j++] = linha[i];
        }
        i++;
    }
    linha_modificada[j] = '\0';

    strcpy(linha, linha_modificada);
}

int main() {
    FILE *arqOriginal = fopen("2019-Nov.csv", "r");
    FILE *arqNovo = fopen("arquivo_modificado.csv", "w");

    if (arqOriginal == NULL || arqNovo == NULL) {
        printf("Erro ao abrir os arquivos.\n");
        return 1;
    }


    char linha[1024];


    while (fgets(linha, sizeof(linha), arqOriginal) != NULL) {
        substituirVirgulasDuplas(linha);
        fputs(linha, arqNovo);
    }

    fclose(arqOriginal);
    fclose(arqNovo);

    printf("Arquivo processado e salvo como 'arquivo_modificado.csv'.\n");

    return 0;
}

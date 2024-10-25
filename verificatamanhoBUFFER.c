#include <stdio.h>
#include <stdlib.h>

int main() {

    size_t tamanho = 1024 * 1024 * 512;  // 512 MB
    char *buffer;

    while ((buffer = malloc(tamanho)) != NULL) {
        printf("Alocou %zu bytes com sucesso.\n", tamanho);
        free(buffer);
        tamanho *= 2;  // Tenta alocar mais memória na próxima iteração
    }

    printf("Não foi possível alocar %zu bytes.\n", tamanho);

    printf("\n\n um tamanho a se considerar no BUFFER_SIZE é %zu",tamanho/240);

    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINHAS 19000
#define ERRO -1

typedef struct {
    double id;
    char numero[25];
    char data_ajuizamento[24];
    char id_classe[100];
    char id_assunto[100];
    int ano_eleicao;
} processo;

void apagar_aspas(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != '"') *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

void apagar_chaves(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != '{' && *src != '}') *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

void limpar_data(char *data) {
    char *espaco = strchr(data, ' ');
    if (espaco) *espaco = '\0';
    if (data[0] == '"') memmove(data, data + 1, strlen(data));
    char *fim = strrchr(data, '"');
    if (fim) *fim = '\0';
}

processo *LerDados(const char *nomeArquivo, int *quantidade) {
    FILE *fp = fopen(nomeArquivo, "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo.\n");
        exit(ERRO);
    }

    processo *dados = malloc(MAX_LINHAS * sizeof(processo));
    if (!dados) {
        printf("Erro de alocação de memória.\n");
        fclose(fp);
        exit(ERRO);
    }

    char *linha = NULL;
    size_t tamanho = 0;
    int i = 0;

    getline(&linha, &tamanho, fp); // pular cabeçalho

    while (getline(&linha, &tamanho, fp) != -1 && i < MAX_LINHAS) {
        linha[strcspn(linha, "\r\n")] = '\0';

        // Tokenização dos campos
        char *campos[6];
        int campo = 0;
        char *ptr = linha;
        int dentro_aspas = 0;
        campos[campo++] = ptr;

        while (*ptr && campo < 6) {
            if (*ptr == '"') {
                dentro_aspas = !dentro_aspas;
            } else if (*ptr == ',' && !dentro_aspas) {
                *ptr = '\0';
                campos[campo++] = ptr + 1;
            }
            ptr++;
        }

        if (campo < 6) continue;

        dados[i].id = atof(campos[0]);
        strncpy(dados[i].numero, campos[1], 24);
        dados[i].numero[24] = '\0';

        strncpy(dados[i].data_ajuizamento, campos[2], 23);
        dados[i].data_ajuizamento[23] = '\0';

        strncpy(dados[i].id_classe, campos[3], 99);
        dados[i].id_classe[99] = '\0';

        strncpy(dados[i].id_assunto, campos[4], 99);
        dados[i].id_assunto[99] = '\0';

        dados[i].ano_eleicao = atoi(campos[5]);

        apagar_aspas(dados[i].id_classe);
        apagar_aspas(dados[i].id_assunto);
        limpar_data(dados[i].data_ajuizamento);

        i++;
    }

    free(linha);
    fclose(fp);
    *quantidade = i;
    return dados;
}

void ordenar_ID(processo *dados, int qtd) {
    for (int i = 0; i < qtd - 1; i++) {
        for (int j = 0; j < qtd - i - 1; j++) {
            if (dados[j].id > dados[j + 1].id) {
                processo temp = dados[j];
                dados[j] = dados[j + 1];
                dados[j + 1] = temp;
            }
        }
    }

    FILE *fp = fopen("id_ORDERNADO.csv", "w");
    if (fp == NULL) {
        printf("Erro ao abrir o arquivo para escrita.\n");
        return;
    }

    // ✅ Escreve o cabeçalho
    fprintf(fp, "id,numero,data_ajuizamento,id_classe,id_assunto,ano_eleicao\n");

    for (int i = 0; i < qtd; i++) {
        fprintf(fp, "%.0lf,\"%s\",%s,%s,%s,%d\n", dados[i].id, dados[i].numero,
                dados[i].data_ajuizamento, dados[i].id_classe,
                dados[i].id_assunto, dados[i].ano_eleicao);
    }

    fclose(fp);
    printf("Arquivo id_ORDERNADO.csv criado com sucesso.\n");
    printf("Processos ordenados por ID:\n");
}


void ordenar_data_ajuizamento(processo *dados, int qtd) {
    for (int i = 0; i < qtd - 1; i++) {
        for (int j = 0; j < qtd - i - 1; j++) {
            if (strcmp(dados[j].data_ajuizamento, dados[j + 1].data_ajuizamento) < 0) {
                processo temp = dados[j];
                dados[j] = dados[j + 1];
                dados[j + 1] = temp;
            }
        }
    }

    
}

int calcular_dias_tramitacao(char *data_ajuizamento) {
    struct tm data_ajuiz = {0};
    sscanf(data_ajuizamento, "%d-%d-%d", &data_ajuiz.tm_year, &data_ajuiz.tm_mon, &data_ajuiz.tm_mday);
    data_ajuiz.tm_year -= 1900;
    data_ajuiz.tm_mon -= 1;

    time_t t_ajuiz = mktime(&data_ajuiz);
    time_t t_atual = time(NULL);

    return (int)(difftime(t_atual, t_ajuiz) / (60 * 60 * 24));
}

void menu_opcao(processo *dados, int qtd) {
    printf("Escolha uma opção:\n");
    printf("1. Ordenar por crescente - ID\n");
    printf("2. Ordenar por decrescente - Data de Ajuizamento\n");
    printf("3. Contar quantos processos estão vinculados a um determinado \"id_classe\"\n");
    printf("4. Identificar quantos \"id_assuntos\" constam nos processos presentes na base de dados\n");
    printf("5. Listar todos os processos que estão vinculados a mais de um assunto\n");
    printf("6. Indicar a quantos dias um processo está em tramitação na justiça\n");
    printf("7. Sair\n");

    int opcao;
    scanf("%d", &opcao);

    switch (opcao) {
        case 1:
            ordenar_ID(dados, qtd);
            // for (int i = 0; i < qtd; i++) {
            //     printf("%.0lf,\"%s\",%s,%s,%s,%d\n", dados[i].id, dados[i].numero,
            //         dados[i].data_ajuizamento, dados[i].id_classe,
            //         dados[i].id_assunto, dados[i].ano_eleicao);
            // }
            break;
        case 2:
            ordenar_data_ajuizamento(dados, qtd);
            for (int i = 0; i < qtd; i++) {
                printf("%.0lf,\"%s\",%s,%s,%s,%d\n", dados[i].id, dados[i].numero,
                    dados[i].data_ajuizamento, dados[i].id_classe,
                    dados[i].id_assunto, dados[i].ano_eleicao);
            }
            break;
        case 6:
            for (int i = 0; i < qtd; i++) {
                int dias = calcular_dias_tramitacao(dados[i].data_ajuizamento);
                printf("Processo ID %.0lf está em tramitação há %d dias.\n", dados[i].id, dias);
            }
            break;
        default:
            printf("Opção não implementada ainda.\n");
            break;
    }
}

int main() {
    int qtd = 0;
    processo *dados = LerDados("processo_043_202409032338.csv", &qtd);

    menu_opcao(dados, qtd);

    free(dados);
    system("pause");
    return 0;
}

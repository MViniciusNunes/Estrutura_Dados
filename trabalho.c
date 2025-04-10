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

// Corrige campos entre aspas, com chaves duplicadas, etc.
void CorrigirCampo(char *dados) {
    char *inicio = strchr(dados, '{');
    char *fim = strrchr(dados, '}');

    if (inicio && fim && fim > inicio) {
        memmove(dados, inicio + 1, fim - inicio - 1);
        dados[fim - inicio - 1] = '\0';
    }
}


void remover_chaves(char *dados); // Function prototype

void remover_chaves_aspas(char *dados) {
    // Remove aspas externas (se existirem)
    if (dados[0] == '"') {
        memmove(dados, dados + 1, strlen(dados)); // remove a primeira aspa
        char *ultima_aspa = strrchr(dados, '"');
        if (ultima_aspa) *ultima_aspa = '\0'; // remove a última
    }

    remover_chaves(dados); // Remove chaves
}

void remover_chaves(char *dados) {
    // Remove chaves externas (se existirem)
    if (dados[0] == '{') {
        memmove(dados, dados + 1, strlen(dados)); // remove a chave inicial
        char *ultima_chave = strrchr(dados, '}');
        if (ultima_chave) *ultima_chave = '\0'; // remove a chave final
    }
}

processo *LerDados(const char *nomeArquivo, int *quantidade) {
    FILE *fp = fopen(nomeArquivo, "r");
    if (fp == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        exit(ERRO);
    }

    processo *dados = malloc(MAX_LINHAS * sizeof(processo));
    if (dados == NULL) {
        printf("Erro de alocação de memória.\n");
        exit(ERRO);
    }

    char linha[512];
    fgets(linha, sizeof(linha), fp); // pula o cabeçalho
    int i = 0;

    while (fgets(linha, sizeof(linha), fp) != NULL && i < MAX_LINHAS) {
        linha[strcspn(linha, "\r\n")] = 0;

        // Pega o último campo: ano_eleicao
        char *ultimo = strrchr(linha, ',');
        if (ultimo == NULL) continue;
        dados[i].ano_eleicao = atoi(ultimo + 1);
        *ultimo = '\0';

        int campos_lidos = sscanf(linha, "%lf,\"%24[^\"]\",%23[^,],%99[^,],%99[^,]",
                                  &dados[i].id,
                                  dados[i].numero,
                                  dados[i].data_ajuizamento,
                                  dados[i].id_classe,
                                  dados[i].id_assunto);

        // Ensure strings are null-terminated
        dados[i].numero[24] = '\0';
        dados[i].data_ajuizamento[23] = '\0';
        dados[i].id_classe[99] = '\0';
        dados[i].id_assunto[99] = '\0';

        if (campos_lidos != 5) {
            printf("Erro ao ler linha: %s\n", linha);
            continue; // pula para a próxima linha
        }

        // Corrige os campos com possíveis aspas/chaves duplicadas
        CorrigirCampo(dados[i].id_classe);
        CorrigirCampo(dados[i].id_assunto);

        i++;
    }


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

void limpar_data(char *data) {
    char *espaco = strchr(data, ' ');
    if (espaco != NULL) *(espaco + 9) = '\0'; // corta após os segundos
    if (data[0] == '"') memmove(data, data + 1, strlen(data)); // remove aspas iniciais
    char *fim = strrchr(data, '"');
    if (fim) *fim = '\0'; // remove aspas finais
}


int calcular_dias_tramitacao(char *data_ajuizamento) {
    struct tm data_ajuiz;
    memset(&data_ajuiz, 0, sizeof(struct tm));
    
    // Converte a string para struct tm (só a data, sem a hora)
    sscanf(data_ajuizamento, "%d-%d-%d", 
           &data_ajuiz.tm_year, &data_ajuiz.tm_mon, &data_ajuiz.tm_mday);
    
    data_ajuiz.tm_year -= 1900; // ano desde 1900
    data_ajuiz.tm_mon -= 1;     // mês de 0 a 11
    
    time_t tempo_ajuiz = mktime(&data_ajuiz); // converte para timestamp
    
    time_t tempo_atual = time(NULL);
    
    double segundos = difftime(tempo_atual, tempo_ajuiz);
    int dias = (int)(segundos / (60 * 60 * 24));
    
    return dias;
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

    switch(opcao)
    {
    case 1:
        ordenar_ID(dados, qtd);
        printf("Processos ordenados por ID:\n");
        for (int i = 0; i < qtd; i++) {
            printf("%.0lf,\"%s\",%s,%s,%s,%d\n",
                   dados[i].id,
                   dados[i].numero,
                   dados[i].data_ajuizamento,
                   dados[i].id_classe,
                   dados[i].id_assunto,
                   dados[i].ano_eleicao);
        }
        break;
    case 2:
        ordenar_data_ajuizamento(dados, qtd);
        printf("Processos ordenados por Data de Ajuizamento:\n");
        for (int i = 0; i < qtd; i++) {
            printf("%.0lf,\"%s\",%s,%s,%s,%d\n",
                   dados[i].id,
                   dados[i].numero,
                   dados[i].data_ajuizamento,
                   dados[i].id_classe,
                   dados[i].id_assunto,
                   dados[i].ano_eleicao);
        }
        break;
    case 3:
        // Implementar contagem de processos por id_classe
        break;
    case 4:
        // Implementar contagem de id_assuntos
        break;
    case 5:
        // Implementar listagem de processos com mais de um assunto
        break;
    case 6:
    for (int i = 0; i < qtd; i++) {
        int dias = calcular_dias_tramitacao(dados[i].data_ajuizamento);
        printf("Processo ID %.0lf está em tramitação há %d dias.\n", dados[i].id, (int)dias);
    }
        break;

    default:
    printf("Opção inválida!\n");
        break;
    }
}




int main() {
    int qtd = 0;
    processo *dados = LerDados("processo_043_202409032338.csv", &qtd);

    menu_opcao(dados, qtd);

    // ordenar_data_ajuizamento(dados, qtd);

    /*for (int i = 0; i < 5000; i++) {
        printf("%.0lf,\"%s\",%s,%s,%s,%d\n",
               dados[i].id,
               dados[i].numero,
               dados[i].data_ajuizamento,
               dados[i].id_classe,
               dados[i].id_assunto,
               dados[i].ano_eleicao);
    }*/


    

    free(dados);
    system("pause");
        return 0;
}

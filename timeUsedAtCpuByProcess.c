// entrada ./test 1234

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Biblioteca que contém constante HZ (número de jiffies por segundo).
#include <sys/param.h>

float obterUsoDaCPUPeloProcesso(int pid, int tempoAtualizacao)
{
    int vNULLI;
    char vNULLC[100];
    char nomeArquivo[100];

    int pid_stat;
    int jiffiesUsuario;
    int jiffiesKernel;
    float jiffiesTotal1;
    float jiffiesTotal2;
    float porcentagem;
    FILE* stat;

    sprintf(nomeArquivo, "/proc/%d/stat", pid);

    // Abre aquivo /proc/PID/stat.
    if ( (stat = fopen(nomeArquivo, "r")) != NULL ){
        fscanf(stat, "%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &pid_stat, vNULLC, vNULLC, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &jiffiesUsuario, &jiffiesKernel);
        fclose(stat);

        // Soma total de jiffies.
        jiffiesTotal1 = (float)(jiffiesUsuario + jiffiesKernel);
    }

    sleep(tempoAtualizacao);

    // Abre aquivo /proc/PID/stat.
    if ( (stat = fopen(nomeArquivo, "r")) != NULL ){
        fscanf(stat, "%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &vNULLI, vNULLC, vNULLC, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &jiffiesUsuario, &jiffiesKernel);
        fclose(stat);

        // Soma total de jiffies.
        jiffiesTotal2 = (float)(jiffiesUsuario + jiffiesKernel);
    }

    // NO FINAL FOI FEITA DIVISÃO POR 4, POIS SÃO 4 NÚCLEOS
    porcentagem = (( (jiffiesTotal2 - jiffiesTotal1) / (HZ * tempoAtualizacao) ) * 100.00) / 4;

    return porcentagem;
}

int obtemMemoriaTotalProcesso(int pid)
{
    int vNULLI;
    FILE* statm;
    char nomeArquivo[100];
    int numeroDePaginasMemoria;
    int memoriaEmKb = 0;

    // Obtém o nome do arquivo.
    sprintf(nomeArquivo, "/proc/%d/statm", pid);

    // Abre o arquivo statm do processo.
    if ( (statm = fopen(nomeArquivo, "r")) != NULL )
    {
        fscanf(statm, "%d\t%d\t%d", &vNULLI, &vNULLI, &numeroDePaginasMemoria);

        fclose(statm);

        // Obtém o uso de memória em Kbytes. Tamanho de página = 4 KB.
        memoriaEmKb = numeroDePaginasMemoria * 4;
    }
    return memoriaEmKb;
}

int main (int argc, char *argv[])
{
    int pid;
    int tempo_atualizacao = 2;
    int i = 0;

    printf("------------------------------------\n");
    printf("ID |   PID   |  Mémoria  |   CPU    |\n");
    printf("------------------------------------\n");

    pid = atoi(argv[1]);

    // LAÇO DE REPETIÇÃO QUE FICA IMPRIMINDO AS INFORMAÇÕES
    while(1) {

        if (i < 10){
            printf("0");
        }
        printf("%d |   %d  | %d KB | %f |\n", i, pid, obtemMemoriaTotalProcesso(pid), obterUsoDaCPUPeloProcesso(pid, tempo_atualizacao));
        i++;
    }
}

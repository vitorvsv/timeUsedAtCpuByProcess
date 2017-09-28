#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>

// Biblioteca que contém constante HZ (número de jiffies por segundo).
#include <sys/param.h>

// Cria estrutura de informações sobre a memória.
struct memoria
{
    int memoriaTotal;
    int memoriaLivre;
    int cached;
    int swapCached;
    int swapTotal;
    int swapLivre;
    float porcentagemUso;
    float porcentagemUsoSwap;
};

// Cria estrutura de informações de CPU e processamento.
struct cpu
{
    int jiffiesTotal;
    int jiffiesOcioso;
    float porcentagemUso;
};

// Cria struct com informações do processo.
struct processo
{
    int pid;
    char *nomeProcesso;
    char *estadoProcesso;
};


// Semafaros.
pthread_mutex_t mutex_strtok;
pthread_mutex_t mutex_printw;

/**
 * Lê arquivo.
 *
 * @param char[1024] Caminho absoluto do arquivo.
 * @return char[4000] Conteúdo do arquivo.
 */
char *lerArquivo(char nomeDoArquivo[1024])
{
    FILE* arquivo;
    char conteudo[4000];
    size_t bytes_lidos;
    
    if ( (arquivo = fopen (nomeDoArquivo, "r")) != NULL )
    {
        bytes_lidos = fread (conteudo, 1, sizeof (conteudo), arquivo);
        fclose (arquivo);

        conteudo[bytes_lidos] = '\0';
    }
    
    return conteudo;
}

/**
 * Obtém Informações da CPU.
 */
struct cpu *obterInformacoesCPU()
{
    // Cria variável "cpu", alocando memória proporcional a struct cpu.
    struct cpu *cpu = (struct cpu*)malloc(sizeof(struct cpu));
    
    FILE* stat;
    
    char vNull;
    int jiffiesUsuario;
    int jiffiesUsuarioNice;
    int jiffiesKernel;
    int cpuOciosa1;
    int cpuOciosa2;
    int jiffiesES;
    int jiffiesIrq;
    int jiffiesSoftIrq;
    int cpuTotal1 = 0;
    int cpuTotal2 = 0;
    float cpuTotal;
    float cpuOciosa;
    float porcentagemProcessamento;

    stat = fopen ("/proc/stat", "r");
    
    // Obtém os valores de jiffies.
    fscanf(stat, "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &vNull, &jiffiesUsuario, &jiffiesUsuarioNice, &jiffiesKernel, &cpuOciosa1, &jiffiesES, &jiffiesIrq, &jiffiesSoftIrq);
    fclose(stat);
    
    cpuTotal1 = jiffiesUsuario + jiffiesUsuarioNice + jiffiesKernel + cpuOciosa1 + jiffiesES + jiffiesIrq + jiffiesSoftIrq;
    sleep(1);
    
    // Obtém os valores de jiffies novamente.
    stat = fopen ("/proc/stat", "r");
    fscanf(stat, "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &vNull, &jiffiesUsuario, &jiffiesUsuarioNice, &jiffiesKernel, &cpuOciosa2, &jiffiesES, &jiffiesIrq, &jiffiesSoftIrq);
    fclose(stat);
    
    cpuTotal2 = jiffiesUsuario + jiffiesUsuarioNice + jiffiesKernel + cpuOciosa2 + jiffiesES + jiffiesIrq + jiffiesSoftIrq;
    
    cpuOciosa = (float)(cpuOciosa2 - cpuOciosa1);
    cpuTotal = (float)(cpuTotal2 - cpuTotal1);
    
    // Calcula a porcentagem de uso da CPU.
    porcentagemProcessamento = ((cpuTotal - cpuOciosa) / cpuTotal) * 100;
    
    cpu->jiffiesTotal = cpuTotal;
    cpu->jiffiesOcioso = cpuOciosa;
    cpu->porcentagemUso = porcentagemProcessamento;
    
    return cpu;
}

/**
 *  Obtém uso da CPU pelo processo.
 * 
 * @param int pid Número PID do processo.
 * @param int tempoAtualizacao Tempo em que a informação é atualizada.
 * @return float Porcentagem de uso do processador.
 */
float obterUsoDaCPUPeloProcesso(int pid, int tempoAtualizacao)
{
    int vNULLI;
    char vNULLC[100];
    char nomeArquivo[100];
    
    int jiffiesUsuario;
    int jiffiesKernel;
    float jiffiesTotal1;
    float jiffiesTotal2;
    float porcentagem;
    FILE* stat;
    
    sprintf(nomeArquivo, "/proc/%d/stat", pid);

    // Abre aquivo /proc/PID/stat.
    if ( (stat = fopen(nomeArquivo, "r")) != NULL )
    {
        fscanf(stat, "%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &vNULLI, vNULLC, vNULLC, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &jiffiesUsuario, &jiffiesKernel);
        fclose(stat);

        // Soma total de jiffies.
        jiffiesTotal1 = (float)(jiffiesUsuario + jiffiesKernel);
    }
    
    sleep(tempoAtualizacao);

    // Abre aquivo /proc/PID/stat.
    if ( (stat = fopen(nomeArquivo, "r")) != NULL )
    {
        fscanf(stat, "%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &vNULLI, vNULLC, vNULLC, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &jiffiesUsuario, &jiffiesKernel);
        fclose(stat);

        // Soma total de jiffies.
        jiffiesTotal2 = (float)(jiffiesUsuario + jiffiesKernel);
    }
   
    porcentagem = ( (jiffiesTotal2 - jiffiesTotal1) / (HZ * tempoAtualizacao) ) * 100.00;
    
    return porcentagem;
}

/**
 * Obtém uso da memória do processo.
 * 
 * @param int pid Número PID do processo.
 * @return int Quantidade de memória em KB.
 */
int obterUsoDeMemoria(int pid)
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
        fscanf(statm, "%d\t%d", &vNULLI, &numeroDePaginasMemoria);

        fclose(statm);

        // Obtém o uso de memória em Kbytes. Tamanho de página = 4 KB.
        memoriaEmKb = numeroDePaginasMemoria * 4;
    }
    
    return memoriaEmKb;
}

/**
 *  Obtém informação do processo. Obtém o nome do processo e o status dele.
 * 
 * @param int pid Número PID do processo.
 * @return struct processo Informações do processo.
 */
struct processo *obterInformacoesDoProcesso(int pid)
{
    struct processo *processo = (struct processo*)malloc(sizeof(struct processo));
    processo->pid = pid;
    
    char status[4000];
    char nomeArquivo[100];
    char *linha;
    char nomeProcesso[1024];
    char estadoProcesso[1024];
    
    // Obtém o nome do arquivo.
    sprintf(nomeArquivo, "/proc/%d/status", pid);
    
    // Obtém conteúdo do arquivo.
    strcpy(status, lerArquivo(nomeArquivo));
    
    if ( strlen(status) > 0 )
    {
        // Obtém nome do processo.
        linha = strstr(status, "Name:");
        sscanf (linha, "Name: %s", nomeProcesso);
        processo->nomeProcesso = nomeProcesso;

        // Obtém estado do processo.
        linha = strstr(status, "State:");

        // %[^\n]s, faz obter toda a String, até que ache a quebra de linha.
        sscanf (linha, "State: %[^\n]s", estadoProcesso);
        //strcpy(&processo->estadoProcesso, estadoProceso);
        processo->estadoProcesso = estadoProcesso;
    }
    
    return processo;
}

/**
 * Obtém o tempo que a máquina está ligada em segundos.
 *
 * @return char[100] Tempo que a máquina está ligada em segundos.
 */
char *obterUptime()
{
    char uptime[100];
    char tempo[200];    
    int tempoSegundos;
    int horas;
    int minutos; 
    int segundos;
        
    // Abre arquivo /proc/uptime e atribui em uptime.
    strcpy(uptime, lerArquivo("/proc/uptime"));
    
    // Obtém a primeira ocorrência antes do espaço.
    strtok(uptime, " ");
    
    // Retira o conteúdo depois do "." e converte em inteiro.
    strtok(uptime, ".");
    
    tempoSegundos = atoi(uptime);
 
    horas = (tempoSegundos / 3600);
    minutos = (tempoSegundos -(3600 * horas)) / 60;
    segundos = (tempoSegundos -(3600 * horas)-(minutos * 60));
    
    sprintf (tempo, "%dh : %dm :%ds",horas, minutos,segundos);
    
   return tempo;
}

/**
 * Obtém o load average,
 *
 * char[100] Load average.
 */
char *obterLoadAverage()
{
    char loadAVG[100];
    
    float load1;
    float load2; 
    float load3;
    
    FILE* loadavg;
    
    loadavg = fopen("/proc/loadavg", "r");
    fscanf(loadavg, "%f %f %f", &load1, &load2, &load3);
    fclose(loadavg);
    
    sprintf(loadAVG, "%1.2f %1.2f %1.2f", load1, load2, load3);
    
    return loadAVG;
}

/**
 * Obtém informações sobre a memória da máquina.
 *
 * @return struct memoria Informações da memória da máquina.
 */
struct memoria *obterInformacaoDaMemoria()
{
    char *linha;
    char conteudo[4000];
    char temporario[100];
    
    // Cria variável "info", alocando memória proporcional a struct memoria.
    struct memoria *info = (struct memoria*)malloc(sizeof(struct memoria));
        
    strcpy(conteudo, lerArquivo("/proc/meminfo"));
    
    // Obtém memória total.
    linha = strstr(conteudo, "MemTotal");
    sscanf (linha, "MemTotal: %s", temporario);
    info->memoriaTotal = atoi(temporario);
    
    // Obtém memória livre.
    linha = strstr(conteudo, "MemFree");
    sscanf (linha, "MemFree: %s", temporario);
    info->memoriaLivre = atoi(temporario);
    
    // Obtém cached.
    linha = strstr(conteudo, "Cached");
    sscanf (linha, "Cached: %s", temporario);
    info->cached = atoi(temporario);
    
    // Obtém swap cached.
    linha = strstr(conteudo, "SwapCached");
    sscanf (linha, "SwapCached: %s", temporario);
    info->swapCached = atoi(temporario);
    
    // Obtém memória swap total.
    linha = strstr(conteudo, "SwapTotal");
    sscanf (linha, "SwapTotal: %s", temporario);
    info->swapTotal = atoi(temporario);
    
    // Obtém espaço livre em swap.
    linha = strstr(conteudo, "SwapFree");
    sscanf (linha, "SwapFree: %s", temporario);
    info->swapLivre = atoi(temporario);
    
    // Obtém uso de memória.
    info->porcentagemUso = ((float)(info->memoriaTotal - info->memoriaLivre) / (float)info->memoriaTotal) * 100;
    
    // Obtém uso de memória SWAP.
    info->porcentagemUsoSwap = ((float)(info->swapTotal - info->swapLivre) / (float)info->swapTotal) * 100;
    
    return info;
}

/**
 *  Função que testa se o processo existe,
 * 
 * @param int pid Número do pid.
 * @return int Retorna 1 se o processo existe e 0 se não existe,
 */
int testarExistenciaProcesso(int pid)
{
    int retorno = 0;
    FILE* stat;
    char nomeArquivo[50];
    
    // Obtém o nome do arquivo.
    sprintf(nomeArquivo, "/proc/%d/stat", pid);
    
    if ( (stat = fopen(nomeArquivo, "r")) != NULL )
    {
        fclose(stat);
        retorno = 1;
    }
    
    return retorno;
}
/**
 * Thread que gera o cabeçalho.
 */
void *threadCabecalho()
{
    char conteudo[4000];
    int i;

    while(1)
    {
        // Atualiza a tela.
        refresh();
        
        // Obtém informação da cpu.
        struct cpu *cpu = obterInformacoesCPU();
        
        // Obtém informações da memória.
        struct memoria *memoria = obterInformacaoDaMemoria();
        
        // Carga média.
        char loadavg[100];
        strcpy(loadavg, obterLoadAverage());
        
        // Uptime.
        char uptime[100];
        strcpy(uptime, obterUptime());
        
        pthread_mutex_lock(&mutex_printw);
         
        // Move o cursos para a linha 1 e coluna 1.
        move(1,0);
        
        // Limpa cabeçalho.
        printw("                           \n               \n                    \n                                      \n                                 \n                                \n                                      \n                                  ");
       
        // Atualiza tela.
        refresh();
        
        // Move o cursos para a linha 1 e coluna 1.
        move(1,1);
        
        // Imprime o cabeçalho.
        printw("                          MULTI-TOP \n\n +-------------------------------------------------------+\n  CPU: %3.2f% \n  Memória: %3.2f% \n  Memória SWAP: %3.2f% \n  Uptime: %s \n  Carga média: %s \n +-------------------------------------------------------+", cpu->porcentagemUso, memoria->porcentagemUso, memoria->porcentagemUsoSwap, uptime, loadavg );
        
        // Atualiza tela.
        refresh();
        
        pthread_mutex_unlock(&mutex_printw);
    }
}
/**
 * Thread processo.
 * 
 * @param void Múltiplos parâmetros em uma String[PID + tempo de atualização + posição na tela].
 */
void *threadProcesso( void *parametros )
{
    int pid;
    int tempoAtualizacao;
    int posicaoY;
    char dispositivoMonitorado[3];
    char parametrosDaThread[10];
    
    // Trava semaforo strtok.
    pthread_mutex_lock(&mutex_strtok);
    
    sprintf(parametrosDaThread, "%s", (char *) parametros);
    
    // Obtém o pid do parâmetro.
    pid = atoi(strtok(parametrosDaThread, "."));
    
    // Obtém o tempo de atualização passado por parâmetro.
    tempoAtualizacao = atoi(strtok('\0', "."));
    
    // Obtém a posição Y.
    posicaoY = atoi(strtok('\0', "."));

    // Obtém o dispositivo que será monitorado
    strcpy(dispositivoMonitorado, strtok('\0', "."));
    
    // Destrava semaforo strtok.
    pthread_mutex_unlock(&mutex_strtok);
    
    float porcentagemCpu;
    int memoriaEmKb;
    char nomeProcesso[1024];
    char estadoProcesso[1024];
    char conteudo[2048];
   
    while(1)
    {
        if ( testarExistenciaProcesso(pid) == 1)
        {
            // Obtém informações sobre o processo.
            struct processo *processo = obterInformacoesDoProcesso(pid);

            // Obtém o nome do processo.
            strcpy(nomeProcesso, processo->nomeProcesso);

            // Obtém o estado do processo.
            strcpy(estadoProcesso, processo->estadoProcesso);

            // Adiciona o nome do processo à variavel conteudo.
            sprintf(conteudo, "[%d] - %s \n", pid, nomeProcesso);

            // Obtém o uso da CPU pelo processo.
            if ( strcmp(dispositivoMonitorado, "c") == 0 || strcmp(dispositivoMonitorado, "mc") == 0 || strcmp(dispositivoMonitorado, "cm") == 0  )
            {
                porcentagemCpu = obterUsoDaCPUPeloProcesso(pid, tempoAtualizacao);

                // Concatena o uso da CPU em porcentagem.
                sprintf(conteudo, "%s CPU: %3.1f%% ", conteudo, porcentagemCpu);
            }
            else
            {
                sleep(tempoAtualizacao);
            }

            // Obtém o uso de memória em KB.
            if ( strcmp(dispositivoMonitorado, "m") == 0 || strcmp(dispositivoMonitorado, "mc") == 0 || strcmp(dispositivoMonitorado, "cm") == 0)
            {
                memoriaEmKb = obterUsoDeMemoria(pid);

                // Concatena o uso de memória em KBytes.
                sprintf(conteudo, "%s Memória: %d KB ", conteudo, memoriaEmKb);
            }

            // Concatena o final do conteúdo.
            sprintf(conteudo, "%s Estado: %s", conteudo, estadoProcesso);
        }
        else
        {
            sprintf(conteudo, "[%d] - O processo não existe ou foi finalizado! \n CPU: --    Memória: 0 KB  Estado: -", pid);
            sleep(tempoAtualizacao);
        }
        
        // Concatena o separador de conteúdo.
        sprintf(conteudo, "%s \n ---------------------------------------------------------", conteudo);
        
        // Trava o printw.
        pthread_mutex_lock(&mutex_printw);
        
        // Atualiza a tela.
        refresh();
        
        // Move o cursor para a linha 20 e coluna 1.
        move(posicaoY,0);
        
        //Limpa as informações do processo.
        printw("                                                                     \n                                                                                   \n                                                      ");
        
        // Atualiza a tela.
        refresh();
        
        // Move o cursor para a linha 20 e coluna 1.
        move(posicaoY,1);
        printw("%s", conteudo);
        
        // Atualiza tela.
        refresh();
        
        // Destrava printw.
        pthread_mutex_unlock(&mutex_printw);
    }
}

/**
 * Função principal.
 *
 * @param void.
 * @return int Retorno padrão ou erro.
 */
int main (int argc,char *argv[])
{
    // Inicia ncurses.
    initscr();
    
    // Esconde o cursor.
    curs_set(0);
    
    // Limpa a tela.
    clear();

    // Atualiza a tela.
    refresh();
    
    pthread_t vThreadCabecalho;
    pthread_t threads[argc-1];
    char parametrosAux[100];
    char *parametros;
    int i;
    int posicaoY;
    char pids[argc-1][5];
    char tempoAtualizacao[argc-1][5];
    char dispositivoMonitoramento[argc-1][3];
    int contadorParametros = 1;
    
    int contadorPid = 0;
    int contadorPidAux = 0;
    int contadorTempoAtualizacao = 0;
    int contadorDispositivo = 0;
    
    // Faz o tratamento de parâmetros.
    for(i = 0; i < argc; i++)
    {
        if ( i == 0 )
        {
            continue;
        }
        
        if ( contadorParametros == 1)
        {
            // Copia parâmetro para variavel pid.
            strcpy(pids[contadorPid], argv[i]);
            contadorPid++;
            contadorParametros = 2;
        }
        else if ( contadorParametros == 2 )
        {
            // Copia parâmetro para variavel pid.
            strcpy(tempoAtualizacao[contadorTempoAtualizacao], argv[i]);
            contadorTempoAtualizacao++;
            contadorParametros = 3;
        }
        else
        {
            // Copia parâmetro para variavel pid.
            strcpy(dispositivoMonitoramento[contadorDispositivo], argv[i]);
            contadorDispositivo++;
            contadorParametros = 1;
        }
    }
    
    // Criação das threads.
    pthread_create( &vThreadCabecalho, NULL, threadCabecalho, NULL);
    
    contadorPidAux = contadorPid;
    contadorPid = 0;
    contadorTempoAtualizacao = 0;
    contadorDispositivo = 0;
    
    for ( i = 0; i < contadorPidAux; i++ )
    {
        // Calcula posição vertical da informação do processo.
        
        if ( i == 0 )
        {
            posicaoY = 11;
        }
        else
        {
            posicaoY = posicaoY + 4;
        }
        
        // Parametros devem ser no formato PID + "." + tempo de atualização da informação + "." + posição na tela + "." + dispositivo monitorado.
        sprintf(parametrosAux, "%s.%s.%d.%s", pids[contadorPid++], tempoAtualizacao[contadorTempoAtualizacao++], posicaoY, dispositivoMonitoramento[contadorDispositivo++] );
        parametros = parametrosAux;
        
        // Cria a thread.
        pthread_create( &threads[i], NULL, threadProcesso, (void *) parametros);
       
        // Delay para criação das threads, sem esse delay as threads se perdem.
        sleep(1);
    }
    
    // Finalização das threads.
    for ( i = 0; i < contadorPidAux; i++ )
    {
        pthread_join( threads[i], NULL);
    }
    
    // Finaliza ncurses.
    endwin();
    
    printf("\n\n\n\n");
}
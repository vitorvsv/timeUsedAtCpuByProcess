#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Biblioteca que contém constante HZ (número de jiffies por segundo).
#include <sys/param.h>

// http://stackoverflow.com/questions/1420426/calculating-cpu-usage-of-a-process-in-linux
// http://ewx.livejournal.com/579283.html
// http://www.quora.com/What-is-the-simplest-and-most-accurate-way-to-measure-the-memory-used-by-a-program-in-a-programming-contest-environment

/*
void *PrintHello(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("Hello World! It's me, thread #%ld!\n", tid);
   pthread_exit(NULL);
}

void *PreencheMatriz(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("Hello World! It's me, thread #%ld!\n", tid);
   pthread_exit(NULL);
}
*/


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
    
    int pid_stat;
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
        fscanf(stat, "%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &pid_stat, vNULLC, vNULLC, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &jiffiesUsuario, &jiffiesKernel);
        fclose(stat);

	//printf("jiffiesUsuario 1 = %d, jiffiesKernel 1 = %d , %d \n", jiffiesUsuario, jiffiesKernel, pid_stat);

        // Soma total de jiffies.
        jiffiesTotal1 = (float)(jiffiesUsuario + jiffiesKernel);


    }
    
    sleep(tempoAtualizacao);

    // Abre aquivo /proc/PID/stat.
    if ( (stat = fopen(nomeArquivo, "r")) != NULL )
    {
        fscanf(stat, "%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", &vNULLI, vNULLC, vNULLC, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &vNULLI, &jiffiesUsuario, &jiffiesKernel);
        fclose(stat);

	//printf("jiffiesUsuario 2 = %d, jiffiesKernel 2 = %d \n", jiffiesUsuario, jiffiesKernel);

        // Soma total de jiffies.
        jiffiesTotal2 = (float)(jiffiesUsuario + jiffiesKernel);


    }

	
   	//printf("HZ = %d \n", HZ);

	// HZ = jiffies por segundo definido no include #include <sys/param.h>
 	// CALCULO DA PORCENTAGEM FINAL, NUMERO JIFFIES FINAL (jiffiesTotal2) - NUMERO DE JIFFIES INCIAL (jiffiesTotal1), DIVIDE-SE POR NÚMERO JIFFIES POR SEGUNDO (HZ) * TEMPO DA AMOSTRAGEM (tempoAtualizacao) 

	// NO FINAL FOI FEITA DIVISÃO POR 4, POIS SÃO 4 NÚCLEOS
    porcentagem = (( (jiffiesTotal2 - jiffiesTotal1) / (HZ * tempoAtualizacao) ) * 100.00) / 4;
    
    return porcentagem;
}

int obtemMemoriaTotalProcesso(int pid) {

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


int main (int argc, char *argv[]) {

	int pid;
	int tempo_atualizacao = 3;
	//char opcao_ptr = malloc(2 * sizeof(char));
	char opcao_memoria = 0;
	char opcao_cpu = 0;
	int i = 0;
	int j = 0;

	//printf("memoria total: %d \n", obtemMemoriaTotal());

	printf("parametro: %s \n", argv[0]);		
	printf("parametro: %s \n", argv[1]);		
	printf("parametro: %s , %c , %c \n", argv[2], argv[2][0], argv[2][1]);		
	printf("parametro: %s \n", argv[3]);



	// ./multi_top 7114 c 2

	switch(argc) {
		
		// SE O NÚMERO DE PARÂMETROS FOR 1 SIGNIFICA QUE O USUÁRIO NÃO FORNECEU NENHUM PARÂMETRO
		case 1:
			printf("informe os parâmetros, uso ./multi_top pid mc tempo_atualizacao");
		break;

		// SE O NÚMERO DE PARÂMETROS FOR 4 SIGNIFICA QUE O USUÁRIO FORNECEU 3 PARÂMETROS
		case 4:
			tempo_atualizacao = atoi(argv[3]); // ÚLTIMO PARAMETRO

		// SE O NÚMERO DE PARÂMETROS FOR 3 SIGNIFICA QUE O USUÁRIO FORNECEU 2 PARÂMETROS
		// ANÁLISA O 3° PARÂMTRO, VERIFICA SE O USUÁRIO ESCOLHEU MOSTRAR DADOS CPU OU MEMÓRIA
		case 3:


			for(j = 0; j < 2;j++) {
				if(argv[2][j] == 'm') {
					opcao_memoria = 1;
				} 
				if(argv[2][j] == 'c') {
					opcao_cpu = 1;
				}
			}


		// SE O NÚMERO DE PARÂMETROS FOR 2 SIGNIFICA QUE O USUÁRIO FORNECEU 1 PARÂMETRO, O PID
		case 2:

			pid = atoi(argv[1]);

			// LAÇO DE REPETIÇÃO QUE FICA IMPRIMINDO AS INFORMAÇÕES
			while(1) {
				if(opcao_memoria == 1) {
					printf("%d ) memoria total pid %d: %d KB\n", i, pid, obtemMemoriaTotalProcesso(pid));
					if(opcao_cpu == 0) {
						sleep(tempo_atualizacao);					
					}

				}
				if(opcao_cpu == 1) {
					printf("%d ) uso cpu pid %d: %f \n", i, pid, obterUsoDaCPUPeloProcesso(pid, tempo_atualizacao));				
				}

				i++;
			}

		break;

		default:
		break;


	}



	

	/* Read the entire contents of /proc/cpuinfo into the buffer. */
//	fp = fopen("/proc/meminfo", "r");
//	bytes_read = fread(buffer, 1, sizeof (buffer), fp);
//	fclose (fp);
//	/* Bail if read failed or if buffer isn’t big enough. */
//	if (bytes_read == 0 || bytes_read == sizeof (buffer)) {
//		return 0;
//	}
//
//	/* NUL-terminate the text. */
//	buffer[bytes_read] = '\0';
//	/* Locate the line that starts with “cpu MHz”. */
//	match = strstr(buffer, "MemTotal");
//	if (match == NULL)
//	return 0;
//	/* Parse the line to extract the clock speed. */
//	sscanf (match, "MemTotal : %d", &memoria_total);
//
//	printf("memoria total: %d \n", memoria_total);














   
//	pthread_t threads[NUM_THREADS];
//	pthread_t threads[NUM_THREADS];

	//pthread_t t1;
	//pthread_t t2;

   	//int rc1, rc2;
	//long t;

//	int * matriz_a = malloc(NUM_COLUNAS * NUM_LINHAS * sizeof (int *));
//	int * matriz_b = malloc(NUM_COLUNAS * NUM_LINHAS * sizeof (int *));	

	//rc1 = pthread_create(&t1, NULL, PreencheMatriz, (void *)0);
	//rc2 = pthread_create(&t2, NULL, PreencheMatriz, (void *)1);

/*
   for(t=0; t<NUM_THREADS; t++){
      printf("In main: creating thread %ld\n", t);
      rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         return(-1);
      }
   }
*/


	/* Last thing that main() should do */
	//pthread_exit(NULL);
}

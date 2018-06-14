//************************************************
//************************************************
//
//    VERSAO 1.0 MPI & OpenMP - Heat Diffusion 
//
//************************************************
//************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>


#define MASTER 0				/*** Id do primeiro processo ***/
#define BEGIN_TAG 1				/*** Tag de mensagens de inicio ***/
#define PUP_TAG 2				/*** Tag de mensagens de troca de informacao entre processos imediatamente acima ***/
#define PDOWN_TAG 3 				/*** Tag de mensagens de troca de informacao entre processos imediatamente abaixo ***/
#define END_TAG 4				/*** Tag de mensagens de fim ***/
#define TIME_TAG 5				/*** Tag de mensagens de comunicacao de tempos de computacao e comunicacao ***/
#define NONE 0 					/*** Indica que nao tem processo vizinho ***/
#define TIME_RESOLUTION 1000			/*** Tempos em milisegundos ***/


/*** Variaveis de medicao dos tempos ***/
double initial_global_time, final_global_time, total_global_duration;
double initial_comput_time, final_comput_time, total_comput_duration = (double) 0, comput_time_received;
double initial_commun_time, final_commun_time, total_commun_duration = (double) 0, commun_time_received;
	


/*** Aloca a matriz. Alocacao efetuada de forma continua na memoria. ***/
float **allocMatrix(int linhas, int colunas) {
	float *dados = (float *)malloc(linhas*colunas*sizeof(float));
	float **array = (float **)malloc(linhas*sizeof(float*));
	for(int i = 0; i < linhas; i++) {
		array[i] = &(dados[colunas*i]);
	}

	return array;
}


/*** Inicia a matriz. Primeira linha a 100, restantes a 0 ***/
void initiateMatrix(float **M, int N) {
	//Preenche a primeira linha a 100
	for(int i = 0; i < N; i++) M[0][i] = (float) 100;
	
	//Preenche o restante a 0
	for(int i = 1; i < N; i++) {
		for(int j = 0; j < N; j++) {
			M[i][j] = (float) 0;
		}
	}
}


/*** Escreve a matriz para ficheiro. Apenas 10x10 elementos estao a ser escritos ***/
void printMatrix(float **M, FILE *f) {
	for(int i = 0; i < 14; i++) {
		for(int j = 0; j < 14; j++) {
			fprintf(f, "%.3f ", M[i][j]);
		}
		fprintf(f, "\n");
	}
}


/*** Inicia a contagem do tempo de execucao total ***/
void start_global_time(void) {
	initial_global_time = MPI_Wtime();
}


/*** Termina a contagem do tempo de execucao total ***/
void stop_global_time(void) {
	final_global_time = MPI_Wtime();
	total_global_duration = ((final_global_time - initial_global_time)*TIME_RESOLUTION);
}


/*** Inicia a contagem do tempo de comunicacao ***/
void start_communication_time(void) {
	initial_commun_time = MPI_Wtime();
}


/*** Termina a contagem do tempo de comunicacao ***/
void stop_communication_time(void) {
	final_commun_time = MPI_Wtime();
	total_commun_duration += ((final_commun_time - initial_commun_time)*TIME_RESOLUTION);
}


/*** Inicia a contagem do tempo de computacao ***/
void start_computation_time(void) {
	initial_comput_time = MPI_Wtime();
}


/*** Termina a contagem do tempo de computacao ***/
void stop_computation_time(void) {
	final_comput_time = MPI_Wtime();
	total_comput_duration += ((final_comput_time - initial_comput_time)*TIME_RESOLUTION);
}


/*** M_Old vai conter a versao da iteracao anterior. M_New vai conter a versao da ultima iteracao. ***/
void iterate(float **M_New, float **M_Old, int N, int offset, int numLines, int threads_OMP) {
	int startRow;
	int endRow;
	int i, j;

	if(offset == 0) startRow = offset + 1;
	else startRow = offset - 1;
	if((offset + numLines) == N) endRow = N-1;
	else endRow = offset + numLines + 1;

	#pragma omp parallel num_threads(threads_OMP)
	{
		#pragma omp parallel shared(M_New, M_Old) private(i, j)
		{
			//Guarda a ultima solucao em M_Old
			#pragma omp for
			for(i = startRow; i < endRow; i++) {
				for(j = 1; j < N-1; j++) {
					M_Old[i][j] = M_New[i][j];
				}
			}
		}
	}

	/***********************************************************/
	
	if(offset == 0) startRow = offset + 1;
	else startRow = offset;
	if((offset + numLines) == N) endRow = N-1;
	else endRow = offset + numLines;

	#pragma omp parallel num_threads(threads_OMP)
	{
		#pragma omp parallel shared(M_New, M_Old) private(i, j)
		{
			//Calcula os novos valores dos pontos interiores para M_New.
			//O valor dos pontos e a media das posicoes imediatamente acima, baixo, esquerda, direita e da propria posicao.
			#pragma omp for
			for(i = startRow; i < endRow; i++) {
				for(j = 1; j < N-1; j++) {
					M_New[i][j] = (M_Old[i-1][j] + M_Old[i+1][j] + M_Old[i][j-1] + M_Old[i][j+1] + M_Old[i][j])/5;
				}
			}
		}
	}
}



int main(int argc, char *argv[]) {
	float **heatPlate_new;			/*** Declaracao matriz (quadrada) nova ***/
	float **heatPlate_old;  		/*** Declaracao matriz (quadrada) antiga ***/
	int matrix_size;			/*** Tamanho da matriz recebida dinamicamente como argumento pela script criada ***/
	int n_max_iterations;			/*** Numero maximo de iteracoes recebido dinamicamente como argumento pela script criada ***/
	int n_max_threads_OMP; 			/*** Numero maximo de iteracoes em OpenMP ***/
	FILE *f = NULL;

	/*** MPI ***/
	int procID;				/*** Id do processo ***/
	int numProcs;				/*** Numero total de processos ***/
	int numWorkers;				/*** Numero de workers ***/
	int lineDivision;			/*** Numero (minimo) de linhas que cada processo vai efetuar ***/
	int lineExtra;				/*** Numero extra de linhas que tem de ser processadas devido ao resto divisao inteira != 0 ***/
	int offset;				/*** Salto efetuado na divisao do numero de linhas para cada processo ***/
	int numLines;				/*** Numero exato de linhas a processar por processo ***/
	int processUp, processDown;		/*** Processos imediatamente acima e a baixo de outro processo para troca de mensagens entre eles ***/
	MPI_Status status;			/*** Status do processo ***/




	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &procID);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

	numWorkers = numProcs - 1;		/*** O processo 0 (MASTER) esta reservado para coordenacao dos restantes ***/

	matrix_size = atoi(argv[1]);		/*** Le o valor do tamanho da matriz (a partir da script) ***/
	n_max_iterations = atoi(argv[2]);	/*** Le o valor do numero maximo de iteracoes a efetuar (a partir da script) ***/
	n_max_threads_OMP = atoi(argv[3]);	/*** Le o valor do numero de threads a utilizar em OpenMP (a partir da script) ***/

	
	/*** Alocacao das matrizes ***/
	heatPlate_new = allocMatrix(matrix_size, matrix_size);
	heatPlate_old = allocMatrix(matrix_size, matrix_size);


	/************************************************************   MASTER   ************************************************************/ 
	if(procID == MASTER) {
		lineDivision = (matrix_size/numWorkers);	/*** Divide o numero de linhas a processar pelo numero de Workers que existem ***/	
		lineExtra = (matrix_size%numWorkers);		/*** Caso o resto divisao interira != 0 entao existem mais linhas a processar ***/	
		offset = 0;					/*** Salto a efetuar nas linhas da matriz ***/
		
		//initial_global_time = MPI_Wtime(); 		/*** Inicia a contagem do tempo total de execucao (no processo mais lento) ***/
		start_global_time();				/*** Inicia a contagem do tempo total de execucao (no processo mais lento) ***/

		/*** Efetua particao da matriz, determina vizinhos de cada processo e envia para cada processo o conjunto de informacao a processar ***/
		for(int procDest = 1; procDest <= numWorkers; procDest++) {
			/*** Particao eficiente e compacta da matriz ***/
			if(procDest <= lineExtra) numLines = lineDivision + 1;
			else numLines = lineDivision;

			/*** Determina os vizinhos do processo "procDest" ***/
			if(procDest == 1) processUp = NONE;
			else processUp = procDest - 1;
			if(procDest == numWorkers) processDown = NONE;
			else processDown = procDest + 1;

			/*** Envia a informacao relevante aos processos ***/
			start_communication_time();
			MPI_Send(&offset, 1, MPI_INT, procDest, BEGIN_TAG, MPI_COMM_WORLD);
			MPI_Send(&numLines, 1, MPI_INT, procDest, BEGIN_TAG, MPI_COMM_WORLD);
			MPI_Send(&processUp, 1, MPI_INT, procDest, BEGIN_TAG, MPI_COMM_WORLD);
			MPI_Send(&processDown, 1, MPI_INT, procDest, BEGIN_TAG, MPI_COMM_WORLD);
			stop_communication_time();

			/*** Incrementa offset pelo numero linhas para que o proximo saiba onde comecar ***/
			offset = offset + numLines;
		}

		/*** Espera pelos resultados de cada processo ***/
		start_communication_time();
		for(int procOrigin = 1; procOrigin <= numWorkers; procOrigin++) {	
			MPI_Recv(&offset, 1, MPI_INT, procOrigin, END_TAG, MPI_COMM_WORLD, &status);
			MPI_Recv(&numLines, 1, MPI_INT, procOrigin, END_TAG, MPI_COMM_WORLD, &status);
			MPI_Recv(&(heatPlate_new[offset][0]), numLines*matrix_size, MPI_FLOAT, procOrigin, END_TAG, MPI_COMM_WORLD, &status);
		}
		stop_communication_time();
		stop_global_time();				/*** Termina a contagem do tempo total de execucao ***/

		/*** Recebe os tempos de comunicacao e computacao de cada processo ***/
		for(int procOrigin = 1; procOrigin <= numWorkers; procOrigin++) {
			MPI_Recv(&commun_time_received, 1, MPI_DOUBLE, procOrigin, TIME_TAG, MPI_COMM_WORLD, &status); //Comunicacao
			MPI_Recv(&comput_time_received, 1, MPI_DOUBLE, procOrigin, TIME_TAG, MPI_COMM_WORLD, &status); //Computacao

			//printf("Processo %d: comunicacao = %.3f; computacao = %.3f\n", procOrigin, commun_time_received, comput_time_received);
			total_commun_duration += commun_time_received;
			total_comput_duration += comput_time_received;
		}

		printf("Tempo execucao global = %.3f; Tempo comunicacao = %.3f; Tempo computacao = %.3f\n", total_global_duration, total_commun_duration, total_comput_duration);

		/*** Guarda matriz final para ficheiro ***/
		f = fopen("finalMatrix", "wa");
		printMatrix(heatPlate_new, f);	
		fclose(f);
	}

	/************************************************************   WORKERS   ************************************************************/
	if(procID != MASTER) {
		/*** Inicia valores das matrizes ***/
		initiateMatrix(heatPlate_new, matrix_size);
		initiateMatrix(heatPlate_old, matrix_size);

		/*** Recebe informacao do MASTER ***/
		start_communication_time();
		MPI_Recv(&offset, 1, MPI_INT, 0, BEGIN_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&numLines, 1, MPI_INT, 0, BEGIN_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&processUp, 1, MPI_INT, 0, BEGIN_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&processDown, 1, MPI_INT, 0, BEGIN_TAG, MPI_COMM_WORLD, &status);
		stop_communication_time();	

		/*** Efetua ate N_MAX iteracoes ***/
		for(int iteration = 0; iteration < n_max_iterations; iteration++) {	
			/*** Efetua a comunicacao das bordas de particao entre processos ***/
			/***				 Se o processo for PAR ENVIA, se for IMPAR RECEBE 					***/
			start_communication_time();
			if((procID%2) == 0) {
				if(processUp != NONE)
					MPI_Send(&(heatPlate_new[offset][0]), matrix_size, MPI_FLOAT, processUp, PUP_TAG, MPI_COMM_WORLD);
				if(processDown != NONE)
					MPI_Send(&(heatPlate_new[offset+numLines-1][0]), matrix_size, MPI_FLOAT, processDown, PDOWN_TAG, MPI_COMM_WORLD);
			}
			else {
				if(processUp != NONE)
					MPI_Recv(&(heatPlate_new[offset-1][0]), matrix_size, MPI_FLOAT, processUp, PDOWN_TAG, MPI_COMM_WORLD, &status);
				if(processDown != NONE)
					MPI_Recv(&(heatPlate_new[offset+numLines][0]), matrix_size, MPI_FLOAT, processDown, PUP_TAG, MPI_COMM_WORLD, &status);
			}
			/*** 				Se o processo for IMPAR ENVIA, se for PAR RECEBE 					***/
			if((procID%2) != 0) {
				if(processUp != NONE)
					MPI_Send(&(heatPlate_new[offset][0]), matrix_size, MPI_FLOAT, processUp, PUP_TAG, MPI_COMM_WORLD);
				if(processDown != NONE)
					MPI_Send(&(heatPlate_new[offset+numLines-1][0]), matrix_size, MPI_FLOAT, processDown, PDOWN_TAG, MPI_COMM_WORLD);
			}
			else {
				if(processUp != NONE)
					MPI_Recv(&(heatPlate_new[offset-1][0]), matrix_size, MPI_FLOAT, processUp, PDOWN_TAG, MPI_COMM_WORLD, &status);
				if(processDown != NONE)
					MPI_Recv(&(heatPlate_new[offset+numLines][0]), matrix_size, MPI_FLOAT, processDown, PUP_TAG, MPI_COMM_WORLD, &status);
			}
			stop_communication_time();
			
			/*** Efetua o calculo da difusao do calor para a sua parte da matrix ***/
			start_computation_time();
			iterate(heatPlate_new, heatPlate_old, matrix_size, offset, numLines, n_max_threads_OMP);
			stop_computation_time();
		}
		
		/*** Processo envia para o MASTER, o resultado do calculo da sua parte da matriz ***/
		start_communication_time();
		MPI_Send(&offset, 1, MPI_INT, MASTER, END_TAG, MPI_COMM_WORLD);
		MPI_Send(&numLines, 1, MPI_INT, MASTER, END_TAG, MPI_COMM_WORLD);
		MPI_Send(&(heatPlate_new[offset][0]), numLines*matrix_size, MPI_FLOAT, MASTER, END_TAG, MPI_COMM_WORLD);
		stop_communication_time();
		
		/*** Envia para o MASTER os seus tempos de comunicacao e computacao ***/
		MPI_Send(&total_commun_duration, 1, MPI_DOUBLE, MASTER, TIME_TAG, MPI_COMM_WORLD); //Comunicacao
		MPI_Send(&total_comput_duration, 1, MPI_DOUBLE, MASTER, TIME_TAG, MPI_COMM_WORLD); //Computacao
	}

	/*** Liberta memoria ***/
	free(heatPlate_new);
	free(heatPlate_old);

	/*** Termina o MPI ***/
	MPI_Finalize();
}




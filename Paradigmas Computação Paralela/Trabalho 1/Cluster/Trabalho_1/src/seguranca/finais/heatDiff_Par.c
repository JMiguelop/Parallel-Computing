//************************************************
//************************************************
//
//     VERSAO PARALELA V1.0 - Heat Diffusion 
//
//************************************************
//************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>




/*** Inicia a matriz. Primeira linha a 100, restantes a 0 ***/
void initiateMatrix_new(float **M, int N) {
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
	for(int i = 0; i < 10; i++) {
		for(int j = 0; j < 10; j++) {
			fprintf(f, "%.3f ", M[i][j]);
		}
		fprintf(f, "\n");
	}
}


/*** Efetua as iteracoes ate N_MAX. M_Old vai conter a versao da iteracao anterior. M_New vai conter a versao da ultima iteracao. ***/
/*** NOTA: Nao existem "race conditions" no acesso aos dados/variaveis do codigo paralelo uma vez que cada thread contem o seu valor de i,j e M_New/M_Old sao partilhadas pelas threads mas apenas existem atribuicoes e leituras de valores, nao simultaneos, para a mesma variavel. ***/
void iterate(float **M_New, float **M_Old, int N, int N_MAX) {
	int i, j;

	//Itera ate N_MAX iteracoes
	for(int iteration = 0; iteration < N_MAX; iteration++) {
		//Criacao de um conjunto de threads
		#pragma omp parallel shared(M_New, M_Old) private(i, j)
		{ 
			//Guarda a ultima solucao em M_Old
			#pragma omp for
			for(i = 0; i < N; i++) {
				for(j = 0; j < N; j++) {
					M_Old[i][j] = M_New[i][j];
				}
			}
		
			//Calcula os novos valores dos pontos interiores para M_New.
			//O valor dos pontos, para cada posicao, e a media das posicoes imediatamente acima,baixo,esquerda,direita e da propria posicao.
			#pragma omp for
			for(i = 1; i < N-1; i++) {
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
	int matrix_size;			/*** Tamanho da matriz recebido dinamicamente como argumento pela script criada ***/
	int n_max_iterations;			/*** Numero maximo de iteracoes recebido dinamicamente como argumento pela script criada ***/
	double startTime, finalTime;		/*** Valores para calcular o tempo de execucao ***/
	FILE *f = NULL;

	matrix_size = atoi(argv[1]);		/*** Le o valor do tamanho da matriz (a partir da script) ***/
	n_max_iterations = atoi(argv[2]);	/*** Le o valor do numero maximo de iteracoes a efetuar (a partir da script) ***/

	/*** Alocacao das matrizes ***/
	heatPlate_new = (float **)malloc(matrix_size * sizeof(*heatPlate_new));
	heatPlate_old = (float **)malloc(matrix_size * sizeof(*heatPlate_old));
	//Alocacao das "linhas"
	for(int i = 0; i < matrix_size; i++) {
		heatPlate_new[i] = (float *)malloc(matrix_size * sizeof(**heatPlate_new));
		heatPlate_old[i] = (float *)malloc(matrix_size * sizeof(**heatPlate_old));
	}

	/*** Iniciar valores matriz_new ***/
	initiateMatrix_new(heatPlate_new, matrix_size);
	
	/*** Guarda matriz inicial para ficheiro ***/
	f = fopen("initialMatrix", "wa");
	printMatrix(heatPlate_new, f);
	fclose(f);
	
	/*** Numero de threads a utilizar ***/
	omp_set_num_threads(atoi(argv[3]));

	/*** Efetua as iteracoes (ate N_MAX). Inicia a contagem do tempo de execucao. ***/
	startTime = omp_get_wtime();
	iterate(heatPlate_new, heatPlate_old, matrix_size, n_max_iterations);
	finalTime = omp_get_wtime();

	printf("Time par: %.3f ms\n", (finalTime - startTime)*1000);
	
	/*** Guarda matriz final para ficheiro ***/
	f = fopen("finalMatrix", "wa");
	printMatrix(heatPlate_new, f);
	fclose(f);

	/*** Liberta memoria utilizada ***/
	free(heatPlate_new);
	free(heatPlate_old);

	return 1;
}




//************************************************
//************************************************
//
//	VERSAO SEQUENCIAL - Heat Diffusion 
//
//************************************************
//************************************************
#include <stdio.h>
#include <stdlib.h>

/*** N_MAX ITERACOES ***/
#define N_MAX 8132

/*** TAMANHO DAS MATRIZES ***/
#define M1 1024
#define M2 2048
#define M3 4096
#define M4 8132





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
			fprintf(f, "%f ", M[i][j]);
		}
		fprintf(f, "\n");
	}
}


/*** Efetua as iteracoes ate N_MAX. M_Old vai conter a versao da iteracao anterior. M_New vai conter a versao da ultima iteracao. ***/
void iterate(float **M_New, float **M_Old, int N) {
	//Itera ate N_MAX iteracoes
	for(int iteration = 0; iteration < N_MAX; iteration++) {
		
		//Guarda a ultima solucao em M_Old
		for(int i = 0; i < N; i++) {
			for(int j = 0; j < N; j++) {
				M_Old[i][j] = M_New[i][j];
			}
		}
		
		//Calcula os novos valores dos pontos interiores para M_New.
		//O valor dos pontos, para cada posicao, e a media das posicoes imediatamente acima, baixo, esquerda, direita e da propria posicao.
		for(int i = 1; i < N-1; i++) {
			for(int j = 1; j < N-1; j++) {
				M_New[i][j] = (M_Old[i-1][j] + M_Old[i+1][j] + M_Old[i][j-1] + M_Old[i][j+1] + M_Old[i][j])/5;
			}
		}
	}
}



int main() {
	float **heatPlate_new;	/*** Declaracao matriz (quadrada) nova ***/
	float **heatPlate_old;  /*** Declaracao matriz (quadrada) antiga ***/
	FILE *f = NULL;



	/*** Alocacao das matrizes ***/
	heatPlate_new = (float **)malloc(M1 * sizeof(*heatPlate_new));
	heatPlate_old = (float **)malloc(M1 * sizeof(*heatPlate_old));
	//Alocacao das "linhas"
	for(int i = 0; i < M1; i++) {
		heatPlate_new[i] = (float *)malloc(M1 * sizeof(**heatPlate_new));
		heatPlate_old[i] = (float *)malloc(M1 * sizeof(**heatPlate_old));
	}

	/*** Iniciar valores matriz_new ***/
	initiateMatrix_new(heatPlate_new, M1);
	
	/*** Guarda matriz inicial para ficheiro ***/
	f = fopen("initialMatrix", "wa");
	printMatrix(heatPlate_new, f);
	fclose(f);
	
	/*** Efetua as iteracoes (ate N_MAX) ***/
	iterate(heatPlate_new, heatPlate_old, M1);
	
	/*** Guarda matriz final para ficheiro ***/
	f = fopen("finalMatrix", "wa");
	printMatrix(heatPlate_new, f);
	fclose(f);

	return 1;
}




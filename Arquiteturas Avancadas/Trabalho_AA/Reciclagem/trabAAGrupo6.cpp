#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <omp.h>
#include <string>
#include <vector>
#include "papi.h"

#define MAX_THREADS 1 			// maximum number of threads to run
#define REPETITIONS 8 			// Number of measurements repetitions
#define TIME_RESOLUTION 1000000		// time measuring resolution (us)
#define NUMEVENTS 3
#define CPU_FREQUENCY_431 2.66
/*
 *	Matrix-matrix arithmetic
 *
 */

using namespace std;

//long long unsigned initial_time;
//vector<long long unsigned> sequential_measurements;
//vector<long long unsigned> measurements[MAX_THREADS/4 + 1];	// position 0 is for the sequential version
//timeval t;
//float sum;

/****** A_MAIS ************************************************************
void clearCache (void) {
	double clearcache[30000000];

	for (unsigned i = 0; i < 30000000; ++i)
		clearcache[i] = i;
}


long long unsigned minElement (int thread) {
	long long unsigned best = 0;

	if (thread == - 1) {
		for (auto i : sequential_measurements) {
		if (i < best || !best)
			best = i;
		}
	} 
	else {
		for (auto i : measurements[thread]) {
			if (i < best || !best)
				best = i;
		}
	}

return best;
}


void writeResults (void) {
	ofstream file ("timings.dat");

	// write the optimised version
	file << 0 << " " << minElement(-1) << endl;

	// write the optimised version
	for (unsigned i = 0; (i * 4) <= MAX_THREADS; ++i)
		if ((i * 4) == 0)
			file << 1 << " " << minElement(i) << endl;
		else
			file << i * 4 << " " << minElement(i) << endl;

	file.close();
}
*****************************************************************************/




/**** A_MAIS  MEDICAO DE TEMPOS
void start (void) {
	gettimeofday(&t, NULL);
	initial_time = t.tv_sec * TIME_RESOLUTION + t.tv_usec;
}


long long unsigned stop (int thread) {
	gettimeofday(&t, NULL);
	long long unsigned final_time = t.tv_sec * TIME_RESOLUTION + t.tv_usec;

	if (thread == -1)
		sequential_measurements.push_back(final_time - initial_time);
	else
		measurements[thread].push_back(final_time - initial_time);

	return final_time - initial_time;
}
*****/

void fillMatrices (float **A, float **B, float **C, int N) {

	for (int i = 0; i < N; ++i) {	
		for (int j = 0; j < N; ++j) {
			A[i][j] = ((float) rand()) / ((float) RAND_MAX);
			B[i][j] = (float) 1;
			C[i][j] = 0;
		}
	}
}


/***** Ex_2.2 *******/
void matrixMult(float **A, float **B, float **C, int N) {
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			for (int k = 0; k < N; ++k) {
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
}



/**** Ex_2.3 *******
 * Apenas alterei a ordem dos ciclos. Nao sei se será necessário mudar alguma coisa nos indices do cálculo
*/
void matrixMultKJI(float **A, float **B, float **C, int N) {
        for (int k = 0; k < N; ++k) {
		for (int j = 0; j < N; ++j) {
			for (int i = 0; i < N; ++i) {                
                                C[i][j] += A[i][k] * B[k][j];
                        }
                }
        }
}

//Na funcao matrixMultKJI, e a matriz A que e acedida por colunas. Logo, transpoem-se a matriz A, trocando os indices, passando assim a ser acedida linha por linha
void matrixMultTRANSP(float **A, float **B, float **C, int N) {
        for (int k = 0; k < N; ++k) {
		for (int j = 0; j < N; ++j) {
			for (int i = 0; i < N; ++i) {                
                                C[i][j] += A[k][i] * B[k][j]; //NOTA: o resultado final da matriz, comparado com a versao original KJI vai ser diferente mas o que interessa e a diferenca de tempos
                        }
                }
        }
}




/* A_MAIS
void matrixMultLab13(void) {
	
	//#pragma omp parallel
	//#pragma omp simd collapse(2)
	//#pragma omp parallel for
	//#pragma omp simd
	for (unsigned i = 0; i < SIZE; ++i) {
		for (unsigned j = 0; j < SIZE; ++j) {
			sum = 0;
			//#pragma vector always
			//#pragma ivdep
			for (unsigned k = 0; k < SIZE; ++k) {
				//compute(&A[i][k], B[k][j], &sum);
				sum += A[i][k]*B[k][j];
			}
			C[i][j] = sum;
		}
	}
}
*/


int main () {

	/** PAPI **/
	long long values[NUMEVENTS];						/* Store the values of each counter */
	int events[NUMEVENTS] = {PAPI_FP_INS, PAPI_TOT_INS, PAPI_TOT_CYC};	/* Array with counters we'll measure */
	int retval;
	int EventSet;

	/*** Ex_2.3.1 ***/	
	int N = 48;		//Matriz size wich can be hold just in L1 Cache Level;
	//int N_L2 = 144;		//Matriz size wich can be hold just in L2 Cache Level;
	//int N_L3 = 1000;	//Matriz size wich can be hold just in L3 Cache Level;
	//int N_MEM = 2048;	//Matriz size wich canot be held neither L1, L2 or L3, and need to be stored on MAIN MEMORY
	
	float **A, **B, **C;
	
	//ALOCACAO DAS MATRIZES -> NECESSARIO UTILIZAR MALLOC's PARA SE PODER PASSAR O APONTADOR PARA AS MATRIZES.
	//Alocacao das "colunas"
	A = (float **)malloc(N * sizeof(*A));
	B = (float **)malloc(N * sizeof(*B));
	C = (float **)malloc(N * sizeof(*C));

	//Alocacao das "linhas"
	for(int i = 0; i < N; i++) {
		A[i] = (float *)malloc(N * sizeof(**A));
		B[i] = (float *)malloc(N * sizeof(**B));
		C[i] = (float *)malloc(N * sizeof(**C));
	}

	//Filling the matrices
	fillMatrices(A, B, C, N);

		

	//long long unsigned time;
	
	/*** USING PAPI ***/
	retval = PAPI_library_init(PAPI_VER_CURRENT);		/* Init PAPI Library */
	retval = PAPI_create_eventset(&EventSet);
	retval = PAPI_add_events(EventSet,events,NUMEVENTS);
	PAPI_start_counters((int*)events,NUMEVENTS);
	
	retval = PAPI_start(EventSet);				/** START MEASURING TIME ***/
	for (unsigned i = 0; i < REPETITIONS; ++i) {
		//clearCache();
		//start();
		matrixMult(A, B, C, N);
	}
 	retval = PAPI_stop_counters(values,NUMEVENTS);		/** STOP MEASURING TIME **/
	
	double CPI = ((double) values[2])/((double) values[1]);
	printf("CPI calculated: %f\n",CPI);
	printf("Number of total CPU cycles (#CC): %f\n", (double) values[2]);
	printf("Number of total executed instructions (#I): %f\n", (double) values[1]);
	printf("Number of total total Floating point instructions (#FP ): %f\n", (double) values[0]);
	
	/* Texec = #I * CPI * Tcc */
	double Texec = ((double) values[1]) * CPI * (1/CPU_FREQUENCY_431); 
	printf("Execution time of matrixMul :%f\n", Texec);

	printf("retval value: %d\n", retval);

	/** The matrices should be freeded!!! ***/
	free(A);
	free(B);
	free(C);
	PAPI_shutdown();					/* Finished using PAPI. All resources should be "freeded" */

	//stop(-1);
	
	/* A_MAIS
	// optimised run! (SET MAX_THREADS to 1 for the HW 2.1 - it's not parallel yet)
	for (unsigned i = 0; (i * 4) <= MAX_THREADS; ++i) {
		//omp_set_num_threads(i * 4);
		for (unsigned j = 0; j < REPETITIONS; ++j) {
			clearCache();
			start();
			// insert here your new function call
			//#pragma omp parallel
			//matrixMultLab13();
			stop(i);
		}
	}
	*/
	//writeResults();

return 1;
}

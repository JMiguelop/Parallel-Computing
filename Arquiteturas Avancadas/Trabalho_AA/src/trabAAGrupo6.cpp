#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <omp.h>
#include <string>
#include <vector>
#include <papi.h>
#include <string.h>

#define MAX_THREADS 1 			// maximum number of threads to run
#define REPETITIONS 8 			// Number of measurements repetitions
#define TIME_RESOLUTION 1000000		// time measuring resolution (us)
#define NUMEVENTS 8

/*** Ex_2_3_1 - TAMANHO DAS MATRIZES ***/
#define L1 48
#define L2 144
#define L3 1000
#define RAM 2048

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
 * Just exchanged the cycles order
 * */
void matrixMultKJI(float **A, float **B, float **C, int N) {
        for (int k = 0; k < N; ++k) {
		for (int j = 0; j < N; ++j) {
			for (int i = 0; i < N; ++i) {                
                                C[i][j] += A[i][k] * B[k][j];
                        }
                }
        }
}

/**** Ex_2.3 *****
 * Matrix A accessed by columns. 
 * The A matrix is transposed exachanging his indexes (A[i][k] => A[k][i]) so can be accessed row-by-row.
 */
void matrixMultTRANSP(float **A, float **B, float **C, int N) {
        for (int k = 0; k < N; ++k) {
		for (int j = 0; j < N; ++j) {
			for (int i = 0; i < N; ++i) {                
                                C[i][j] += A[k][i] * B[k][j]; 
				//The final result is different compared with original matrixMultKJI but we are just looking at the impact of performance 
                        }
                }
        }
}


/**********
 * Print multiplication result (C matrices) to file.
 * ********/
void printCMatrix(float **C, FILE *f) {
	for(int i = 0; i < 10; i++) {			/********* JUST 10 ELEMENTS BEING PRINTED!!! **********/
		for(int j = 0; j < 10; j++) {
			fprintf(f, "%f ", C[i][j]);
		}
		fprintf(f, "\n");
	}

	fprintf(f, "\n\n\n\n\n");
}


/*** Reset C matrix after each multiplication ***/
void resetCMatrix(float **C, int tam) {
	for(int i = 0; i < tam; i++) {
		for(int j = 0; j < tam; j++) {
			C[i][j] = (float)0;
		}
	}
}


int main (int argc, char *argv[]) {
	/** PAPI **/
	long long values[NUMEVENTS];	/* Store the values of each counter */
	int events[NUMEVENTS] = {PAPI_FP_INS, PAPI_TOT_INS, PAPI_TOT_CYC, PAPI_L2_TCA, PAPI_L2_TCM, PAPI_L3_TCA, PAPI_L3_TCM, PAPI_L3_TCM};	/* Array with counters we'll measure */
	int retval;
	int EventSet = PAPI_NULL;
	//long long unsigned Texec;
	long long unsigned Texec;
	double CPI;

	/** To obtain Texec by subtraction of times **/
	//long long unsigned PAPI_start;
	//long long unsigned PAPI_stop;
	
	/*** Ex_2.3.1 ***/	
	int N = 48;		//Matriz size wich can be hold just in L1 Cache Level;
	//int N_L2 = 144;	//Matriz size wich can be hold just in L2 Cache Level;
	//int N_L3 = 1000;	//Matriz size wich can be hold just in L3 Cache Level;
	//int N_MEM = 2048;	//Matriz size wich canot be held neither L1, L2 or L3, and need to be stored on MAIN MEMORY
	

	
	//Para poder-mos passar o tamanho das matrizes pela linha de comandos!!!!! Para utilzar faz-se: qsub -v tam=L1 runSearch para correr o tamanho L1 por exemplo
	/*
	int N;
	if(argc > 1) {
		if(strcmp(argv[1], "L1") == 0) N = L1;	
		if(strcmp(argv[1], "L2") == 0) N = L2;
		if(strcmp(argv[1], "L3") == 0) N = L3;
		if(strcmp(argv[1], "RAM") == 0) N = RAM;
	}*/



	float **A, **B, **C;
	FILE *f = NULL;	



	/** Matrices allocation **/
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

	
	/**** Ex_2.3.2 Validate operations (Printing multiplication result to file ***/
	f = fopen("matrixMultAB_BA", "wa");
	matrixMult(A, B, C, N);
	fprintf(f, "MULTIPLICATION A*B\n");
	printCMatrix(C, f);
	resetCMatrix(C, N);
	matrixMult(B, A, C, N);
	fprintf(f, "MULTIPLICATION B*A\n");
	printCMatrix(C, f);
	fclose(f);
		
	
	/*** INIT PAPI ***/
	retval = PAPI_library_init(PAPI_VER_CURRENT);		
	retval = PAPI_create_eventset(&EventSet);
	retval = PAPI_add_events(EventSet,events,NUMEVENTS);

	printf("\n\n\n******************************************************* RESULTS TO K-J-I ORIGINAL *******************************************************\n\n");
	for (unsigned i = 0; i < REPETITIONS; ++i) {	
	
		/*** START THE COUNTERS ***/
		PAPI_start_counters((int*)events,NUMEVENTS);
		retval = PAPI_start(EventSet);		// <- last version working
		//PAPI_start = PAPI_get_real_usec();		// Times in microseconds.
		
		/*** MULTIPLICACAO K-J-I ORIGINAL ***/
		matrixMultKJI(A, B, C, N);
		
		/*** STOP THE COUNTERS ***/
		retval = PAPI_stop(EventSet,values);		// <- last version working
		//PAPI_stop = PAPI_get_real_usec();		// Times in microseconds.
		retval = PAPI_stop_counters(values,NUMEVENTS);
		
		/*** OBTER RESULTADOS ***/
		printf("\n\t------------------------------ EXECUTION: %d ------------------------------\n", i+1);

		/** 	CPI = #CC/#I 	**/
		CPI = ((double) values[2])/((double) values[1]);
		printf("\tCPI: %f\n",CPI);
		printf("\tTotal CPU cycles (#CC): %f\n", (double) values[2]);
		printf("\tTotal executed instructions (#I): %f\n", (double) values[1]);
		printf("\tTtotal Floating point instructions (#FP): %f\n", (double) values[0]);
		printf("\tPercentage of Miss Rate on L1: !!! PAPI_L1_TCA NOT AVAILABLE at compute-431 @ SeARCH\n");
		printf("\tPercentage of Miss Rate on L2: %f\n", (double)values[4]/(double)values[3]);
		printf("\tPercentage of Miss Rate on L3: %f\n", (double)values[6]/(double)values[5]);
		printf("\t#Acesses on RAM per instruction: %f\n", (double)values[7]/(double)values[1]);		//Forma do Sergio, nao acho que seja assim!
		
		/* Texec = #I * CPI * Tcc */
		Texec = ((double) values[1]) * CPI * (1/CPU_FREQUENCY_431); 
		//Texec = PAPI_stop - PAPI_start;		//Texec in microseconds. Equivalent to Wall-time clock.
		printf("\tExecution time of K-J-I original: %lld microseconds.\n", Texec);

		printf("\t--------------------------------------------------------------------------\n\n");
	}

	//Volto a colocar os valores da matrix C a 0 para correr a versao transposta
	resetCMatrix(C, N);

	printf("\n\n\n******************************************************* RESULTS TO K-J-I TRANSPOSED *******************************************************\n\n");
	for (unsigned i = 0; i < REPETITIONS; ++i) {
		
		/*** START THE COUNTERS ***/
		PAPI_start_counters((int*)events,NUMEVENTS);
		retval = PAPI_start(EventSet);		//<--- last version working
		//PAPI_start = PAPI_get_real_usec();		// Times in microseconds. 
		
		/*** MULTIPLICACAO K-J-I TRANSPOSTA ***/
		matrixMultTRANSP(A, B, C, N);
		
		/*** STOP THE COUNTERS ***/
		retval = PAPI_stop(EventSet,values);		//<--- last version working
		//PAPI_stop = PAPI_get_real_usec();		// Times in microseconds.
		retval = PAPI_stop_counters(values,NUMEVENTS);
		
		/*** OBTER RESULTADOS ***/
		printf("\n\t------------------------------ EXECUTION: %d ------------------------------\n", i+1);

		/** 	CPI = #CC/#I 	**/
		CPI = ((double) values[2])/((double) values[1]);
		printf("\tCPI calculated: %f\n",CPI);
		printf("\tTotal CPU cycles (#CC): %f\n", (double) values[2]);
		printf("\tTotal executed instructions (#I): %f\n", (double) values[1]);
		printf("\tTotal Floating point instructions (#FP): %f\n", (double) values[0]);
		printf("\tPercentage of Miss Rate on L1: !!! --> PAPI_L1_TCA <--  NOT AVAILABLE on compute-431 @ SeARCH !!!\n");
		printf("\tPercentage of Miss Rate on L2: %f\n", (double)values[4]/(double)values[3]);
		printf("\tPercentage of Miss Rate on L3: %f\n", (double)values[6]/(double)values[5]);
		printf("\t#Acesses on RAM per instruction: %f\n", (double)values[7]/(double)values[1]);		
		
		/* Texec = #I * CPI * Tcc */
		Texec = ((double) values[1]) * CPI * (1/CPU_FREQUENCY_431); 	// <--- last version working
		//Texec = PAPI_stop - PAPI_start;		//Texec in microseconds. Equivalent to Wall-time clock.
		printf("\tExecution time of K-J-I transposta: %lld microseconds\n", Texec);

		printf("\t--------------------------------------------------------------------------\n\n");
	}

	/** The matrices should be freeded!!! ***/
	free(A);
	free(B);
	free(C);
	PAPI_shutdown();					/* Finished using PAPI. All resources should be "freeded" */
	//writeResults();

return 1;
}

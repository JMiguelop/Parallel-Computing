#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <omp.h>
#include <string>
#include <vector>

//#define N 1536				// Only power of 2 to simplify the code
#define MAX_THREADS 1 			// maximum number of threads to run
#define REPETITIONS 1 			// Number of measurements repetitions
#define TIME_RESOLUTION 1000000		// time measuring resolution (us)


/*
 *	Matrix-matrix arithmetic
 *
 */

using namespace std;

//int N = 1536;
//float A[N][N], B[N][N], C[N][N];

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

void fillMatrices (float **A, float **B, int N) {

	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			A[i][j] = ((float) rand()) / ((float) RAND_MAX);
			B[i][j] = ((float) rand()) / ((float) RAND_MAX);
		}
	}
}


/***** Ex_2.2 *******/
void matrixMult (float **A, float **B, float **C, int N) {
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
 * /
void matrixMultKJI (float **A, float **B, float **C, int N) {
        for (unsigned k = 0; k < N; ++k) {
		for (unsigned j = 0; j < N; ++j) {
			for (unsigned i = 0; i < N; ++i) {                
                                &(C[i][j]) += &(A[i][k]) * &(B[k][j]);
                        }
                }
        }
}
***/


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
	
	//long long unsigned time;
	//int N = 1536;
	int N = 2;
	float A[N][N], B[N][N], C[N][N];
	
	fillMatrices(&(A[N]),&(B[N]),N);

	// run the original code
	for (unsigned i = 0; i < REPETITIONS; ++i) {
		//clearCache();
		//start();
		matrixMult(&A,&B,&C,N);
		
		/** The matrices should be freeded!!! ***/
		//free(&A);
		//free(&B);
		//free(&C);
		//stop(-1);
	}

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

#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <string>
#include <vector>
#include <omp.h>

#define SIZE 1532	// Only powers of 2 to simplify the code
#define REPETITIONS 1 // Number of measurements repetitions
#define TIME_RESOLUTION 1000000	// time measuring resolution (us)

#ifdef AVX
	#define VEC_SIZE SIZE/8
#elif SSE
	#define VEC_SIZE SIZE/4
#endif

/*
 *	Example of matrix-matrix arithmetic
 *
 *	Students are supposed to test/implement the following optimisations
 *	for the multiplication:
 *		-> pointer alias (-fno-strict-aliasing)
 *		-> use a local variable to put the result on registers
 *		-> Tiling
 *		-> use icc auto vectorisation and analyse the report
 *		-> use icc -O1,2,3
 */

using namespace std;

float m1[SIZE][SIZE], m2[SIZE][SIZE], result[SIZE][SIZE];
long long unsigned initial_time;
vector<long long unsigned> measurements;
timeval t;

void start (void) {
	gettimeofday(&t, NULL);
	initial_time = t.tv_sec * TIME_RESOLUTION + t.tv_usec;
}


long long unsigned stop (void) {
	gettimeofday(&t, NULL);
	long long unsigned final_time = t.tv_sec * TIME_RESOLUTION + t.tv_usec;

	measurements.push_back(final_time - initial_time);

	return measurements.back();
}

long long unsigned min_element (void) {
	long long unsigned best = 0;

	for (auto i : measurements) {
		if (i < best || !best)
			best = i;
	}

	return best;
}

void fillMatrices (void) {

	for (unsigned i = 0; i < SIZE; ++i) {
		for (unsigned j = 0; j < SIZE; ++j) {
			m1[i][j] = ((float) rand()) / ((float) RAND_MAX);
			m2[i][j] = ((float) rand()) / ((float) RAND_MAX);
		}
	}
}

// Unoptimised version
void matrixMultNaive (float **mat1, float **mat2, float **res) {

	#pragma omp parallel for
	for (unsigned i = 0; i < SIZE; ++i) {
		for (unsigned j = 0; j < SIZE; ++j) {
			res[i][j] = 0;
			for (unsigned k = 0; k < SIZE; ++k) {
				res[i][j] += mat1[i][k] * mat2[k][j];
			}
		}
	}
}

int main (int argc, char *argv[]) {
	float **mat1, **mat2, **res;
	long long unsigned time;

	mat1 = new float*[SIZE];
	mat2 = new float*[SIZE];
	res = new float*[SIZE];

	for (unsigned i = 0; i < SIZE; ++i) {
		mat1[i] = new float[SIZE];
		mat2[i] = new float[SIZE];
		res[i] = new float[SIZE];

		for (unsigned j = 0; j < SIZE; ++j) {
			mat1[i][j] = m1[i][j];
			mat2[i][j] = m2[i][j];
		}
	}

	fillMatrices();

	double *clearcache;

	for (unsigned i = 0; i < REPETITIONS; ++i) {
		free(clearcache);
		clearcache = new double [3300000];

		start();
		matrixMultNaive(mat1, mat2, res);
		stop();
	}

	cout << "Execution time for " << omp_get_num_threads() << " thread(s): " << min_element() << " us" << endl;
	measurements.clear();

	return 1;
}

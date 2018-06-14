#include <cstdlib>
#include <iostream>
#include <papi.h>
#include <stdio.h>

#define SIZE 128
#define NUM_EVENTS 2

int Events[NUM_EVENTS]={PAPI_FP_INS,PAPI_TOT_CYC};
int EventSet;

long long values[NUM_EVENTS];

int retval;

using namespace std;

int main (void) {
	float a[SIZE][SIZE], b[SIZE][SIZE], c[SIZE][SIZE];
	int i, j, k;

	retval = PAPI_library_init(PAPI_VER_CURRENT);
	retval = PAPI_create_eventset(&EventSet);
	retval = PAPI_add_events(EventSet,Events,NUM_EVENTS);

	retval = PAPI_start(EventSet);



	for (int x = 0; x < SIZE; x++) {
		for (int y = 0; y < SIZE; y++) {
			a[x][y] = rand()/RAND_MAX;
			b[x][y] = rand()/RAND_MAX;
			c[x][y] = 0;
		}
	}

	//Multiplication Logic
	for (i = 0; i < SIZE; i++) {
		for (j = 0; j < SIZE; j++) {
			for (k = 0; k < SIZE; k++) {
				c[i][j] += a[i][k] * b[k][j];
			}
		}
	}

	retval = PAPI_stop(EventSet,values);
	
	printf("%d\n", values[0]);
	printf("%d\n", values[1]);	
	

	return 0;
}

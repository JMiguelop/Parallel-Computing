//
// helloflops1
//
// A simple example to try 
// to get lots of Flops on 
// Intel(r) Xeon Phi(tm) co-processors.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>
//
// dtime -
//
// utility routine to return 
// the current wall clock time
//
double dtime()
{
    double tseconds = 0.0;
    struct timeval mytime;
    gettimeofday(&mytime,(struct timezone*)0);
    tseconds = (double)(mytime.tv_sec + mytime.tv_usec*1.0e-6);
    return( tseconds );
}


#define FLOPS_ARRAY_SIZE (1024*1024) 
#define MAXFLOPS_ITERS 100000000
#define LOOP_COUNT 128
// Floating pt ops per inner loop iteration
#define FLOPSPERCALC 2
#define NUM_THREADS 4
// define some arrays - 64 byte aligned for fastest cache access 
float fa[FLOPS_ARRAY_SIZE] __attribute__((align(64)));
float fb[FLOPS_ARRAY_SIZE] __attribute__((align(64)));

//
// Main program - pedal to the metal...calculate tons o'flops!
// 
int main(int argc, char *argv[] ) 
{
    int i,j,k;
    double tstart, tstop, ttime;
	double gflops = 0.0;
   	float a=1.1;

    
    //
    // initialize the compute arrays 
    //

    printf("Initializing\r\n");
    for(i=0; i<FLOPS_ARRAY_SIZE; i++){
    	fa[i] = (float)i + 0.1;
        fb[i] = (float)i + 0.2;
    }	

    printf("Starting Compute\r\n");

    tstart = dtime();	
    
    // loop many times to really get lots of calculations - SAXPY part!
    // omp_set_num_threads(NUM_THREADS);
    omp_set_num_threads(NUM_THREADS);
	#pragma omp parallel for private ( j , k )
	for ( i = 0 ; i < NUM_THREADS; i++) {
		int  tid = omp_get_thread_num();
		for(j=0; j<MAXFLOPS_ITERS; j++)  
		{
			for(k=0; k<LOOP_COUNT; k++) {
				fa[k+(tid*LOOP_COUNT)] = a * fa[k+(tid*LOOP_COUNT)] + fb[k+(tid*LOOP_COUNT)];
			}
		}
	}

     tstop = dtime();

     // # of gigaflops we just calculated  
     gflops = (double)( 1.0e-9*LOOP_COUNT*MAXFLOPS_ITERS*FLOPSPERCALC);    

 // elasped time
     ttime = tstop - tstart;

     //
     // Print the results
     //
     if ((ttime) > 0.0)
     {
         printf("GFlops = %10.3lf, Secs = %10.3lf, GFlops per sec = %10.3lf\r\n",
                 gflops, ttime, gflops/ttime);
     }	 
     return( 0 );
}


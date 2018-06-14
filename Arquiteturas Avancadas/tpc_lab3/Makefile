CC              =       gcc
CFLAGS          =      -fopenmp -O3 -ftree-vectorize 
FC		=	ifort
FFLAGS		=      -array64byte -fopenmp -O3 -vec-report=3
USER = a59905
MICDIR          =      /home/$(USER)
MICLIBS		=      /opt/intel/composer_xe_2013.1.117/compiler/lib/mic/

#-no-vec

hflops1: helloflops1.c
	$(CC) $(CFLAGS) helloflops1.c -o helloflops1_xphi

hflops3o: helloflops3offload.c
	$(CC) $(CFLAGS) helloflops3offload.c -o helloflops3o_xeon

all: hflops1 hflops3o

miccopy: 
	scp *_xphi $(USER)@mic0:~/

miclibcopy:
	scp $(MICLIBS)/libiomp5.so $(USER)@mic0:~/
	@echo ""
	@echo "REMEMBER TO export LD_LIBRARY_PATH=$(MICDIR) ON THE COPROCESSOR (if needed)"
	@echo ""


clean: 
	rm -f ./*_xphi
	rm -f ./*_xeon




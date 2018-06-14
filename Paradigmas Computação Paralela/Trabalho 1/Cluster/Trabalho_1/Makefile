################################################################################
# Makefile for PCP
#
# by Joao Lopes
################################################################################

SHELL = /bin/sh

##BIN_NAME = trabalho_PCP_1
##PAPI_DIR = /share/apps/papi/5.3.2

CXX = gcc
LD  = gcc

SEQ = heatDiff_Seq
PAR = heatDiff_Par
BIN = bin
BIN_SEQ = trabalho_PCP_1_seq
BIN_PAR = trabalho_PCP_1_parallel_v2

#CXX = icpc
#LD  = icpc

#GNU
#-fopenmp 

#GNU
# WITH OPTIMIZATIONS
#CXXFLAGS   = -O2 -Wall -Wextra -std=c++11 -fopenmp -ftree-vectorize -fopt-info-vec-all

# WITHOUT OPTIMIZATIONS
#CXXFLAGS = -Wall -Wextra -std=c++11 -L $(PAPI_DIR)/lib -I $(PAPI_DIR)/include -lpapi
CXXFLAGS = -O3 -Wall -Wextra -std=c99 -fopenmp


#INTEL
#-openmp

#INTEL
#CXXFLAGS   = -O2 -Wall -Wextra -std=c++11 -openmp -vec-report3

ifeq ($(DEBUG),yes)
	CXXFLAGS += -ggdb3
endif

################################################################################
# Control awesome stuff
################################################################################

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst src/*.c,build/*.o,$(SRC))
DEPS = $(patsubst build/*.o,build/*.d,$(OBJ))

vpath %.c $(SRC_DIR)

################################################################################
# Rules
################################################################################

.DEFAULT_GOAL = all

$(BUILD_DIR)/%.d: %.c
	$(CXX) -M $(CXXFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

$(BIN_DIR)/$(BIN_SEQ): $(BUILD_DIR)/$(SEQ).o $(BUILD_DIR)/$(SEQ).d 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(BUILD_DIR)/$(SEQ).o

$(BIN_DIR)/$(BIN_PAR): $(BUILD_DIR)/$(PAR).o $(BUILD_DIR)/$(PAR).d 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(BUILD_DIR)/$(PAR).o

##$(BIN_DIR)/$(BIN_NAME): $(DEPS) $(OBJ)
##	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJ)

checkdirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

all: checkdirs $(BIN_DIR)/$(BIN_SEQ) $(BIN_DIR)/$(BIN_PAR)

clean:
	rm -f $(BUILD_DIR)/* $(BIN_DIR)/* 

################################################################################
# Makefile for general code snippets
#
# by André Pereira (LIP-Minho)
################################################################################

SHELL = /bin/sh

BIN_NAME = vector_kernel


CXX = nvcc
LD  = nvcc


CXXFLAGS   = -O3

ifeq ($(AVX),yes)
	CXXFLAGS += -mavx -DAVX
else
	ifeq ($(SSE),yes)
		CXXFLAGS += -msse4.1 -DSSE
	endif
endif

ifeq ($(DEBUG),yes)
	CXXFLAGS += -ggdb3
endif

################################################################################
# Control awesome stuff
################################################################################

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build
SRC = $(wildcard $(SRC_DIR)/*.cu)
OBJ = $(patsubst src/%.cu,build/%.o,$(SRC))
DEPS = $(patsubst build/%.o,build/%.d,$(OBJ))
BIN = $(BIN_NAME)

vpath %.cu $(SRC_DIR)

################################################################################
# Rules
################################################################################

.DEFAULT_GOAL = all

$(BUILD_DIR)/%.d: %.cu
	$(CXX) -M $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LIBS)

$(BUILD_DIR)/%.o: %.cu
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LIBS)

$(BIN_DIR)/$(BIN_NAME): $(DEPS) $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJ) $(LIBS)

checkdirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

all: checkdirs $(BIN_DIR)/$(BIN_NAME)

clean:
	rm -f $(BUILD_DIR)/* $(BIN_DIR)/* 

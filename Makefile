CSOURCES = aiger.c

CPPSOURCES = checker.cpp carsolver.cpp mainsolver.cpp model.cpp utility.cpp data_structure.cpp main.cpp \
	minisat/core/Solver.cc minisat/utils/Options.cc minisat/utils/System.cc
#CSOURCES = aiger.c
#CPPSOURCES = bfschecker.cpp checker.cpp carsolver.cpp mainsolver.cpp model.cpp utility.cpp data_structure.cpp main.cpp \
	glucose/core/Solver.cc glucose/utils/Options.cc glucose/utils/System.cc

OBJS = checker.o carsolver.o mainsolver.o model.o main.o utility.o data_structure.o aiger.o\
	Solver.o Options.o System.o

# run O2 optimization
CFLAG = -O2 -I../ -I./minisat -D__STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -c -g -fpermissive
# CFLAG = -I../ -I./minisat -D__STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -c -g -fpermissive
#CFLAG = -I../ -I./glucose -D__STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -c -g 

LFLAG = -g -lz -lpthread 

GCC = gcc

GXX = g++

# close compiler warning manually 2021-7-10 start
CLOSEWARNING = -w
# end

DMC: $(CSOURCES) $(CPPSOURCES)
	$(GCC) $(CFLAG) $(CSOURCES) $(CLOSEWARNING)
	$(GCC) $(CFLAG) -std=c++11 $(CPPSOURCES) $(CLOSEWARNING)
	$(GXX) -o UAIR $(OBJS) $(LFLAG) $(CLOSEWARNING)
	rm *.o

clean: 
	rm UAIR
	
.PHONY: UAIR

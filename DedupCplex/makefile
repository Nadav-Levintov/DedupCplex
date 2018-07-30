CPLEX_DIR = /home/nadavl/Project2/cplex


#TODO: need to make this makefile work

CXX = g++
CXXFLAGS = -g -Wall -O
CXXFLAGS += -I$(CPLEX_DIR)/include

LINK.o = $(LINK.cc)
LDFLAGS += -L$(CPLEX_DIR)/lib/x86-64_linux/static_pic
LDLIBS += -lcplex -lm -pthread


all: main

main.o: cpxmacro.h


clean:
    $(RM) *~ *.o main

.PHONY: all clean
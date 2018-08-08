SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX programs
# please copy this makefile and set CPLEXDIR and CONCERTDIR to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

CPLEXDIR      = /home/nadavl/Project2/cplex/cplex
CONCERTDIR    = /home/nadavl/Project2/cplex/concert

# ---------------------------------------------------------------------
# Compiler selection 
# ---------------------------------------------------------------------

CCC = g++ -O0

# ---------------------------------------------------------------------
# Compiler options 
# ---------------------------------------------------------------------

CCOPT = -std=c++11 -m64 -O0 -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -DDEBUG -g

# ---------------------------------------------------------------------
# Link options and libraries
# ---------------------------------------------------------------------

CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
CCLNFLAGS = -lconcert -lilocplex -lcplex -lm -lpthread -ldl

all:
	make all_cpp

execute: all
	make execute_cpp
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

EXDIR         = $(CPLEXDIR)/examples
EXINC         = $(EXDIR)/include
EXDATA        = $(EXDIR)/data
EXSRCCPP      = $(EXDIR)/src/cpp

CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 


#------------------------------------------------------------
#  make all      : to compile the examples. 
#  make execute  : to compile and execute the examples.
#------------------------------------------------------------

CPP_EX = DedupCplex

all_cpp: $(CPP_EX)

execute_cpp: $(CPP_EX)
	 ./DedupCplex
	 

# ------------------------------------------------------------

clean :
	/bin/rm -rf *.o *~ *.class
	/bin/rm -rf $(C_EX) $(CX_EX) $(CPP_EX)
	/bin/rm -rf *.mps *.ord *.sos *.lp *.sav *.net *.msg *.log *.clp


DedupCplex: DedupCplex.o
	$(CCC) $(CCFLAGS) $(CCLNDIRS) -o DedupCplex DedupCplex.o $(CCLNFLAGS)
DedupCplex.o: DedupCplex.cpp
	$(CCC) -c $(CCFLAGS) DedupCplex.cpp -o DedupCplex.o

# Local Variables:
# mode: makefile
# End:

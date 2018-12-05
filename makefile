UNAME := $(shell uname)

EXE                    = pytorch_cat.exe
EXE_MPI	               = pytorch_cat_MPI.exe

COMPILATOR             = g++
COMPILATOR_MPI         = mpic++

COMPILATOR_OPTIONS     = -g -ansi 
COMPILATOR_OPTIONS_MPI = $(COMPILATOR_OPTIONS) -DUSE_MPI


LIB_DIR                = $(NOMAD_HOME)/lib
LIB_NOMAD              = libnomad.so 
LIB_NOMAD_MPI          = libnomad.MPI.so

CXXFLAGS               =           
ifeq ($(UNAME), Linux)
CXXFLAGS              += -Wl,-rpath,'$(LIB_DIR)'
CXXFLAGS              += -ansi
endif
CXXFLAGS_MPI 	       = $(CXXFLAGS) -DUSE_MPI


LDLIBS                 = -lm -lnomad
LDLIBS_MPI             = -lm -lmpi -lnomad.MPI

INCLUDE                = -I$(NOMAD_HOME)/src -I$(NOMAD_HOME)/ext/sgtelib/src -I.

COMPILE                = $(COMPILATOR) $(COMPILATOR_OPTIONS) $(INCLUDE) -c
COMPILE_MPI            = $(COMPILATOR_MPI) $(COMPILATOR_OPTIONS_MPI) $(INCLUDE) -c

OBJS                   = pytorch_cat.o
OBJS_MPI               = pytorch_cat_MPI.o


ifndef NOMAD_HOME
define ECHO_NOMAD
	@echo Please set NOMAD_HOME environment variable!
	@false
endef
endif


$(EXE): $(OBJS)
	$(ECHO_NOMAD)
	@echo "   building the scalar version ..."
	@echo "   exe file : "$(EXE)
	@$(COMPILATOR) -o $(EXE) $(OBJS) $(LDLIBS) $(CXXFLAGS) -L$(LIB_DIR) 
ifeq ($(UNAME), Darwin)
	@install_name_tool -change $(LIB_NOMAD) $(NOMAD_HOME)/lib/$(LIB_NOMAD) $(EXE)
endif

$(EXE_MPI): $(OBJS_MPI) 
	$(ECHO_NOMAD)
	@echo "   building the MPI version ..."
	@echo "   exe file : "$(EXE_MPI)
	@$(COMPILATOR_MPI) -o  $(EXE_MPI) $(OBJS_MPI) $(LDLIBS_MPI) $(CXXFLAGS_MPI) -L$(LIB_DIR)
ifeq ($(UNAME), Darwin)
	@install_name_tool -change $(LIB_NOMAD_MPI) $(NOMAD_HOME)/lib/$(LIB_NOMAD_MPI) $(EXE_MPI)
endif

pytorch_cat.o: pytorch_cat.cpp
	$(ECHO_NOMAD)
	@$(COMPILE) pytorch_cat.cpp

pytorch_cat_MPI.o: pytorch_cat.cpp 
	$(ECHO_NOMAD)
	@$(COMPILE_MPI) pytorch_cat.cpp -o $@


mpi: $(EXE_MPI)

all: $(EXE) $(EXE_MPI)

clean: ;
	@echo "   cleaning obj files"
	@rm -f $(OBJS) $(OBJS_MPI)

del: ;
	@echo "   cleaning trash files"
	@rm -f core *~
	@echo "   cleaning obj files"
	@rm -f $(OBJS) $(OBJS_MPI)
	@echo "   cleaning exe file"
	@rm -f $(EXE) $(EXE_MPI)



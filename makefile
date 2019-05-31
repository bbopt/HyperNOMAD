ifndef ($(VARIANT))
VARIANT             = release
endif

UNAME := $(shell uname)

EXE                    = hypernomad.exe

COMPILATOR             = g++

COMPILATOR_OPTIONS     = -std=c++14  

LIB_DIR                = $(NOMAD_HOME)/lib
LIB_NOMAD              = libnomad.so 

CXXFLAGS               =           
ifeq ($(UNAME), Linux)
CXXFLAGS              += -Wl,-rpath,'$(LIB_DIR)'
CXXFLAGS              += -ansi
endif


LDLIBS                 = -lm -lnomad

INCLUDE                = -I$(NOMAD_HOME)/src -I$(NOMAD_HOME)/ext/sgtelib/src -I.

COMPILE                = $(COMPILATOR) $(COMPILATOR_OPTIONS) $(INCLUDE) -c


TOP                    = $(abspath .)
BUILD_DIR              = $(TOP)/build/$(VARIANT)
SRC		       = $(TOP)/src/nomad_optimizer
BIN_DIR                = $(TOP)/bin

EXE                   := $(addprefix $(BIN_DIR)/,$(EXE))


OBJS                   = fileutils.o hypernomad.o hyperParameters.o
OBJS                  := $(addprefix $(BUILD_DIR)/,$(OBJS))

ifndef NOMAD_HOME
define ECHO_NOMAD
	@echo Please set NOMAD_HOME environment variable!
	@false
endef
endif


$(EXE): $(OBJS)
	$(ECHO_NOMAD)
	@mkdir -p $(BIN_DIR)
	@echo "   building HYPERNOMAD ..."
	@$(COMPILATOR) -o $(EXE) $(OBJS) $(LDLIBS) $(CXXFLAGS) -L$(LIB_DIR) 
ifeq ($(UNAME), Darwin)
	@install_name_tool -change $(LIB_NOMAD) $(NOMAD_HOME)/lib/$(LIB_NOMAD) $(EXE)
endif
	@ln -fs $(EXE) $(TOP)/examples/.
        @echo     
	@echo    To be able to run the example 
        @echo    the HYPERNOMAD_HOME environment variable 
	@echo    must be set to $(TOP)

$(BUILD_DIR)/%.o: $(SRC)/%.cpp $(SRC)/hyperParameters.hpp $(SRC)/fileutils.hpp
	$(ECHO_NOMAD)
	@mkdir -p $(BUILD_DIR)
	@$(COMPILE) $< -o $@


all: $(EXE)

clean: ;
	@echo "   cleaning obj files"
	@rm -f $(OBJS)

del: ;
	@echo "   cleaning trash files"
	@rm -f core *~
	@echo "   cleaning obj files"
	@rm -f $(OBJS) 
	@echo "   cleaning exe file"
	@rm -f $(EXE) 
	@echo "   cleaning build dir"
	@rm -rf $(BUILD_DIR)



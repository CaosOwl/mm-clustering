#flags for the compilation
CXXFLAGS := -Wall -Wextra -Wno-unused -Wno-unused-parameter -fPIC -std=c++0x -g -Wl,--as-needed `root-config --cflags --libs`


#define directories
OBJDIR     := obj
SRCDIR     := source/src
INCLUDEDIR := source/include
EXEDIR     := examples

#GLOBAL PARAMETER
#STDOPT := -O2

#ANALYSIS DIRECTORIES
ANACONDADIR = /home/deppy/.anaconda3
H5DIR       = /opt/HighFive/HighFive
H5LIBPATH  := $(ANACONDADIR)/pkgs/hdf5-1.10.2-hba1933b_1/lib #local computer

#INCLUDE FILE DIRECTORY
CXXFLAGS += -I$(INCLUDEDIR)/
CXXFLAGS += -I/usr/include/eigen3 #eigen standard include path

#FILES
INCLUDEFILES := $(wildcard $(INCLUDEDIR)/*.hh)
SRCFILES     := $(wildcard $(SRCDIR)/*.cc)
OBJECTFILES  := $(patsubst $(SRCDIR)/%.cc,     $(OBJDIR)/%.o,  $(SRCFILES))

#add exception to files
#EXCEPTIONFILES := $(filter-out $(SRCDIR)/utilities/OrgFile.cc $(SRCDIR)/utilities/OrgTable.cc, $(UTILITYFILES))

LIBS := -lGeom -lEG -lMinuit  -lTMVA -lSpectrum -lboost_program_options -lboost_system -lboost_filesystem

#LIBS += -lboost_program_options -lboost_system

#HDF5LIBS :=  -L$(H5LIBPATH) -lhdf5 -Wl,-rpath $(H5LIBPATH)

TARGETS := template test_clustering create_toy

all: $(TARGETS)

#RULE TO MAKE OBJECTS
$(OBJECTFILES): $(OBJDIR)/%o : $(SRCDIR)/%cc $(INCLUDEDIR)/%hh
	@mkdir -p $(OBJDIR)
	@$(CXX) $(CXXFLAGS)  -o $@ -c $< $(LIB_PATH) $(STDOPT) $(LIBS)
	@echo -e "object file \033[1;31m"$@"\033[0m has been created"

#ANALYSIS EXECUTABLES
%: $(EXEDIR)/%.cc $(OBJECTFILES) $(INCLUDEFILES)
	$(CXX) -o $(OBJDIR)/$@.o -c $< $(CXXFLAGS) $(STDOPT) 
	$(CXX) -o $@ $(OBJDIR)/$@.o $(OBJECTFILES) $(CXXFLAGS) $(STDOPT) $(LIBS)
	@echo -e "executable \033[1;32m"$@"\033[0m has been created"

clean:
	@rm -f  $(TARGETS)
	@rm -rf $(OBJDIR)	
	@find . -name "*~" -exec rm {} \;
	@find . -name "*.o" -exec rm {} \;


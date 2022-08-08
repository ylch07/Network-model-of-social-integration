# This is the GNU makefile for the codes of the population model.
#
# The codes can be compiled in MacOS or in Linux.
# It requires the g++ compiler and the openGL library (gl & glut), 
#    which usually come with the installation of the operating system.
# To compile, type the follows in a terminal window:
#        make adapt
#    Then a executable file named 'adapt' will be generated in the
#    current folder. 
#
# Author Yao-li Chuang 
####################################################################

#CPP = g++ -O3 -arch i386 -mmacosx-version-min=10.5 
CPP = g++ -O3 
#CPP = /opt/intel/bin/icc -O3
#CPP = nvcc -arch=sm_30 -O3 

# Check the OS system (MacOS or Linux) and set the graphic libraries accordingly.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	GLFLAGS= -framework GLUT -framework OpenGL \
	         -framework Cocoa -g -Wno-deprecated
endif
ifeq ($(UNAME_S),Linux)
	GLFLAGS= -lglut -lGL -lGLU -lX11 -lm -L/usr/X11R6/lib \
                 -Wno-psabi
endif

NODE = Node
MODEL = Model
GRAPH = Graphics
STATS = Stats
OBJ = OF

adapt :  $(OBJ)/AgentC.o $(OBJ)/NodeC.o $(OBJ)/NodeListC.o \
         $(OBJ)/ModelC.o $(OBJ)/StatC.o $(OBJ)/GraphModelC.o \
         Main.cxx Main.H CCommon.h $(GRAPH)/GraphicCommon.hpp
	$(CPP) -o adapt $(OBJ)/AgentC.o $(OBJ)/NodeC.o \
                        $(OBJ)/NodeListC.o $(OBJ)/ModelC.o \
                        $(OBJ)/StatC.o $(OBJ)/GraphModelC.o \
                        Main.cxx \
                        $(LDFLAGS) $(GLFLAGS)

$(OBJ)/AgentC.o : $(GRAPH)/AgentC.cxx $(GRAPH)/AgentC.hpp CCommon.h | $(OBJ)
	$(CPP) -c $(GRAPH)/AgentC.cxx -o $(OBJ)/AgentC.o
$(OBJ)/NodeC.o : $(NODE)/NodeC.cxx $(NODE)/NodeC.hpp CCommon.h | $(OBJ)
	$(CPP) -c $(NODE)/NodeC.cxx -o $(OBJ)/NodeC.o
$(OBJ)/NodeListC.o : $(NODE)/NodeListC.cxx $(NODE)/NodeListC.hpp \
                  $(NODE)/NodeC.hpp CCommon.h | $(OBJ)
	$(CPP) -c $(NODE)/NodeListC.cxx -o $(OBJ)/NodeListC.o
$(OBJ)/ModelC.o : $(MODEL)/ModelC.cxx $(NODE)/NodeListC.hpp CCommon.h | $(OBJ)
	$(CPP) -c $(MODEL)/ModelC.cxx -o $(OBJ)/ModelC.o
$(OBJ)/StatC.o : $(STATS)/StatC.cxx $(NODE)/NodeListC.hpp CCommon.h | $(OBJ)
	$(CPP) -c $(STATS)/StatC.cxx -o $(OBJ)/StatC.o
$(OBJ)/GraphModelC.o : $(GRAPH)/GraphModelC.cxx $(NODE)/NodeListC.hpp \
                    CCommon.h | $(OBJ)
	$(CPP) -c $(GRAPH)/GraphModelC.cxx -o $(OBJ)/GraphModelC.o

$(OBJ):
	mkdir -p $(OBJ)

clean :
	rm -f $(OBJ)/*.o *~
	rmdir  $(OBJ)

.PHONY: clean $(OBJ)

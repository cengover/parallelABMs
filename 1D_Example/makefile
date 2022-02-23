#************************************************************************************************************
#
# Repast HPC Tutorial Makefile
#
#************************************************************************************************************

MPICXX=/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/MPICH/bin/mpicxx -std=c++11 

BOOST_INCLUDE=-I/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/Boost/Boost_1.61/include/
BOOST_LIB_DIR=-L/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/Boost/Boost_1.61/lib/
BOOST_LIBS=-lboost_mpi-mt -lboost_serialization-mt -lboost_system-mt -lboost_filesystem-mt

REPAST_HPC_INCLUDE=-I/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/repast_hpc-2.2.0/include/
REPAST_HPC_LIB_DIR=-L/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/repast_hpc-2.2.0/lib/
REPAST_HPC_LIB=-lrepast_hpc-2.2.0

.PHONY: all
all : 


.PHONY: clean_output_files
clean_output_files:
	rm -f *.csv
	rm -f *.txt
	rm -f ./output/*.csv
	rm -f ./output/*.txt
	rm -f ./logs/*.*

.PHONY: clean_compiled_files
clean_compiled_files:
	rm -f *.exe
	rm -f ./bin/*.exe
	rm -f *.o
	rm -f ./object/*.o
	
.PHONY: clean
clean: clean_compiled_files clean_output_files
	#rm -f *.cpp
	#rm -f ./src/*.cpp
	#rm -f *.props
	#rm -f ./props/*.props
	#rm -f ./include/*.h

.PHONY: RepastHPC_Demo_03
RepastHPC_Demo_03: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Main.cpp -o ./objects/Demo_03_Main.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Model.cpp -o ./objects/Demo_03_Model.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Agent.cpp -o ./objects/Demo_03_Agent.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/Demo_03.exe  ./objects/Demo_03_Main.o ./objects/Demo_03_Model.o ./objects/Demo_03_Agent.o $(REPAST_HPC_LIB) $(BOOST_LIBS)


.PHONY: RepastHPC_Demo_03_Step_G
RepastHPC_Demo_03_Step_G : RepastHPC_Demo_03

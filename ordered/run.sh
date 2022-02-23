MPICXX=/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/MPICH/bin/mpicxx -std=c++11 

BOOST_INCLUDE=-I/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/Boost/Boost_1.61/include/
BOOST_LIB_DIR=-L/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/Boost/Boost_1.61/lib/
BOOST_LIBS=-lboost_mpi-mt -lboost_serialization-mt -lboost_system-mt -lboost_filesystem-mt

REPAST_HPC_INCLUDE=-I/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/repast_hpc-2.2.0/include/
REPAST_HPC_LIB_DIR=-L/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/repast_hpc-2.2.0/lib/
REPAST_HPC_LIB=-lrepast_hpc-2.2.0


total=100
procs=(1 2 4 5 8 10 20 40)
for ((rn=1 ; rn<=$total ; rn++))
do
	cd ./props
	t=`echo "$rn*1" | bc -l`
	echo $t
	sed "s/global.random.seed = 1/global.random.seed = $t/g" modelcopy.props > model.props
	cd ..
	for i in ${procs[@]}
	do
		/home/ozi/Documents/repasthpc/repast_hpc-2.2.0/MANUAL_INSTALL/MPICH/bin/mpirun -n $i  ./bin/Demo_03.exe ./props/config.props ./props/model.props

	done
done

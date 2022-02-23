total=100
procs=(1 2 4 5 10 20 40)
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

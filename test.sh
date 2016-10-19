#!/bin/bash

#numbers are given or 16
if [ $# -lt 1 ];then 
        inpNumbers=16;
else
        inpNumbers=$1;
fi;

#mpi cpp compilation
mpic++ --prefix /usr/local/share/OpenMPI -o pms pms.cpp

#series of random numbers generation
#should be exponential of base 2

dd if=/dev/random bs=1 count=$inpNumbers of=numbers &>/dev/null


#counting of cpus
x=`bc -l <<< "l($inpNumbers)/l(2)"`
cpus=`echo $x | xargs printf "%.*f\n" 0`
cpus=$(($cpus + 1))
#running program
#mpirun --prefix /usr/local/share/OpenMPI -np $cpus ./pms numbers

mpirun -np $cpus ./pms
#cleanup
rm -f pms numbers




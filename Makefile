COMPILER=mpic++


all:
	$(COMPILER) -o pms pms.cpp

merlin:
	$(COMPILER) --prefix /usr/local/share/OpenMPI -o pms pms.cpp

run:
	mpirun -np 2 ./pms numbers-2-0

run4:
	mpirun -np 3 ./pms numbers-4-0

run16:
	mpirun -np 5 ./pms numbers-16-0
	
push:
	scp pms.cpp test1.sh  xdvora0y@eva.fit.vutbr.cz:/homes/eva/xd/xdvora0y/PRL


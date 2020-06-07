vm_simulator : vm_simulator.o kernel.o process.o
	gcc -o ./vm_simulator vm_simulator.o kernel.o process.o 

vm_simulator.o : kernel.h process.h vm_simulator.c
	gcc -c vm_simulator.c

process.o : process.c
	gcc -c process.c

kernel.o : kernel.c
	gcc -c kernel.c

clean:
	rm -f *.o
u
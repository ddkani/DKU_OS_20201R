fs_simulator :  fs_simulator.o kernel.o functions.o
	gcc -o ./fs_simulator fs_simulator.o kernel.o functions.o

fs_simulator.o : kernel.h config.h fs_simulator.c
	gcc -c fs_simulator.c

kernel.o : fs.h config.h functions.h kernel.c
	gcc -c kernel.c

functions.o : functions.c
	gcc -c functions.c

clean:
	rm -f *.o
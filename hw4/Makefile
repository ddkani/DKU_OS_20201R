fs_simulator :  main.o kernel.o functions.o
	gcc -o ./fs_simulator main.o kernel.o functions.o

main.o : kernel.h config.h main.c
	gcc -c main.c

kernel.o : fs.h config.h functions.h kernel.c
	gcc -c kernel.c

functions.o : functions.c
	gcc -c functions.c

clean:
	rm -f *.o
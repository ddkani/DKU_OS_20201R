sish : main.o functions.o
	gcc -o ./sish main.o functions.o

main.o : functions.h main.c
	gcc -c main.c

functions.o : functions.c
	gcc -c functions.c

clean:
	rm -f *.o

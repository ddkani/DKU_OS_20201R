wordcount : wordcount.o
	gcc -o ./wordcount wordcount.c -pthread

wordcount.o : wordcount.c
	gcc -c wordcount.c


clean:
	rm -f *.o

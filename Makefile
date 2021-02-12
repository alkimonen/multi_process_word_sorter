all: pwc twc
pwc: pmc.c
	gcc -Wall -std=gnu99 -g -o pwc pmc.c -lrt 
twc: tmc.c
	gcc -Wall -std=gnu99 -pthread -g -o twc tmc.c
clean:
	rm -fr pwc pmc.o twc tmc.o *~
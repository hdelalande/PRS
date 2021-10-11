CLFAGS = -g -Wall -DDEBUG

all: serveur-1

serveur: serveur-1.o
	gcc ${CLFAGS} serveur-1.o -o serveur-1

serveur.o: serveur.c
	gcc ${CLFAGS} -c serveur-1.c -o serveur-1.o

clean:
	rm -f serveur-1 *.o

default: clean all

server: server.c ../errors.c ../errors.h
	gcc -o server server.c ../errors.c -lpthread

gardener: gardener.c ../errors.c ../errors.h
	gcc -o gardener gardener.c ../errors.c -lpthread

flowers: flowers.c ../errors.c ../errors.h
	gcc -o flowers flowers.c ../errors.c -lpthread

all: server gardener flowers

clean: 
	rm -rf server gardener flowers

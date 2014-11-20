all:
	gcc -o latebench -O9 main.c -lmemcached -lrt -lpthread
debug:
	gcc -o latebench -g main.c -lmemcached -lrt -lpthread

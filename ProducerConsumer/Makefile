#all: Makefile shmem.c semaphores.c functions.h main.c
#	gcc shmem.c semaphores.c main.c -o main
all: main consumer

main: main.c shmem.c semaphores.c functions.h 
	gcc -o shmem.c semaphores.c main.c main

consumer: Consumer.c shmem.c semaphores.c functions.h 
	gcc -o shmem.c semaphores.c Consumer.c consumer
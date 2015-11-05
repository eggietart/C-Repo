SYSC 4001 - Assignment #2
Student: Agatha Wong
Student #: 100825966

To compile the files, in the command line window type:

	$make

Otherwise, you can compile both consumer and producer programs seperately:

	$ gcc Producer.c -o producer functions.h semaphores.c shmem.c
	$ gcc Consumer.c -o consumer functions.h semaphores.c shmem.c
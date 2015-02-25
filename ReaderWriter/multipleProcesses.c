#include <stdio.h>
#include <sys/types.h>

struct stock
{
	char name;
	int value;
};



int main(int argc, char const *argv[])
{
	int tempInt = createReaders();
	return 0;
}

int createProcesses()
{
	FILE *file;
	char c;
	int a = 'A';
	int C = 97; /* 'a' = 97 */
	int i;
	int r, pid;

	remove("f.txt");

	for (i = 1; i <= 26; i++)
	{
		if ((pid = fork()) != 0)
		{
			/* parent process pid != 0 */
			/* wait for child to terminate */

			waitpid(pid);
			a++;
		}
		else
		{
			/* child process pid = 0 */
			/* write next character to file */
			file = fopen("f.txt", "a");
			fprintf(file, "%c", a);
			fclose(file);
			exit(1);

		}
		file = fopen("f.txt", "r");
	}

	while (1)
	{
		fscanf(file, "%c", &c);
		if (feof(file))
			break;
		printf("%c\n", c);
	}
	fclose(file);
	printf("\n");

	return 1;
}

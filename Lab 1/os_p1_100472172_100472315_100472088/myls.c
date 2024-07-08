//P1-SSOO-22/23

#include <linux/limits.h> // Header file for PATH_MAX
#include <stdio.h>		  // Header file for system call printf
#include <unistd.h>		  // Header file for system call gtcwd
#include <sys/types.h>	  // Header file for system calls opendir, readdir y closedir
#include <dirent.h>       // Header file for types DIR and dirent
#include <err.h>          // Header file for system call err
#include <errno.h>        // Header file for errno

int print_ls(char *dir_name) {
	/*
	** This function prints each element of the dir_name directory
	*/
	// We start by opening the directory
	DIR *directory = opendir(dir_name);
	// Checking the
	if (directory == NULL) {
		closedir(directory);
        printf("cannot access '%s': there is no such directory\n", dir_name);
		return -1;
	}

	// Data type that stores useful information from a directory
	struct dirent *dir;

	// This loop iterates through each element of the directory
	while ((dir = readdir(directory))) {
		// Printing the name of the given element
		printf("%s\n", dir->d_name);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	// We will store the current path in this variable if necessary
	char path_buffer[PATH_MAX];
	int i;

	switch (argc) {
		// If there is just one argument (the name of the program), we are calling the program for the current directory
		case 1:
			// We obtain the current working directory and store it in the path_buffer variable
			getcwd(path_buffer, PATH_MAX);
			// Now we call the function that prints each element of the directory
			print_ls(path_buffer);
			break;
		// If there are two arguments, then we have to do the 'ls' in the given directory
		case 2:
			// We call the function that prints each element of the passed directory
			print_ls(argv[1]);
			break;
		// Else we finish the program as a failur
		default:
			for (i=1;i<argc;i++) {
				printf("%s:\n", argv[i]);
				if (print_ls(argv[i]) < 0)
					return -1;
				if (i != argc-1) {
				    printf("\n");
				}
			}
			break;
	}
	return 0;
}


#include <stdio.h>    // Header file for printf
#include <fcntl.h>    // Header file for open, O_RDONLY
#include <unistd.h>   // Header file for read, close
#include <stdbool.h>  // Header file for the type bool
#include <err.h>      // Header file for err
#include <errno.h>    // Header file for errno

#define BUFSIZE 1


int print_wc(const char *filename) {
    int fd;
    // Opening the file and checking that there are no errors
    if ((fd = open(filename, O_RDONLY)) < 0) {
        close(fd);
        printf("cannot access '%s': there is no such file\n", filename);
        return -1;
    }

    // Iterators to count the number of lines, bytes and words, respectively
    int lines = 0;
    int bytes = 0;
    int words = 0;
    // This variable is useful to distinguish whether we are going through a word (for the word count)
    bool capturing = true;
    // Buffer where the current character of the file will be stored
    char buffer;
    // Loop that goes through all the characters of the file until EOF (end of file)
    while (read(fd, &buffer, BUFSIZE) > 0) {
        // A switch statement proves to be more useful for our current program
        // notice the lack of 'break' statements for the first two cases. This is
        // intentional since '\n', ' ' and '\t' share the functionality of enabling the word count
        switch (buffer) {
            case '\n':
                ++lines;
            case ' ':
            case '\t':
                capturing = true;
                break;
            default:
                if (capturing) {
                    ++words;
                    capturing = false;
                }
                break;
        }
        ++bytes;
    }
    // we do not need the file anymore, so we close it
    close(fd);

    // Finally, we print the result
    printf("%d %d %d %s\n", lines, words, bytes, filename);
    return 0;
}

int main(int argc, char *argv[]) {
     
    // Checking correct number of arguments (more than two does not affect the program)
    if (argc < 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }
    int i;
    for (i=1;i<argc;i++) {
        if (print_wc(argv[i]) < 0)
            return -1;
    }


    return 0;
}

//P1-SSOO-22/23

#include <stdbool.h>
#include <stdio.h>
#include <string.h>       // Header file for strncmp, strnlen
#include <stdlib.h>       // Header file for qsort, free
#include <fcntl.h>        // Header file for open, O_RDONLY
#include <unistd.h>       // Header file for write, close, lseek
#include <sys/param.h>    // Header file for MAX

// Since we need a given buffer-size, we considered 30 bytes to be an adequate value for the exercise
#define BUFSIZE 1
// To avoid using arbitrary values, we also made a macro with the name of the input file (which will not change)
#define ENV_FILE "env.txt"

// This struct will store the essential information necessary
typedef struct {
    int position; // The line number of the 'env' variable in env.txt
    char *string; // The string of characters of that given 'env' variable
    int length; // The length of the string
} EnvLine;

EnvLine get_entry_line(char const *entry_name, const char * file_name) {
    /*
    ** This function opens the file from the beginning and iterates through
    ** every line until the 'env' variable is found (or it is not)
    */

    // The initial values for our struct will be -1 (to express error),
    // NULL (there is no string to point at) and 0 (the lack of a string means no length)
    EnvLine env_line = {-1, NULL, 0};

    // We now open the file
    int fd = open(file_name, O_RDONLY);

    // Checking if it could be correctly open
    if (fd < 0) {
        env_line.string = "there were some problems opening the file";
        close(fd);
        return env_line;
    }

    bool found = false;
    bool capturing = true;
    int i = 0;
    int j = 0;
    char buffer;
    // This loop looks for the line where the 'env' variable we are looking for is located
    while (read(fd, &buffer, BUFSIZE) > 0) {
        // This counter is necessary to later assign the correct amount of bytes to the memory allocation of the line's string
        if (found) {
            // which will stop once the new line character is found, which also ends the loop
            if (buffer == '\n') {
                break;
            }
            ++j; // We decided not to count the '\n' because it could be the case that the end of the file did not finish with '\n', and thus we would have to add it after this loop.
                 // Thus, we just omitted it for all the cases to add it later instead, and hence we ensure that it works the same for all possible cases.
            continue;
        }

        // If we are here, it means that the string has not been found, and thus we check the next line
        if (buffer == '\n') {
            // We reached that state if the string equality was not satisfied
            capturing = true;
            ++i;
            j = 0;
            continue;
        }

        // This is mainly to avoid further operations in an iteration of a loop that will not be of any use for the solution
        if (!capturing)
            continue; // We do not need to update j because we will not use it until a potential line is found

        // However, if we are capturing, it means that the first character/s of a line are equal to the respective ones
        // of the 'env' variable that was passed as an argument
        if (entry_name[j] == buffer && buffer != '=') {
            ++j;
            continue;
        } else if (entry_name[j] == '\0' && buffer == '=') { // If this is true, we just found the variable we were looking for
            found = true;
        }
        // Else, if the character was not the same, we stop looking for te variable until next line
        capturing = false;
    }

    // If, by the end of the file, the variable was not found, we finish the function as an error (making use of the struct we created)
    if (!found) {
        env_line.string = "the variable could not be found";
        close(fd);
        return env_line;
    }

    // If the line was found, we already have the length of the string
    env_line.string= malloc(j+2); // We make space for the string and for an extra '\n' character and '\0' (because we only counted until the '\n' character, which was not included in the count)

    // We need to go back to the beginning of the line to store all its characters
    // Notice that the file descriptor is actually moved one position before the
    // initial character. Otherwise we would skip the first char of the line
    lseek(fd, -(j+2), SEEK_CUR);

    // Finally, we make a second loop in which we insert the given line into the array we just created
    int k = 0;
    while (read(fd, &buffer, BUFSIZE) > 0) {
        if (buffer == '\n') {
            break;
        } else {
            env_line.string[k] = buffer;
        }
        ++k;
    }
    // It can be the case that the 'env' variable was the last one, and thus no new line character was recorded.
    // Thus, we introduce '\0' after the loop
    env_line.string[j+1] = '\n';
    env_line.string[j+2] = '\0';

    // We are done collecting information of the 'env' variable's line.
    // Now we store it in our struct and finish the function.
    env_line.position = i;
    env_line.length = j+2;

    close(fd);
    return env_line;
}


int main(int argc, char *argv[])
{
    // This would mean that either the output file or an 'env' variable was not introduced
    if (argc < 3) {
        printf("Too few arguments\n");
        return -1;
    }

    // Here we define some constant variables for the program
    const char *out_file = argv[argc-1]; // Name of the output file
    const int env_num = argc-2; // Number of 'env' variables that (should) appear as arguments

    // We then open the file where we will write into
    int file = creat(out_file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    // Checking the file was properly open
    if (file < 0) {
        close(file);
        return -1;
    }

    // Here we initialize a struct we designed to store the essential information for the program (which is defined above)
    EnvLine entries[env_num];
    // Counter to express the number of variables passed as arguments that did not appear in the input file
    int failures = 0;

    for (int i=0;i<env_num;i++) {
        // To avoid modifying the argument string (since it is a pointer), we make it constant
        const char *current = argv[i+1];
        EnvLine env_line = get_entry_line(current, ENV_FILE);

        // Error checking that all variables are correct. If not the program ends
        if (env_line.position < 0) {
            // We use the string member of the struct to report the reason why the line could not be obtained
            printf("There was an error obtaining the line for %s: %s \n", current, env_line.string);
            ++failures;
        }
        // Introduce the line struct into the array
        entries[i] = env_line;
    }

    // Sorting the list to enter the entries in the correct order with a simple bubble sort
    int x, y;
    EnvLine temp;
    for(x = 0; x < env_num - 1; x++){
        for(y = 0; y < env_num - x - 1; y++){
            if(entries[y].position > entries[y + 1].position){
                temp = entries[y];
                entries[y] = entries[y + 1];
                entries[y + 1] = temp;
            }
        }
    }

    // If everything was correct up to this point, we create the file and insert the entries
    // Also, notice that it omits the wrong values (if any) and takes into account the program's name and the output file
    for (int i=2;i<argc-failures;i++) {
        // We just want to write it, we do not need to modify it (that is why is const)
        const EnvLine line = entries[i+failures-2];
        //printf("string: %s\n", line.string); // \n is not necessary since the string already has a '\n' character
        // Check the man page for fwrite for further information
        // We write the line, passing as arguments:
        // string, size of each element (just a byte since they are char), the length of the line, and the file to write into
        write(file, line.string, line.length);
        free(line.string);
    }
    // Cleaning up
    close(file);
    return 0;
}

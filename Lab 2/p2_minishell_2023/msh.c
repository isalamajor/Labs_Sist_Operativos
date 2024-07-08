// P2-SSOO-22/23

// MSH main file
// Write your msh source code here

// #include "parser.h"
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h> /* NULL */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#define MAX_COMMANDS 8
#define EXIT_MSG "****  Exiting MSH ****\n"
#define SHELL_MSG "MSH>>"

// files in case of redirection
static char filev[3][64];

void siginthandler(int param) {
  write(STDERR_FILENO, EXIT_MSG, strlen(EXIT_MSG));
  _exit(EXIT_SUCCESS);
}

/* Timer */
static pthread_t timer_thread;
static unsigned long mytime = 0;

void *timer_run() {
  while (1) {
    usleep(1000);
    mytime++;
  }
}

void exec_command(int command_num, char ***commandvv, int infd, int outfd, int errfd) {
  int current_command = command_num - 1;
  // We want all the errors to be redirected to the specified file descriptor, not just the first one or the last one as with input and output
  dup2(errfd, STDERR_FILENO);
  // It does not matter if we put it here since it is going to be overwritten in the child process (with the dup2)
  // and it is only going to be useful during the first function call (which will be the last command to execute)
  dup2(outfd, STDOUT_FILENO);
  if (current_command > 0) {
    int pid;
    int pipefd[2];
    if (pipe(pipefd) != 0)
      perror("Failure creating pipe");

    if ((pid = fork()) == 0) {
      // Child process
      dup2(pipefd[STDOUT_FILENO], STDOUT_FILENO);
      close(pipefd[STDIN_FILENO]);
      close(pipefd[STDOUT_FILENO]);
      // Since the output must only be redirected for the final command
      // (which is actually the first one being passed due to the recursive
      // nature of this function) we now switch from our custom fd to
      // STDOUT_FILENO
      exec_command(current_command, commandvv, infd, STDOUT_FILENO, errfd);
    } else {
      // Parent process
      dup2(pipefd[STDIN_FILENO], STDIN_FILENO);
      close(pipefd[STDIN_FILENO]);
      close(pipefd[STDOUT_FILENO]);
      wait(&pid);
    }
  } else {
    // This ensures that only the first command in commandvv gets its input
    // from the correct file descriptor (either stdin or the one specified by
    // '<')
    dup2(infd, STDIN_FILENO);
  }
  execvp(commandvv[current_command][0], commandvv[current_command]);

  // This part should not be reached
  // This will print "'<command> <arg1> ...<argN>' is not a valid command"
  fprintf(stderr, "'");
  for (int i = 0; commandvv[current_command][i] != NULL; i++) {
    if (i != 0) fprintf(stderr, " "); // Just for better formatting of the resulting string (no trailing spaces)
    fprintf(stderr, "%s", commandvv[current_command][i]);
  }
  fprintf(stderr, "' is not a valid command\n");
}

void mycalc(char **command, int *acc) {
  char operation[4];
  int operand1, operand2;
  int res;
  int length = 0;
  for (int i = 0; command[i] != NULL; i++) {
    ++length;
  }

  if (length == 4 && // We first check that there are the adequate amount of parameters
      sscanf(command[1], "%d", &operand1) == 1 &&
      sscanf(command[3], "%d", &operand2) == 1 &&
      sscanf(command[2], "%[^3]s", operation) == 1) { // If the operation's length is greater than 3, this condition won't be satisfied
    if (!strcmp(operation, "add")) {
    // Sum operation
      res = operand1 + operand2;
      *acc += res; // we pass the variable as a pointer and update its value
      fprintf(stderr, "[OK] %d + %d = %d; Acc %d\n", operand1, operand2, res,
              *acc);
      return;
    } else if (!strcmp(operation, "mul")) {
    // Multiplication operation
      res = operand1 * operand2;
      fprintf(stderr, "[OK] %d * %d = %d\n", operand1, operand2, res);
      return;
    } else if (!strcmp(operation, "div")) {
    // Division operation
      if (!operand2) { // For the case 'mycalc X div 0'
        puts("[ERROR] Division by zero is not allowed");
        return;
      }
      res = operand1 / operand2;
      int remainder = operand1 % operand2;
      fprintf(stderr, "[OK] %d / %d = %d; Remainder %d\n", operand1, operand2, res, remainder);
      return;
    }
  }
  // If we reach this, something was wrong with the structure of the command
  puts("[ERROR] The structure of the command is mycalc <operand_1> "
       "<add/mul/div> <operand_2>");
}

void mytime_cmd() {
  int timer = mytime / 1000; // To get the time in seconds
  int hours = timer / 3600;
  int mins = timer / 60;
  int secs = timer % 60;
  // We properly print the result in the terminal, with the trailing zeroes if necessary
  fprintf(stderr, "%02d:%02d:%02d\n", hours, mins, secs);
}

/**
 * Main sheell  Loop
 */
int main(int argc, char *argv[]) {
  /**** Do not delete this code.****/
  int end = 0;
  int executed_cmd_lines = -1;
  char *cmd_line = NULL;
  char *cmd_lines[10];

  if (!isatty(STDIN_FILENO)) {
    cmd_line = (char *)malloc(100);
    while (scanf(" %[^\n]", cmd_line) != EOF) {
      if (strlen(cmd_line) <= 0)
        return 0;
      cmd_lines[end] = (char *)malloc(strlen(cmd_line) + 1);
      strcpy(cmd_lines[end], cmd_line);
      end++;
      fflush(stdin);
      fflush(stdout);
    }
  }

  pthread_create(&timer_thread, NULL, timer_run, NULL);

  /*********************************/

  char ***argvv = NULL;
  int num_commands;
  int acc = 0; // Variable for mycalc

  signal(SIGINT, siginthandler);

  while (1) {
    int status = 0;
    int command_counter = 0;
    int in_background = 0;

    // Prompt
    write(STDERR_FILENO, SHELL_MSG, strlen(SHELL_MSG));

    // Get command
    /********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN
     * NORMAL/CORRECTION MODE***************/
    executed_cmd_lines++;
    if (end != 0 && executed_cmd_lines < end) {
      command_counter = read_command_correction(&argvv, filev, &in_background,
                                                cmd_lines[executed_cmd_lines]);
    } else if (end != 0 && executed_cmd_lines == end) {
      return 0;
    } else {
      command_counter =
          read_command(&argvv, filev, &in_background); // NORMAL MODE
    }
    /************************************************************************************************/

    /************************ STUDENTS CODE ********************************/
    if (command_counter <= 0)
      continue;

    if (command_counter > MAX_COMMANDS) {
      printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
      continue;
    }
    // Checking conditions for internal command 'mycalc'
    if (!strcmp(argvv[0][0], "mycalc")) {
      if (command_counter != 1 || in_background || strcmp(filev[0], "0") ||
          strcmp(filev[1], "0") || strcmp(filev[2], "0")) {
        puts("[ERROR] 'mycalc' does not accept command sequences, file "
             "redirections or background execution");
        continue;
      }

      // Execute function
      mycalc(argvv[0], &acc);
      continue;
    }

    // Checking conditions for internal command 'mytime', same as 'mycalc'
    if (!strcmp(argvv[0][0], "mytime")) {
      if (command_counter != 1 || in_background || strcmp(filev[0], "0") ||
          strcmp(filev[1], "0") || strcmp(filev[2], "0")) {
        puts("[ERROR] 'mytime' does not accept command sequences, file "
             "redirections or background execution");
        continue;
      }

      // In this case, 'mytime' needs no parameters, which means that, if the
      // array does not end (has the null pointer) in the second element, that's
      // because a parameter was added
      if (argvv[0][1] != NULL) {
        puts("[ERROR] 'mytime' requires no parameters");
        continue;
      }

      // Execute function
      mytime_cmd();
      continue;
    }

    // Define the file descriptor used for the input
    int in;
    if (strcmp(filev[0], "0")) { // If the string is not '0', there is some file redirection
      if ((in = open(filev[0], O_RDONLY)) < 0) { // We only need to read the file for the input
        fprintf(stderr, "Some problem occurred opening '%s'\n", filev[0]);
        continue;
      }
    } else {
      in = STDIN_FILENO; // Otherwise use the default file descriptor
    }

    // Define the file descriptor used for the output. Similar to the input
    int out;
    if (strcmp(filev[1], "0")) {
      if ((out = open(filev[1], O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0) {
        fprintf(stderr, "Some problem occurred opening '%s'\n", filev[1]);
        continue;
      }
    } else {
      out = STDOUT_FILENO;
    }

    // Define the file descriptor used for the errors. Similar to input and output
    int err;
    if (strcmp(filev[2], "0")) {
      if ((err = open(filev[2], O_CREAT | O_APPEND | O_WRONLY, 0666)) < 0) {
        fprintf(stderr, "Some problem occurred opening '%s'\n", filev[2]);
        continue;
      }
    } else {
      err = STDERR_FILENO;
    }

    int cpid;
    // Fork necessary to make programs work in background
    if ((cpid = fork()) == 0) { // We exectute the commands in this child
      int pid;
      if ((pid = fork()) != 0) {
        // If process is the parent, we just wait until the child finish
        // execution
        wait(&pid);
        exit(0);
      } else {
        // If process is the child, we execute the command(s) with our custom function
        exec_command(command_counter, argvv, in, out, err);
      }
    } else {
      // If the command sequence should run in background, we wouldn't wait for the
      // command execution to finish, but continue with the loop to accept new input
      if (!in_background) {
        waitpid(cpid, &cpid, 0);
      }
    }

    // If the file descriptors are not the default ones (0, 1 and 2), then close them
    if (in != STDIN_FILENO)
      close(in);
    if (out != STDOUT_FILENO)
      close(out);
    if (err != STDERR_FILENO)
      close(err);
  }

  return 0;
}

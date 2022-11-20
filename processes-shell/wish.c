#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct Command {
  int numArgs;
  char **args;
  int hasRedirection;
  char *output;
} Command;

void printError() {
  char msg[30] = "An error has occurred\n";
  write(STDERR_FILENO, msg, strlen(msg));
}

int hasTrailingChar(char *str, char c) {
  char *end = str + strlen(str) - 1;
  return *end == c;
}

char *trim(char *str) {
  char *end;

  // Trim leading space
  while (isspace((unsigned char)*str)) {
    str++;
  }

  if (*str == '\0')
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) {
    end--;
  }

  end[1] = '\0';
  return str;
}

char *concat(char *a, char *b) {
  char *result = malloc(strlen(a) + strlen(b) + 1);
  strcpy(stpcpy(result, a), b);
  return result;
}

int split(char *str, char *delim, char *tokens[]) {
  char *token;
  int i = 0;

  while ((token = strsep(&str, delim)) != NULL) {
    if (strlen(token) == 0) {
      continue;
    }
    tokens[i++] = token;
  }
  tokens[i] = '\0';
  return i;
}

int hasFile(char *file) { return access(file, X_OK) == 0; }

void execute(char *filepath, char *args[]) {
  int pid = fork();

  if (pid == -1) {
    return;
  }

  if (pid == 0) {
    execv(filepath, args);
  } else {
    // parent process
    waitpid(pid, NULL, 0);
  }
}

char *readInput(FILE *fp) {
  char *line = NULL;
  size_t len = 0;
  ssize_t charsRead = getline(&line, &len, fp);
  return charsRead == -1 ? NULL : line;
}

int splitCommands(char *line, char **commands) {
  char *lineTrimmed = trim(line);
  return split(lineTrimmed, "&", commands);
}

Command parseCommand(char *line) {
  Command command;
  command.hasRedirection = 0;
  command.output = NULL;
  char *lineTrimmed = trim(line);

  char *found = strstr(lineTrimmed, ">");
  if (found != NULL) {
    command.hasRedirection = 1;
    *found = '\0';
    char *output = trim(found + 1);

    int outputLength = strlen(output);
    if (outputLength > 0 && strstr(output, " ") == NULL) {
      command.output = malloc(outputLength + 1);
      strcpy(command.output, output);
    }
  }

  char *argsLine = lineTrimmed;
  char **args = malloc(100 * sizeof(*args));
  int numArgs = split(argsLine, " ", args);

  command.numArgs = numArgs;
  command.args = args;
  return command;
}

void handleCommand(Command command, char *paths[], int *pathSize) {
  if (command.numArgs == 0) {
    if (command.hasRedirection) {
      printError();
    }
    return;
  }

  int fileDescriptor = -1;

  if (command.hasRedirection) {
    if (command.output == NULL) {
      printError();
      return;
    }

    fileDescriptor = dup(STDOUT_FILENO);
    close(STDOUT_FILENO);
    open(command.output, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  }

  if (strcmp(command.args[0], "exit") == 0) {
    if (command.numArgs != 1) {
      printError();
    }
    exit(0);
  }

  if (strcmp(command.args[0], "cd") == 0) {
    if (command.numArgs != 2) {
      printError();
    }

    chdir(command.args[1]);
    return;
  } else if (strcmp(command.args[0], "path") == 0) {
    for (int i = 1; i < command.numArgs; i++) {
      int length = strlen(command.args[i]);
      paths[i - 1] = malloc((length + 1) * sizeof(char));
      strcpy(paths[i - 1], command.args[i]);
    }
    *pathSize = command.numArgs - 1;
    return;
  }

  int isSuccessful = 0;

  for (int i = 0; i < *pathSize; i++) {
    char *filepath = concat(concat(paths[i], "/"), command.args[0]);

    if (hasFile(filepath)) {
      execute(filepath, command.args);
      isSuccessful = 1;
      break;
    }
  }

  if (!isSuccessful) {
    printError();
  }

  if (fileDescriptor != -1) {
    dup2(fileDescriptor, STDOUT_FILENO);
  }
}

int main(int argc, char *argv[]) {
  char *paths[100];
  paths[0] = "/bin";
  int pathSize = 1;
  FILE *fp;
  int isInteractive;

  if (argc == 1) {
    // interactie mode
    fp = stdin;
    isInteractive = 1;
  } else if (argc == 2) {
    // batch mode
    fp = fopen(argv[1], "r");

    if (fp == NULL) {
      printError();
      exit(1);
    }

    isInteractive = 0;
  } else {
    printError();
    exit(1);
  }

  while (1) {
    if (isInteractive) {
      printf("wish> ");
    }

    char *line = readInput(fp);

    if (line == NULL) {
      exit(0);
    }

    char *lineCopy = line;
    char **commandStrs = malloc(100 * sizeof(*commandStrs));
    int numCommands = splitCommands(lineCopy, commandStrs);

    if (numCommands == -1) {
      printError();
      continue;
    }

    Command commands[numCommands];

    for (int i = 0; i < numCommands; i++) {
      commands[i] = parseCommand(commandStrs[i]);
    }

    if (numCommands == 1) {
      handleCommand(commands[0], paths, &pathSize);
    } else {
      for (int i = 0; i < numCommands; i++) {
        int pid = fork();

        if (pid == -1) {
          continue;
        }

        if (pid == 0) {
          handleCommand(commands[i], paths, &pathSize);
          exit(0);
        }
      }

      while (numCommands > 0) {
        wait(NULL);
        --numCommands;
      }
    }

    free(line);

    for (int i = 0; i < numCommands; i++) {
      free(commands[i].output);
      free(commands[i].args);
    }
  }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void grep(FILE *fp, char *key) {
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, fp)) != -1) {
    if (strstr(line, key)) {
      printf("%s", line);
    }
  }

  free(line);
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("wgrep: searchterm [file ...]\n");
    exit(EXIT_FAILURE);
  }

  char *key = argv[1];

  if (argc == 2) {
    grep(stdin, key);
  }

  for (int i = 2; i < argc; i++) {
    char *filename = argv[i];
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
      printf("wgrep: cannot open file\n");
      exit(EXIT_FAILURE);
    }

    grep(fp, key);
    fclose(fp);
  }

  exit(EXIT_SUCCESS);
}

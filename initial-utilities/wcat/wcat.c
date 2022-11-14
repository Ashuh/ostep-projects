#include <stdio.h>
#include <stdlib.h>

#define SIZE 100

int main(int argc, char *argv[]) {
  if (argc == 1) {
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    char *filename = argv[i];
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
      printf("wcat: cannot open file\n");
      exit(1);
    }

    char buffer[SIZE];
    while (fgets(buffer, SIZE, fp) != NULL) {
      printf("%s", buffer);
    }

    fclose(fp);
  }
}

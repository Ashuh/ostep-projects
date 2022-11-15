#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write(int32_t count, int8_t c) {
  fwrite(&count, sizeof(count), 1, stdout);
  fwrite(&c, sizeof(c), 1, stdout);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wzip: file1 [file2 ...]\n");
    exit(1);
  }

  int8_t prevC = EOF;
  int32_t streak = 0;

  for (int i = 1; i < argc; i++) {
    char *filename = argv[i];
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
      printf("wcat: cannot open file\n");
      exit(1);
    }

    int8_t c;
    while ((c = fgetc(fp)) != EOF) {
      if (prevC == EOF || c == prevC) {
        streak++;
      } else {
        write(streak, prevC);
        streak = 1;
      }

      prevC = c;
    }

    fclose(fp);
  }

  write(streak, prevC);
  exit(0);
}

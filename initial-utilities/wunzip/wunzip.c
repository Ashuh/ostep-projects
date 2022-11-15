#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wunzip: file1 [file2 ...]\n");
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    char *filename = argv[i];
    FILE* fp = fopen(filename, "r");

    if (fp == NULL) {
      printf("wcat: cannot open file\n");
      exit(1);
    }

    int8_t* buffer = malloc(5);

    while ((fread(buffer, 5, 1, fp)) == 1) {
        int32_t count = *((int32_t*) buffer);
        int8_t c = *(buffer + 4);

        for (int i = 0; i < count; i++)
        {
            printf("%c", c);
        }
    }

    free(buffer);
    fclose(fp);
  }

  exit(0);
}

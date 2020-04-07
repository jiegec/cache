#include "cache.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: cache [trace_file]\n");
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("unable to open trace file");
    return 1;
  }

  std::vector<Trace> traces = readTrace(fp);

  fclose(fp);
  return 0;
}
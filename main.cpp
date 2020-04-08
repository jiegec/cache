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
  char buffer[1024];

  for (size_t block_size : {8, 32, 64}) {
    for (Algorithm algo :
         {Algorithm::LRU, Algorithm::PLRU, Algorithm::Random}) {
      for (WriteHitPolicy hit :
           {WriteHitPolicy::Writeback, WriteHitPolicy::Writethrough}) {
        for (WriteMissPolicy miss : {WriteMissPolicy::WriteAllocate,
                                     WriteMissPolicy::WriteNonAllocate}) {
          for (size_t assoc :
               {cache_size / block_size, (size_t)1, (size_t)4, (size_t)8}) {
            // no meaning when assoc=1 and algo=LRU/PLRU
            if (assoc == 1 && algo != Algorithm::Random) {
              continue;
            }

            Cache cache(block_size, assoc, algo, hit, miss);

            sprintf(buffer, "%s_%zu_%d_%d_%d_%zu.trace", argv[1], block_size,
                    algo, hit, miss, assoc);
            printf("Writing to %s\n", buffer);
            FILE *trace = fopen(buffer, "w");
            assert(trace);

            sprintf(buffer, "%s_%zu_%d_%d_%d_%zu.info", argv[1], block_size,
                    algo, hit, miss, assoc);
            FILE *info = fopen(buffer, "w");
            assert(info);
            cache.run(traces, trace, info);
            fclose(trace);
            fclose(info);
          }
        }
      }
    }
  }

  fclose(fp);
  return 0;
}
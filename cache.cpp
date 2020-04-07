#include "cache.h"

std::vector<Trace> readTrace(FILE *fp) {
  char buffer[1024];
  std::vector<Trace> res;
  size_t num_r = 0, num_w = 0, num_u = 0;
  char temp;
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    size_t len = strlen(buffer);
    if (len > 0) {
      Trace trace;
      if (buffer[0] == 'r' || buffer[0] == 'l') {
        trace.kind = Kind::Read;
        sscanf(buffer, "%c%llx", &temp, &trace.addr);
        num_r++;
      } else if (buffer[0] == 'w' || buffer[0] == 's') {
        trace.kind = Kind::Write;
        sscanf(buffer, "%c%llx", &temp, &trace.addr);
        num_w++;
      } else {
        trace.kind = Kind::Unspecified;
        sscanf(buffer, "%llx", &trace.addr);
        num_u++;
      }
      res.push_back(trace);
    }
  }
  printf("Read %ld entries: %ld reads, %ld writes, %ld unspecified\n", res.size(),
         num_r, num_w, num_u);
  return res;
}
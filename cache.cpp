#include "cache.h"

size_t log2(size_t num) {
  assert(num != 0);
  size_t res = 0;
  while (num > 1) {
    assert((num & 1) == 0);
    num >>= 1;
    res++;
  }
  return res;
}

Cache::Cache(size_t block_size, size_t assoc, Algorithm algo,
             WriteHitPolicy hit_policy, WriteMissPolicy miss_policy) {
  this->block_size = block_size;
  this->assoc = assoc;
  this->algo = algo;
  this->hit_policy = hit_policy;
  this->miss_policy = miss_policy;

  this->num_set = cache_size / block_size / assoc;
  this->block_size_lg2 = log2(this->block_size);
  this->assoc_lg2 = log2(this->assoc);
  this->num_set_lg2 = log2(this->num_set);
}

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
  printf("Read %ld entries: %ld reads, %ld writes, %ld unspecified\n",
         res.size(), num_r, num_w, num_u);
  return res;
}

void Cache::run(const std::vector<Trace> &traces, FILE *trace, FILE *info) {}
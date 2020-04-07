#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>
#include <stdlib.h>
#include <vector>

enum Kind { Read, Write, Unspecified };

struct Trace {
  Kind kind;
  uint64_t addr;
};

class Cache {

};

std::vector<Trace> readTrace(FILE *fp);

#endif
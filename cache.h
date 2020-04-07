#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>
#include <stdlib.h>
#include <vector>

enum Kind { Read, Write, Unspecified };
enum Algorithm { LRU, Random, PLRU };
enum WriteHitPolicy { Writethrough, Writeback };
enum WriteMissPolicy { WriteAllocate, WriteNonAllocate };

struct Trace {
  Kind kind;
  uint64_t addr;
};

// 128KB
const size_t cache_size = 128 * 1024;

class Cache {
  // these are not stored in hardware
  // cache parameters
  size_t block_size;
  size_t assoc;
  // cache constants
  size_t num_set; // cache_size / block_size / assoc
  size_t block_size_lg2; // log2(block_size)
  size_t assoc_lg2; // log2(assoc)
  size_t num_set_lg2; // log2(num_set)
  // algorithm and policy
  Algorithm algo;
  WriteHitPolicy hit_policy;
  WriteMissPolicy miss_policy;

  // these are stored in hardware

public:
  Cache(size_t block_size, size_t assoc, Algorithm algo,
        WriteHitPolicy hit_policy, WriteMissPolicy miss_policy);
};

std::vector<Trace> readTrace(FILE *fp);

#endif
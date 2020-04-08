#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>
#include <stdlib.h>
#include <vector>

#include "bitvec.h"

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

struct CacheLine {
  // metadata:
  // | dirty | valid | tag |
  // for 8-way 8B-block, low 52bits are used
  // for all-assoc 8B-block, low 63bits are used
  BitVec metadata;
  uint64_t get_dirty() { return metadata.get(0, 0); }
  void set_dirty(bool dirty) { metadata.set(0, 0, dirty); }
  uint64_t get_valid() { return metadata.get(1, 1); }
  void set_valid(bool valid) { metadata.set(1, 1, valid); }
  uint64_t get_tag() { return metadata.get(2, metadata.width() - 1); }
  void set_tag(uint64_t tag) { metadata.set(2, metadata.width() - 1, tag); }

  CacheLine(size_t width) : metadata(width) {}
};

class Cache {
private:
  // these are not stored in hardware
  // cache parameters
  size_t block_size;
  size_t assoc;
  // cache constants
  size_t num_set;        // cache_size / block_size / assoc
  size_t block_size_lg2; // log2(block_size)
  size_t assoc_lg2;      // log2(assoc)
  size_t num_set_lg2;    // log2(num_set)
  size_t tag_width;      // 64 - num_set_lg2 - block_size_lg2
  // algorithm and policy
  Algorithm algo;
  WriteHitPolicy hit_policy;
  WriteMissPolicy miss_policy;
  // statistics and output
  size_t num_hit;
  size_t num_miss;
  FILE *trace;
  FILE *info;

  // these are stored in hardware
  // num_set * assoc elements
  std::vector<CacheLine> all_cachelines;

  void read(const Trace &access);
  void write(const Trace &access);

public:
  Cache(size_t block_size, size_t assoc, Algorithm algo,
        WriteHitPolicy hit_policy, WriteMissPolicy miss_policy);
  ~Cache();

  void run(const std::vector<Trace> &traces, FILE *trace, FILE *info);
};

std::vector<Trace> readTrace(FILE *fp);

#endif
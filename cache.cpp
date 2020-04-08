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
  this->tag_width = 64 - this->num_set_lg2 - this->block_size_lg2;

  this->all_cachelines.resize(this->num_set * this->assoc,
                              CacheLine(tag_width + 2));
  this->num_hit = 0;
  this->num_miss = 0;
}

Cache::~Cache() {}

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

void Cache::run(const std::vector<Trace> &traces, FILE *trace, FILE *info) {
  this->trace = trace;
  this->info = info;
  fprintf(info, "Block size: %ld Bytes\n", this->block_size);
  fprintf(info, "Assoc: %ld-way\n", this->assoc);
  fprintf(info, "Number of cacheline: %ld\n", this->num_set * this->assoc);
  fprintf(info, "Tag width: %ld\n", this->tag_width);
  fprintf(info, "Index width: %ld\n", this->num_set_lg2);
  fprintf(info, "Offset width: %ld\n", this->block_size_lg2);
  // for each cacheline: tag+dirty+valid
  fprintf(info, "Metadata usage in cacheline: %ld Bytes\n",
          this->num_set * this->assoc * (this->tag_width + 2));
  for (const Trace &access : traces) {
    if (access.kind == Kind::Read) {
      read(access);
    } else if (access.kind == Kind::Write) {
      write(access);
    } else {
      // unsupported
      assert(false);
    }
  }
  fprintf(info, "Stats: %ld hit, %ld miss, %.2f%% miss rate", this->num_hit, this->num_miss, 100.0 * (this->num_miss) / (this->num_hit + this->num_miss));
}

void Cache::read(const Trace &access) {
  uint64_t tag = (access.addr >> num_set_lg2) >> block_size_lg2;
  uint64_t index = (access.addr >> block_size_lg2) & (num_set - 1);
  CacheLine *cacheline = &all_cachelines[index * assoc];

  // find matching cacheline
  for (size_t i = 0; i < assoc; i++) {
    if (cacheline[i].get_valid() && cacheline[i].get_tag() == tag) {
      // hit
      fprintf(trace, "Hit\n");
      num_hit++;

      // update state
      return;
    }
  }

  // miss
  fprintf(trace, "Miss\n");
  num_miss++;

  size_t victim = 0;
  if (algo == Algorithm::LRU) {
  } else if (algo == Algorithm::Random) {
    bool evict = true;
    for (size_t i = 0; i < assoc; i++) {
      if (!cacheline[i].get_valid()) {
        // found empty
        evict = false;
        victim = i;
        break;
      }
    }
    if (evict) {
      victim = rand() % assoc;
    }
  } else if (algo == Algorithm::PLRU) {
  }

  cacheline[victim].set_valid(true);
  cacheline[victim].set_dirty(false);
  cacheline[victim].set_tag(tag);
}

void Cache::write(const Trace &access) {}
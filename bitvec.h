#ifndef _BITVEC_H_
#define _BITVEC_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// [to:from]
inline uint64_t extract(uint64_t value, uint64_t from, uint64_t to) {
  if (to == 63) {
    return value >> from;
  } else {
    return (value >> from) & (((uint64_t)1 << (to - from + 1)) - 1);
  }
}

class BitVec {
private:
  size_t bits;
  uint64_t *data;

public:
  BitVec(size_t bits) {
    size_t n = (bits + 63) / 64;
    data = new uint64_t[n];
    this->bits = bits;
    memset(data, 0, sizeof(uint64_t) * n);
  }
  ~BitVec() { delete data; }

  // like verilog [to:from]
  uint64_t get(size_t from, size_t to) {
    assert(from <= to);
    assert(from + 64 > to);
    assert(to < bits);
    if ((from / 64) == (to / 64)) {
      // simple case
      return extract(data[from / 64], from % 64, to % 64);
    } else {
      // two parts
      uint64_t low = extract(data[from / 64], from % 64, 63);
      uint64_t high = extract(data[to / 64], 0, to % 64);
      return low | (high << (64 - from));
    }
  }

  void set(size_t from, size_t to, uint64_t value) {
    assert(from <= to);
    assert(from + 64 > to);
    assert(to < bits);
    assert(value == extract(value, 0, to - from));
    if ((from / 64) == (to / 64)) {
      // simple case
      data[from / 64] ^= extract(data[from / 64], from % 64, to % 64)
                         << (from % 64);
      data[from / 64] |= value << (from % 64);
    } else {
      // two parts
      uint64_t low = extract(value, 0, 63 - (from % 64));
      uint64_t high = extract(value, 64 - (from % 64), to - from);
      data[from / 64] = extract(data[from / 64], 0, (from % 64 - 1)) | (low << (from % 64));
      data[to / 64] = (extract(data[to / 64], to % 64 + 1, 63) << (to % 64 + 1)) | high;
    }
  }
};

#endif
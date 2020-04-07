#include "bitvec.h"

#include <algorithm>
#include <stdlib.h>
#include <vector>

int main() {
  for (size_t bits :
       {1, 2, 63, 64, 65, 66, 100, 126, 127, 128, 129, 64 * 64, 64 * 100}) {
    BitVec bitvec(bits);
    std::vector<bool> ref;
    ref.resize(bits, false);
    for (int i = 0; i < 100000; i++) {
      size_t action = rand() % 3;
      size_t from = rand() % bits;
      size_t to = std::min(from + rand() % 64, bits - 1);
      if (action == 0) {
        // set
        uint64_t num = (uint64_t)rand();
        num = (num << 16) + (uint64_t)rand();
        num = (num << 16) + (uint64_t)rand();
        num = (num << 16) + (uint64_t)rand();
        if (to - from != 63) {
          num = num & (((uint64_t)1 << (to - from + 1)) - 1);
        }
        bitvec.set(from, to, num);
        for (size_t i = from; i <= to; i++) {
          ref[i] = (num & ((uint64_t)1 << (i - from))) != 0;
        }
        printf("Set [%zu:%zu] = %llx\n", to, from, num);
      } else {
        // get
        uint64_t num = 0;
        for (size_t i = from; i <= to; i++) {
          if (ref[i]) {
            num |= (uint64_t)1 << (i - from);
          }
        }
        printf("Get [%zu:%zu] = %llx %llx\n", to, from, num,
               bitvec.get(from, to));
        assert(bitvec.get(from, to) == num);
      }
    }
  }
  return 0;
}
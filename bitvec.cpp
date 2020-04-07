#include "bitvec.h"

#include <algorithm>
#include <stdlib.h>
#include <vector>

int main() {
  srand(time(NULL));
  for (size_t bits : {1, 2, 63, 64, 65, 100, 64 * 100}) {
    BitVec bitvec(bits);
    std::vector<bool> ref;
    ref.resize(bits, false);
    for (int i = 0; i < 1000; i++) {
      size_t action = rand() % 2;
      size_t from = rand() % bits;
      size_t to = std::min(from + rand() % 64, bits - 1);
      if (action == 0) {
        // set
        uint64_t num = (uint64_t)rand();
        num = (num << 16) + (uint64_t)rand();
        num = (num << 16) + (uint64_t)rand();
        num = (num << 16) + (uint64_t)rand();
        if (to - from != 63) {
          num = num & ((1 << (to - from + 1)) - 1);
        }
        bitvec.set(from, to, num);
        for (size_t i = from; i <= to; i++) {
          ref[i] = num & (1 << (i - from));
        }
      } else if (action == 1) {
        // get
        size_t num = 0;
        for (size_t i = from; i <= to; i++) {
          if (ref[i]) {
            num |= 1 << (i - from);
          }
        }
        assert(bitvec.get(from, to) == num);
      }
    }
  }
  return 0;
}
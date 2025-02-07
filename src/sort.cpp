#include "sort.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <queue>
#include <unistd.h>
#include <vector>

namespace extsort {
  void external_sort(int fd_input, uint64_t size, int fd_output,uint64_t mem_size) {
    // Create size / mem_size chuncks of temporary files
    int n_chunks = (size + mem_size - 1) / mem_size;
    std::vector<std::FILE *> chunks(n_chunks);
    for (int i = 0; i < n_chunks; i++) {
      chunks[i] = std::tmpfile();
    }
    
    std::vector<uint32_t> buffer(mem_size);
    for (int i = 0; i < n_chunks; i++) {
      auto n_read = read(fd_input, buffer.data(), mem_size);

      // Convert to uint64_t and sort
      auto nums = reinterpret_cast<uint64_t *>(buffer.data());
      int n = (n_read + sizeof(uint64_t) - 1) / sizeof(uint64_t);
      std::cout << "number of bytes read: " << n_read << ", number of elements to be sorted: " << n << "\n";
      std::sort(nums, nums + n);

      // Write sorted chunk to disk
      auto n_written = fwrite(nums, sizeof(uint64_t), n, chunks[i]);
      std::rewind(chunks[i]);
      std::cout << "number of bytes written: " << (n_written * sizeof(uint64_t)) << "\n";
    }

    // Merge step
    
  }
}

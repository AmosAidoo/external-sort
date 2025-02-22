#ifndef SORT_H
#define SORT_H

#include <cstddef>
#include <cstdint>
namespace extsort {
  struct buffer_manager {
    size_t index;
    size_t n_items;
  };
  void external_sort(int fd_input, uint64_t size, int fd_output, uint64_t mem_size);
}

#endif // SORT_H
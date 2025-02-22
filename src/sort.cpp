#include "sort.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
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
      if (!chunks[i]) {
        perror("tmpfile failed");
        return;
      }
    }


    std::vector<uint64_t> buffer(mem_size / sizeof(uint64_t));
    
    for (int i = 0; i < n_chunks; i++) {
      auto n_read = read(fd_input, buffer.data(), buffer.size() * sizeof(uint64_t));

      // Convert to uint64_t and sort
      int n = n_read / sizeof(uint64_t);
      std::sort(buffer.begin(), buffer.begin() + n);

      // Write sorted chunk to disk
      auto n_written = fwrite(buffer.data(), sizeof(uint64_t), n, chunks[i]);
      std::rewind(chunks[i]);
    }

    // Check if there is more left
    auto n_read = read(fd_input, buffer.data(), buffer.size() * sizeof(uint64_t));
    if (n_read > 0) {
      chunks.push_back(std::tmpfile());
      int n = n_read / sizeof(uint64_t);
      std::sort(buffer.begin(), buffer.begin() + n);

      auto n_written = fwrite(buffer.data(), sizeof(uint64_t), n, chunks.back());
      std::rewind(chunks.back());
      n_chunks++;
    }

    // Merge step
    size_t buffer_size_per_chunk = mem_size / (n_chunks + 1);
    size_t n_items = buffer_size_per_chunk / sizeof(uint64_t);

    std::vector<std::vector<uint64_t>> input_buffers(n_chunks, std::vector<uint64_t>(n_items));
    std::vector<uint64_t> output_buffer(n_items);
    size_t output_buffer_index = 0;

    // Min-heap priority queue
    using Element = std::pair<uint64_t, size_t>; // (value, chunk_index)
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> queue;

    // Manage the individual indexes of the input buffers
    std::vector<buffer_manager> input_buffer_mgrs(n_chunks, {0, 0});

    for (int i = 0; i < n_chunks; i++) {
      size_t n_read =
          fread(input_buffers[i].data(), sizeof(uint64_t), n_items, chunks[i]);
      input_buffer_mgrs[i] = {0, n_read};

      if (n_read > 0) {
        queue.push({input_buffers[i][0], i});
        input_buffer_mgrs[i].index++;
      }
    }

    while (!queue.empty()) {
      auto [value, chunk_idx] = queue.top();
      queue.pop();
      output_buffer[output_buffer_index++] = value;

      // If output buffer is full, write to disk
      if (output_buffer_index == n_items) {
        size_t n_written = write(fd_output, output_buffer.data(),
                                 output_buffer_index * sizeof(uint64_t));
        output_buffer_index = 0;
      }

      // Refill queue from chunk buffer
      if (input_buffer_mgrs[chunk_idx].index <
          input_buffer_mgrs[chunk_idx].n_items) {
        queue.push(
            {input_buffers[chunk_idx][input_buffer_mgrs[chunk_idx].index],
             chunk_idx});
        input_buffer_mgrs[chunk_idx].index++;
      } else {
        // Refill buffer from file if empty
        size_t n_read = fread(input_buffers[chunk_idx].data(), sizeof(uint64_t),
                              n_items, chunks[chunk_idx]);
        if (n_read > 0) {
          input_buffer_mgrs[chunk_idx] = {0, n_read};
          queue.push({input_buffers[chunk_idx][0], chunk_idx});
          input_buffer_mgrs[chunk_idx].index++;
        }
      }
    }

    // Flush remaining elements
    if (output_buffer_index > 0) {
      size_t n_written = write(fd_output, output_buffer.data(),
                               output_buffer_index * sizeof(uint64_t));
    }

    for (auto chunk : chunks) {
      fclose(chunk);
    }
  }
}

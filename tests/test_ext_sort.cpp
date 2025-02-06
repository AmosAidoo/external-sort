#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <sys/fcntl.h>
#include <unistd.h>

// Contains out external sort function
// that we want to test
#include "sort.h"

class ExternalSortTest : public testing::Test {
protected:
  std::string input_file;
  std::string output_file;

  void SetUp() override {
    input_file = testing::TempDir() + "input_test.bin";
    output_file = testing::TempDir() + "output_test.bin";
  }

  void TearDown() override {
    remove(input_file.c_str());
    remove(output_file.c_str());
  }

  void GenerateNumbers(uint64_t n) {
    std::ofstream file(input_file, std::ios::binary | std::ios::trunc);

    // Using the same seed from the generator script
    // provided in the course materials
    // https://db.in.tum.de/teaching/ss15/moderndbs/resources/1/gen.tar.gz?lang=en
    std::mt19937_64 rng{88172645463325252ull};
    try {
      std::filesystem::resize_file(input_file, n * sizeof(uint64_t));
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "Warning: Could not preallocate file space: " << e.what()
                << "\n";
    }
    for (uint64_t i = 0; i < n; ++i) {
      uint64_t x = rng();
      file.write(reinterpret_cast<const char *>(&x), sizeof(x));
      if (!file) {
        std::cerr << "Error writing to file: " << input_file << "\n";
        return;
      }
    }
  }

  void TestFileIsSorted(int fd, uint64_t size, uint64_t mem_size) {
    // 0 or 1 element is already sorted
    if (size <= sizeof(uint64_t)) {
      SUCCEED();
      return;
    }

    std::vector<uint8_t> buffer(mem_size);

    ssize_t num_read;
    uint64_t prev_last_value = 0;
    bool prev_last_value_set = false;
    while (true) {
      num_read = read(fd, buffer.data(), mem_size);
      ASSERT_NE(num_read, -1);
      
      if (num_read == 0)
        break;

      auto arr = reinterpret_cast<uint64_t *>(buffer.data());
      if (prev_last_value_set) {
        ASSERT_GE(arr[0], prev_last_value);
      }
      for (int i = 1; i < num_read / sizeof(uint64_t); i++) {
        std::cout << "Index " << i << ": " << arr[i]
                  << " (previous: " << arr[i - 1] << ")\n";
        ASSERT_GE(arr[i], arr[i - 1]);
      }
      prev_last_value = arr[num_read / sizeof(uint64_t) - 1];
      prev_last_value_set = true;
    }
  }
};

void printErrNo();

TEST_F(ExternalSortTest, Sort800BFileWith100BMem) {
  constexpr uint64_t mem_size = 100;
  GenerateNumbers(100);
  int fd = open(input_file.c_str(), O_RDONLY);
  if (fd == -1)
    printErrNo();
  ASSERT_NE(fd, -1);

  auto size = lseek(fd, 0, SEEK_END);
  ASSERT_EQ(size, 800);
  lseek(fd, 0, SEEK_SET);

  int fd_output =
      open(output_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd_output == -1)
    printErrNo();
  ASSERT_NE(fd_output, -1);

  extsort::external_sort(fd, size, fd_output, mem_size);

  auto out_size = lseek(fd_output, 0, SEEK_END);
  ASSERT_EQ(size, out_size);
  lseek(fd_output, 0, SEEK_SET);

  TestFileIsSorted(fd_output, size, mem_size);

  close(fd);
  close(fd_output);
}

void printErrNo() {
  switch (errno) {
  case EACCES:
    printf("Permission denied: Check directory permissions.\n");
    break;
  case ENOENT:
    printf("No such file or directory: Ensure the path exists.\n");
    break;
  case ENOSPC:
    printf("No space left on device: Free up disk space.\n");
    break;
  case EMFILE:
    printf("Too many open files: Close some files or increase limit.\n");
    break;
  case ENFILE:
    printf("File table overflow: The system-wide file limit is reached.\n");
    break;
  case EROFS:
    printf(
        "Read-only file system: Check if the disk is mounted as read-only.\n");
    break;
  case EINVAL:
    printf("Invalid argument: Make sure you provided the correct flags.\n");
    break;
  default:
    perror("open failed");
  }
}
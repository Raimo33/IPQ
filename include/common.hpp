#pragma once

#include <cstddef>
#include <atomic>
#include <array>
#include <string_view>
#include <fcntl.h>
#include <sys/mman.h>
#include <exception>

#ifndef CACHELINE_SIZE
# define CACHELINE_SIZE 64
#endif

template <size_t N>
concept is_power_of_two = (N > 0) && ((N & (N - 1)) == 0);

static_assert(std::atomic<size_t>::is_always_lock_free, "atomic must be lock-free for IPC");

template <typename Derived>
struct SharedMemoryBase
{
  static Derived* create(std::string_view name)
  {
    int fd = shm_open(name.data(), O_CREAT | O_RDWR, 0666);
    ftruncate(fd, size);
    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
    close(fd);

    if (addr == MAP_FAILED) return nullptr;

    return new (addr) Derived{};
  }

  static Derived* open(std::string_view name)
  {
    int fd = shm_open(name.data(), O_RDWR, 0666);
    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
    close(fd);

    if (addr == MAP_FAILED)
      return nullptr;

    return static_cast<Derived*>(addr);
  }

  static void close(Derived* data)
  {
    munmap(data, Derived::size);
  }

  static void destroy(std::string_view name)
  {
    shm_unlink(name.data());
  }

  static constexpr size_t size = sizeof(Derived);
};


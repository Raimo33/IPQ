/*================================================================================

File: IQueueCRTP.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-06-09 20:06:35                                                

================================================================================*/

#pragma once

#include <cstddef>
#include <fcntl.h>
#include <sys/mman.h>
#include <atomic>
#include <array>
#include <exception>

#ifndef CACHELINE_SIZE
# define CACHELINE_SIZE 64
#endif

template <size_t N>
concept is_power_of_two = (N > 0) && ((N & (N - 1)) == 0);

namespace ipq
{

template <typename Derived, typename Item, size_t Capacity>
  requires is_power_of_two<Capacity> && std::is_trivially_constructible_v<Item> && std::is_trivially_destructible_v<Item> && std::is_trivially_copyable_v<Item>
class IQueueCRTP
{
  public:

    explicit IQueueCRTP(const int fd)
    {
      bool error = false;

      error |= ftruncate(fd, sizeof(SharedData)) == -1;
      void *addr = mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
      error |= (addr == MAP_FAILED);
      error |= mlock(addr, sizeof(SharedData)) == -1;

      _data = static_cast<SharedData*>(addr);
      _data->write_idx.store(0, std::memory_order_relaxed);
      _data->read_idx.store(0, std::memory_order_relaxed);
      _data->buffer.fill(Item{});

      if (error) [[unlikely]]
        throw_error("Failed to initialize shared memory queue");
    }

    ~IQueueCRTP(void) noexcept {
      munmap(data, sizeof(SharedData));
    }

    inline void push(const Item &item) noexcept {
      derived()->push_impl(item);
    }

    template<typename... Args>
    inline void emplace(Args &&...args) noexcept {
      derived()->emplace_impl(std::forward<Args>(args)...);
    }
    
    inline bool pop(Item &out) noexcept {
      return derived()->pop_impl(out);
    }

  protected:
    static constexpr size_t _wrap_mask = Capacity - 1;

    struct SharedData
    {
      alignas(CACHELINE_SIZE) std::atomic<size_t> write_idx;
      alignas(CACHELINE_SIZE) std::atomic<size_t> read_idx;
      alignas(CACHELINE_SIZE) std::array<Item, Capacity> buffer;
    };

    SharedData *_data;

  private:
    inline Derived *derived(void) noexcept { return static_cast<Derived*>(this); }
    inline const Derived *derived(void) const noexcept { return static_cast<const Derived*>(this); }

    static void throw_error(std::string_view message)
    {
    #ifdef __cpp_exceptions
      throw std::runtime_error(message);
    #else
      write(STDERR_FILENO, message.data(), message.size());
      write(STDERR_FILENO, "\n", 1);
      std::abort();
    #endif
    }
};

}

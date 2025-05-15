/*================================================================================

File: IQueueCRTP.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-15 17:14:45                                                

================================================================================*/

#pragma once

#include <cstddef>
#include <fcntl.h>
#include <sys/mman.h>
#include <atomic>

#include "QueueUtils.hpp"

namespace ipq
{

template <typename Derived, typename Item, std::size_t Capacity>
class IQueueCRTP
{
  static_assert(Capacity % 2 == 0, "Capacity must be a power of 2");

  public:

    class Producer
    {
      public:
        explicit Producer(Derived* d) : derived(d) {}

        template <typename ForwardItem>
        inline void push(ForwardItem&& item) noexcept {
          derived->push_impl(std::forward<ForwardItem>(item));
        }

        template <typename ForwardItem>
        inline bool try_push(ForwardItem&& item) noexcept {
          return derived->try_push_impl(std::forward<ForwardItem>(item));
        }

        inline void clear(void) noexcept {
          derived->clear_impl();
        }

      private:
        Derived* derived;
    };

    class Consumer
    {
      public:
        explicit Consumer(Derived* d) : derived(d) {}

        inline bool try_pop(Item& out) noexcept {
          return derived->try_pop_impl(out);
        }

      private:
        Derived* derived;
    };

    Producer producer() { return Producer(static_cast<Derived*>(this)); }
    Consumer consumer() { return Consumer(static_cast<Derived*>(this)); }

    explicit IQueueCRTP(std::string_view name) : name(name)
    {
      const int fd = shm_open(name.data(), O_CREAT | O_RDWR, 0666);
      ftruncate(fd, sizeof(SharedData));
      void *addr = mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
      close(fd);

      if (addr == MAP_FAILED)
        utils::throwError("Failed to create shared memory region");

      data = static_cast<SharedData*>(addr);
    }

    ~IQueueCRTP(void) noexcept
    {
      munmap(data, sizeof(SharedData));
      shm_unlink(name.data());
    }

  protected:

    struct SharedData
    {
      alignas(CACHELINE_SIZE) std::atomic<std::size_t> write_idx;
      alignas(CACHELINE_SIZE) std::atomic<std::size_t> read_idx;
      std::array<Item, Capacity> buffer;
    };

    SharedData *data;

  private:

    std::string name;
};

}

/*================================================================================

File: IQueueCRTP.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-23 18:02:03                                                

================================================================================*/

#pragma once

#include <cstddef>
#include <fcntl.h>
#include <sys/mman.h>
#include <atomic>
#include <array>

#include "utils.hpp"

namespace ipq
{

template <typename Derived, typename Item, size_t Capacity>
  requires (Capacity > 0) && ((Capacity & (Capacity - 1)) == 0)
class IQueueCRTP
{
  public:

    explicit IQueueCRTP(std::string_view name) : name(name)
    {
      const int fd = shm_open(name.data(), O_CREAT | O_RDWR, 0666);
      ftruncate(fd, sizeof(SharedData));
      void *addr = mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
      close(fd);

      if (addr == MAP_FAILED)
        utils::throw_error("Failed to create shared memory region");

      data = static_cast<SharedData*>(addr);
    }

    ~IQueueCRTP(void) noexcept {
      munmap(data, sizeof(SharedData));
    }

    template <typename ForwardItem>
    inline void push(ForwardItem &&item) noexcept {
      derived->push_impl(std::forward<ForwardItem>(item));
    }
    
    inline bool try_pop(Item &out) noexcept {
      return derived->try_pop_impl(out);
    }

    inline void clear(void) noexcept {
      derived->clear_impl();
    }

    inline void destroy(void) noexcept {
      shm_unlink(name.data());
    }

  protected:
    static constexpr size_t wrap_mask = Capacity - 1;

    struct SharedData
    {
      alignas(CACHELINE_SIZE) std::atomic<size_t> write_idx;
      alignas(CACHELINE_SIZE) std::atomic<size_t> read_idx;
      std::array<Item, Capacity> buffer;
    };

    SharedData *data;

  private:

    std::string name;
};

}

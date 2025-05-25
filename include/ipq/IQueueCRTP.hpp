/*================================================================================

File: IQueueCRTP.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-25 18:47:18                                                

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

    explicit IQueueCRTP(const int fd)
    {
      void *addr = mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);

      if (addr == MAP_FAILED) [[unlikely]]
        utils::throw_error("Failed to create shared memory region");

      data = static_cast<SharedData*>(addr);
    }

    ~IQueueCRTP(void) noexcept {
      munmap(data, sizeof(SharedData));
    }

    template <typename ForwardItem>
    inline void push(ForwardItem &&item) noexcept {
      derived()->push_impl(std::forward<ForwardItem>(item));
    }
    
    inline bool pop(Item &out) noexcept {
      return derived()->pop_impl(out);
    }

  protected:
    static constexpr size_t wrap_mask = Capacity - 1;
    static constexpr size_t flush_frequency = Capacity / 4;
    static constexpr size_t flush_mask = flush_frequency - 1;

    struct SharedData
    {
      alignas(CACHELINE_SIZE) std::atomic<size_t> write_idx;
      alignas(CACHELINE_SIZE) std::atomic<size_t> read_idx;
      std::array<Item, Capacity> buffer;
    };

    SharedData *data;

  private:
    inline Derived *derived(void) noexcept { return static_cast<Derived*>(this); }
    inline const Derived *derived(void) const noexcept { return static_cast<const Derived*>(this); }
};

}

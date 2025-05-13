/*================================================================================

File: IQueueCRTP.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-13 14:27:00                                                

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

    explicit IQueueCRTP(std::string_view name) : name(name)
    {
      fd = shm_open(name.data(), O_CREAT | O_RDWR | O_SYNC, 0666);
      ftruncate(fd, sizeof(SharedData));
      void *addr = mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);

      if (addr == MAP_FAILED)
        utils::throwError("Failed to create shared memory region");

      data = static_cast<SharedData*>(addr);
    }

    ~IQueueCRTP(void) noexcept
    {
      munmap(data, sizeof(SharedData));
      shm_unlink(name.data());
      close(fd);
    }

    template <typename ForwardItem>
    void push(ForwardItem&& item) {
      derived().push_impl(std::forward<ForwardItem>(item));
    }

    template <typename ForwardItem>
    bool try_push(ForwardItem&& item) {
      return derived().try_push_impl(std::forward<ForwardItem>(item));
    }

    Item pop(void) {
      return derived().pop_impl();
    }

    const Item& front(void) const {
      return derived().front_impl();
    }

    bool isEmpty(void) const {
      return derived().isEmpty_impl();
    }

    bool isFull(void) const {
      return derived().isFull_impl();
    }

    std::size_t size(void) const {
      return derived().size_impl();
    }

    void clear(void) {
      derived().clear_impl();
    }

  protected:

    struct SharedData
    {
      alignas(CACHELINE_SIZE) std::atomic<std::size_t> write_idx;
      alignas(CACHELINE_SIZE) std::atomic<std::size_t> read_idx;
      std::array<Item, Capacity> buffer;
    };

    SharedData* data;

  private:

    int fd;
    std::string name;

    Derived& derived(void) noexcept {
      return static_cast<Derived&>(*this);
    }

    const Derived& derived(void) const noexcept {
      return static_cast<const Derived&>(*this);
    }
};

}

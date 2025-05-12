/*================================================================================

File: IQueueCRTP.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-12 18:01:10                                                

================================================================================*/

#pragma once

#include <cstddef>
#include "QueueUtils.hpp"

namespace ipcqueue
{

template <typename Derived, typename Item, std::size_t Capacity>
class IQueueCRTP
{
  public:

    explicit IQueueCRTP(std::string_view name) : state{0, 0, {}}
    {
      fd = shm_open(name.data(), O_CREAT | O_RDWR | O_SYNC, 0666);
      ftruncate(fd, sizeof(SharedState));
      void *addr = mmap(nullptr, sizeof(SharedState), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);

      if (addr == MAP_FAILED)
        utils::throwError("Failed to create shared memory region");

      state = static_cast<SharedState*>(addr);
    }

    ~IQueueCRTP(void) noexcept
    {
      munmap(state, sizeof(SharedState));
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

    Item peek(void) const {
      return derived().peek_impl();
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

    std::size_t capacity(void) const {
      return derived().capacity_impl();
    }

    void clear(void) {
      derived().clear_impl();
    }

  protected:

    struct SharedState
    {
      alignas(CACHELINE_SIZE) std::size_t head;
      alignas(CACHELINE_SIZE) std::size_t tail;
      std::array<Item, Capacity> buffer;
    } state;

  private:

    int fd;

    Derived& derived(void) noexcept {
      return static_cast<Derived&>(*this);
    }

    const Derived& derived(void) const noexcept {
      return static_cast<const Derived&>(*this);
    }
};

}

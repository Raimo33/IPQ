/*================================================================================

File: SPSCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-12 21:18:06                                                

================================================================================*/

#include "IQueueCRTP.hpp"

namespace ipcqueue
{

template <typename Item, size_t Capacity>
class SPSCQueue : public IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>
{
  using Base = IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>;
  static constexpr std::size_t Mask = Capacity - 1;

  public:

    explicit SPSCQueue(std::string_view name) : Base(name) {}

    template <typename ForwardItem>
    void push_impl(ForwardItem&& item) noexcept
    {
      auto local_write_idx = data.write_idx.load(std::memory_order_relaxed);
      auto next_write_idx = (local_write_idx + 1) & Mask;

      data.buffer[local_write_idx] = std::forward<ForwardItem>(item);
      data.write_idx.store(next_write_idx, std::memory_order_release);
    }

    template <typename ForwardItem>
    bool try_push_impl(ForwardItem&& item) noexcept
    {
      auto local_write_idx = data.write_idx.load(std::memory_order_relaxed);
      auto local_read_idx = data.read_idx.load(std::memory_order_acquire);
      auto next_write_idx = (local_write_idx + 1) & Mask;

      if (next_write_idx == local_read_idx) [[unlikely]]
        return false;

      data.buffer[local_write_idx] = std::forward<ForwardItem>(item);
      data.write_idx.store(next_write_idx, std::memory_order_release);

      return true;
    }

    Item pop_impl(void) noexcept
    {
      auto local_read_idx = data.read_idx.load(std::memory_order_relaxed);
      auto next_read_idx = (local_read_idx + 1) & Mask;

      Item item = std::move(data.buffer[local_read_idx]);
      data.read_idx.store(next_read_idx, std::memory_order_release);

      return item;
    }

    inline const Item& front_impl(void) const noexcept {
      return data.buffer[data.read_idx.load(std::memory_order_relaxed)];
    }

    inline bool isEmpty_impl(void) const noexcept {
      return data.read_idx.load(std::memory_order_relaxed) == data.write_idx.load(std::memory_order_acquire);
    }

    bool isFull_impl(void) const noexcept
    {
      auto local_write_idx = data.write_idx.load(std::memory_order_relaxed);
      auto local_read_idx = data.read_idx.load(std::memory_order_acquire);
      auto next_write_idx = (local_write_idx + 1) & Mask;

      return next_write_idx == local_read_idx;
    }

    std::size_t size_impl(void) const noexcept
    {
      auto local_write_idx = data.write_idx.load(std::memory_order_relaxed);
      auto local_read_idx = data.read_idx.load(std::memory_order_acquire);

      return (local_write_idx - local_read_idx) & Mask;
    }

    void clear_impl(void) noexcept
    {
      data.write_idx.store(0, std::memory_order_release);
      data.read_idx.store(0, std::memory_order_relaxed);
    }
};

}
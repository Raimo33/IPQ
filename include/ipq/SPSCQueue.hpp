/*================================================================================

File: SPSCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-15 17:14:45                                                

================================================================================*/

#pragma once

#include "IQueueCRTP.hpp"

namespace ipq
{

template <typename Item, std::size_t Capacity>
class SPSCQueue : public IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>
{
  using Base = IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>;
  using Base::data;
  static constexpr std::size_t mask = Capacity - 1;

  public:

    explicit SPSCQueue(std::string_view name) : Base(name) {}

    template <typename ForwardItem>
    inline void push_impl(ForwardItem&& item) noexcept
    {
      const auto local_write_idx = data->write_idx.load(std::memory_order_relaxed);
      data->buffer[local_write_idx & mask] = std::forward<ForwardItem>(item);
      data->write_idx.store(local_write_idx + 1, std::memory_order_release);
    }

    template <typename ForwardItem>
    bool try_push_impl(ForwardItem&& item) noexcept
    {
      const auto local_write_idx = data->write_idx.load(std::memory_order_relaxed);
      const auto local_read_idx = data->read_idx.load(std::memory_order_acquire);

      if ((local_write_idx - local_read_idx) == Capacity)
        return false;

      data->buffer[local_write_idx & mask] = std::forward<ForwardItem>(item);
      data->write_idx.store(local_write_idx + 1, std::memory_order_release);
      return true;
    }

    inline bool try_pop_impl(Item& out) noexcept
    {
      const auto local_read_idx = data->read_idx.load(std::memory_order_relaxed);
      const auto local_write_idx = data->write_idx.load(std::memory_order_acquire);

      if (local_read_idx == local_write_idx) [[unlikely]]
          return false;

      out = data->buffer[local_read_idx & mask];
      data->read_idx.store(local_read_idx + 1, std::memory_order_release);
      return true;
    }

    void clear_impl(void) noexcept
    {
      data->write_idx.store(0, std::memory_order_relaxed);
      data->read_idx.store(0, std::memory_order_relaxed);
    }
};

}
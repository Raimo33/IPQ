/*================================================================================

File: SPMCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-23 18:02:03                                                

================================================================================*/

#pragma once

#include "IQueueCRTP.hpp"

namespace ipq
{

template <typename Item, std::size_t Capacity>
class SPMCQueue : public IQueueCRTP<SPMCQueue<Item, Capacity>, Item, Capacity>
{
  using Base = IQueueCRTP<SPMCQueue<Item, Capacity>, Item, Capacity>;
  using Base::data;

  public:
    explicit SPMCQueue(std::string_view name) : Base(name) {}

    template <typename ForwardItem>
    inline void push_impl(ForwardItem &&item) noexcept
    {
      const size_t local_write_idx = data->write_idx.load(std::memory_order_relaxed);
      data->buffer[local_write_idx & wrap_mask] = std::forward<ForwardItem>(item);
      data->write_idx.store(local_write_idx + 1, std::memory_order_release);
    }

    inline bool try_pop_impl(Item &out) noexcept
    {
      size_t local_read_idx = data->read_idx.load(std::memory_order_relaxed);

      while (true)
      {
        const size_t local_write_idx = data->write_idx.load(std::memory_order_acquire);

        if (local_read_idx == local_write_idx) [[unlikely]]
          return false;

        if (data->read_idx.compare_exchange_weak(local_read_idx, local_read_idx + 1, std::memory_order_acquire, std::memory_order_relaxed)) [[likely]]
        {
          out = std::move(data->buffer[local_read_idx & wrap_mask]);
          return true;
        }
      }

      std::unreachable();
    }

    inline void clear_impl(void) noexcept
    {
      data->write_idx.store(0, std::memory_order_relaxed);
      data->read_idx.store(0, std::memory_order_relaxed);
    }
};

}
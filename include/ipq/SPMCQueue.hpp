/*================================================================================

File: SPMCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-06-09 20:06:35                                                

================================================================================*/

#pragma once

#include "IQueueCRTP.hpp"

namespace ipq
{

template <typename Item, size_t Capacity>
class SPMCQueue : public IQueueCRTP<SPMCQueue<Item, Capacity>, Item, Capacity>
{
  using Base = IQueueCRTP<SPMCQueue<Item, Capacity>, Item, Capacity>;
  using Base::_data;
  using Base::_wrap_mask;

  public:
    explicit SPMCQueue(const int fd) :
      Base(fd),
      _local_write_idx(0),
      _cached_write_idx(0) {}

    void push_impl(const Item &item) noexcept
    {
      _data->buffer[_local_write_idx & _wrap_mask] = item;
      _data->write_idx.store(++_local_write_idx, std::memory_order_release);
    }

    template<typename... Args>
    void emplace_impl(Args &&...args) noexcept
    {
      new (&_data->buffer[_local_write_idx & _wrap_mask]) Item(std::forward<Args>(args)...);
      _data->write_idx.store(++_local_write_idx, std::memory_order_release);
    }

    bool pop_impl(Item &out) noexcept
    {
      size_t local_read_idx = _data->read_idx.load(std::memory_order_relaxed);

      while (true)
      {
        if (local_read_idx == _cached_write_idx) [[unlikely]]
        {
          _cached_write_idx = _data->write_idx.load(std::memory_order_acquire);
          if (local_read_idx == _cached_write_idx) [[unlikely]]
            return false;
        }

        if (_data->read_idx.compare_exchange_weak(local_read_idx, local_read_idx + 1, std::memory_order_acquire, std::memory_order_relaxed)) [[likely]]
        {
          out = _data->buffer[local_read_idx & _wrap_mask];
          return true;
        }

        std::this_thread::yield();
      }

      std::unreachable();
    }

  private:
    size_t _local_write_idx = 0;
    size_t _cached_write_idx = 0;
};

}
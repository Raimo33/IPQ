/*================================================================================

File: SPSCQueue.hpp                                                             
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
class SPSCQueue : public IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>
{
  using Base = IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>;
  using Base::data;
  using Base::wrap_mask;

  public:
    explicit SPSCQueue(const int fd) :
      Base(fd),
      _local_read_idx(0),
      _local_write_idx(0),
      _cached_write_idx(0) {}

    void push_impl(const Item &item) noexcept
    {
      data->buffer[_local_write_idx & wrap_mask] = item;
      data->write_idx.store(++_local_write_idx, std::memory_order_release);
    }

    bool pop_impl(Item &out) noexcept
    {
      if (_local_read_idx == _cached_write_idx) [[unlikely]]
      {
        _cached_write_idx = data->write_idx.load(std::memory_order_acquire);
        if (_local_read_idx == _cached_write_idx) [[unlikely]]
          return false;
      }

      out = data->buffer[_local_read_idx & wrap_mask];
      _local_read_idx++;
      return true;
    }

  private:
    size_t _local_read_idx;
    size_t _local_write_idx;
    size_t _cached_write_idx;
};

}
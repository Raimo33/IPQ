/*================================================================================

File: SPSCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-14 20:28:36                                                

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
      const auto local_write_idx = data->write_idx.fetch_add(1, /**/);
      data->buffer[local_write_idx & mask] = std::forward<ForwardItem>(item);
    }

    template <typename ForwardItem>
    bool try_push_impl(ForwardItem&& item) noexcept
    {
      const auto local_write_idx = data->write_idx.load(/**/);
      const auto local_read_idx = data->read_idx.load(/**/);

      if ((local_write_idx - local_read_idx) == Capacity)
        return false;

      data->buffer[local_write_idx & mask] = std::forward<ForwardItem>(item);
      data->write_idx.store(local_write_idx + 1, /**/);
      return true;
    }

    inline Item pop_impl(void) noexcept
    {
      auto local_read_idx = data->read_idx.fetch_add(1, /**/);
      return data->buffer[local_read_idx & mask];
    }

    inline const Item& front_impl(void) const noexcept
    {
      auto local_read_idx = data->read_idx.load(/**/);
      return data->buffer[local_read_idx & mask];
    }

    inline bool isEmpty_impl(void) const noexcept
    {
      return data->read_idx.load(/**/) == data->write_idx.load(/**/);
    }

    bool isFull_impl(void) const noexcept
    {
      return (data->write_idx.load(/**/) - data->read_idx.load(/**/) == Capacity);
    }

    std::size_t size_impl(void) const noexcept
    {
      return data->write_idx.load(/**/) - data->read_idx.load(/**/);
    }

    void clear_impl(void) noexcept
    {
      data->write_idx.store(0, /**/);
      data->read_idx.store(0, /**/);
    }
};

}
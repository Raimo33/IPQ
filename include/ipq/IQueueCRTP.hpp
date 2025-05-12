#pragma once

#include <cstddef>

using std::size_t;

namespace ipcqueue
{
  template <typename Derived, typename Item, size_t Capacity>
  class IQueueCRTP
  {
    public:
      IQueueCRTP(void) noexcept;

      void push(const Item& item);
      void push(Item&& item);
      bool try_push(const Item& item);
      bool try_push(Item&& item);
      Item pop(void);
      Item peek(void) const;

      bool isEmpty(void) const;
      bool isFull(void) const;
      size_t size(void) const;
      size_t capacity(void) const;
      void clear(void);
    
    protected:
      size_t head;
      size_t tail;
      std::array<Item, Capacity> buffer;

    private:
      Derived& derived(void) noexcept;
      const Derived& derived(void) const noexcept;
  };

}

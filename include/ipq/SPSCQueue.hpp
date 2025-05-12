/*================================================================================

File: SPSCQueue.hpp                                                             
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-12 18:01:10                                                

================================================================================*/

#include "IQueueCRTP.hpp"

namespace ipcqueue
{

template <typename Item, size_t Capacity>
class SPSCQueue : public IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>
{
  using Base = IQueueCRTP<SPSCQueue<Item, Capacity>, Item, Capacity>;

  public:

    explicit SPSCQueue(std::string_view name) : Base(name) {}

    template <typename ForwardItem>
    void push_impl(ForwardItem&& item) noexcept {
      //TODO implement (compare and swap?)
    }

    template <typename ForwardItem>
    bool try_push_impl(ForwardItem&& item) noexcept;

    Item pop_impl(void) noexcept;
    Item peek_impl(void) const noexcept;
    bool isEmpty_impl(void) const noexcept;
    bool isFull_impl(void) const noexcept;
    size_t size_impl(void) const noexcept;
    size_t capacity_impl(void) const noexcept;
    void clear_impl(void) noexcept;
};

}
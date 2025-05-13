/*================================================================================

File: QueueUtils.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-13 13:22:19                                                

================================================================================*/

#pragma once

#include <cstdint>
#include <new>
#include <string_view>
#include <iostream>
#include <exception>

#ifndef CACHELINE_SIZE
# define CACHELINE_SIZE std::hardware_destructive_interference_size
#endif

namespace ipcqueue
{
  
namespace utils
{
  [[noreturn]] [[gnu::noinline]] void throwError(std::string_view message)
  {
    #ifdef __EXCEPTIONS
      throw std::runtime_error(message.data());
    #else
      std::cerr << message << std::endl;
      std::abort();
    #endif
  }
}

}
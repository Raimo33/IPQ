/*================================================================================

File: QueueUtils.hpp                                                            
Creator: Claudio Raimondi                                                       
Email: claudio.raimondi@pm.me                                                   

created at: 2025-05-12 18:01:10                                                 
last edited: 2025-05-23 18:02:03                                                

================================================================================*/

#pragma once

#include <cstdint>
#include <new>
#include <string_view>
#include <iostream>
#include <exception>

#ifndef CACHELINE_SIZE
# define CACHELINE_SIZE 64
#endif

namespace ipq
{
  
namespace utils
{
  [[noreturn]] [[gnu::noinline]] void throw_error(std::string_view message)
  {
    #ifdef __EXCEPTIONS
      throw std::runtime_error(message.data());
    #else
      std::cerr << message << std::endl;
      std::terminate();
    #endif
  }
}

}
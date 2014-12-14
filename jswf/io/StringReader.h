//
//  StringReader.h
//  jswf/io
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf_io__StringReader__
#define __jswf_io__StringReader__

#include "GenericReader.h"

namespace jswf {
  namespace io {
    class StringReader : public GenericReader {
    public:
      std::string string;
      
      StringReader(std::string string = "") : string(string) {}
      
      uint8_t  readU8();
      uint16_t readU16();
      uint32_t readU32();
      
      std::string readString(); // Terminated by NUL byte
      std::string readString(size_t); // A given number of characters
      
      void align(uint8_t bytes);
      uint64_t readUB(uint8_t nbits);
      
      std::string readMatching() {
        align(1);
        
        size_t start = pos;
        pos = string.length();
        return string.substr(start);
      }
    };
  }
}

#endif
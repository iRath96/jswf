//
//  GenericReader.h
//  jswf/io
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf_io__GenericReader__
#define __jswf_io__GenericReader__

#include <stdio.h>
#include <stdint.h>

#include <string>

namespace jswf {
  typedef double fb_t;
  typedef int64_t sb_t;
  typedef uint64_t ub_t;
  typedef float fixed8_t; // stored like int16
  
  namespace io {
    class GenericReader {
    public:
      size_t pos = 0;
      uint8_t bitpos = 0;
      
      virtual uint8_t  readU8()  = 0;
      virtual uint16_t readU16() = 0;
      virtual uint32_t readU32() = 0;
      
      virtual int8_t  readS8()  { return (int8_t) readU8();  }
      virtual int16_t readS16() { return (int16_t)readU16(); }
      virtual int32_t readS32() { return (int32_t)readU32(); }
      
      virtual std::string readString() = 0;
      virtual std::string readString(size_t length) = 0;
      
      virtual ub_t readUB(uint8_t nbits) = 0;
      virtual sb_t readSB(uint8_t nbits);
      virtual double readFB(uint8_t nbits) { return (fb_t)(readSB(nbits)) / (1 << 16); }
      
      virtual void align(uint8_t bytes) = 0;
      
      virtual std::string readMatching() = 0;
    };
  }
}

#endif
//
//  types.h
//  jswf
//
//  Created by Alexander Rath on 27.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_types_h
#define jswf_types_h

#include <string>
#include <stdint.h>

namespace jswf {
  typedef double fb_t;
  typedef int64_t sb_t;
  typedef uint64_t ub_t;
  typedef float fixed8_t; // stored like int16
  
  typedef uint8_t u8_t;
  typedef uint16_t u16_t;
  typedef uint32_t u32_t;
  typedef uint32_t u30_t;
  
  typedef int8_t s8_t;
  typedef int16_t s16_t;
  typedef int32_t s24_t;
  typedef int32_t s32_t;
  
  typedef double d64_t;
  
  typedef std::string string;
}

#endif
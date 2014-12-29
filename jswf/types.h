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
  typedef double fb_t; //!< Represents `FB` values \todo Could store them differently
  typedef int64_t sb_t; //!< Represents `SB` values (only up to 64 bits supported)
  typedef uint64_t ub_t; //!< Represents `UB` values (only up to 64 bits supported)
  typedef float fixed8_t; // stored like int16
  
  typedef uint8_t  u8_t;  //!< Represents `U8`
  typedef uint16_t u16_t; //!< Represents `U16`
  typedef uint32_t u32_t; //!< Represents `U32`
  typedef uint32_t u30_t; //!< Represents `U30`
  
  typedef int8_t  s8_t;  //!< Represents `S8`
  typedef int16_t s16_t; //!< Represents `S16`
  typedef int32_t s24_t; //!< Represents `S24`
  typedef int32_t s32_t; //!< Represents `S32`
  
  typedef double d64_t; //!< Represents `D64`
  
  typedef std::string string; //!< Represents strings
}

#endif
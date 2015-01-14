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
  typedef double fb_t; //!< Represents <tt>FB</tt> values \todo Could store them differently
  typedef int64_t sb_t; //!< Represents <tt>SB</tt> values (only up to 64 bits supported)
  typedef uint64_t ub_t; //!< Represents <tt>UB</tt> values (only up to 64 bits supported)
  typedef float fixed8_t; // stored like int16
  
  typedef uint8_t  u8_t;  //!< Represents <tt>U8</tt>
  typedef uint16_t u16_t; //!< Represents <tt>U16</tt>
  typedef uint32_t u32_t; //!< Represents <tt>U32</tt>
  typedef uint32_t u30_t; //!< Represents <tt>U30</tt>
  
  typedef int8_t  s8_t;  //!< Represents <tt>S8</tt>
  typedef int16_t s16_t; //!< Represents <tt>S16</tt>
  typedef int32_t s24_t; //!< Represents <tt>S24</tt>
  typedef int32_t s32_t; //!< Represents <tt>S32</tt>
  
  typedef double d64_t; //!< Represents <tt>D64</tt>
  
  typedef std::string string; //!< Represents strings
}

#endif
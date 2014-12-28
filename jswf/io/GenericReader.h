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
#include "types.h"

namespace jswf {
  namespace io {
    /**
     * Servers as reader for primitive data-types like integers, doubles and strings.
     */
    class GenericReader {
    public:
      size_t pos = 0; //!< Our current byte position. \todo Protected with optional seek.
      uint8_t bitpos = 0; //!< Our current bit position [0;8) inside the byte position
      
      virtual u8_t  readU8()  = 0; //!< Reads a byte-aligned 8-bit unsigned integer
      virtual u16_t readU16() = 0; //!< Reads a byte-aligned 16-bit unsigned integer (little endian)
      virtual u32_t readU32() = 0; //!< Reads a byte-aligned 32-bit unsigned integer (little endian)
      
      virtual s8_t  readS8()  { return (s8_t) readU8();  } //!< Reads a byte-aligned 8-bit signed integer
      virtual s16_t readS16() { return (s16_t)readU16(); } //!< Reads a byte-aligned 16-bit signed integer (little endian)
      virtual s32_t readS32() { return (s32_t)readU32(); } //!< Reads a byte-aligned 32-bit signed integer (little endian)
      
      virtual string readString() = 0; //!< Reads a NUL-terminated string (C-string)
      virtual string readString(size_t length) = 0; //!< Reads a string of a given length
      
      virtual ub_t readUB(uint8_t nbits) = 0; //!< Reads a unsigned bit-field of length \ref nbits
      virtual sb_t readSB(uint8_t nbits); //!< Reads a signed bit-field of length \ref nbits, sign-extended
      virtual fb_t readFB(uint8_t nbits) { return (fb_t)(readSB(nbits)) / (1 << 16); } //!< Reads a FIXED8.8
      
      virtual void align(uint8_t bytes) = 0; //!< Aligns the reader to a multiple of 'bytes', resets bit-position
      
      virtual string readRemaining() = 0; //!< Reads all remaining bytes (bit-position rounded up)
      
      virtual bool eof() = 0; //!< Returns true if the end of stream is reached, false otherwise.
      
#pragma mark AVM2
      
      virtual s32_t readS24()  = 0; //!< Reads a byte-aligned 24-bit unsigned integer (little endian)
      virtual u32_t readVU30() = 0; //!< Reads a byte-aligned, variable-length encoded 30-bit unsigned integer
      virtual u32_t readVU32() = 0; //!< Reads a byte-aligned, variable-length encoded 32-bit unsigned integer
      virtual s32_t readVS32() = 0; //!< Reads a byte-aligned, variable-length encoded 32-bit signed integer
      virtual d64_t readD64()  = 0; //!< Reads a IEE-754 double-precision floating point number
    };
  }
}

#endif
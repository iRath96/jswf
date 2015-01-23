//
//  StringReader.h
//  jswf/io
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

/**
 * @file
 * Defines jswf::io::StringReader.
 */

#ifndef __jswf_io__StringReader__
#define __jswf_io__StringReader__

#include "GenericReader.h"

namespace jswf {
  namespace io {
    /**
     * Implements io::GenericReader using a string as stream.
     */
    class StringReader : public GenericReader {
    public:
      std::string string; //!< The string we are reading from.
      
      /**
       * Constructs a StringReader with a given string.
       * @param [in] string The string to be used as data source
       */
      StringReader(std::string string = "") : string(string) {}
      
      uint8_t  readU8();
      uint16_t readU16();
      uint32_t readU32();
      
      std::string readString();
      std::string readString(size_t);
      
      /**
       * Adds an offset to the current reading position, does not change <tt>bitPos</tt>.
       * @param [in] offset The offset to add to <tt>pos</tt>, can be negative.
       */
      void seek(int offset) {
        pos += offset;
      }
      
      /**
       * Sets the current reading position, does not change <tt>bitPos</tt>.
       * @param [in] pos The new value for <tt>pos</tt>, cannot be negative.
       */
      void seek_abs(size_t pos) {
        this->pos = pos;
      }
      
      void align(uint8_t bytes);
      uint64_t readUB(uint8_t nbits);
      
      std::string readRemaining() {
        align(1);
        
        size_t start = pos;
        pos = string.length();
        return string.substr(start);
      }
      
      bool eof() { return pos >= string.length(); } // TODO:2014-12-27:alex >= is stupid. Should be ==.
      
#pragma mark AVM2
      
      s32_t readS24();
      u32_t readVU30();
      u32_t readVU32();
      s32_t readVS32();
      d64_t readD64();
    };
  }
}

#endif
//
//  GenericReader.cpp
//  jswf/io
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "GenericReader.h"

using namespace jswf;
using namespace jswf::io;

sb_t GenericReader::readSB(uint8_t nbits) {
  // I assert that nbits is no bigger than 63.
  
  ub_t result = readUB(nbits);
  if(result & (1 << (nbits - 1))) { // Sign bit set (negative)
    result ^= (1 << nbits) - 1;
    result = -(result + 1);
  }
  
  return result;
}
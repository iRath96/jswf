//
//  StringReader.cpp
//  jswf/io
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "StringReader.h"

using namespace jswf::io;

#define read_byte (uint8_t)(string[pos++])
uint8_t  StringReader::readU8()  { align(1); return read_byte; }
uint16_t StringReader::readU16() { align(1); return read_byte | (read_byte << 8); }
uint32_t StringReader::readU32() {
  align(1);
  
  uint32_t v = 0;
  for(uint8_t i = 0; i < 4; ++i) v |= read_byte << (i << 3);
  return v;
}

std::string StringReader::readString() {
  align(1);
  
  std::string result = "";
  while(char c = string[pos++]) result += c;
  return result;
}

std::string StringReader::readString(size_t length) {
  align(1);
  
  size_t a = pos,
         b = pos += length;
  return string.substr(a, b);
}

void StringReader::align(uint8_t bytes) {
  if(bitpos) {
    ++pos;
    bitpos = 0;
  }
  
  pos += (bytes - pos) % bytes;
}

uint64_t StringReader::readUB(uint8_t nbits) {
  size_t a = (pos << 3) | bitpos;
  size_t b = a + nbits;
  
  size_t end_pos = b >> 3;
  uint8_t end_bitpos = b & 7;
  
  uint64_t result;
  
  if(pos == end_pos) // Same byte read
    result = ((uint8_t)(string[pos]) & (0xff >> bitpos)) >> (8-end_bitpos);
  else { // Multiple bytes
    bool ends_in_byte = end_bitpos > 0;
    result = (uint8_t)(string[pos]) & (0xff >> bitpos);
    size_t stop_pos = end_pos + (ends_in_byte ? 1 : 0);
    for(size_t p = pos+1; p < stop_pos; ++p) result = (result << 8) | (uint8_t)string[p];
    if(ends_in_byte) result = (result >> (8-end_bitpos));
  }
  
  pos = end_pos;
  bitpos = end_bitpos;
  
  return result;
}
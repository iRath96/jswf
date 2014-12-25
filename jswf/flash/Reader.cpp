//
//  reader->cpp
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "Reader.h"
#include "Header.h"
#include "StringReader.h"
#include "TagFactory.h"

#include <zlib.h>

using namespace jswf;
using namespace jswf::flash;

std::string zlibInflate(const std::string &input) {
  z_stream strm;
  const unsigned char *in = (const unsigned char *)input.c_str();
  
#define CHUNK (256<<10)
  std::string out = "";
  unsigned char out_buffer[CHUNK];
  
  // allocate inflate state
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  
  int zret = inflateInit(&strm);
  if(zret != Z_OK) throw "zlib inflateInit failed.";
  
  do {
    strm.avail_in = (uInt)input.length();
    strm.next_in = const_cast<unsigned char *>(in); // Urgh.
    
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out_buffer;
      
      zret = inflate(&strm, Z_NO_FLUSH);
      if(zret == Z_STREAM_ERROR) throw "zlib inflate stream error.";
      
      switch(zret) {
        case Z_NEED_DICT: zret = Z_DATA_ERROR;
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
          inflateEnd(&strm);
          throw "zlib inflate error.";
      }
      
      int have = CHUNK - strm.avail_out;
      out.append((char *)out_buffer, have);
    } while(strm.avail_out == 0);
  } while(zret != Z_STREAM_END);
  inflateEnd(&strm);
#undef CHUNK
  
  return out;
}

void Reader::readHeader(Header &header) {
  char b = reader->readU8();
  switch(b) {
    case 'F': header.compression = Compression::Uncompressed; break;
    case 'C': header.compression = Compression::ZLib; break;
    default: throw "tomatoes";
  }
  
  if(reader->readU16() != 0x5357) // 'WS'
    throw "'WS' Signature not found in header";
  
  header.version = reader->readU8();
  header.fileSize = reader->readU32();
  
  if(header.compression == Compression::ZLib) {
    std::string buffer = zlibInflate(reader->readMatching());
    reader.reset(new io::StringReader(buffer));
    
    if(buffer.length() + 8 != header.fileSize)
      throw "File-size mismatch.";
  }
  
  readRect(header.rect);
  
  header.frameRate = reader->readU16();
  header.frameCount = reader->readU16();
}

tags::Tag *Reader::readTag() {
  size_t p1 = reader->pos;
  
  uint16_t u = reader->readU16();
  uint16_t type = u >> 6;
  
  uint32_t length = u & 0x3f;
  if(length == 0x3f) length = reader->readU32();
  
  printf("%d, len:%d, pos:%lu\n", type, length, p1);
  
  std::string payload = reader->readString(length);
  return TagFactory::create(type, payload);
}

void Reader::readRect(Rect &rect) {
  uint8_t nbits = reader->readUB(5);
#define read(field) rect.field = reader->readSB(nbits)
  read(x0);
  read(x1);
  read(y0);
  read(y1);
#undef read
}

void Reader::readMatrix(Matrix &matrix) {
  if(reader->readUB(1) == 1) { // Scale information included
    uint8_t nbits = reader->readUB(5);
    matrix.sx = reader->readFB(nbits);
    matrix.sy = reader->readFB(nbits);
  } else {
    matrix.sx = 1;
    matrix.sy = 1;
  }
  
  if(reader->readUB(1) == 1) { // Rotation information included
    uint8_t nbits = reader->readUB(5);
    matrix.r0 = reader->readFB(nbits);
    matrix.r1 = reader->readFB(nbits);
  } else {
    matrix.r0 = 0;
    matrix.r1 = 0;
  }
  
  uint8_t nbits = reader->readUB(5);
  matrix.tx = reader->readSB(nbits);
  matrix.ty = reader->readSB(nbits);
}

void Reader::readColorTransform(ColorTransform &transform, bool withAlpha) {
  reader->align(1);
  
  bool hasAdd = reader->readUB(1),
       hasMult = reader->readUB(1);
  
  uint8_t nbits = reader->readUB(4);
  
  if(hasMult) {
    transform.rM = reader->readSB(nbits);
    transform.gM = reader->readSB(nbits);
    transform.bM = reader->readSB(nbits);
    if(withAlpha) transform.aM = reader->readSB(nbits);
  }
  
  if(hasAdd) {
    transform.rA = reader->readSB(nbits);
    transform.gA = reader->readSB(nbits);
    transform.bA = reader->readSB(nbits);
    if(withAlpha) transform.aA = reader->readSB(nbits);
  }
}

#pragma mark Read colors

void Reader::readRGB(RGBA &rgba) {
  rgba.r = reader->readU8();
  rgba.g = reader->readU8();
  rgba.b = reader->readU8();
  rgba.a = 255;
}

void Reader::readRGBA(RGBA &rgba) {
  rgba.r = reader->readU8();
  rgba.g = reader->readU8();
  rgba.b = reader->readU8();
  rgba.a = reader->readU8();
}

void Reader::readARGB(RGBA &rgba) {
  rgba.a = reader->readU8();
  rgba.r = reader->readU8();
  rgba.g = reader->readU8();
  rgba.b = reader->readU8();
}
//
//  Reader.h
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf_flash__Reader__
#define __jswf_flash__Reader__

#include <stdio.h>
#include "GenericReader.h"

namespace jswf {
  namespace flash {
    namespace tags { class Tag; }
    
    class Header;
    class Shape;
    
    struct Rect;
    struct RGBA;
    struct Matrix;
    
    class Reader {
    public:
      std::shared_ptr<jswf::io::GenericReader> reader;
      Reader(jswf::io::GenericReader &r) { reader.reset(&r); }
      Reader(std::shared_ptr<jswf::io::GenericReader> &r) { reader = r; }
      
      void readHeader(Header &);
      tags::Tag *readTag();
      
      void readRect(Rect &);
      void readMatrix(Matrix &);
      
      // Colors
      
      void readRGB(RGBA &);
      void readRGBA(RGBA &);
      void readARGB(RGBA &);
    };
  }
}

#endif